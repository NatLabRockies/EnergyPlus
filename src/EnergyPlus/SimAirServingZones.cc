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

// C++ Headers
#include <algorithm>
#include <cmath>
#include <memory>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/AirLoopHVACDOAS.hh>
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchInputManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataConvergParams.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DesiccantDehumidifiers.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/EvaporativeCoolers.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/Furnaces.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HVACControllers.hh>
#include <EnergyPlus/HVACDXHeatPumpSystem.hh>
#include <EnergyPlus/HVACDuct.hh>
#include <EnergyPlus/HVACHXAssistedCoolingCoil.hh>
#include <EnergyPlus/HVACInterfaceManager.hh>
#include <EnergyPlus/HVACMultiSpeedHeatPump.hh>
#include <EnergyPlus/HVACUnitaryBypassVAV.hh>
#include <EnergyPlus/HVACVariableRefrigerantFlow.hh>
#include <EnergyPlus/HeatRecovery.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/Humidifiers.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/MixerComponent.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/SimAirServingZones.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/SplitterComponent.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/SystemAvailabilityManager.hh>
#include <EnergyPlus/UnitarySystem.hh>
#include <EnergyPlus/UserDefinedComponents.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/ZonePlenum.hh>

namespace EnergyPlus::SimAirServingZones {

// MODULE INFORMATION
//       AUTHOR:  Russ Taylor, Dan Fisher, Fred Buhl
//       DATE WRITTEN:  Oct 1997
//       MODIFIED:  Dec 1997 Fred Buhl; Richard Liesen  Apr 1998,
//                  Dec 1999 Fred Buhl
//                  22Aug2010 Craig Wray - added Fan:ComponentModel
//       RE-ENGINEERED:  This is new code, not reengineered

// PURPOSE OF THIS MODULE:
// Contains the data and code for simulating the HVAC forced
// air systems.

// METHODOLOGY EMPLOYED:
// Successive iteration forward from the return air inlet to the supply air outlets.

using namespace DataLoopNode;
using namespace DataAirLoop;
using namespace DataSizing;
using namespace DataZoneEquipment;
using namespace DataAirSystems;

void ManageAirLoops(EnergyPlusData &state,
                    bool const FirstHVACIteration, // TRUE if first full HVAC iteration in an HVAC timestep
                    bool &SimAir,                  // TRUE means air loops must be (re)simulated
                    bool &SimZoneEquipment         // TRUE means zone equipment must be (re) simulated
)
{

    // SUBROUTINE INFORMATION
    //             AUTHOR:  Russ Taylor, Dan Fisher, Fred Buhl
    //       DATE WRITTEN:  Oct 1997
    //           MODIFIED:  Dec 1997 Fred Buhl
    //      RE-ENGINEERED:  This is new code, not reengineered

    // PURPOSE OF THIS SUBROUTINE:
    // This is the manager subroutine for the air loop simulation.
    // Called from SimSelectedEquipment, which is called from SimHVAC,
    // which is called from ManageHVAC, the top level system/plant driver.
    // The subroutine performs the usual manager functions: it calls the
    // Get, Init, Sim, Update, and Report routines.

    auto &AirLoopControlInfo = state.dataAirLoop->AirLoopControlInfo;

    using MixedAir::ManageOutsideAirSystem;

    if (state.dataSimAirServingZones->GetAirLoopInputFlag) { // First time subroutine has been entered
        GetAirPathData(state);                               // Get air loop descriptions from input file
        state.dataSimAirServingZones->GetAirLoopInputFlag = false;
    }

    // Initialize air loop related parameters
    InitAirLoops(state, FirstHVACIteration);

    // Call the AirLoop Simulation
    if (state.dataGlobal->SysSizingCalc) {
        SizeAirLoops(state);
    } else {
        SimAirLoops(state, FirstHVACIteration, SimZoneEquipment);
    }

    // This flag could be used to resimulate only the air loops that needed additional iterations.
    // This flag would have to be moved inside SimAirLoops to gain this flexibility.
    SimAir = std::any_of(
        AirLoopControlInfo.begin(), AirLoopControlInfo.end(), [](DataAirLoop::AirLoopControlData const &e) { return e.ResimAirLoopFlag; });
}

// Get Input Section of the Module
//******************************************************************************

void GetAirPathData(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION
    //             AUTHOR:  Fred Buhl
    //       DATE WRITTEN:  Jan 1998
    //           MODIFIED: Richard Liesen April 1998, Fred Buhl Dec 1999
    //      RE-ENGINEERED:  This is new code, not reengineered

    // PURPOSE OF THIS SUBROUTINE:
    // Input all the data needed to simulate the air loops in the problem.

    // METHODOLOGY EMPLOYED:
    // Use the various "Get" routines from the InputProcessor module to
    // obtain input data and store it in the data structures defined in MODULE SimAirServingZones

    // REFERENCES: This gets the following object:
    // AirLoopHVAC,
    //        \min-fields 10
    //        \memo Defines a central forced air system
    //    A1, \field Name
    //        \required-field
    //        \type alpha
    //        \reference AirPrimaryLoops
    //    A2, \field Controller List Name
    //        \note Enter the name of an AirLoopHVAC:ControllerList object.
    //        \type object-list
    //        \object-list ControllerLists
    //    A3, \field Availability Manager List Name
    //        \note Enter the name of an AvailabilityManagerAssignmentList object.
    //        \type object-list
    //        \object-list SystemAvailabilityManagerLists
    //    N1, \field Design Primary Air Flow Rate
    //        \default 0
    //        \units m3/s
    //        \autosizable
    //    A4, \field BranchList Name
    //        \note Name of a BranchList containing all the branches in this air loop
    //        \required-field
    //        \type object-list
    //        \object-list BranchLists
    //    A5, \field ConnectorList Name
    //        \note Name of a Connector List containing all the splitters and mixers in the loop
    //        \type object-list
    //        \object-list ConnectorLists
    //    A6, \field Supply Side Inlet Node Name
    //        \note Name of inlet node where return air enters the supply side of the air loop
    //        \required-field
    //    A7, \field Demand Side Outlet Node Name
    //        \note Name of outlet node where return air leaves the demand side and enters the supply side.
    //        \required-field
    //    A8, \field Demand Side Inlet Node Names
    //        \note Name of a Node or NodeList containing the inlet node(s) supplying air to zone equipment.
    //        \required-field
    //    A9; \field Supply Side Outlet Node Names
    //        \note Name of a Node or NodeList containing the outlet node(s) supplying air to the demand side.
    //        \required-field

    // Using/Aliasing
    using BranchInputManager::GetBranchData;
    using BranchInputManager::GetBranchList;
    using BranchInputManager::GetLoopMixer;
    using BranchInputManager::GetLoopSplitter;
    using BranchInputManager::GetNumSplitterMixerInConntrList;
    using BranchInputManager::NumBranchesInBranchList;
    using BranchInputManager::NumCompsInBranch;
    using HVACControllers::CheckCoilWaterInletNode;
    using HVACControllers::GetControllerActuatorNodeNum;
    using MixedAir::FindOAMixerMatchForOASystem;
    using MixedAir::GetNumOASystems;
    using MixedAir::GetOACompListNumber;
    using MixedAir::GetOACompName;
    using MixedAir::GetOACompType;
    using MixedAir::GetOACompTypeNum;
    using MixedAir::GetOAMixerInletNodeNumber;
    using MixedAir::GetOASysControllerListIndex;
    using MixedAir::GetOASysNumCoolingCoils;
    using MixedAir::GetOASysNumHeatingCoils;
    using MixedAir::GetOASysNumHXs;
    using MixedAir::GetOASysNumSimpControllers;
    using MixedAir::GetOASystemNumber;
    using NodeInputManager::GetNodeNums;
    using NodeInputManager::GetOnlySingleNode;
    using WaterCoils::GetCoilWaterInletNode;

    // SUBROUTINE PARAMETER DEFINITIONS:
    constexpr std::string_view RoutineName("GetAirPathData: ");

    auto &OutsideAirSys = state.dataAirLoop->OutsideAirSys;
    auto &AirLoopControlInfo = state.dataAirLoop->AirLoopControlInfo;

    // SUBROUTINE LOCAL VARIABLE DEFINITIONS
    int NumNumbers;                // number of numbers returned by GetObjectItem
    Array1D<Real64> Numbers;       // numbers (REAL(r64)s) returned by GetObjectItem
    Array1D_string cNumericFields; // Numeric field names
    Array1D_bool lNumericBlanks;   // Logical array, numeric field input BLANK = .TRUE.
    int NumAlphas;                 // number of strings returned by GetObjectItem
    int NumParams;
    int MaxNumbers;
    int MaxAlphas;
    Array1D_string Alphas;            // alpha strings returned by GetObjectItem
    Array1D_string cAlphaFields;      // Alpha field names
    Array1D_bool lAlphaBlanks;        // Logical array, alpha field input BLANK = .TRUE.
    std::string CurrentModuleObject;  // Object type for getting and error messages
    int NumNodes;                     // number of nodes returned by GetNodeNums
    Array1D_int NodeNums;             // node numbers returned by GetNodeNums
    int NodeNum;                      // a node number
    int AirSysNum;                    // an air system (air loop) number
    int OANum;                        // outside air system index
    int OAMixNum;                     // outside air mixer index
    int IOStat;                       // status number returned by GetObjectItem
    int NumControllers;               // number of controllers
    int ControllerListNum;            // Controller List index
    int ControllerNum;                // Controller index
    int I;                            // do loop index
    int BranchNum;                    // branch index
    int CompNum;                      // component index
    int NumCompsOnBranch;             // Number of components on a branch
    int OutBranchNum;                 // outlet branch index
    int InBranchNum;                  // inlet branch index
    std::string ControllerName;       // controller name
    std::string ControllerType;       // controller type
    std::string BranchListName;       // Name of a Branch List object
    std::string ControllerListName;   // Name of a controller list object
    std::string AvailManagerListName; // Name of an availability manager list object
    std::string ConnectorListName;    // Name of a connector list object
    Array1D_string BranchNames;       // Branch names from GetBranchList call
    Array1D_string CompTypes;         // Component types from GetBranchList call
    Array1D_string CompNames;         // Component names from GetBranchList call
    Array1D_string InletNodeNames;    // Component inlet node names from GetBranchData call
    Array1D_string OutletNodeNames;   // Component outlet node names from GetBranchData call
    Array1D_string NodeNames;         // Outlet node names from GetLoopSplitter call
    Array1D_int NodeNumbers;          // Outlet node numbers from GetLoopSplitter call
    Array1D_int InletNodeNumbers;     // Component inlet node numbers from GetBranchData call
    Array1D_int OutletNodeNumbers;    // Component outlet node numbers from GetBranchData call
    DataBranchAirLoopPlant::PressureCurveType PressCurveType;
    int PressCurveIndex;
    bool ErrorsFound(false); // TRUE if errors detected in input
    Array1D_bool PackagedUnit;
    int test;
    int count;
    bool ErrInList;
    int ConListNum(0);          // index of a Connector List object in the input
    bool SplitterExists(false); // TRUE if there is a slitter in a primary air system
    bool MixerExists(false);    // TRUE if there is a mixer in a primary air system
    bool errFlag;
    bool IsNotOK;
    /////////// hoisted into namespace
    // static int TestUniqueNodesNum( 0 );
    ///////////////////////////
    int NumOASysSimpControllers; // number of simple controllers in the OA Sys of an air primary system
    int NumOASysControllers;     // total number of controllers in the OA Sys
    int OASysContListNum;        // index of the controller list of the OA Sys
    int OASysControllerNum;      // index of OA Sys simple controller in the air primary system controller lists
    bool NodeNotFound;           // true if matching actuator node not found
    CompType CompType_Num;       // numeric equivalent for component type
    std::string CompType;        // component type
    int WaterCoilNodeNum;        // numeric equivalent for water coil node number
    int ActuatorNodeNum;         // numeric equivalent for controller actuator node number
    Array1D_string MatchNodeName(3);

    struct AirUniqueNodes
    {
        // Members
        std::string NodeName;
        std::string AirLoopName;
        std::string FieldName;
        bool NodeNameUsed;

        // Default Constructor
        AirUniqueNodes() : NodeNameUsed(false)
        {
        }
    };

    // Object Data
    Array1D<AirUniqueNodes> TestUniqueNodes;

    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "AirLoopHVAC", NumParams, MaxAlphas, MaxNumbers);
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "ConnectorList", NumParams, NumAlphas, NumNumbers);
    MaxAlphas = max(MaxAlphas, NumAlphas);
    MaxNumbers = max(MaxNumbers, NumNumbers);
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "AirLoopHVAC:ControllerList", NumParams, NumAlphas, NumNumbers);
    MaxAlphas = max(MaxAlphas, NumAlphas);
    MaxNumbers = max(MaxNumbers, NumNumbers);

    Numbers.allocate(MaxNumbers);
    cNumericFields.allocate(MaxNumbers);
    lNumericBlanks.allocate(MaxNumbers);
    Alphas.allocate(MaxAlphas);
    cAlphaFields.allocate(MaxAlphas);
    lAlphaBlanks.allocate(MaxAlphas);

    // Initialize some local arrays
    Numbers = 0.0;
    cNumericFields = "";
    lNumericBlanks = true;
    Alphas = "";
    cAlphaFields = "";
    lAlphaBlanks = true;

    state.dataSimAirServingZones->NumOfTimeStepInDay = state.dataGlobal->TimeStepsInHour * Constant::iHoursInDay;

    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "NodeList", NumParams, NumAlphas, NumNumbers);
    NodeNums.dimension(NumParams, 0);

    // Find number of primary air systems, update Num in state and make local convenience copy
    int NumPrimaryAirSys = state.dataHVACGlobal->NumPrimaryAirSys =
        state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirLoopHVAC");
    TestUniqueNodes.allocate(NumPrimaryAirSys * 4); // used to look at specific nodes that must be unique, fields A6-A9

    state.dataAirSystemsData->PrimaryAirSystems.allocate(NumPrimaryAirSys); // allocate the primary air sys data array
    state.dataAirLoop->AirToZoneNodeInfo.allocate(NumPrimaryAirSys);        // allocate the array that stores the air sys / zone equp connection data
    state.dataAirLoop->AirLoopZoneInfo.allocate(NumPrimaryAirSys);          // allocate array that has cleaner list of zones attached to air loop
    state.dataAirLoop->AirToOANodeInfo.allocate(NumPrimaryAirSys);          // allocate the array that stores the OA node connections (reporting)
    PackagedUnit.allocate(NumPrimaryAirSys);
    AirLoopControlInfo.allocate(NumPrimaryAirSys);
    state.dataAirLoop->AirLoopFlow.allocate(NumPrimaryAirSys);
    state.dataConvergeParams->AirLoopConvergence.allocate(NumPrimaryAirSys);
    state.dataSize->UnitarySysEqSizing.allocate(NumPrimaryAirSys);
    if (state.afn->distribution_simulated) {
        state.dataAirLoop->AirLoopAFNInfo.allocate(NumPrimaryAirSys);
    }

    state.dataHVACGlobal->GetAirPathDataDone = true; // used by UnitarySystem::getUnitarySystemInputData to determine if airloops are setup yet
    if (NumPrimaryAirSys <= 0) {
        TestUniqueNodes.deallocate();
        NodeNums.deallocate();
        return;
    }

    // Loop through the primary air systems and obtain the data for each system
    for (AirSysNum = 1; AirSysNum <= NumPrimaryAirSys; ++AirSysNum) {
        auto &primaryAirSystems = state.dataAirSystemsData->PrimaryAirSystems(AirSysNum);
        auto &airLoopZoneInfo = state.dataAirLoop->AirToZoneNodeInfo(AirSysNum);
        NumOASysControllers = 0;
        NumOASysSimpControllers = 0;
        OASysContListNum = 0;
        PackagedUnit(AirSysNum) = false;
        primaryAirSystems.OASysExists = false; // init Outside Air system connection data to none
        primaryAirSystems.isAllOA = false;
        primaryAirSystems.OASysInletNodeNum = 0;
        primaryAirSystems.OASysOutletNodeNum = 0;
        primaryAirSystems.NumOAHeatCoils = 0;
        primaryAirSystems.NumOACoolCoils = 0;
        AirLoopControlInfo(AirSysNum).fanOp = HVAC::FanOp::Continuous; // initialize to constant fan mode for all air loops
        state.dataAirLoop->AirLoopFlow(AirSysNum).FanPLR = 1.0;        // initialize to 1 for all air loops

        CurrentModuleObject = "AirLoopHVAC";

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 AirSysNum,
                                                                 Alphas,
                                                                 NumAlphas,
                                                                 Numbers,
                                                                 NumNumbers,
                                                                 IOStat,
                                                                 lNumericBlanks,
                                                                 lAlphaBlanks,
                                                                 cAlphaFields,
                                                                 cNumericFields); // get all the input data for the air system

        // Assign the air system data to the simulation variables.
        // Data needed to simulate the system goes into PrimaryAirSystem.
        // Data connecting the air system to the zone equipment goes into AirToZoneNodeInfo (in DataLoopNode).
        Util::IsNameEmpty(state, Alphas(1), CurrentModuleObject, ErrorsFound);
        primaryAirSystems.Name = Alphas(1);
        airLoopZoneInfo.AirLoopName = Alphas(1);
        if (NumAlphas < 9) {
            ShowSevereError(state, format("{}{}=\"{}\", insufficient information.", RoutineName, CurrentModuleObject, Alphas(1)));
            ShowContinueError(state, "...Have supplied less than 9 alpha fields.");
            ErrorsFound = true;
            continue;
        }
        if (NumNumbers < 1) {
            ShowSevereError(state, format("{}{}=\"{}\", insufficient information.", RoutineName, CurrentModuleObject, Alphas(1)));
            ShowContinueError(state, "...Have supplied less than 1 numeric field.");
            ErrorsFound = true;
            continue;
        }
        primaryAirSystems.DesignVolFlowRate = Numbers(1);
        if (!lNumericBlanks(2)) {
            primaryAirSystems.DesignReturnFlowFraction = Numbers(2);
        }
        // Only allow one return air node (at the loop level)
        airLoopZoneInfo.NumReturnNodes = 1;
        // Allocate the return air node arrays
        airLoopZoneInfo.AirLoopReturnNodeNum.allocate(airLoopZoneInfo.NumReturnNodes);
        airLoopZoneInfo.ZoneEquipReturnNodeNum.allocate(airLoopZoneInfo.NumReturnNodes);
        airLoopZoneInfo.ReturnAirPathNum.allocate(airLoopZoneInfo.NumReturnNodes);
        // fill the return air node arrays with node numbers
        airLoopZoneInfo.ReturnAirPathNum(1) = 0;
        airLoopZoneInfo.AirLoopReturnNodeNum(1) = GetOnlySingleNode(state,
                                                                    Alphas(6),
                                                                    ErrorsFound,
                                                                    DataLoopNode::ConnectionObjectType::AirLoopHVAC,
                                                                    Alphas(1),
                                                                    DataLoopNode::NodeFluidType::Air,
                                                                    DataLoopNode::ConnectionType::Inlet,
                                                                    NodeInputManager::CompFluidStream::Primary,
                                                                    ObjectIsParent);
        if (!lAlphaBlanks(7)) {
            airLoopZoneInfo.ZoneEquipReturnNodeNum(1) = GetOnlySingleNode(state,
                                                                          Alphas(7),
                                                                          ErrorsFound,
                                                                          DataLoopNode::ConnectionObjectType::AirLoopHVAC,
                                                                          Alphas(1),
                                                                          DataLoopNode::NodeFluidType::Air,
                                                                          DataLoopNode::ConnectionType::Outlet,
                                                                          NodeInputManager::CompFluidStream::Primary,
                                                                          ObjectIsParent);
        } else {
            // If no return path, set this to zero to trigger special handling when calling UpdateHVACInterface
            airLoopZoneInfo.ZoneEquipReturnNodeNum(1) = 0;
        }

        // work on unique nodes
        test = Util::FindItemInList(Alphas(6), TestUniqueNodes, &AirUniqueNodes::NodeName, state.dataSimAirServingZones->TestUniqueNodesNum);
        if (test == 0) {
            ++state.dataSimAirServingZones->TestUniqueNodesNum;
            TestUniqueNodes(state.dataSimAirServingZones->TestUniqueNodesNum).NodeName = Alphas(6);
            TestUniqueNodes(state.dataSimAirServingZones->TestUniqueNodesNum).AirLoopName = Alphas(1);
            TestUniqueNodes(state.dataSimAirServingZones->TestUniqueNodesNum).FieldName = cAlphaFields(6);
            TestUniqueNodes(state.dataSimAirServingZones->TestUniqueNodesNum).NodeNameUsed = true;
        } else {
            ShowSevereError(state, format("{}{}=\"{}\", duplicate node name.", RoutineName, CurrentModuleObject, Alphas(1)));
            ShowContinueError(state, format("...used for {}=\"{}\"", cAlphaFields(6), Alphas(6)));
            ShowContinueError(
                state,
                format("...first used in {}=\"{}\" for {}", CurrentModuleObject, TestUniqueNodes(test).AirLoopName, TestUniqueNodes(test).FieldName));
            ErrorsFound = true;
        }
        if (!lAlphaBlanks(7)) {
            test = Util::FindItemInList(Alphas(7), TestUniqueNodes, &AirUniqueNodes::NodeName, state.dataSimAirServingZones->TestUniqueNodesNum);
            if (test == 0) {
                ++state.dataSimAirServingZones->TestUniqueNodesNum;
                TestUniqueNodes(state.dataSimAirServingZones->TestUniqueNodesNum).NodeName = Alphas(7);
                TestUniqueNodes(state.dataSimAirServingZones->TestUniqueNodesNum).AirLoopName = Alphas(1);
                TestUniqueNodes(state.dataSimAirServingZones->TestUniqueNodesNum).FieldName = cAlphaFields(7);
                TestUniqueNodes(state.dataSimAirServingZones->TestUniqueNodesNum).NodeNameUsed = true;
            } else {
                ShowSevereError(state, format("{}{}=\"{}\", duplicate node name.", RoutineName, CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("...used for {}=\"{}\"", cAlphaFields(7), Alphas(7)));
                ShowContinueError(state,
                                  format("...first used in {}=\"{}\" for {}",
                                         CurrentModuleObject,
                                         TestUniqueNodes(test).AirLoopName,
                                         TestUniqueNodes(test).FieldName));
                ErrorsFound = true;
            }
        }
        test = Util::FindItemInList(Alphas(8), TestUniqueNodes, &AirUniqueNodes::NodeName, state.dataSimAirServingZones->TestUniqueNodesNum);
        if (test == 0) {
            ++state.dataSimAirServingZones->TestUniqueNodesNum;
            TestUniqueNodes(state.dataSimAirServingZones->TestUniqueNodesNum).NodeName = Alphas(8);
            TestUniqueNodes(state.dataSimAirServingZones->TestUniqueNodesNum).AirLoopName = Alphas(1);
            TestUniqueNodes(state.dataSimAirServingZones->TestUniqueNodesNum).FieldName = cAlphaFields(8);
            TestUniqueNodes(state.dataSimAirServingZones->TestUniqueNodesNum).NodeNameUsed = true;
        } else {
            ShowSevereError(state, format("{}{}=\"{}\", duplicate node name/list.", RoutineName, CurrentModuleObject, Alphas(1)));
            ShowContinueError(state, format("...used for {}=\"{}\"", cAlphaFields(8), Alphas(8)));
            ShowContinueError(
                state,
                format("...first used in {}=\"{}\" for {}", CurrentModuleObject, TestUniqueNodes(test).AirLoopName, TestUniqueNodes(test).FieldName));
            ErrorsFound = true;
        }
        test = Util::FindItemInList(Alphas(9), TestUniqueNodes, &AirUniqueNodes::NodeName, state.dataSimAirServingZones->TestUniqueNodesNum);
        if (test == 0) {
            ++state.dataSimAirServingZones->TestUniqueNodesNum;
            TestUniqueNodes(state.dataSimAirServingZones->TestUniqueNodesNum).NodeName = Alphas(9);
            TestUniqueNodes(state.dataSimAirServingZones->TestUniqueNodesNum).AirLoopName = Alphas(1);
            TestUniqueNodes(state.dataSimAirServingZones->TestUniqueNodesNum).FieldName = cAlphaFields(9);
            TestUniqueNodes(state.dataSimAirServingZones->TestUniqueNodesNum).NodeNameUsed = true;
        } else {
            ShowSevereError(state, format("{}{}=\"{}\", duplicate node name/list.", RoutineName, CurrentModuleObject, Alphas(1)));
            ShowContinueError(state, format("...used for {}=\"{}\"", cAlphaFields(9), Alphas(9)));
            ShowContinueError(
                state,
                format("...first used in {}=\"{}\" for {}", CurrentModuleObject, TestUniqueNodes(test).AirLoopName, TestUniqueNodes(test).FieldName));
            ErrorsFound = true;
        }
        // this test depends on the controlled zone input having been "gotten"
        test = 0;
        for (count = 1; count <= state.dataZoneEquip->NumReturnAirPaths; ++count) {
            if (state.dataZoneEquip->ReturnAirPath(count).OutletNodeNum == airLoopZoneInfo.ZoneEquipReturnNodeNum(1)) {
                test = state.dataZoneEquip->ReturnAirPath(count).OutletNodeNum;
                break;
            }
        }
        if ((test == 0) && (airLoopZoneInfo.NumReturnNodes > 0)) { // there, see if it's in the controlled zone info
            for (count = 1; count <= state.dataGlobal->NumOfZones; ++count) {
                for (int retNode = 1; retNode <= state.dataZoneEquip->ZoneEquipConfig(count).NumReturnNodes; ++retNode) {
                    if (state.dataZoneEquip->ZoneEquipConfig(count).ReturnNode(retNode) != airLoopZoneInfo.ZoneEquipReturnNodeNum(1)) {
                        continue;
                    }
                    test = count;
                    break;
                }
                if (test == count) {
                    break;
                }
            }
        }
        if ((test == 0) && (airLoopZoneInfo.NumReturnNodes > 0) && !lAlphaBlanks(7)) {
            ShowSevereError(state, format("{}{}=\"{}\", invalid.", RoutineName, CurrentModuleObject, Alphas(1)));
            ShowContinueError(state, format("{} (Return Air Path or ZoneHVAC:EquipmentConnections) not valid = \"{}\".", cAlphaFields(7), Alphas(7)));
            ErrorsFound = true;
        }
        // Get the supply nodes
        ErrInList = false;
        GetNodeNums(state,
                    Alphas(8),
                    NumNodes,
                    NodeNums,
                    ErrInList,
                    DataLoopNode::NodeFluidType::Air,
                    DataLoopNode::ConnectionObjectType::AirLoopHVAC,
                    primaryAirSystems.Name,
                    DataLoopNode::ConnectionType::Inlet,
                    NodeInputManager::CompFluidStream::Primary,
                    ObjectIsParent,
                    false,
                    cAlphaFields(8));
        if (ErrInList) {
            ErrorsFound = true;
        }
        // Allow at most 3 supply nodes (for a 3 deck system)
        if (NumNodes > 3) {
            ShowSevereError(state, format("{}{}=\"{}\", too many nodes.", RoutineName, CurrentModuleObject, primaryAirSystems.Name));
            ShowContinueError(state, format("Only 1st 3 Nodes will be used from {}=\"{}\".", cAlphaFields(8), Alphas(8)));
            ErrorsFound = true;
        }
        if (NumNodes == 0) {
            ShowSevereError(state, format("{}{}=\"{}\", too few nodes.", RoutineName, CurrentModuleObject, primaryAirSystems.Name));
            ShowContinueError(state, "There must be at least 1 supply node in the system.");
            ErrorsFound = true;
        }
        airLoopZoneInfo.NumSupplyNodes = NumNodes;
        // Allocate the supply node arrays in AirToZoneNodeInfo
        airLoopZoneInfo.ZoneEquipSupplyNodeNum.allocate(airLoopZoneInfo.NumSupplyNodes);
        airLoopZoneInfo.AirLoopSupplyNodeNum.allocate(airLoopZoneInfo.NumSupplyNodes);
        airLoopZoneInfo.SupplyDuctType.allocate(airLoopZoneInfo.NumSupplyNodes);
        airLoopZoneInfo.SupplyDuctBranchNum.allocate(airLoopZoneInfo.NumSupplyNodes);
        airLoopZoneInfo.SupplyAirPathNum.allocate(airLoopZoneInfo.NumSupplyNodes);

        // Fill the supply node arrays with node numbers
        for (I = 1; I <= airLoopZoneInfo.NumSupplyNodes; ++I) {
            airLoopZoneInfo.ZoneEquipSupplyNodeNum(I) = NodeNums(I);
            airLoopZoneInfo.SupplyDuctType(I) = HVAC::AirDuctType::Invalid;
            airLoopZoneInfo.SupplyDuctBranchNum(I) = 0;
            airLoopZoneInfo.SupplyAirPathNum(I) = 0;
        }
        ErrInList = false;
        GetNodeNums(state,
                    Alphas(9),
                    NumNodes,
                    NodeNums,
                    ErrInList,
                    DataLoopNode::NodeFluidType::Air,
                    DataLoopNode::ConnectionObjectType::AirLoopHVAC,
                    primaryAirSystems.Name,
                    DataLoopNode::ConnectionType::Outlet,
                    NodeInputManager::CompFluidStream::Primary,
                    ObjectIsParent,
                    false,
                    cAlphaFields(9));
        if (ErrInList) {
            ErrorsFound = true;
        }
        if (NumNodes != airLoopZoneInfo.NumSupplyNodes) {
            ShowSevereError(state, format("{}{}=\"{}\", node mismatch.", RoutineName, CurrentModuleObject, Alphas(1)));
            ShowContinueError(state,
                              format("...number of air system exit nodes [{}] must match number of zone equip inlet nodes [{}].",
                                     NumNodes,
                                     airLoopZoneInfo.NumSupplyNodes));
            ErrorsFound = true;
        }
        for (I = 1; I <= airLoopZoneInfo.NumSupplyNodes; ++I) {
            airLoopZoneInfo.AirLoopSupplyNodeNum(I) = NodeNums(I);
        }
        airLoopZoneInfo.NumZonesCooled = 0;
        airLoopZoneInfo.NumZonesHeated = 0;
        // Branch, Controller, Availability Manager and Connector List Names to access later
        ControllerListName = Alphas(2);
        BranchListName = Alphas(4);
        AvailManagerListName = Alphas(3);
        ConnectorListName = Alphas(5);
        primaryAirSystems.NumBranches = NumBranchesInBranchList(state, BranchListName);
        if (primaryAirSystems.NumBranches == 0) {
            ShowSevereError(state, format("{}{}=\"{}\", insufficient information.", RoutineName, CurrentModuleObject, primaryAirSystems.Name));
            ShowContinueError(state, "...there must be at least 1 branch specified.");
            ErrorsFound = true;
        }
        BranchNames.allocate(primaryAirSystems.NumBranches);
        BranchNames = "";
        // get the branch lists
        GetBranchList(state, primaryAirSystems.Name, BranchListName, primaryAirSystems.NumBranches, BranchNames, "Air");
        primaryAirSystems.Branch.allocate(primaryAirSystems.NumBranches);
        // Cycle through all of the branches and set up the branch data
        for (BranchNum = 1; BranchNum <= primaryAirSystems.NumBranches; ++BranchNum) {
            primaryAirSystems.Branch(BranchNum).Name = BranchNames(BranchNum);
            NumCompsOnBranch = NumCompsInBranch(state, BranchNames(BranchNum));
            if (NumCompsOnBranch <= 0) {
                ShowSevereError(state, format("{}{}=\"{}\", insufficient information.", RoutineName, CurrentModuleObject, primaryAirSystems.Name));
                ShowContinueError(state, format("...Branch=\"{}\", no components on branch.", BranchNames(BranchNum)));
                ErrorsFound = true;
                continue;
            }
            CompTypes.allocate(NumCompsOnBranch);
            CompNames.allocate(NumCompsOnBranch);
            InletNodeNames.allocate(NumCompsOnBranch);
            InletNodeNumbers.dimension(NumCompsOnBranch, 0);
            OutletNodeNames.allocate(NumCompsOnBranch);
            OutletNodeNumbers.dimension(NumCompsOnBranch, 0);

            GetBranchData(state,
                          primaryAirSystems.Name,
                          BranchNames(BranchNum),
                          PressCurveType,
                          PressCurveIndex,
                          NumCompsOnBranch,
                          CompTypes,
                          CompNames,
                          InletNodeNames,
                          InletNodeNumbers,
                          OutletNodeNames,
                          OutletNodeNumbers,
                          ErrorsFound); // Placeholders for plant branch pressure data (not used in air loops)
            primaryAirSystems.Branch(BranchNum).Comp.allocate(NumCompsOnBranch);
            primaryAirSystems.Branch(BranchNum).TotalComponents = NumCompsOnBranch;

            primaryAirSystems.Branch(BranchNum).TotalNodes = NumCompsOnBranch + 1;
            primaryAirSystems.Branch(BranchNum).NodeNum.allocate(NumCompsOnBranch + 1);
            primaryAirSystems.Branch(BranchNum).NodeNum(1) = InletNodeNumbers(1);
            primaryAirSystems.Branch(BranchNum).DuctType = HVAC::AirDuctType::Main;

            // If first node is an outdoor air node, then consider this to have a simple OA system (many places check for this)
            if (OutAirNodeManager::CheckOutAirNodeNumber(state, InletNodeNumbers(1))) {
                primaryAirSystems.OASysExists = true;
                primaryAirSystems.isAllOA = true;
                primaryAirSystems.OASysInletNodeNum = InletNodeNumbers(1);
                primaryAirSystems.OASysOutletNodeNum = InletNodeNumbers(1);
                primaryAirSystems.OAMixOAInNodeNum = InletNodeNumbers(1);
                state.dataAirLoop->AirToOANodeInfo(AirSysNum).OASysExists = true;
                state.dataAirLoop->AirToOANodeInfo(AirSysNum).OASysInletNodeNum = InletNodeNumbers(1);
                state.dataAirLoop->AirToOANodeInfo(AirSysNum).OASysOutletNodeNum = InletNodeNumbers(1);
            }
            for (CompNum = 1; CompNum <= primaryAirSystems.Branch(BranchNum).TotalComponents; ++CompNum) {

                primaryAirSystems.Branch(BranchNum).Comp(CompNum).TypeOf = CompTypes(CompNum);
                primaryAirSystems.Branch(BranchNum).Comp(CompNum).Name = CompNames(CompNum);
                primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompIndex = 0;
                primaryAirSystems.Branch(BranchNum).Comp(CompNum).NodeNameIn = InletNodeNames(CompNum);
                primaryAirSystems.Branch(BranchNum).Comp(CompNum).NodeNumIn = InletNodeNumbers(CompNum);
                primaryAirSystems.Branch(BranchNum).Comp(CompNum).NodeNameOut = OutletNodeNames(CompNum);
                primaryAirSystems.Branch(BranchNum).Comp(CompNum).NodeNumOut = OutletNodeNumbers(CompNum);
                primaryAirSystems.Branch(BranchNum).NodeNum(CompNum + 1) = OutletNodeNumbers(CompNum);

                // Check for Outside Air system; if there, store its connection node numbers to primary air system
                if (Util::SameString(CompTypes(CompNum), "AirLoopHVAC:OutdoorAirSystem")) {
                    if (primaryAirSystems.OASysExists) {
                        ShowSevereError(
                            state, format("{}{}=\"{}\", too many outdoor air systems.", RoutineName, CurrentModuleObject, primaryAirSystems.Name));
                        ShowContinueError(state, "Only one AirLoopHVAC:OutdoorAirSystem allowed.");
                        ErrorsFound = true;
                        continue;
                    }
                    primaryAirSystems.OASysExists = true;
                    primaryAirSystems.OASysInletNodeNum = InletNodeNumbers(CompNum);
                    primaryAirSystems.OASysOutletNodeNum = OutletNodeNumbers(CompNum);
                    state.dataAirLoop->AirToOANodeInfo(AirSysNum).OASysExists = true;
                    state.dataAirLoop->AirToOANodeInfo(AirSysNum).OASysInletNodeNum = InletNodeNumbers(CompNum);
                    state.dataAirLoop->AirToOANodeInfo(AirSysNum).OASysOutletNodeNum = OutletNodeNumbers(CompNum);
                    OANum = GetOASystemNumber(state, CompNames(CompNum));
                    if (OANum > 0) {
                        NumOASysSimpControllers = GetOASysNumSimpControllers(state, OANum);
                        primaryAirSystems.NumOAHeatCoils = GetOASysNumHeatingCoils(state, OANum);
                        primaryAirSystems.NumOACoolCoils = GetOASysNumCoolingCoils(state, OANum);
                        primaryAirSystems.NumOAHXs = GetOASysNumHXs(state, OANum);
                        OASysContListNum = GetOASysControllerListIndex(state, OANum);
                        OAMixNum = FindOAMixerMatchForOASystem(state, OANum);
                        if (OAMixNum > 0) {
                            primaryAirSystems.OAMixOAInNodeNum = GetOAMixerInletNodeNumber(state, OAMixNum);
                        } else {
                            ShowSevereError(state, format("{}{}=\"{}\", item not found.", RoutineName, CurrentModuleObject, primaryAirSystems.Name));
                            ShowContinueError(state,
                                              format("OutdoorAir:Mixer for AirLoopHVAC:OutdoorAirSystem=\"{}\" not found.", CompNames(CompNum)));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state, format("{}{}=\"{}\", item not found.", RoutineName, CurrentModuleObject, primaryAirSystems.Name));
                        ShowContinueError(state, format("AirLoopHVAC:OutdoorAirSystem=\"{}\" not found.", CompNames(CompNum)));
                        ShowContinueError(state, format("  referenced in Branch=\"{}\".", primaryAirSystems.Branch(BranchNum).Name));
                        ErrorsFound = true;
                    }
                }
                {
                    std::string const componentType = uppercased(CompTypes(CompNum));
                    if (componentType == "COILSYSTEM:COOLING:DX") {
                        PackagedUnit(AirSysNum) = true;
                    } else if (componentType == "COILSYSTEM:HEATING:DX") {
                        PackagedUnit(AirSysNum) = true;
                    } else if (componentType == "COILSYSTEM:COOLING:WATER") {
                        PackagedUnit(AirSysNum) = true;
                    } else if (componentType == "AIRLOOPHVAC:UNITARYSYSTEM") {
                        PackagedUnit(AirSysNum) = true;
                    } else if (componentType == "AIRLOOPHVAC:UNITARY:FURNACE:HEATONLY") {
                        PackagedUnit(AirSysNum) = true;
                    } else if (componentType == "AIRLOOPHVAC:UNITARY:FURNACE:HEATCOOL") {
                        PackagedUnit(AirSysNum) = true;
                    } else if (componentType == "AIRLOOPHVAC:UNITARYHEATONLY") {
                        PackagedUnit(AirSysNum) = true;
                    } else if (componentType == "AIRLOOPHVAC:UNITARYHEATCOOL") {
                        PackagedUnit(AirSysNum) = true;
                    } else if (componentType == "AIRLOOPHVAC:UNITARYHEATPUMP:AIRTOAIR") {
                        PackagedUnit(AirSysNum) = true;
                    } else if (componentType == "AIRLOOPHVAC:UNITARYHEATPUMP:WATERTOAIR") {
                        PackagedUnit(AirSysNum) = true;
                    } else if (componentType == "AIRLOOPHVAC:UNITARYHEATCOOL:VAVCHANGEOVERBYPASS") {
                        PackagedUnit(AirSysNum) = true;
                    } else if (componentType == "AIRLOOPHVAC:UNITARYHEATPUMP:AIRTOAIR:MULTISPEED") {
                        PackagedUnit(AirSysNum) = true;
                    }
                }

            } // end of component loop

            primaryAirSystems.Branch(BranchNum).ControlType = "";
            primaryAirSystems.Branch(BranchNum).NodeNumIn = InletNodeNumbers(1);
            primaryAirSystems.Branch(BranchNum).NodeNumOut = OutletNodeNumbers(NumCompsOnBranch);

            CompTypes.deallocate();
            CompNames.deallocate();
            InletNodeNames.deallocate();
            InletNodeNumbers.deallocate();
            OutletNodeNames.deallocate();
            OutletNodeNumbers.deallocate();

        } // end of branch loop

        BranchNames.deallocate();

        // find and store the primary air system outlet branch reference numbers
        primaryAirSystems.NumOutletBranches = airLoopZoneInfo.NumSupplyNodes;
        for (OutBranchNum = 1; OutBranchNum <= 3; ++OutBranchNum) {
            primaryAirSystems.OutletBranchNum[OutBranchNum - 1] = 0;
            if (OutBranchNum > primaryAirSystems.NumOutletBranches) {
                break;
            }
            MatchNodeName(OutBranchNum) = state.dataLoopNodes->NodeID(airLoopZoneInfo.AirLoopSupplyNodeNum(OutBranchNum));
            for (BranchNum = 1; BranchNum <= primaryAirSystems.NumBranches; ++BranchNum) {
                if (airLoopZoneInfo.AirLoopSupplyNodeNum(OutBranchNum) == primaryAirSystems.Branch(BranchNum).NodeNumOut) {
                    primaryAirSystems.OutletBranchNum[OutBranchNum - 1] = BranchNum;
                }
            }
        }
        //  Check for errors
        for (OutBranchNum = 1; OutBranchNum <= primaryAirSystems.NumOutletBranches; ++OutBranchNum) {
            if (primaryAirSystems.OutletBranchNum[OutBranchNum - 1] != 0) {
                continue;
            }
            ShowSevereError(state, format("{}{}=\"{}\", branch in error.", RoutineName, CurrentModuleObject, primaryAirSystems.Name));
            ShowContinueError(state, "Probable missing or misspelled node referenced in the branch(es):");
            for (BranchNum = 1; BranchNum <= primaryAirSystems.NumBranches; ++BranchNum) {
                ShowContinueError(state, format("Possible Error in Branch Object=\"{}\".", primaryAirSystems.Branch(BranchNum).Name));
            }
            ShowContinueError(state, format("...looking to match to Node=\"{}\".", MatchNodeName(OutBranchNum)));
            ErrorsFound = true;
        }

        // find and store the primary air system inlet branch numbers
        primaryAirSystems.NumInletBranches = airLoopZoneInfo.NumReturnNodes;
        for (InBranchNum = 1; InBranchNum <= primaryAirSystems.NumInletBranches; ++InBranchNum) {
            primaryAirSystems.InletBranchNum[InBranchNum - 1] = 0;
            for (BranchNum = 1; BranchNum <= primaryAirSystems.NumBranches; ++BranchNum) {
                if (airLoopZoneInfo.AirLoopReturnNodeNum(InBranchNum) == primaryAirSystems.Branch(BranchNum).NodeNumIn) {
                    primaryAirSystems.InletBranchNum[InBranchNum - 1] = BranchNum;
                }
            }
            if (primaryAirSystems.InletBranchNum[InBranchNum - 1] == 0) {
                ShowSevereError(state, format("{}{}=\"{}\", connection to zone.", RoutineName, CurrentModuleObject, primaryAirSystems.Name));
                ShowContinueError(state, "No Connection found for Return Air from Zone");
                ShowContinueError(
                    state, format("Expected node name =\"{}\".", state.dataLoopNodes->NodeID(airLoopZoneInfo.AirLoopReturnNodeNum(InBranchNum))));
                ErrorsFound = true;
            }
        }

        // Check to see if a spliter and/or mixer exist
        SplitterExists = false;
        MixerExists = false;

        if (ConnectorListName != std::string()) {
            ConListNum = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "ConnectorList", ConnectorListName);
            if (ConListNum > 0) {
                state.dataInputProcessing->inputProcessor->getObjectItem(
                    state, "ConnectorList", ConListNum, Alphas, NumAlphas, Numbers, NumNumbers, IOStat);
                if ((Util::SameString(Alphas(2), "Connector:Splitter")) || (Util::SameString(Alphas(4), "Connector:Splitter"))) {
                    SplitterExists = true;
                }
                if ((Util::SameString(Alphas(2), "Connector:Mixer")) || (Util::SameString(Alphas(4), "Connector:Mixer"))) {
                    MixerExists = true;
                }
            } else {
                ShowSevereError(state, format("{}{}=\"{}\", connector list object.", RoutineName, CurrentModuleObject, primaryAirSystems.Name));
                ShowContinueError(state, format("ConnectorList object=\"{}\" not found in input.", ConnectorListName));
            }
            errFlag = false;
            GetNumSplitterMixerInConntrList(
                state, "AirLoop", ConnectorListName, state.dataLoopNodes->NumofSplitters, state.dataLoopNodes->NumofMixers, errFlag);
            if (errFlag) {
            }
        }

        // If there is a SPLITTER, get its data
        if (SplitterExists) {
            state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "Connector:Splitter", NumParams, NumAlphas, NumNodes);
            NodeNames.allocate(NumAlphas);
            NodeNumbers.allocate(NumAlphas);
            GetLoopSplitter(state,
                            primaryAirSystems.Name,
                            ConnectorListName,
                            primaryAirSystems.Splitter.Name,
                            primaryAirSystems.Splitter.Exists,
                            primaryAirSystems.Splitter.NodeNameIn,
                            primaryAirSystems.Splitter.NodeNumIn,
                            primaryAirSystems.Splitter.TotalOutletNodes,
                            NodeNames,
                            NodeNumbers,
                            ErrorsFound);

            primaryAirSystems.Splitter.NodeNameOut.allocate(primaryAirSystems.Splitter.TotalOutletNodes);
            primaryAirSystems.Splitter.NodeNumOut.allocate(primaryAirSystems.Splitter.TotalOutletNodes);
            primaryAirSystems.Splitter.BranchNumOut.allocate(primaryAirSystems.Splitter.TotalOutletNodes);

            for (NodeNum = 1; NodeNum <= primaryAirSystems.Splitter.TotalOutletNodes; ++NodeNum) {

                primaryAirSystems.Splitter.NodeNameOut(NodeNum) = NodeNames(NodeNum);
                primaryAirSystems.Splitter.NodeNumOut(NodeNum) = NodeNumbers(NodeNum);

                primaryAirSystems.Splitter.BranchNumOut(NodeNum) = 0;
                for (BranchNum = 1; BranchNum <= primaryAirSystems.NumBranches; ++BranchNum) {

                    if (primaryAirSystems.Branch(BranchNum).NodeNumIn == primaryAirSystems.Splitter.NodeNumOut(NodeNum)) {
                        primaryAirSystems.Splitter.BranchNumOut(NodeNum) = BranchNum;
                        break;
                    }
                }
            }

            primaryAirSystems.Splitter.BranchNumIn = 0;
            for (BranchNum = 1; BranchNum <= primaryAirSystems.NumBranches; ++BranchNum) {

                if (primaryAirSystems.Branch(BranchNum).NodeNumOut == primaryAirSystems.Splitter.NodeNumIn) {
                    primaryAirSystems.Splitter.BranchNumIn = BranchNum;
                    break;
                }
            }

            if (allocated(NodeNames)) {
                NodeNames.deallocate();
                NodeNumbers.deallocate();
            }

        } else {
            primaryAirSystems.Splitter.Exists = false;
            primaryAirSystems.Splitter.NodeNumIn = 0;
            primaryAirSystems.Splitter.BranchNumIn = 0;
            primaryAirSystems.Splitter.NodeNameIn = "";
            primaryAirSystems.Splitter.TotalOutletNodes = 0;
            primaryAirSystems.Splitter.NodeNumOut.allocate(0);
            primaryAirSystems.Splitter.BranchNumOut.allocate(0);
            primaryAirSystems.Splitter.NodeNameOut.allocate(0);
        }

        // If there is a MIXER, get its data
        if (MixerExists) {
            state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "Connector:Mixer", NumParams, NumAlphas, NumNodes);
            NodeNames.allocate(NumAlphas);
            NodeNumbers.allocate(NumAlphas);
            GetLoopMixer(state,
                         primaryAirSystems.Name,
                         ConnectorListName,
                         primaryAirSystems.Mixer.Name,
                         primaryAirSystems.Mixer.Exists,
                         primaryAirSystems.Mixer.NodeNameOut,
                         primaryAirSystems.Mixer.NodeNumOut,
                         primaryAirSystems.Mixer.TotalInletNodes,
                         NodeNames,
                         NodeNumbers,
                         ErrorsFound);

            primaryAirSystems.Mixer.NodeNameIn.allocate(primaryAirSystems.Mixer.TotalInletNodes);
            primaryAirSystems.Mixer.NodeNumIn.allocate(primaryAirSystems.Mixer.TotalInletNodes);
            primaryAirSystems.Mixer.BranchNumIn.allocate(primaryAirSystems.Mixer.TotalInletNodes);

            for (NodeNum = 1; NodeNum <= primaryAirSystems.Mixer.TotalInletNodes; ++NodeNum) {

                primaryAirSystems.Mixer.NodeNameIn(NodeNum) = NodeNames(NodeNum);
                primaryAirSystems.Mixer.NodeNumIn(NodeNum) = NodeNumbers(NodeNum);

                primaryAirSystems.Mixer.BranchNumIn(NodeNum) = 0;
                for (BranchNum = 1; BranchNum <= primaryAirSystems.NumBranches; ++BranchNum) {

                    if (primaryAirSystems.Branch(BranchNum).NodeNumOut == primaryAirSystems.Mixer.NodeNumIn(NodeNum)) {
                        primaryAirSystems.Mixer.BranchNumIn(NodeNum) = BranchNum;
                        break;
                    }
                }
            }

            primaryAirSystems.Mixer.BranchNumOut = 0;
            for (BranchNum = 1; BranchNum <= primaryAirSystems.NumBranches; ++BranchNum) {

                if (primaryAirSystems.Branch(BranchNum).NodeNumIn == primaryAirSystems.Mixer.NodeNumOut) {
                    primaryAirSystems.Mixer.BranchNumOut = BranchNum;
                    break;
                }
            }

            if (allocated(NodeNames)) {
                NodeNames.deallocate();
                NodeNumbers.deallocate();
            }

        } else {
            primaryAirSystems.Mixer.Exists = false;
            primaryAirSystems.Mixer.NodeNumOut = 0;
            primaryAirSystems.Mixer.BranchNumOut = 0;
            primaryAirSystems.Mixer.NodeNameOut = "";
            primaryAirSystems.Mixer.TotalInletNodes = 0;
            primaryAirSystems.Mixer.NodeNumIn.allocate(0);
            primaryAirSystems.Mixer.BranchNumIn.allocate(0);
            primaryAirSystems.Mixer.NodeNameIn.allocate(0);
        }

        NumControllers = 0;
        if (ControllerListName != std::string()) { // If not blank, then must be there and valid
            // Loop through the controller lists until you find the one attached to this primary air system
            ControllerListNum = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "AirLoopHVAC:ControllerList", ControllerListName);
            if (ControllerListNum > 0) {
                state.dataInputProcessing->inputProcessor->getObjectItem(
                    state, "AirLoopHVAC:ControllerList", ControllerListNum, Alphas, NumAlphas, Numbers, NumNumbers, IOStat);
                // Check the current controller list and if it matches input names
                NumControllers = (NumAlphas - 1) / 2; // Subtract off the controller list name first
                // store all the controller data
                primaryAirSystems.NumControllers = NumControllers + NumOASysSimpControllers;
                primaryAirSystems.ControllerName.allocate(NumControllers + NumOASysSimpControllers);
                primaryAirSystems.ControllerType.allocate(NumControllers + NumOASysSimpControllers);
                primaryAirSystems.ControllerIndex.allocate(NumControllers + NumOASysSimpControllers);
                primaryAirSystems.ControllerIndex = 0;
                primaryAirSystems.ControlConverged.allocate(NumControllers + NumOASysSimpControllers);
                primaryAirSystems.CanBeLockedOutByEcono.allocate(NumControllers + NumOASysSimpControllers);
                for (ControllerNum = NumOASysSimpControllers + 1; ControllerNum <= NumOASysSimpControllers + NumControllers; ++ControllerNum) {
                    ControllerName = Alphas((ControllerNum - NumOASysSimpControllers) * 2 + 1);
                    ControllerType = Alphas((ControllerNum - NumOASysSimpControllers) * 2);
                    primaryAirSystems.ControllerName(ControllerNum) = ControllerName;
                    primaryAirSystems.ControllerType(ControllerNum) = ControllerType;
                    IsNotOK = false;
                    ValidateComponent(state, ControllerType, ControllerName, IsNotOK, CurrentModuleObject);
                    if (IsNotOK) {
                        ShowContinueError(state,
                                          format("{}{}=\"{}\", for ControllerList=\"{}\".",
                                                 RoutineName,
                                                 CurrentModuleObject,
                                                 primaryAirSystems.Name,
                                                 ControllerListName));
                        ErrorsFound = true;
                    }
                    primaryAirSystems.ControlConverged(ControllerNum) = false;
                    primaryAirSystems.CanBeLockedOutByEcono(ControllerNum) = false;
                } // End of ControllerListNum Loop
            } else {
                ShowSevereError(state, format("{}{}=\"{}\", controller list object.", RoutineName, CurrentModuleObject, primaryAirSystems.Name));
                ShowContinueError(state, format("ControllerList object=\"{}\" not found in input.", ControllerListName));
                ErrorsFound = true;
            }
        }
        if (NumOASysSimpControllers > 0) {
            state.dataInputProcessing->inputProcessor->getObjectItem(
                state, "AirLoopHVAC:ControllerList", OASysContListNum, Alphas, NumAlphas, Numbers, NumNumbers, IOStat);
            // allocate air primary system controller lists if not already done
            if (NumControllers == 0) {
                primaryAirSystems.NumControllers = NumOASysSimpControllers;
                primaryAirSystems.ControllerName.allocate(NumOASysSimpControllers);
                primaryAirSystems.ControllerType.allocate(NumOASysSimpControllers);
                primaryAirSystems.ControllerIndex.allocate(NumOASysSimpControllers);
                primaryAirSystems.ControllerIndex = 0;
                primaryAirSystems.ControlConverged.allocate(NumOASysSimpControllers);
                primaryAirSystems.CanBeLockedOutByEcono.allocate(NumOASysSimpControllers);
                primaryAirSystems.ControlConverged = false;
                primaryAirSystems.CanBeLockedOutByEcono = false;
            }
            // loop over the OA Sys controllers and move them up to the primary air system controller lists
            OASysControllerNum = 0;
            NumOASysControllers = (NumAlphas - 1) / 2;
            for (ControllerNum = 1; ControllerNum <= NumOASysControllers; ++ControllerNum) {
                ControllerName = Alphas(ControllerNum * 2 + 1);
                ControllerType = Alphas(ControllerNum * 2);
                if (!Util::SameString(ControllerType, "Controller:OutdoorAir")) {
                    ++OASysControllerNum;
                    primaryAirSystems.ControllerName(OASysControllerNum) = ControllerName;
                    primaryAirSystems.ControllerType(OASysControllerNum) = ControllerType;
                    primaryAirSystems.ControlConverged(OASysControllerNum) = false;
                    primaryAirSystems.CanBeLockedOutByEcono(OASysControllerNum) = true;
                    GetControllerActuatorNodeNum(state, ControllerName, ActuatorNodeNum, errFlag);

                    bool nonLockoutCoilFound = false;
                    WaterCoilNodeNum = -1;
                    // added to fix bug issue #5695, if HW coil on outdoor air system, don't lock out during economizing
                    if (OANum > 0) {
                        for (int OACompNum = 1; OACompNum <= OutsideAirSys(OANum).NumComponents; ++OACompNum) {
                            CompType = OutsideAirSys(OANum).ComponentType(OACompNum);
                            if (Util::SameString(CompType, "Coil:Heating:Water")) {
                                WaterCoilNodeNum = GetCoilWaterInletNode(state, CompType, OutsideAirSys(OANum).ComponentName(OACompNum), ErrorsFound);
                                if (WaterCoilNodeNum == ActuatorNodeNum) {
                                    nonLockoutCoilFound = true;
                                }
                                break;
                            }
                        }
                    }
                    if (!nonLockoutCoilFound) {
                        //         Coil controllers can be entered either in the air loop controller list or the
                        //         OA system controller list. The CanBeLockedOutByEcono should only be set for OA coils
                        //         First get the OA controller actuator node and then compare to the air loop coil water inlet node
                        //         If these node numbers match, the coil is in the main air loop and the lockout flag should be reset to FALSE
                        for (BranchNum = 1; BranchNum <= primaryAirSystems.NumBranches; ++BranchNum) {
                            for (CompNum = 1; CompNum <= primaryAirSystems.Branch(BranchNum).TotalComponents; ++CompNum) {
                                if (Util::SameString(primaryAirSystems.Branch(BranchNum).Comp(CompNum).TypeOf, "AirloopHVAC:OutdoorAirSystem")) {
                                    continue;
                                }
                                CompType = primaryAirSystems.Branch(BranchNum).Comp(CompNum).TypeOf;
                                if (Util::SameString(CompType, "Coil:Cooling:Water:DetailedGeometry") ||
                                    Util::SameString(CompType, "Coil:Heating:Water") || Util::SameString(CompType, "Coil:Cooling:Water")) {
                                    WaterCoilNodeNum =
                                        GetCoilWaterInletNode(state, CompType, primaryAirSystems.Branch(BranchNum).Comp(CompNum).Name, ErrorsFound);
                                    if (WaterCoilNodeNum == ActuatorNodeNum) {
                                        nonLockoutCoilFound = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if (nonLockoutCoilFound) {
                        primaryAirSystems.CanBeLockedOutByEcono(OASysControllerNum) = false;
                    }
                }
            }
        }
        if (NumControllers + NumOASysSimpControllers == 0) {
            if (!PackagedUnit(AirSysNum)) {
                ShowWarningError(state, format("{}{}=\"{}\" has no Controllers.", RoutineName, CurrentModuleObject, primaryAirSystems.Name));
            }
            primaryAirSystems.NumControllers = 0;
            primaryAirSystems.ControllerName.allocate(0);
            primaryAirSystems.ControllerType.allocate(0);
            primaryAirSystems.ControlConverged.allocate(0);
            primaryAirSystems.CanBeLockedOutByEcono.allocate(0);
        }

        errFlag = false;
        Avail::GetAirLoopAvailabilityManager(state, AvailManagerListName, AirSysNum, NumPrimaryAirSys, errFlag);

        if (errFlag) {
            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, primaryAirSystems.Name));
            ErrorsFound = true;
        }

    } // End Air Loop

    Numbers.deallocate();
    cNumericFields.deallocate();
    lNumericBlanks.deallocate();
    Alphas.deallocate();
    cAlphaFields.deallocate();
    lAlphaBlanks.deallocate();

    TestUniqueNodes.deallocate();
    for (AirSysNum = 1; AirSysNum <= NumPrimaryAirSys; ++AirSysNum) {
        auto &primaryAirSystems = state.dataAirSystemsData->PrimaryAirSystems(AirSysNum);
        for (BranchNum = 1; BranchNum <= primaryAirSystems.NumBranches; ++BranchNum) {
            for (CompNum = 1; CompNum <= primaryAirSystems.Branch(BranchNum).TotalComponents; ++CompNum) {

                {
                    std::string const componentType = uppercased(primaryAirSystems.Branch(BranchNum).Comp(CompNum).TypeOf);

                    if (componentType == "AIRLOOPHVAC:OUTDOORAIRSYSTEM") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::OAMixer_Num;

                        // Fan Types for the air sys simulation
                    } else if (componentType == "FAN:CONSTANTVOLUME") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Fan_Simple_CV;

                    } else if (componentType == "FAN:VARIABLEVOLUME") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Fan_Simple_VAV;

                    } else if (componentType == "FAN:SYSTEMMODEL") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Fan_System_Object;
                        auto &comp = primaryAirSystems.Branch(BranchNum).Comp(CompNum);
                        if (comp.CompIndex == 0) {
                            comp.CompIndex = Fans::GetFanIndex(state, comp.Name); // TODO: get rid of this
                            if (comp.CompIndex == 0) {
                                ShowSevereError(state, format("Component {} of type {} not found.", comp.Name, comp.TypeOf));
                            }
                        }

                        state.dataFans->fans(comp.CompIndex)->airPathFlag = true;
                    } else if (componentType == "FAN:COMPONENTMODEL") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Fan_ComponentModel;

                        // Coil Types for the air sys simulation
                        //        HX Assisted coils are not allowed on a branch at this time
                        //        CASE('COILSYSTEM:COOLING:DX:HEATEXCHANGERASSISTED')
                        //          PrimaryAirSystem(AirSysNum)%Branch(BranchNum)%Comp(CompNum)%CompType_Num=DXCoil_CoolingHXAsst
                    } else if (componentType == "COILSYSTEM:COOLING:WATER:HEATEXCHANGERASSISTED") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::WaterCoil_CoolingHXAsst;
                    } else if (componentType == "COIL:HEATING:WATER") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::WaterCoil_SimpleHeat;
                    } else if (componentType == "COIL:HEATING:STEAM") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::SteamCoil_AirHeat;
                    } else if (componentType == "COIL:COOLING:WATER:DETAILEDGEOMETRY") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::WaterCoil_DetailedCool;
                    } else if (componentType == "COIL:COOLING:WATER") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::WaterCoil_Cooling;
                    } else if (componentType == "COIL:HEATING:ELECTRIC") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Coil_ElectricHeat;
                    } else if (componentType == "COIL:HEATING:FUEL") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Coil_GasHeat;

                        // Heat reclaim
                    } else if (componentType == "COIL:HEATING:DESUPERHEATER") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Coil_DeSuperHeat;

                    } else if (componentType == "COILSYSTEM:COOLING:DX") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::DXSystem;
                    } else if (componentType == "COILSYSTEM:HEATING:DX") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::DXHeatPumpSystem;
                    } else if (componentType == "COIL:USERDEFINED") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::CoilUserDefined;
                    } else if (componentType == "AIRLOOPHVAC:UNITARYSYSTEM") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::UnitarySystemModel;
                        UnitarySystems::UnitarySys thisSys;
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).compPointer = thisSys.factory(
                            state, HVAC::UnitarySysType::Unitary_AnyCoilType, primaryAirSystems.Branch(BranchNum).Comp(CompNum).Name, false, 0);
                    } else if (componentType == "COILSYSTEM:COOLING:WATER") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::CoilSystemWater;
                        UnitarySystems::UnitarySys thisSys;
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).compPointer = thisSys.factory(
                            state, HVAC::UnitarySysType::Unitary_AnyCoilType, primaryAirSystems.Branch(BranchNum).Comp(CompNum).Name, false, 0);
                    } else if (componentType == "AIRLOOPHVAC:UNITARY:FURNACE:HEATONLY") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Furnace_UnitarySys_HeatOnly;
                    } else if (componentType == "AIRLOOPHVAC:UNITARY:FURNACE:HEATCOOL") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Furnace_UnitarySys_HeatCool;
                    } else if (componentType == "AIRLOOPHVAC:UNITARYHEATONLY") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Furnace_UnitarySys_HeatOnly;
                    } else if (componentType == "AIRLOOPHVAC:UNITARYHEATCOOL") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Furnace_UnitarySys_HeatCool;
                    } else if (componentType == "AIRLOOPHVAC:UNITARYHEATPUMP:AIRTOAIR") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Furnace_UnitarySys_HeatCool;
                    } else if (componentType == "AIRLOOPHVAC:UNITARYHEATPUMP:WATERTOAIR") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Furnace_UnitarySys_HeatCool;

                    } else if (componentType == "AIRLOOPHVAC:UNITARYHEATCOOL:VAVCHANGEOVERBYPASS") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::UnitarySystem_BypassVAVSys;

                        // Humidifier Types for the air system simulation
                    } else if (componentType == "HUMIDIFIER:STEAM:ELECTRIC") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Humidifier;

                    } else if (componentType == "HUMIDIFIER:STEAM:GAS") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Humidifier;

                        // Evap Cooler Types for the air system simulation
                    } else if (componentType == "EVAPORATIVECOOLER:DIRECT:CELDEKPAD") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::EvapCooler;
                    } else if (componentType == "EVAPORATIVECOOLER:INDIRECT:CELDEKPAD") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::EvapCooler;
                    } else if (componentType == "EVAPORATIVECOOLER:INDIRECT:WETCOIL") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::EvapCooler;
                    } else if (componentType == "EVAPORATIVECOOLER:INDIRECT:RESEARCHSPECIAL") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::EvapCooler;
                    } else if (componentType == "EVAPORATIVECOOLER:DIRECT:RESEARCHSPECIAL") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::EvapCooler;

                        // Desiccant Dehumidifier Types for the air system simulation
                    } else if (componentType == "DEHUMIDIFIER:DESICCANT:NOFANS") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Desiccant;
                    } else if (componentType == "DEHUMIDIFIER:DESICCANT:SYSTEM") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Desiccant;

                        // Heat recovery
                    } else if (componentType == "HEATEXCHANGER:AIRTOAIR:FLATPLATE") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::HeatXchngr;

                    } else if (componentType == "HEATEXCHANGER:AIRTOAIR:SENSIBLEANDLATENT") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::HeatXchngr;

                    } else if (componentType == "HEATEXCHANGER:DESICCANT:BALANCEDFLOW") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::HeatXchngr;

                        // Ducts
                    } else if (componentType == "DUCT") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::Duct;

                    } else if (componentType == "AIRLOOPHVAC:UNITARYHEATPUMP:AIRTOAIR:MULTISPEED") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::UnitarySystem_MSHeatPump;

                    } else if (componentType == "ZONEHVAC:TERMINALUNIT:VARIABLEREFRIGERANTFLOW") {
                        primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num = CompType::ZoneVRFasAirLoopEquip;

                    } else if (componentType == "FAN:ONOFF" || componentType == "COIL:COOLING:DX:SINGLESPEED" ||
                               componentType == "COIL:HEATING:DX:SINGLESPEED" || componentType == "COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE" ||
                               componentType == "COIL:COOLING:DX:MULTISPEED" || componentType == "COIL:HEATING:DX:MULTISPEED") {
                        ShowSevereError(state, format("{}{} = \"{}\".", RoutineName, CurrentModuleObject, primaryAirSystems.Name));
                        ShowContinueError(
                            state, format("..Invalid Air Loop Component Type = \"{}\".", primaryAirSystems.Branch(BranchNum).Comp(CompNum).TypeOf));
                        ShowContinueError(state,
                                          format("..Air Loop Component Name = \"{}\".", primaryAirSystems.Branch(BranchNum).Comp(CompNum).Name));
                        ShowContinueError(state, format("..reference Branch = \"{}\".", primaryAirSystems.Branch(BranchNum).Name));
                        ShowContinueError(state,
                                          "...This component may only be referenced by a parent component such as "
                                          "AirLoopHVAC:Unitary:Furnace:HeatCool or similar.");
                        ErrorsFound = true;

                    } else {
                        ShowSevereError(state, format("{}{} = \"{}\".", RoutineName, CurrentModuleObject, primaryAirSystems.Name));
                        ShowContinueError(
                            state, format("..Invalid Air Loop Component Type = \"{}\".", primaryAirSystems.Branch(BranchNum).Comp(CompNum).TypeOf));
                        ShowContinueError(state,
                                          format("..Air Loop Component Name = \"{}\".", primaryAirSystems.Branch(BranchNum).Comp(CompNum).Name));
                        ShowContinueError(state, format("..reference Branch = \"{}\".", primaryAirSystems.Branch(BranchNum).Name));
                        ErrorsFound = true;
                    }
                }
            }
        }
    }

    // check that actuator nodes are matched by a water coil inlet node

    for (AirSysNum = 1; AirSysNum <= NumPrimaryAirSys; ++AirSysNum) {
        auto &primaryAirSystems = state.dataAirSystemsData->PrimaryAirSystems(AirSysNum);
        for (BranchNum = 1; BranchNum <= primaryAirSystems.NumBranches; ++BranchNum) {
            for (CompNum = 1; CompNum <= primaryAirSystems.Branch(BranchNum).TotalComponents; ++CompNum) {
                CompType_Num = primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num;
                if (CompType_Num == CompType::WaterCoil_DetailedCool || CompType_Num == CompType::WaterCoil_SimpleHeat ||
                    CompType_Num == CompType::WaterCoil_Cooling) {
                    WaterCoilNodeNum = GetCoilWaterInletNode(state,
                                                             primaryAirSystems.Branch(BranchNum).Comp(CompNum).TypeOf,
                                                             primaryAirSystems.Branch(BranchNum).Comp(CompNum).Name,
                                                             ErrorsFound);
                    CheckCoilWaterInletNode(state, WaterCoilNodeNum, NodeNotFound);
                    if (NodeNotFound) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}{}=\"{}\", invalid actuator.",
                                               RoutineName,
                                               CurrentModuleObject,
                                               primaryAirSystems.Branch(BranchNum).Comp(CompNum).Name));
                        ShowContinueError(state,
                                          "...this coil requires a water coil controller and the inlet node of a water coil must also be an "
                                          "actuator node of a water coil controller.");
                    }
                }
            }
        }
    }

    OANum = GetNumOASystems(state);
    for (int OASysNum = 1; OASysNum <= OANum; ++OASysNum) {
        int NumInList = GetOACompListNumber(state, OASysNum);
        for (int OACompNum = 1; OACompNum <= NumInList; ++OACompNum) {
            CompType_Num = GetOACompTypeNum(state, OASysNum, OACompNum);
            if (CompType_Num == CompType::WaterCoil_DetailedCool || CompType_Num == CompType::WaterCoil_SimpleHeat ||
                CompType_Num == CompType::WaterCoil_Cooling) {
                WaterCoilNodeNum =
                    GetCoilWaterInletNode(state, GetOACompType(state, OASysNum, OACompNum), GetOACompName(state, OASysNum, OACompNum), ErrorsFound);
                CheckCoilWaterInletNode(state, WaterCoilNodeNum, NodeNotFound);
                UnitarySystems::isWaterCoilHeatRecoveryType(state, WaterCoilNodeNum, NodeNotFound);
                if (NodeNotFound) {
                    ErrorsFound = true;
                    ShowSevereError(
                        state, format("{}{}=\"{}\", invalid actuator.", RoutineName, CurrentModuleObject, GetOACompName(state, OASysNum, OACompNum)));
                    ShowContinueError(state,
                                      "...this coil requires a water coil controller and the inlet node of a water coil must also be an actuator "
                                      "node of a water coil controller.");
                }
            }
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, format("{}Errors found retrieving input for {}.", RoutineName, CurrentModuleObject));
    }

    for (AirSysNum = 1; AirSysNum <= NumPrimaryAirSys; ++AirSysNum) {
        SetupOutputVariable(state,
                            "Air System Simulation Cycle On Off Status",
                            Constant::Units::None,
                            state.dataAirLoop->PriAirSysAvailMgr(AirSysNum).availStatus,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            state.dataAirSystemsData->PrimaryAirSystems(AirSysNum).Name);
    }

    if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirLoopHVAC:DedicatedOutdoorAirSystem") > 0) {
        if (state.dataAirLoopHVACDOAS->GetInputOnceFlag) {
            AirLoopHVACDOAS::getAirLoopHVACDOASInput(state);
            state.dataAirLoopHVACDOAS->GetInputOnceFlag = false;
        }
    }
}

// End of Get Input subroutines for the Module
//******************************************************************************

// Beginning Initialization Section of the Module
//******************************************************************************

void InitAirLoops(EnergyPlusData &state, bool const FirstHVACIteration) // TRUE if first full HVAC iteration in an HVAC timestep
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   April 1998
    //       MODIFIED       Dec 1999 Fred Buhl

    // PURPOSE OF THIS SUBROUTINE:
    // Initializes the primary air system simulation

    // METHODOLOGY EMPLOYED:
    // (1) For the first simulation in an HVAC timestep, the air system is initialized to
    //     design air flow rates.
    // (2) For subsequent simulations, air flow data is set by the zone equipment inlet
    //     nodes and the return air node.
    // (3) Other air system node data such as temperatures and humidity ratios are only
    //     initialized at the start of an environment (run period or design day).

    int const numPrimaryAirSys = state.dataHVACGlobal->NumPrimaryAirSys;

    bool ErrorsFound = false;
    state.dataHVACGlobal->AirLoopInit = true;

    // Do the one time initializations
    if (state.dataSimAirServingZones->InitAirLoopsOneTimeFlag) {

        // Figure out what zones are served by each primary air system (air loop) and
        // store the results in AirToZoneNodeInfo()%CoolCtrlZoneNums and AirToZoneNodeInfo()%HeatCtrlZoneNums

        // Temporary air loop zone data
        struct AirloopZone
        {
            int ctrlZoneNum = 0;         // Controlled zone num
            int zoneInletNode = 0;       // Zone supply inlet node
            int termUnitInletNode = 0;   // Terminal unit inlet node
            int termUnitSizingIndex = 0; // Terminal unit sizing index
        };

        EPVector<AirloopZone> cooledZone;
        EPVector<AirloopZone> heatedZone;
        size_t atuArraySize = max(static_cast<size_t>(state.dataGlobal->NumOfZones), state.dataDefineEquipment->AirDistUnit.size());
        cooledZone.allocate(atuArraySize);
        heatedZone.allocate(atuArraySize);

        state.dataSimAirServingZones->MassFlowSetToler = DataConvergParams::HVACFlowRateToler * 0.00001;

        for (int SupAirPath = 1; SupAirPath <= state.dataZoneEquip->NumSupplyAirPaths; ++SupAirPath) {

            int NumAllSupAirPathNodes = 0;
            int SupAirPathNodeNum = 0;
            int SupAirPathOutNodeNum = 0;
            int NumSupAirPathOutNodes = 0;
            int NumSupAirPathNodes = 0;
            int NumSupAirPathIntNodes = 0;

            // each supply air path may have up to one splitter and one plenum.  Check for all combinations count
            // all nodes (including duplicates)
            for (int CompNum = 1; CompNum <= state.dataZoneEquip->SupplyAirPath(SupAirPath).NumOfComponents; ++CompNum) {
                if (Util::SameString(state.dataZoneEquip->SupplyAirPath(SupAirPath).ComponentType(CompNum), "AirLoopHVAC:ZoneSplitter")) {
                    int SplitterNum = Util::FindItemInList(state.dataZoneEquip->SupplyAirPath(SupAirPath).ComponentName(CompNum),
                                                           state.dataSplitterComponent->SplitterCond,
                                                           &SplitterComponent::SplitterConditions::SplitterName);
                    if (SplitterNum == 0) {
                        ShowSevereError(
                            state,
                            format("AirLoopHVAC:ZoneSplitter not found={}", state.dataZoneEquip->SupplyAirPath(SupAirPath).ComponentName(CompNum)));
                        ShowContinueError(state, format("Occurs in AirLoopHVAC:SupplyPath={}", state.dataZoneEquip->SupplyAirPath(SupAirPath).Name));
                        ErrorsFound = true;
                    }
                    state.dataZoneEquip->SupplyAirPath(SupAirPath).SplitterIndex(CompNum) = SplitterNum;
                    NumAllSupAirPathNodes += state.dataSplitterComponent->SplitterCond(SplitterNum).NumOutletNodes + 1;
                } else if (Util::SameString(state.dataZoneEquip->SupplyAirPath(SupAirPath).ComponentType(CompNum), "AirLoopHVAC:SupplyPlenum")) {
                    int PlenumNum = Util::FindItemInList(state.dataZoneEquip->SupplyAirPath(SupAirPath).ComponentName(CompNum),
                                                         state.dataZonePlenum->ZoneSupPlenCond,
                                                         &ZonePlenum::ZoneSupplyPlenumConditions::ZonePlenumName);
                    if (PlenumNum == 0) {
                        ShowSevereError(
                            state,
                            format("AirLoopHVAC:SupplyPlenum not found={}", state.dataZoneEquip->SupplyAirPath(SupAirPath).ComponentName(CompNum)));
                        ShowContinueError(state, format("Occurs in AirLoopHVAC:SupplyPath={}", state.dataZoneEquip->SupplyAirPath(SupAirPath).Name));
                        ErrorsFound = true;
                    }
                    state.dataZoneEquip->SupplyAirPath(SupAirPath).PlenumIndex(CompNum) = PlenumNum;
                    NumAllSupAirPathNodes += state.dataZonePlenum->ZoneSupPlenCond(PlenumNum).NumOutletNodes + 1;
                }
            }
            EPVector<int> supNode;
            EPVector<DataZoneEquipment::AirNodeType> supNodeType;
            EPVector<int> supNodeCompNum;
            supNode.allocate(NumAllSupAirPathNodes);
            supNodeType.allocate(NumAllSupAirPathNodes);
            supNodeCompNum.allocate(NumAllSupAirPathNodes);

            // figure out the order of the splitter and plenum in the path, by flagging the first node of the component
            // as either a 'pathinlet' or a 'compinlet'
            for (int CompNum = 1; CompNum <= state.dataZoneEquip->SupplyAirPath(SupAirPath).NumOfComponents; ++CompNum) {
                int SplitterNum = state.dataZoneEquip->SupplyAirPath(SupAirPath).SplitterIndex(CompNum);
                int PlenumNum = state.dataZoneEquip->SupplyAirPath(SupAirPath).PlenumIndex(CompNum);
                if (SplitterNum > 0) {
                    ++SupAirPathNodeNum;
                    supNode(SupAirPathNodeNum) = state.dataSplitterComponent->SplitterCond(SplitterNum).InletNode;
                    supNodeCompNum(SupAirPathNodeNum) = CompNum;
                    if (CompNum == 1) {
                        supNodeType(SupAirPathNodeNum) = DataZoneEquipment::AirNodeType::PathInlet;
                    } else {
                        supNodeType(SupAirPathNodeNum) = DataZoneEquipment::AirNodeType::CompInlet;
                    }
                    for (int SplitterOutNum = 1; SplitterOutNum <= state.dataSplitterComponent->SplitterCond(SplitterNum).NumOutletNodes;
                         ++SplitterOutNum) {
                        ++SupAirPathNodeNum;
                        supNode(SupAirPathNodeNum) = state.dataSplitterComponent->SplitterCond(SplitterNum).OutletNode(SplitterOutNum);
                        supNodeCompNum(SupAirPathNodeNum) = CompNum;
                        supNodeType(SupAirPathNodeNum) = DataZoneEquipment::AirNodeType::Invalid;
                    }
                } else if (PlenumNum > 0) {
                    ++SupAirPathNodeNum;
                    supNode(SupAirPathNodeNum) = state.dataZonePlenum->ZoneSupPlenCond(PlenumNum).InletNode;
                    supNodeCompNum(SupAirPathNodeNum) = CompNum;
                    if (CompNum == 1) {
                        supNodeType(SupAirPathNodeNum) = DataZoneEquipment::AirNodeType::PathInlet;
                    } else {
                        supNodeType(SupAirPathNodeNum) = DataZoneEquipment::AirNodeType::CompInlet;
                    }
                    for (int PlenumOutNum = 1; PlenumOutNum <= state.dataZonePlenum->ZoneSupPlenCond(PlenumNum).NumOutletNodes; ++PlenumOutNum) {
                        ++SupAirPathNodeNum;
                        supNode(SupAirPathNodeNum) = state.dataZonePlenum->ZoneSupPlenCond(PlenumNum).OutletNode(PlenumOutNum);
                        supNodeCompNum(SupAirPathNodeNum) = CompNum;
                        supNodeType(SupAirPathNodeNum) = DataZoneEquipment::AirNodeType::Invalid;
                    }
                }
            }

            // find the nodes that connect a splitter and a plenum
            for (int SupNodeIndex = 1; SupNodeIndex <= NumAllSupAirPathNodes; ++SupNodeIndex) {
                if (supNodeType(SupNodeIndex) == DataZoneEquipment::AirNodeType::Invalid) {
                    for (int SupNodeIndex2 = SupNodeIndex + 1; SupNodeIndex2 <= NumAllSupAirPathNodes; ++SupNodeIndex2) {
                        if ((supNode(SupNodeIndex) == supNode(SupNodeIndex2)) &&
                            (supNodeType(SupNodeIndex2) == DataZoneEquipment::AirNodeType::CompInlet)) {
                            supNodeType(SupNodeIndex) = DataZoneEquipment::AirNodeType::Intermediate;
                            break;
                        }
                    }
                }
            }

            //  the rest of the nodes are outlet nodes and count the duplicated intermediate nodes
            for (int SupNodeIndex = 1; SupNodeIndex <= NumAllSupAirPathNodes; ++SupNodeIndex) {
                if (supNodeType(SupNodeIndex) == DataZoneEquipment::AirNodeType::Invalid) {
                    ++NumSupAirPathOutNodes;
                    supNodeType(SupNodeIndex) = DataZoneEquipment::AirNodeType::Outlet;
                } else if (supNodeType(SupNodeIndex) == DataZoneEquipment::AirNodeType::Intermediate) {
                    ++NumSupAirPathIntNodes;
                }
            }

            //  eliminate the duplicates to find the number of nodes in the supply air path
            NumSupAirPathNodes = NumAllSupAirPathNodes - NumSupAirPathIntNodes;
            SupAirPathNodeNum = 0;

            state.dataZoneEquip->SupplyAirPath(SupAirPath).OutletNode.allocate(NumSupAirPathOutNodes);
            state.dataZoneEquip->SupplyAirPath(SupAirPath).OutletNodeSupplyPathCompNum.allocate(NumSupAirPathOutNodes);
            state.dataZoneEquip->SupplyAirPath(SupAirPath).Node.allocate(NumSupAirPathNodes);
            state.dataZoneEquip->SupplyAirPath(SupAirPath).NodeType.allocate(NumSupAirPathNodes);
            state.dataZoneEquip->SupplyAirPath(SupAirPath).NumNodes = NumSupAirPathNodes;
            state.dataZoneEquip->SupplyAirPath(SupAirPath).NumOutletNodes = NumSupAirPathOutNodes;

            // transfer data from the local SupNode array to the SupplyAirPath data structure
            for (int SupNodeIndex = 1; SupNodeIndex <= NumAllSupAirPathNodes; ++SupNodeIndex) {
                if (supNodeType(SupNodeIndex) == DataZoneEquipment::AirNodeType::PathInlet ||
                    supNodeType(SupNodeIndex) == DataZoneEquipment::AirNodeType::Intermediate ||
                    supNodeType(SupNodeIndex) == DataZoneEquipment::AirNodeType::Outlet) {
                    ++SupAirPathNodeNum;
                    // map the local node numbers to the HVAC (global) node numbers
                    state.dataZoneEquip->SupplyAirPath(SupAirPath).Node(SupAirPathNodeNum) = supNode(SupNodeIndex);
                    state.dataZoneEquip->SupplyAirPath(SupAirPath).NodeType(SupAirPathNodeNum) = supNodeType(SupNodeIndex);
                }
                if (supNodeType(SupNodeIndex) == DataZoneEquipment::AirNodeType::Outlet) {
                    ++SupAirPathOutNodeNum;
                    // map the outlet node number to the HVAC (global) node number
                    state.dataZoneEquip->SupplyAirPath(SupAirPath).OutletNode(SupAirPathOutNodeNum) = supNode(SupNodeIndex);
                    state.dataZoneEquip->SupplyAirPath(SupAirPath).OutletNodeSupplyPathCompNum(SupAirPathOutNodeNum) = supNodeCompNum(SupNodeIndex);
                }
            }
        }

        // Now loop over the air loops
        for (int AirLoopNum = 1; AirLoopNum <= numPrimaryAirSys; ++AirLoopNum) {
            auto &thisPrimaryAirSys = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum);
            auto &thisAirLoopControlInfo = state.dataAirLoop->AirLoopControlInfo(AirLoopNum);
            for (size_t num = 1; num <= atuArraySize; ++num) {
                cooledZone(num).ctrlZoneNum = 0;
                heatedZone(num).ctrlZoneNum = 0;
                cooledZone(num).zoneInletNode = 0;
                heatedZone(num).zoneInletNode = 0;
                cooledZone(num).termUnitInletNode = 0;
                heatedZone(num).termUnitInletNode = 0;
                cooledZone(num).termUnitSizingIndex = 0;
                heatedZone(num).termUnitSizingIndex = 0;
            }
            int NumZonesCool = 0;
            int NumZonesHeat = 0;
            int NumComponentsInSys = 0;

            // count the number of components in this primary air system
            for (int BranchNum = 1; BranchNum <= thisPrimaryAirSys.NumBranches; ++BranchNum) {
                NumComponentsInSys += thisPrimaryAirSys.Branch(BranchNum).TotalComponents;
            }
            // set the Simple flag
            if (thisPrimaryAirSys.NumBranches == 1 && NumComponentsInSys == 1) {
                thisAirLoopControlInfo.Simple = true;
            }

            auto &thisAirToZoneNodeInfo = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum);
            // loop over the air loop's output nodes
            for (int OutNum = 1; OutNum <= thisAirToZoneNodeInfo.NumSupplyNodes; ++OutNum) {
                int ZoneSideNodeNum = thisAirToZoneNodeInfo.ZoneEquipSupplyNodeNum(OutNum);
                // find the corresponding branch number
                int OutBranchNum = thisPrimaryAirSys.OutletBranchNum[OutNum - 1];
                thisAirToZoneNodeInfo.SupplyDuctBranchNum(OutNum) = OutBranchNum;

                // find the supply air path corresponding to each air loop outlet node
                int SupAirPathNum = 0;
                // loop over the air loop's output nodes
                for (int SupAirPath = 1; SupAirPath <= state.dataZoneEquip->NumSupplyAirPaths; ++SupAirPath) {
                    if (ZoneSideNodeNum == state.dataZoneEquip->SupplyAirPath(SupAirPath).InletNodeNum) {
                        SupAirPathNum = SupAirPath;
                        break;
                    }
                }
                int NumSupAirPathOutNodes = 0;
                thisAirToZoneNodeInfo.SupplyAirPathNum(OutNum) = SupAirPathNum;
                if (SupAirPathNum > 0) {
                    NumSupAirPathOutNodes = state.dataZoneEquip->SupplyAirPath(SupAirPathNum).NumOutletNodes;
                }

                // Now Loop over the Supply Air Path outlet nodes and find out which zone and which air terminal
                // unit on that zone is connected to that supply air path.

                for (int SupAirPathOutNodeNum = 1; SupAirPathOutNodeNum <= NumSupAirPathOutNodes; ++SupAirPathOutNodeNum) {
                    int FoundSupPathZoneConnect = false;
                    // loop over all controlled zones.
                    for (int CtrlZoneNum = 1; CtrlZoneNum <= state.dataGlobal->NumOfZones; ++CtrlZoneNum) {
                        if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).IsControlled) {
                            continue;
                        }
                        // Loop over the air distribution unit inlets for each controlled zone.
                        // Look for a match between the zone splitter outlet node and the air distribution unit inlet node.
                        // When match found save the controlled zone number in CtrlZoneNumsCool or CtrlZoneNumsHeat
                        for (int ZoneInNum = 1; ZoneInNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).NumInletNodes; ++ZoneInNum) {

                            // BEGIN COOLING: Check for a match between the cooling air distribution unit inlet
                            // and the supply air path outlet
                            if (state.dataZoneEquip->SupplyAirPath(SupAirPathNum).OutletNode(SupAirPathOutNodeNum) ==
                                state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInNum).InNode) {
                                ++NumZonesCool;
                                // Set Duct Type for branch for dual duct
                                if (NumZonesCool == 1 && OutBranchNum > 1) {
                                    thisPrimaryAirSys.Branch(OutBranchNum).DuctType = HVAC::AirDuctType::Cooling;
                                }
                                if (NumZonesCool == 1) {
                                    thisAirToZoneNodeInfo.SupplyDuctType(OutNum) = HVAC::AirDuctType::Cooling;
                                }
                                cooledZone(NumZonesCool).ctrlZoneNum = CtrlZoneNum;
                                cooledZone(NumZonesCool).zoneInletNode = state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).InletNode(ZoneInNum);
                                cooledZone(NumZonesCool).termUnitInletNode =
                                    state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInNum).InNode;
                                cooledZone(NumZonesCool).termUnitSizingIndex =
                                    state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInNum).TermUnitSizingIndex;
                                if (AirLoopNum > 0) {
                                    if (thisPrimaryAirSys.OASysExists) {
                                        state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).ZoneHasAirLoopWithOASys = true;
                                    }
                                }
                                state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).InletNodeAirLoopNum(ZoneInNum) = AirLoopNum;
                                FoundSupPathZoneConnect = true;

                                // set the supply air path
                                state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInNum).SupplyAirPathExists = true;

                                // Once a match is found between a supply air path outlet node and an air distribution inlet
                                // node, we go on to the next supply air path outlet.  Therefore, *both* the air distribution
                                // unit loop and the controlled zone loop may be exited.
                                goto ControlledZoneLoop_exit;
                            } // end check for cooling air distribution units

                            // END COOLING: end check for match between supply air path outlet and cooling air
                            // distribution inlet

                            // BEGIN HEATING: If we don't get a match, check for a heating match
                            if (state.dataZoneEquip->SupplyAirPath(SupAirPathNum).OutletNode(SupAirPathOutNodeNum) ==
                                state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInNum).InNode) {
                                ++NumZonesHeat;
                                // Set Duct Type for branch for dual duct
                                if (NumZonesHeat == 1 && OutBranchNum > 1) {
                                    thisPrimaryAirSys.Branch(OutBranchNum).DuctType = HVAC::AirDuctType::Heating;
                                }
                                if (NumZonesHeat == 1) {
                                    thisAirToZoneNodeInfo.SupplyDuctType(OutNum) = HVAC::AirDuctType::Heating;
                                }
                                heatedZone(NumZonesHeat).ctrlZoneNum = CtrlZoneNum;
                                heatedZone(NumZonesHeat).zoneInletNode = state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).InletNode(ZoneInNum);
                                heatedZone(NumZonesHeat).termUnitInletNode =
                                    state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInNum).InNode;
                                heatedZone(NumZonesHeat).termUnitSizingIndex =
                                    state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInNum).TermUnitSizingIndex;
                                if (state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).InletNodeAirLoopNum(ZoneInNum) == 0) {
                                    state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).InletNodeAirLoopNum(ZoneInNum) = AirLoopNum;
                                }
                                FoundSupPathZoneConnect = true;

                                // Set the supply air path flag
                                state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInNum).SupplyAirPathExists = true;

                                // Once a match is found between a supply air path outlet node and an air distribution inlet
                                // node, we go on to the next supply air path outlet.  Therefore, *both* the air distribution
                                // unit loop and the controlled zone loop may be exited.
                                goto ControlledZoneLoop_exit;
                            } // end check for heatingair distribution units
                        }
                    }
                ControlledZoneLoop_exit:;

                    // If the supply air path is not connected to either a heating or a cooling air distribution
                    // unit...we have a problem!
                    if (!FoundSupPathZoneConnect) {
                        ShowSevereError(
                            state,
                            format("Node {} connects to no component",
                                   state.dataLoopNodes->NodeID(state.dataZoneEquip->SupplyAirPath(SupAirPathNum).OutletNode(SupAirPathOutNodeNum))));
                        ShowContinueError(state, format("Occurs in Supply Air Path={}", state.dataZoneEquip->SupplyAirPath(SupAirPathNum).Name));
                        ShowContinueError(state, "Check the connection to a ZoneHVAC:EquipmentConnections object");
                        ShowContinueError(state, "Check if this component is missing from the Supply Air Path");
                        ErrorsFound = true;
                    }
                }

                // What if there is no supply air path & the air loop outlet is just hooked directly to
                // an air distribution unit of a single zone? In this case look for a match between
                // ZoneSideNodeNum and a zone's air distribution unit inlets.
                if (SupAirPathNum == 0) {

                    for (int CtrlZoneNum = 1; CtrlZoneNum <= state.dataGlobal->NumOfZones; ++CtrlZoneNum) {
                        if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).IsControlled) {
                            continue;
                        }
                        // Loop over the air distribution unit inlets for each controlled zone.
                        // Look for a match between the zone equip inlet node and the air distribution unit inlet node.
                        // When match found save the controlled zone number in CtrlZoneNumsCool or CtrlZoneNumsHeat
                        for (int ZoneInNum = 1; ZoneInNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).NumInletNodes; ++ZoneInNum) {

                            // set supply air path flag
                            state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInNum).SupplyAirPathExists = false;

                            if (ZoneSideNodeNum == state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInNum).InNode) {
                                ++NumZonesCool;
                                // Set Duct Type for branch for dual duct
                                if (NumZonesCool == 1 && OutBranchNum > 1) {
                                    thisPrimaryAirSys.Branch(OutBranchNum).DuctType = HVAC::AirDuctType::Cooling;
                                }
                                cooledZone(NumZonesCool).ctrlZoneNum = CtrlZoneNum;
                                cooledZone(NumZonesCool).zoneInletNode = state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).InletNode(ZoneInNum);
                                cooledZone(NumZonesCool).termUnitInletNode =
                                    state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInNum).InNode;
                                cooledZone(NumZonesCool).termUnitSizingIndex =
                                    state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInNum).TermUnitSizingIndex;
                                if (state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).InletNodeAirLoopNum(ZoneInNum) == 0) {
                                    state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).InletNodeAirLoopNum(ZoneInNum) = AirLoopNum;
                                }
                                goto ControlledZoneLoop2_exit;
                            }

                            if (ZoneSideNodeNum == state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInNum).InNode) {
                                ++NumZonesHeat;
                                // Set Duct Type for branch for dual duct
                                if (NumZonesHeat == 1 && OutBranchNum > 1) {
                                    thisPrimaryAirSys.Branch(OutBranchNum).DuctType = HVAC::AirDuctType::Heating;
                                }
                                heatedZone(NumZonesHeat).ctrlZoneNum = CtrlZoneNum;
                                heatedZone(NumZonesHeat).zoneInletNode = state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).InletNode(ZoneInNum);
                                heatedZone(NumZonesHeat).termUnitInletNode =
                                    state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInNum).InNode;
                                heatedZone(NumZonesHeat).termUnitSizingIndex =
                                    state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInNum).TermUnitSizingIndex;
                                if (state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).InletNodeAirLoopNum(ZoneInNum) == 0) {
                                    state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).InletNodeAirLoopNum(ZoneInNum) = AirLoopNum;
                                }
                                goto ControlledZoneLoop2_exit;
                            }
                        }
                    }
                ControlledZoneLoop2_exit:;
                } // End of no supply air path case
                if ((NumZonesCool + NumZonesHeat) == 0) {
                    ShowSevereError(state, format("An outlet node in AirLoopHVAC=\"{}\" is not connected to any zone", thisPrimaryAirSys.Name));
                    ShowContinueError(state,
                                      format("Could not match ZoneEquipGroup Inlet Node=\"{}\" to any Supply Air Path or controlled zone",
                                             state.dataLoopNodes->NodeID(ZoneSideNodeNum)));
                    ErrorsFound = true;
                }
            }

            // we now know the number of heated and cooled zones served by this primary air system.
            // Allocate the subarrays in AirToZoneNodeInfo
            thisAirToZoneNodeInfo.CoolCtrlZoneNums.allocate(NumZonesCool);
            thisAirToZoneNodeInfo.HeatCtrlZoneNums.allocate(NumZonesHeat);
            thisAirToZoneNodeInfo.CoolZoneInletNodes.allocate(NumZonesCool);
            thisAirToZoneNodeInfo.HeatZoneInletNodes.allocate(NumZonesHeat);
            thisAirToZoneNodeInfo.TermUnitCoolInletNodes.allocate(NumZonesCool);
            thisAirToZoneNodeInfo.TermUnitHeatInletNodes.allocate(NumZonesHeat);
            thisAirToZoneNodeInfo.TermUnitCoolSizingIndex.allocate(NumZonesCool);
            thisAirToZoneNodeInfo.TermUnitHeatSizingIndex.allocate(NumZonesHeat);
            // Move the controlled zone numbers from the scratch arrays into AirToZoneNodeInfo
            for (int ZoneInSysIndex = 1; ZoneInSysIndex <= NumZonesCool; ++ZoneInSysIndex) {
                thisAirToZoneNodeInfo.CoolCtrlZoneNums(ZoneInSysIndex) = cooledZone(ZoneInSysIndex).ctrlZoneNum;
                thisAirToZoneNodeInfo.CoolZoneInletNodes(ZoneInSysIndex) = cooledZone(ZoneInSysIndex).zoneInletNode;
                thisAirToZoneNodeInfo.TermUnitCoolInletNodes(ZoneInSysIndex) = cooledZone(ZoneInSysIndex).termUnitInletNode;
                thisAirToZoneNodeInfo.TermUnitCoolSizingIndex(ZoneInSysIndex) = cooledZone(ZoneInSysIndex).termUnitSizingIndex;
            }

            for (int ZoneInSysIndex = 1; ZoneInSysIndex <= NumZonesHeat; ++ZoneInSysIndex) {
                thisAirToZoneNodeInfo.HeatCtrlZoneNums(ZoneInSysIndex) = heatedZone(ZoneInSysIndex).ctrlZoneNum;
                thisAirToZoneNodeInfo.HeatZoneInletNodes(ZoneInSysIndex) = heatedZone(ZoneInSysIndex).zoneInletNode;
                thisAirToZoneNodeInfo.TermUnitHeatInletNodes(ZoneInSysIndex) = heatedZone(ZoneInSysIndex).termUnitInletNode;
                thisAirToZoneNodeInfo.TermUnitHeatSizingIndex(ZoneInSysIndex) = heatedZone(ZoneInSysIndex).termUnitSizingIndex;
            }

            thisAirToZoneNodeInfo.NumZonesCooled = NumZonesCool;
            thisAirToZoneNodeInfo.NumZonesHeated = NumZonesHeat;

            // now fill the return air bypass information needed by the RAB setpoint manager
            if (thisPrimaryAirSys.Splitter.Exists && thisPrimaryAirSys.Mixer.Exists) {
                thisPrimaryAirSys.RABExists = true;
                for (int BranchNum = 1; BranchNum <= thisPrimaryAirSys.NumBranches; ++BranchNum) {
                    // find the RAB branch; its inlet is a splitter outlet and it outlet is a mixer inlet
                    if ((thisPrimaryAirSys.Branch(BranchNum).NodeNumIn == thisPrimaryAirSys.Splitter.NodeNumOut(1) ||
                         thisPrimaryAirSys.Branch(BranchNum).NodeNumIn == thisPrimaryAirSys.Splitter.NodeNumOut(2)) &&
                        (thisPrimaryAirSys.Branch(BranchNum).NodeNumOut == thisPrimaryAirSys.Mixer.NodeNumIn(1) ||
                         thisPrimaryAirSys.Branch(BranchNum).NodeNumOut == thisPrimaryAirSys.Mixer.NodeNumIn(2)) &&
                        (thisPrimaryAirSys.Branch(BranchNum).TotalComponents == 1) &&
                        (Util::SameString(thisPrimaryAirSys.Branch(BranchNum).Comp(1).TypeOf, "Duct"))) {
                        // set the RAB splitter outlet node and the RAB mixer inlet node
                        thisPrimaryAirSys.RABSplitOutNode = thisPrimaryAirSys.Branch(BranchNum).NodeNumIn;
                        thisPrimaryAirSys.RABMixInNode = thisPrimaryAirSys.Branch(BranchNum).NodeNumOut;
                        // set the other nodes
                        if (thisPrimaryAirSys.Splitter.NodeNumOut(1) == thisPrimaryAirSys.RABSplitOutNode) {
                            thisPrimaryAirSys.OtherSplitOutNode = thisPrimaryAirSys.Splitter.NodeNumOut(2);
                        } else {
                            thisPrimaryAirSys.OtherSplitOutNode = thisPrimaryAirSys.Splitter.NodeNumOut(1);
                        }
                        if (thisPrimaryAirSys.Mixer.NodeNumIn(1) == thisPrimaryAirSys.RABMixInNode) {
                            thisPrimaryAirSys.SupMixInNode = thisPrimaryAirSys.Mixer.NodeNumIn(2);
                        } else {
                            thisPrimaryAirSys.SupMixInNode = thisPrimaryAirSys.Mixer.NodeNumIn(1);
                        }
                        // set the duct type
                        thisPrimaryAirSys.Branch(BranchNum).DuctType = HVAC::AirDuctType::RAB;
                    }
                }
                thisPrimaryAirSys.MixOutNode = thisPrimaryAirSys.Mixer.NodeNumOut;
            }
        }

        // now fill out AirLoopZoneInfo for cleaner struct of zones attached to air loop, moved from MixedAir to here for use with Std. 62.1
        int MaxNumAirLoopZones = 0;
        for (int AirLoopNum = 1; AirLoopNum <= numPrimaryAirSys; ++AirLoopNum) {
            auto const &thisAirToZoneNodeInfo = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum);
            int NumAirLoopZones = thisAirToZoneNodeInfo.NumZonesCooled + thisAirToZoneNodeInfo.NumZonesHeated;
            // NumZonesCooled + NumZonesHeated must be > 0 or Fatal error is issued in SimAirServingZones
            MaxNumAirLoopZones = max(MaxNumAirLoopZones, NumAirLoopZones); // Max number of zones on any air loop being simulated
        }
        // Find the zones attached to each air loop
        for (int AirLoopNum = 1; AirLoopNum <= numPrimaryAirSys; ++AirLoopNum) {
            auto &thisAirToZoneNodeInfo = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum);
            auto &thisAirLoopZoneInfo = state.dataAirLoop->AirLoopZoneInfo(AirLoopNum);
            thisAirLoopZoneInfo.Zone.allocate(MaxNumAirLoopZones);
            thisAirLoopZoneInfo.ActualZoneNumber.allocate(MaxNumAirLoopZones);
            int NumAirLoopCooledZones = thisAirToZoneNodeInfo.NumZonesCooled;
            int AirLoopZones = NumAirLoopCooledZones;
            int NumAirLoopHeatedZones = thisAirToZoneNodeInfo.NumZonesHeated;
            // Store cooling zone numbers in AirLoopZoneInfo data structure
            for (int NumAirLoopCooledZonesTemp = 1; NumAirLoopCooledZonesTemp <= NumAirLoopCooledZones; ++NumAirLoopCooledZonesTemp) {
                thisAirLoopZoneInfo.Zone(NumAirLoopCooledZonesTemp) = thisAirToZoneNodeInfo.CoolCtrlZoneNums(NumAirLoopCooledZonesTemp);
                thisAirLoopZoneInfo.ActualZoneNumber(NumAirLoopCooledZonesTemp) = thisAirToZoneNodeInfo.CoolCtrlZoneNums(NumAirLoopCooledZonesTemp);
            }
            // Store heating zone numbers in AirLoopZoneInfo data structure
            // Only store zone numbers that aren't already defined as cooling zones above
            for (int NumAirLoopHeatedZonesTemp = 1; NumAirLoopHeatedZonesTemp <= NumAirLoopHeatedZones; ++NumAirLoopHeatedZonesTemp) {
                int ZoneNum = thisAirToZoneNodeInfo.HeatCtrlZoneNums(NumAirLoopHeatedZonesTemp);
                bool CommonZone = false;
                for (int NumAirLoopCooledZonesTemp = 1; NumAirLoopCooledZonesTemp <= NumAirLoopCooledZones; ++NumAirLoopCooledZonesTemp) {
                    if (ZoneNum != thisAirToZoneNodeInfo.CoolCtrlZoneNums(NumAirLoopCooledZonesTemp)) {
                        continue;
                    }
                    CommonZone = true;
                }
                if (!CommonZone) {
                    ++AirLoopZones;
                    thisAirLoopZoneInfo.Zone(AirLoopZones) = ZoneNum;
                    thisAirLoopZoneInfo.ActualZoneNumber(AirLoopZones) = ZoneNum;
                }
            }
            thisAirLoopZoneInfo.NumZones = AirLoopZones;
        }

        // now register zone inlet nodes as critical demand nodes in the convergence tracking
        state.dataConvergeParams->ZoneInletConvergence.allocate(state.dataGlobal->NumOfZones);
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            if (state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumInletNodes > 0) {
                state.dataConvergeParams->ZoneInletConvergence(ZoneNum).NumInletNodes = state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumInletNodes;
                state.dataConvergeParams->ZoneInletConvergence(ZoneNum).InletNode.allocate(
                    state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumInletNodes);
                for (int nodeLoop = 1; nodeLoop <= state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumInletNodes; ++nodeLoop) {
                    state.dataConvergeParams->ZoneInletConvergence(ZoneNum).InletNode(nodeLoop).NodeNum =
                        state.dataZoneEquip->ZoneEquipConfig(ZoneNum).InletNode(nodeLoop);
                }
            }
        }

        // now connect return nodes with airloops and corresponding inlet nodes
        ConnectReturnNodes(state);

        state.dataSimAirServingZones->InitAirLoopsOneTimeFlag = false;

        if (ErrorsFound) {
            ShowFatalError(state, "Preceding errors cause termination");
        }

        for (int AirLoopNum = 1; AirLoopNum <= numPrimaryAirSys; ++AirLoopNum) {
            auto &thisPrimaryAirSys = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum);

            int SupFanIndex = 0;
            int RetFanIndex = 0;
            bool FoundOASys = false;
            thisPrimaryAirSys.FanDesCoolLoad = 0.0;
            HVAC::FanType supFanType = HVAC::FanType::Invalid;
            HVAC::FanType retFanType = HVAC::FanType::Invalid;

            bool FoundCentralCoolCoil = false;
            for (int BranchNum = 1; BranchNum <= thisPrimaryAirSys.NumBranches; ++BranchNum) {
                auto &branch = thisPrimaryAirSys.Branch(BranchNum);

                for (int CompNum = 1; CompNum <= branch.TotalComponents; ++CompNum) {
                    auto &comp = branch.Comp(CompNum);
                    CompType compType = comp.CompType_Num;
                    if (compType == CompType::OAMixer_Num) {
                        FoundOASys = true;
                    } else if (compType == CompType::WaterCoil_Cooling || compType == CompType::WaterCoil_DetailedCool ||
                               compType == CompType::WaterCoil_CoolingHXAsst || compType == CompType::DXSystem) {
                        FoundCentralCoolCoil = true;
                    } else if (compType == CompType::Fan_Simple_CV || compType == CompType::Fan_Simple_VAV ||
                               compType == CompType::Fan_ComponentModel || compType == CompType::Fan_System_Object) {
                        if (thisPrimaryAirSys.OASysExists && !thisPrimaryAirSys.isAllOA) {
                            if (FoundOASys) {
                                if (branch.DuctType != HVAC::AirDuctType::Heating) {
                                    SupFanIndex = comp.CompIndex = Fans::GetFanIndex(state, comp.Name);
                                    supFanType = state.dataFans->fans(SupFanIndex)->type;
                                    goto EndOfAirLoop;
                                } else {
                                    // Grab CompIndex but don't set airLoop.supFanType or retFanType?
                                    comp.CompIndex = Fans::GetFanIndex(state, comp.Name);
                                }
                            } else {
                                RetFanIndex = comp.CompIndex = Fans::GetFanIndex(state, comp.Name);
                                retFanType = state.dataFans->fans(RetFanIndex)->type;
                                // no goto here?
                            }
                        } else {
                            SupFanIndex = comp.CompIndex = Fans::GetFanIndex(state, comp.Name);
                            supFanType = state.dataFans->fans(SupFanIndex)->type;
                            goto EndOfAirLoop;
                        }
                    }
                } // for (CompNum)
            } // for (BranchNum)
        EndOfAirLoop:;

            thisPrimaryAirSys.supFanNum = SupFanIndex;
            thisPrimaryAirSys.supFanType = supFanType;

            if (FoundCentralCoolCoil) { // parent systems with fan will need to set the fan placement
                thisPrimaryAirSys.supFanPlace = HVAC::FanPlace::DrawThru;
            } else {
                thisPrimaryAirSys.supFanPlace = HVAC::FanPlace::BlowThru;
            }

            thisPrimaryAirSys.retFanType = retFanType;
            thisPrimaryAirSys.retFanNum = RetFanIndex;
        }
        // Check whether there are Central Heating Coils in the Primary Air System
        for (int AirLoopNum = 1; AirLoopNum <= numPrimaryAirSys; ++AirLoopNum) {
            auto &thisPrimaryAirSys = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum);
            bool FoundCentralHeatCoil = false;
            bool unitaryCoolingCoilExists = false;
            bool unitaryHeatingCoilExists = false;
            for (int BranchNum = 1; !FoundCentralHeatCoil && BranchNum <= thisPrimaryAirSys.NumBranches; ++BranchNum) {
                for (int CompNum = 1; !FoundCentralHeatCoil && CompNum <= thisPrimaryAirSys.Branch(BranchNum).TotalComponents; ++CompNum) {
                    std::string &CompName = thisPrimaryAirSys.Branch(BranchNum).Comp(CompNum).Name;
                    CompType CompTypeNum = thisPrimaryAirSys.Branch(BranchNum).Comp(CompNum).CompType_Num;
                    switch (CompTypeNum) {
                    case CompType::WaterCoil_SimpleHeat:
                    case CompType::Coil_ElectricHeat:
                    case CompType::Coil_GasHeat:
                    case CompType::SteamCoil_AirHeat:
                    case CompType::Coil_DeSuperHeat:
                    case CompType::DXHeatPumpSystem:
                    case CompType::Furnace_UnitarySys_HeatOnly:
                    case CompType::Furnace_UnitarySys_HeatCool:
                    case CompType::UnitarySystem_BypassVAVSys:
                    case CompType::UnitarySystem_MSHeatPump:
                    case CompType::CoilUserDefined:
                        FoundCentralHeatCoil = true;
                        break;
                    case CompType::UnitarySystemModel:
                        // mine HeatCoilExists from UnitarySystem
                        unitaryCoolingCoilExists = false;
                        unitaryHeatingCoilExists = false;
                        UnitarySystems::UnitarySys::getUnitarySysHeatCoolCoil(state, CompName, unitaryCoolingCoilExists, unitaryHeatingCoilExists, 0);
                        if (unitaryHeatingCoilExists) {
                            FoundCentralHeatCoil = true;
                        }
                        break;
                    default:
                        break;
                    }
                } // end of component loop
            } // end of Branch loop
            thisPrimaryAirSys.CentralHeatCoilExists = FoundCentralHeatCoil;
        } // end of AirLoop loop

        // Check whether there are Central Cooling Coils in the Primary Air System
        for (int AirLoopNum = 1; AirLoopNum <= numPrimaryAirSys; ++AirLoopNum) {
            auto &thisPrimaryAirSys = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum);
            bool FoundCentralCoolCoil = false;
            bool unitaryCoolingCoilExists = false;
            bool unitaryHeatingCoilExists = false;
            for (int BranchNum = 1; !FoundCentralCoolCoil && BranchNum <= thisPrimaryAirSys.NumBranches; ++BranchNum) {
                for (int CompNum = 1; !FoundCentralCoolCoil && CompNum <= thisPrimaryAirSys.Branch(BranchNum).TotalComponents; ++CompNum) {
                    CompType CompTypeNum = thisPrimaryAirSys.Branch(BranchNum).Comp(CompNum).CompType_Num;
                    std::string &CompName = thisPrimaryAirSys.Branch(BranchNum).Comp(CompNum).Name;
                    switch (CompTypeNum) {
                    case CompType::WaterCoil_Cooling:
                    case CompType::WaterCoil_DetailedCool:
                    case CompType::WaterCoil_CoolingHXAsst:
                    case CompType::DXSystem:
                    case CompType::Furnace_UnitarySys_HeatCool:
                    case CompType::UnitarySystem_BypassVAVSys:
                    case CompType::UnitarySystem_MSHeatPump:
                    case CompType::CoilUserDefined:
                        FoundCentralCoolCoil = true;
                        break;
                    case CompType::UnitarySystemModel:
                        // mine CoolHeat coil exists from UnitarySys
                        unitaryCoolingCoilExists = false;
                        unitaryHeatingCoilExists = false;
                        UnitarySystems::UnitarySys::getUnitarySysHeatCoolCoil(state, CompName, unitaryCoolingCoilExists, unitaryHeatingCoilExists, 0);
                        if (unitaryCoolingCoilExists) {
                            FoundCentralCoolCoil = true;
                        }
                        break;
                    default:
                        break;
                    }
                } // end of component loop
            } // end of Branch loop
            thisPrimaryAirSys.CentralCoolCoilExists = FoundCentralCoolCoil;
        } // end of AirLoop loop

    } // one time flag

    // Size the air loop branch air flows
    if (!state.dataGlobal->SysSizingCalc && state.dataSimAirServingZones->InitAirLoopsBranchSizingFlag) {

        for (int AirLoopNum = 1; AirLoopNum <= numPrimaryAirSys; ++AirLoopNum) {
            auto const &thisPrimaryAirSys = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum);

            for (int BranchNum = 1; BranchNum <= thisPrimaryAirSys.NumBranches; ++BranchNum) {
                SizeAirLoopBranches(state, AirLoopNum, BranchNum);
            }
        }

        state.dataSimAirServingZones->InitAirLoopsBranchSizingFlag = false;

        // calculate the ratio of air loop design flow to the sum of the zone design flows
        for (int AirLoopNum = 1; AirLoopNum <= numPrimaryAirSys; ++AirLoopNum) {
            auto &thisPrimaryAirSys = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum);
            auto &thisAirToZoneNodeInfo = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum);
            state.dataSimAirServingZones->SumZoneDesFlow = 0.0;
            state.dataAirLoop->AirLoopFlow(AirLoopNum).DesSupply = thisPrimaryAirSys.DesignVolFlowRate * state.dataEnvrn->StdRhoAir;
            state.dataAirLoop->AirLoopFlow(AirLoopNum).DesReturnFrac = thisPrimaryAirSys.DesignReturnFlowFraction;
            for (int ZoneInSysIndex = 1; ZoneInSysIndex <= thisAirToZoneNodeInfo.NumZonesCooled; ++ZoneInSysIndex) {
                state.dataSimAirServingZones->TUInNode = thisAirToZoneNodeInfo.TermUnitCoolInletNodes(ZoneInSysIndex);
                state.dataSimAirServingZones->SumZoneDesFlow += state.dataLoopNodes->Node(state.dataSimAirServingZones->TUInNode).MassFlowRateMax;
            }
            if (state.dataSimAirServingZones->SumZoneDesFlow > HVAC::VerySmallMassFlow) {
                state.dataAirLoop->AirLoopFlow(AirLoopNum).SysToZoneDesFlowRatio =
                    state.dataAirLoop->AirLoopFlow(AirLoopNum).DesSupply / state.dataSimAirServingZones->SumZoneDesFlow;
            } else {
                state.dataAirLoop->AirLoopFlow(AirLoopNum).SysToZoneDesFlowRatio = 1.0;
            }
        }

        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            if (!state.dataZoneEquip->ZoneEquipConfig(ZoneNum).IsControlled) {
                continue;
            }
            // sets design supply air flow rate in the ZoneEquipConfig struct for use with zone air mass balance
            for (int returnNum = 1; returnNum <= state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumReturnNodes; ++returnNum) {
                int airLoop = state.dataZoneEquip->ZoneEquipConfig(ZoneNum).ReturnNodeAirLoopNum(returnNum);
                if (airLoop > 0) {
                    state.dataZoneEquip->ZoneEquipConfig(ZoneNum).AirLoopDesSupply = state.dataAirLoop->AirLoopFlow(airLoop).DesSupply;
                }
            }
        }
    }

    // Do the Begin Environment initializations
    if (state.dataGlobal->BeginEnvrnFlag && FirstHVACIteration && state.dataSimAirServingZones->MyEnvrnFlag) {

        if (numPrimaryAirSys > 0) {
            for (auto &e : state.dataAirLoop->PriAirSysAvailMgr) {
                e.availStatus = Avail::Status::NoAction;
                e.StartTime = 0;
                e.StopTime = 0;
            }
        }

        for (int AirLoopNum = 1; AirLoopNum <= numPrimaryAirSys; ++AirLoopNum) { // Start looping through all of the air loops...
            auto &thisPrimaryAirSys = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum);

            for (int BranchNum = 1; BranchNum <= thisPrimaryAirSys.NumBranches; ++BranchNum) { // loop over all branches in system
                for (int NodeIndex = 1; NodeIndex <= thisPrimaryAirSys.Branch(BranchNum).TotalNodes; ++NodeIndex) { // loop over alll nodes on branch

                    int NodeNum = thisPrimaryAirSys.Branch(BranchNum).NodeNum(NodeIndex);

                    // Initialize the nodes to a standard set of initial conditions that will
                    //  change after the first iteration to a system value
                    state.dataLoopNodes->Node(NodeNum).Temp = 20.0;
                    state.dataLoopNodes->Node(NodeNum).HumRat = state.dataEnvrn->OutHumRat;
                    state.dataLoopNodes->Node(NodeNum).Enthalpy =
                        Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(NodeNum).Temp, state.dataLoopNodes->Node(NodeNum).HumRat);
                    // set the node mass flow rates to the airloop max mass flow rate
                    state.dataLoopNodes->Node(NodeNum).MassFlowRate = state.dataAirLoop->AirLoopFlow(AirLoopNum).DesSupply;
                    state.dataLoopNodes->Node(NodeNum).MassFlowRateMax = state.dataAirLoop->AirLoopFlow(AirLoopNum).DesSupply;
                    state.dataLoopNodes->Node(NodeNum).MassFlowRateMaxAvail = state.dataAirLoop->AirLoopFlow(AirLoopNum).DesSupply;
                    state.dataLoopNodes->Node(NodeNum).MassFlowRateMin = 0.0;
                    state.dataLoopNodes->Node(NodeNum).MassFlowRateSetPoint = 0.0;
                    state.dataLoopNodes->Node(NodeNum).MassFlowRateMinAvail = 0.0;
                    state.dataLoopNodes->Node(NodeNum).Press = state.dataEnvrn->StdBaroPress;
                    state.dataLoopNodes->Node(NodeNum).Quality = 0.0;
                    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        state.dataLoopNodes->Node(NodeNum).CO2 = state.dataContaminantBalance->OutdoorCO2;
                    }
                    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        state.dataLoopNodes->Node(NodeNum).GenContam = state.dataContaminantBalance->OutdoorGC;
                    }

                } // end of loop over nodes on each branch

            } // end of loop through branches in system

        } // end of loop over primary air systems
        state.dataSimAirServingZones->MyEnvrnFlag = false;

    } // End the environment initializations

    if (!state.dataGlobal->BeginEnvrnFlag) {
        state.dataSimAirServingZones->MyEnvrnFlag = true;
    }

    // Do the Begin Day initializations
    if (state.dataGlobal->BeginDayFlag) {
    }

    // There are no hourly initializations done in the heat balance

    // Do the following initializations (every time step).

    for (int AirLoopNum = 1; AirLoopNum <= numPrimaryAirSys; ++AirLoopNum) {
        auto &thisPrimaryAirSys = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum);
        auto &thisAirToZoneNodeInfo = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum);
        auto &thisAirLoopControlInfo = state.dataAirLoop->AirLoopControlInfo(AirLoopNum);
        // zero all MassFlowRateSetPoints
        for (int BranchNum = 1; BranchNum <= thisPrimaryAirSys.NumBranches; ++BranchNum) { // loop over all branches in system
            if (thisPrimaryAirSys.Branch(BranchNum).DuctType == HVAC::AirDuctType::RAB) {
                continue;
            }
            for (int NodeIndex = 1; NodeIndex <= thisPrimaryAirSys.Branch(BranchNum).TotalNodes; ++NodeIndex) { // loop over alll nodes on branch
                int NodeNum = thisPrimaryAirSys.Branch(BranchNum).NodeNum(NodeIndex);
                state.dataLoopNodes->Node(NodeNum).MassFlowRateSetPoint = 0.0;
                // Reset MassFlowRateMaxAvail at start of each HVAC simulation
                if (FirstHVACIteration) {
                    state.dataLoopNodes->Node(NodeNum).MassFlowRateMaxAvail = state.dataLoopNodes->Node(NodeNum).MassFlowRateMax;
                    state.dataLoopNodes->Node(NodeNum).MassFlowRateMinAvail = state.dataLoopNodes->Node(NodeNum).MassFlowRateMin;
                }
            }
        }

        // set the required flow (from zone equipment) at system outlet nodes
        for (int OutNum = 1; OutNum <= thisPrimaryAirSys.NumOutletBranches; ++OutNum) {
            int OutBranchNum = thisPrimaryAirSys.OutletBranchNum[OutNum - 1];
            int NodeNumOut = thisPrimaryAirSys.Branch(OutBranchNum).NodeNumOut;
            int ZoneSideNodeNum = thisAirToZoneNodeInfo.ZoneEquipSupplyNodeNum(OutNum);
            Real64 MassFlowSet = 0.0;

            if (!FirstHVACIteration) {
                MassFlowSet = state.dataLoopNodes->Node(ZoneSideNodeNum).MassFlowRate;
            } else { // first time through in each HVAC timestep, use loop design mass flow rates for required mass flows
                MassFlowSet = state.dataAirLoop->AirLoopFlow(AirLoopNum).DesSupply;
            }
            // Need to make sure that flows are greater than zero
            if (MassFlowSet >= 0.0) {
                state.dataLoopNodes->Node(NodeNumOut).MassFlowRateSetPoint = MassFlowSet;
            } else if (MassFlowSet < 0.0) {
                state.dataLoopNodes->Node(NodeNumOut).MassFlowRateSetPoint = 0.0;
            }

            if (state.dataLoopNodes->Node(NodeNumOut).MassFlowRateSetPoint < state.dataSimAirServingZones->MassFlowSetToler) {
                state.dataLoopNodes->Node(NodeNumOut).MassFlowRateSetPoint = 0.0;
            }

            // Pass the required mass flow upstream to the start of each outlet branch
            for (int BranchNodeIndex = thisPrimaryAirSys.Branch(OutBranchNum).TotalNodes - 1; BranchNodeIndex >= 1; --BranchNodeIndex) {
                int NodeNum = thisPrimaryAirSys.Branch(OutBranchNum).NodeNum(BranchNodeIndex);
                if (thisPrimaryAirSys.OASysExists && (NodeNum == thisPrimaryAirSys.OASysInletNodeNum)) {
                    // need to modify if OA relief and supply not balanced because of exhaust fans
                    state.dataSimAirServingZones->OAReliefDiff = state.dataLoopNodes->Node(thisPrimaryAirSys.OASysOutletNodeNum).MassFlowRate -
                                                                 state.dataLoopNodes->Node(NodeNum).MassFlowRate;
                    if (state.dataSimAirServingZones->OAReliefDiff > 0.0) {
                        state.dataLoopNodes->Node(NodeNum).MassFlowRateSetPoint =
                            state.dataLoopNodes->Node(NodeNumOut).MassFlowRateSetPoint - state.dataSimAirServingZones->OAReliefDiff;
                    } else {
                        state.dataLoopNodes->Node(NodeNum).MassFlowRateSetPoint = state.dataLoopNodes->Node(NodeNumOut).MassFlowRateSetPoint;
                    }
                } else {
                    state.dataLoopNodes->Node(NodeNum).MassFlowRateSetPoint = state.dataLoopNodes->Node(NodeNumOut).MassFlowRateSetPoint;
                }
            } // end loop over branch nodes

        } // end loop over outlet branches

        // [DC/LBNL] Initialize flag for current air loop
        thisAirLoopControlInfo.NewFlowRateFlag = false;

        // start each HVAC simulation at design air flow rate
        if (FirstHVACIteration) {
            // At each new HVAC iteration reset air loop converged flag to avoid attempting a warm restart
            // in SimAirLoop
            thisAirLoopControlInfo.ConvergedFlag = false;

            for (int InNum = 1; InNum <= thisPrimaryAirSys.NumInletBranches; ++InNum) {
                int InBranchNum = thisPrimaryAirSys.InletBranchNum[InNum - 1];
                if (InBranchNum == 0) {
                    ShowFatalError(state, format("Missing Inlet Branch on Primary Air System={}", thisPrimaryAirSys.Name));
                }
                int NodeNumIn = thisPrimaryAirSys.Branch(InBranchNum).NodeNumIn;

                // [DC/LBNL] Save previous mass flow rate
                Real64 MassFlowSaved = state.dataLoopNodes->Node(NodeNumIn).MassFlowRate;

                state.dataLoopNodes->Node(NodeNumIn).MassFlowRate = state.dataAirLoop->AirLoopFlow(AirLoopNum).DesSupply;

                // [DC/LBNL] Detect if air mass flow rate has changed since last air loop simulation
                if (state.dataLoopNodes->Node(NodeNumIn).MassFlowRate != MassFlowSaved) {
                    thisAirLoopControlInfo.NewFlowRateFlag = true;
                }

            } // end loop over inlet branches
            thisAirLoopControlInfo.EconoLockout = false;
        }
        // if a flow rate is specified for the loop use it here
        if (thisAirLoopControlInfo.LoopFlowRateSet && !FirstHVACIteration) {
            for (int InNum = 1; InNum <= thisPrimaryAirSys.NumInletBranches; ++InNum) {
                int InBranchNum = thisPrimaryAirSys.InletBranchNum[InNum - 1];
                int NodeNumIn = thisPrimaryAirSys.Branch(InBranchNum).NodeNumIn;
                state.dataLoopNodes->Node(NodeNumIn).MassFlowRate =
                    state.dataAirLoop->AirLoopFlow(AirLoopNum).DesSupply * state.dataAirLoop->AirLoopFlow(AirLoopNum).ReqSupplyFrac -
                    (state.dataAirLoop->AirLoopFlow(AirLoopNum).SupFlow - state.dataAirLoop->AirLoopFlow(AirLoopNum).SysRetFlow);
            }
        }

    } // end loop over primary air systems
}

void ConnectReturnNodes(EnergyPlusData &state)
{
    // This initializes ZoneEquipConfig.ReturnNodeInletNum and ReturnNodeAirLoopNum
    // Search all return paths to match return nodes with the airloop they are connected to and find the corresponding zone inlet node
    // (same zone, same airloop)

    auto &AirToZoneNodeInfo = state.dataAirLoop->AirToZoneNodeInfo;
    int NumPrimaryAirSys = state.dataHVACGlobal->NumPrimaryAirSys;

    if (!state.dataZoneEquip->ZoneEquipInputsFilled) {
        return;
    }

    bool returnPathFound = false;
    // Loop over all controlled zones
    for (int ctrlZoneNum = 1; ctrlZoneNum <= state.dataGlobal->NumOfZones; ++ctrlZoneNum) {
        auto &thisZoneEquip = state.dataZoneEquip->ZoneEquipConfig(ctrlZoneNum);
        if (!thisZoneEquip.IsControlled) {
            continue;
        }
        // Loop over each return node for this zone
        for (int zoneOutNum = 1; zoneOutNum <= thisZoneEquip.NumReturnNodes; ++zoneOutNum) {
            returnPathFound = false;
            int airLoopNum = 0;
            int thisReturnNode = thisZoneEquip.ReturnNode(zoneOutNum);
            // Loop over all return paths
            for (int retPathNum = 1; retPathNum <= state.dataZoneEquip->NumReturnAirPaths; ++retPathNum) {
                auto const &thisRetPath = state.dataZoneEquip->ReturnAirPath(retPathNum);
                // Find which airloop this return path is on
                for (int sysNum = 1; sysNum <= NumPrimaryAirSys; ++sysNum) {
                    if (AirToZoneNodeInfo(sysNum).NumReturnNodes > 0) {
                        if (thisRetPath.OutletNodeNum == AirToZoneNodeInfo(sysNum).ZoneEquipReturnNodeNum(1)) {
                            airLoopNum = sysNum;
                            AirToZoneNodeInfo(sysNum).ReturnAirPathNum(1) = retPathNum;
                            break;
                        }
                    }
                }
                // Loop over components in return path and each component's inlet nodes
                for (int compNum = 1; compNum <= thisRetPath.NumOfComponents; ++compNum) {
                    DataZoneEquipment::AirLoopHVACZone compType = thisRetPath.ComponentTypeEnum(compNum);
                    if (compType == DataZoneEquipment::AirLoopHVACZone::Mixer) {
                        auto const &thisMixer = state.dataMixerComponent->MixerCond(thisRetPath.ComponentIndex(compNum));
                        for (int inNode = 1; inNode <= thisMixer.NumInletNodes; ++inNode) {
                            if (thisReturnNode == thisMixer.InletNode(inNode)) {
                                thisZoneEquip.ReturnNodeAirLoopNum(zoneOutNum) = airLoopNum; // set the return node airloop num
                                thisZoneEquip.ReturnNodeRetPathNum(zoneOutNum) = retPathNum;
                                thisZoneEquip.ReturnNodeRetPathCompNum(zoneOutNum) = compNum;
                                returnPathFound = true;
                                break; // leave component inlet node loop
                            }
                        }
                    } else if (compType == DataZoneEquipment::AirLoopHVACZone::ReturnPlenum) {
                        auto const &thisPlenum = state.dataZonePlenum->ZoneRetPlenCond(thisRetPath.ComponentIndex(compNum));
                        for (int inNode = 1; inNode <= thisPlenum.NumInletNodes; ++inNode) {
                            if (thisReturnNode == thisPlenum.InletNode(inNode)) {
                                thisZoneEquip.ReturnNodeAirLoopNum(zoneOutNum) = airLoopNum; // set the return node airloop num
                                thisZoneEquip.ReturnNodeRetPathNum(zoneOutNum) = retPathNum;
                                thisZoneEquip.ReturnNodeRetPathCompNum(zoneOutNum) = compNum;
                                returnPathFound = true;
                                break; // leave component inlet node loop
                            }
                        }
                    }
                    if (returnPathFound) {
                        break; // leave return path component loop
                    }
                }
                if (returnPathFound) {
                    break; // leave return path loop
                }
            }

            if (airLoopNum > 0) {
                // Find matching inlet node connected to the same air loop
                for (int inletNum = 1; inletNum <= thisZoneEquip.NumInletNodes; ++inletNum) {
                    if (thisZoneEquip.InletNodeAirLoopNum(inletNum) == airLoopNum) {
                        thisZoneEquip.ReturnNodeInletNum(zoneOutNum) = inletNum;
                        break;
                    }
                }
            }
        } // return nodes loop
    } // controlled zones loop

    // Check for any air loops that may be connected directly to a zone return node
    for (int airLoopNum = 1; airLoopNum <= NumPrimaryAirSys; ++airLoopNum) {
        bool returnFound = false;
        if (AirToZoneNodeInfo(airLoopNum).NumReturnNodes > 0) {
            int zeqReturnNodeNum = AirToZoneNodeInfo(airLoopNum).ZoneEquipReturnNodeNum(1);
            if (zeqReturnNodeNum > 0) {
                for (int ctrlZoneNum = 1; ctrlZoneNum <= state.dataGlobal->NumOfZones; ++ctrlZoneNum) {
                    auto &thisZoneEquip = state.dataZoneEquip->ZoneEquipConfig(ctrlZoneNum);
                    if (!thisZoneEquip.IsControlled) {
                        continue;
                    }
                    for (int zoneOutNum = 1; zoneOutNum <= thisZoneEquip.NumReturnNodes; ++zoneOutNum) {
                        if (thisZoneEquip.ReturnNode(zoneOutNum) == zeqReturnNodeNum) {
                            thisZoneEquip.ReturnNodeAirLoopNum(zoneOutNum) = airLoopNum;
                            returnFound = true;
                            // Find matching inlet node connected to the same air loop
                            for (int inletNum = 1; inletNum <= thisZoneEquip.NumInletNodes; ++inletNum) {
                                if (thisZoneEquip.InletNodeAirLoopNum(inletNum) == airLoopNum) {
                                    thisZoneEquip.ReturnNodeInletNum(zoneOutNum) = inletNum;
                                    break;
                                }
                            }
                            break; // leave zone return node loop
                        }
                        if (returnFound) {
                            break; // leave controlled zone loop
                        }
                    }
                }
            }
        }
    }
}

void SimAirLoops(EnergyPlusData &state, bool const FirstHVACIteration, bool &SimZoneEquipment)
{

    // SUBROUTINE INFORMATION
    //             AUTHOR:  Russ Taylor, Dan Fisher, Fred Buhl
    //       DATE WRITTEN:  Oct 1997
    //           MODIFIED:  Dec 1997 Fred Buhl
    //           MODIFIED:  Apr 1998 Richard Liesen
    //           MODIFIED:  Dec 1999 Fred Buhl
    //           MODIFIED:  Feb 2006 Dimitri Curtil (LBNL)
    //                      - Moved air loop simulation to SimAirLoop() routine.
    //      RE-ENGINEERED:  This is new code, not reengineered

    // PURPOSE OF THIS SUBROUTINE:
    // This is the driver subroutine for the air loop simulation. It simulates
    // each primary air system in the problem and passes the outlet node conditions
    // on to the attached Zone Equipment inlet nodes.

    // METHODOLOGY EMPLOYED:
    // For each primary air system:
    // (1) each component in the system is simulated in natural order, beginning at
    //     the return air inlet and progressing to the supply air outlets. Node data
    //     is passed in the same direction.
    // (2) The controllers and their actions are simulated.
    // (3) Steps 2 and 3 are repeated until the control criteria are satisfied.
    // (4) A mass balance check is performed; if it fails, mass balance is imposed
    //     and steps 1, 2, and 3 are repeated. At the end we should have a correct,
    //     self consistent primary air system simulation.

    // REFERENCES: None

    // Using/Aliasing
    using HVACInterfaceManager::UpdateHVACInterface;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:
    // TRUE if first full HVAC iteration in an HVAC timestep
    // TRUE if Zone Equipment needs to be resimulated.

    // SUBROUTINE LOCAL VARIABLE DEFINITIONS
    Real64 rxTime;
    // Primary Air Sys DO loop index
    int AirLoopNum;
    // Max number of iterations performed by controllers on each air loop
    int AirLoopIterMax;
    // Aggregated number of iterations across all controllers on each air loop
    int AirLoopIterTot;
    // Total number of times SimAirLoopComponents() has been invoked to simulate each air loop
    int AirLoopNumCalls;
    // Primary air system outlet DO loop index
    int AirSysOutNum;
    // DO loop index; there are 2 passes - the 2nd is done only if mass balance fails
    int AirLoopPass;
    // Flag set by ResolveSysFlow; if TRUE, mass balance failed and there must be a second pass
    bool SysReSim;
    DataConvergParams::CalledFrom CalledFrom;

    auto &AirToZoneNodeInfo = state.dataAirLoop->AirToZoneNodeInfo;
    auto &AirLoopControlInfo = state.dataAirLoop->AirLoopControlInfo;

    // Set up output variables
    if (!state.dataSimAirServingZones->OutputSetupFlag) {
        SetupOutputVariable(state,
                            "Air System Simulation Maximum Iteration Count",
                            Constant::Units::None,
                            state.dataSimAirServingZones->salIterMax,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            "SimAir");
        SetupOutputVariable(state,
                            "Air System Simulation Iteration Count",
                            Constant::Units::None,
                            state.dataSimAirServingZones->salIterTot,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            "SimAir");
        SetupOutputVariable(state,
                            "Air System Component Model Simulation Calls",
                            Constant::Units::None,
                            state.dataSimAirServingZones->NumCallsTot,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            "SimAir");
        state.dataSimAirServingZones->OutputSetupFlag = true;
    }

    // BUG: IterMax should not be aggregated as a Sum output variable
    //      We need a new aggregation scheme to track the max value across HVAC steps
    //      instead of summing it up.
    state.dataSimAirServingZones->salIterMax = 0;

    // Reset counters to capture statistics for the current zone time step
    // Aggregate statistics over all HVAC time steps, even the rejected ones, to properly
    // reflect the numerical work. The condition to detect a new HVAC time step is essentially
    // based on the time stamp at the beginning of the current HVAC step (expressed in seconds).
    if (FirstHVACIteration) {
        rxTime = HVACControllers::GetPreviousHVACTime(state);
        if (state.dataSimAirServingZones->SavedPreviousHVACTime != rxTime) {
            state.dataSimAirServingZones->SavedPreviousHVACTime = rxTime;
            state.dataSimAirServingZones->salIterTot = 0;
            state.dataSimAirServingZones->NumCallsTot = 0;
        }
    }

    // Loop over all the primary air loop; simulate their components (equipment)
    // and controllers
    for (AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) { // NumPrimaryAirSys is the number of primary air loops

        // Check to see if System Availability Managers are asking for fans to cycle on or shut off
        // and set fan on/off flags accordingly.
        state.dataHVACGlobal->TurnFansOn = false;
        state.dataHVACGlobal->TurnFansOff = false;
        state.dataHVACGlobal->NightVentOn = false;
        if (state.dataAirLoop->PriAirSysAvailMgr(AirLoopNum).availStatus == Avail::Status::CycleOn) {
            state.dataHVACGlobal->TurnFansOn = true;
        }
        if (state.dataAirLoop->PriAirSysAvailMgr(AirLoopNum).availStatus == Avail::Status::ForceOff) {
            state.dataHVACGlobal->TurnFansOff = true;
        }
        if (AirLoopControlInfo(AirLoopNum).NightVent) {
            state.dataHVACGlobal->NightVentOn = true;
        }

        //   Set current system number for sizing routines
        state.dataSize->CurSysNum = AirLoopNum;

        // 2 passes; 1 usually suffices; 2 is done if ResolveSysFlow detects a failure of mass balance
        for (AirLoopPass = 1; AirLoopPass <= 2; ++AirLoopPass) {

            SysReSim = false;
            AirLoopControlInfo(AirLoopNum).AirLoopPass = AirLoopPass; // save for use without passing as argument

            // Simulate controllers on air loop with current air mass flow rates
            SimAirLoop(state, FirstHVACIteration, AirLoopNum, AirLoopPass, AirLoopIterMax, AirLoopIterTot, AirLoopNumCalls);

            // Update tracker for maximum number of iterations needed by any controller on all air loops
            state.dataSimAirServingZones->salIterMax = max(state.dataSimAirServingZones->salIterMax, AirLoopIterMax);
            // Update tracker for aggregated number of iterations needed by all controllers on all air loops
            state.dataSimAirServingZones->salIterTot += AirLoopIterTot;
            // Update tracker for total number of times SimAirLoopComponents() has been invoked across all air loops
            state.dataSimAirServingZones->NumCallsTot += AirLoopNumCalls;

            // At the end of the first pass, check whether a second pass is needed or not
            if (AirLoopPass == 1) {
                // If simple system, skip second pass
                if (AirLoopControlInfo(AirLoopNum).Simple) {
                    break;
                }
                ResolveSysFlow(state, AirLoopNum, SysReSim);
                // If mass balance OK, skip second pass
                if (!SysReSim) {
                    break;
                }
            }
        }

        // Air system side has been simulated, now transfer conditions across to
        // the zone equipment side, looping through all supply air paths for this
        // air loop.
        for (AirSysOutNum = 1; AirSysOutNum <= AirToZoneNodeInfo(AirLoopNum).NumSupplyNodes; ++AirSysOutNum) {
            if (AirSysOutNum == 1) {
                CalledFrom = DataConvergParams::CalledFrom::AirSystemSupplySideDeck1;
            }
            if (AirSysOutNum == 2) {
                CalledFrom = DataConvergParams::CalledFrom::AirSystemSupplySideDeck2;
            }
            UpdateHVACInterface(state,
                                AirLoopNum,
                                CalledFrom,
                                AirToZoneNodeInfo(AirLoopNum).AirLoopSupplyNodeNum(AirSysOutNum),
                                AirToZoneNodeInfo(AirLoopNum).ZoneEquipSupplyNodeNum(AirSysOutNum),
                                SimZoneEquipment);
        } // ...end of DO loop over supply air paths for this air loop.

    } // End of Air Loop iteration

    if ((int)state.dataAirLoopHVACDOAS->airloopDOAS.size() > 0) {
        int index;
        Real64 OAMassFLowrate = 0.0;
        for (std::size_t loop = 0; loop < state.dataAirLoopHVACDOAS->airloopDOAS.size(); ++loop) {
            auto &thisAirLoopDOASObjec = state.dataAirLoopHVACDOAS->airloopDOAS[loop]; // <- regular reference variable, not a pointer
            if (thisAirLoopDOASObjec.m_AirLoopDOASNum > -1) {
                index = thisAirLoopDOASObjec.m_AirLoopDOASNum;
            } else {
                index = -1;
            }
            thisAirLoopDOASObjec.SimAirLoopHVACDOAS(state, FirstHVACIteration, index);
            OAMassFLowrate += thisAirLoopDOASObjec.SumMassFlowRate;
        }

        if (OAMassFLowrate > 0.0) {
            for (AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys;
                 ++AirLoopNum) { // NumPrimaryAirSys is the number of primary air loops
                state.dataHVACGlobal->TurnFansOn = false;
                state.dataHVACGlobal->TurnFansOff = false;
                state.dataHVACGlobal->NightVentOn = false;
                if (state.dataAirLoop->PriAirSysAvailMgr(AirLoopNum).availStatus == Avail::Status::CycleOn) {
                    state.dataHVACGlobal->TurnFansOn = true;
                }
                if (state.dataAirLoop->PriAirSysAvailMgr(AirLoopNum).availStatus == Avail::Status::ForceOff) {
                    state.dataHVACGlobal->TurnFansOff = true;
                }
                if (AirLoopControlInfo(AirLoopNum).NightVent) {
                    state.dataHVACGlobal->NightVentOn = true;
                }

                //   Set current system number for sizing routines
                state.dataSize->CurSysNum = AirLoopNum;

                // 2 passes; 1 usually suffices; 2 is done if ResolveSysFlow detects a failure of mass balance
                for (AirLoopPass = 1; AirLoopPass <= 2; ++AirLoopPass) {

                    SysReSim = false;
                    AirLoopControlInfo(AirLoopNum).AirLoopPass = AirLoopPass; // save for use without passing as argument

                    // Simulate controllers on air loop with current air mass flow rates
                    SimAirLoop(state, FirstHVACIteration, AirLoopNum, AirLoopPass, AirLoopIterMax, AirLoopIterTot, AirLoopNumCalls);

                    // Update tracker for maximum number of iterations needed by any controller on all air loops
                    state.dataSimAirServingZones->salIterMax = max(state.dataSimAirServingZones->salIterMax, AirLoopIterMax);
                    // Update tracker for aggregated number of iterations needed by all controllers on all air loops
                    state.dataSimAirServingZones->salIterTot += AirLoopIterTot;
                    // Update tracker for total number of times SimAirLoopComponents() has been invoked across all air loops
                    state.dataSimAirServingZones->NumCallsTot += AirLoopNumCalls;

                    // At the end of the first pass, check whether a second pass is needed or not
                    if (AirLoopPass == 1) {
                        // If simple system, skip second pass
                        if (AirLoopControlInfo(AirLoopNum).Simple) {
                            break;
                        }
                        ResolveSysFlow(state, AirLoopNum, SysReSim);
                        // If mass balance OK, skip second pass
                        if (!SysReSim) {
                            break;
                        }
                    }
                }

                // Air system side has been simulated, now transfer conditions across to
                // the zone equipment side, looping through all supply air paths for this
                // air loop.
                for (AirSysOutNum = 1; AirSysOutNum <= AirToZoneNodeInfo(AirLoopNum).NumSupplyNodes; ++AirSysOutNum) {
                    if (AirSysOutNum == 1) {
                        CalledFrom = DataConvergParams::CalledFrom::AirSystemSupplySideDeck1;
                    }
                    if (AirSysOutNum == 2) {
                        CalledFrom = DataConvergParams::CalledFrom::AirSystemSupplySideDeck2;
                    }
                    UpdateHVACInterface(state,
                                        AirLoopNum,
                                        CalledFrom,
                                        AirToZoneNodeInfo(AirLoopNum).AirLoopSupplyNodeNum(AirSysOutNum),
                                        AirToZoneNodeInfo(AirLoopNum).ZoneEquipSupplyNodeNum(AirSysOutNum),
                                        SimZoneEquipment);
                } // ...end of DO loop over supply air paths for this air loop.

            } // End of Air Loop iteration
            // check convergence at the mixer outlet or at the AirLoopDOAS outlet
            AirLoopHVACDOAS::CheckConvergence(state);
        }
    }
    // Reset current system number for sizing routines
    state.dataSize->CurSysNum = 0;
}

void SimAirLoop(EnergyPlusData &state,
                bool const FirstHVACIteration,
                int const AirLoopNum,
                int const AirLoopPass,
                int &AirLoopIterMax,
                int &AirLoopIterTot,
                int &AirLoopNumCalls)
{

    // SUBROUTINE INFORMATION
    //             AUTHOR:  Dimitri Curtil (LBNL)
    //       DATE WRITTEN:  March 2006
    //                      - Fine-tuned outer loop over controllers.
    //                      - Added convergence tracing for air loop controllers.
    //                      - Added mechanism for speculative warm restart after first iteration.
    //      RE-ENGINEERED:  This is new code based on the code that used to be part
    //                      of SimAirLoops().

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine simulates the desired air loop by solving for all the
    // controllers on the air loop in the order they are specified.

    // METHODOLOGY EMPLOYED:
    // To speed up the simulation, we introduced the possiblity to perform the controller
    // simulation on each air loop using a warm restart from the solution obtained
    // at the previous HVAC step iteration. This is only attempted if the air mass flow
    // rate(s) for the air system have not changed since the last iteration.
    // Of course if the warm restart fails, then we perform a normal simulation from
    // a cold start. We refer to this scheme as speculative warm restart.

    // REFERENCES: None

    // Using/Aliasing
    using namespace DataHVACControllers;
    using namespace DataSystemVariables;
    using General::CreateSysTimeIntervalString;
    using HVACControllers::TraceAirLoopControllers;
    using HVACControllers::TrackAirLoopControllers;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:
    // TRUE if first full HVAC iteration in an HVAC timestep
    // Index of the air loop to simulate
    // There are 2 passes - the 2nd is done only if mass balance fails
    // Max number of iterations performed by controllers across this air loop
    // Aggregated number of iterations across all controllers on this air loop
    // Total number of times SimAirLoopComponents() has been invoked to simulate this air loop

    // SUBROUTINE LOCAL VARIABLE DEFINITIONS

    auto &PrimaryAirSystems = state.dataAirSystemsData->PrimaryAirSystems;
    auto &AirLoopControlInfo = state.dataAirLoop->AirLoopControlInfo;

    // Reset air loop trackers to zero
    AirLoopIterMax = 0;
    AirLoopIterTot = 0;
    AirLoopNumCalls = 0;

    // Perform air loop simulation to satisfy convergence for all controllers
    // If first HVAC iteration or new air flow we force a cold restart.
    // Otherwise we attempt a speculative warm restart.
    // TODO: Improve detection of when air flow rate has changed since last air loop simulation
    // TODO: Detect whether warm restart is supported on air loop on very first air loop
    //       simulation only instead of at each HVAC iteration as done now.
    // Only enabled if there are controllers on the air loop
    // Check that the speculative warm restart feature is allowed
    // Never done at first HVAC iteration
    // Never done during sizing
    // Next condition is true whenever the final check for the air loop was converged
    // at the previous SimAirLoop call
    // Next conditions should detect when air mass flow rates have changed
    state.dataSimAirServingZones->DoWarmRestartFlagSAL =
        PrimaryAirSystems(AirLoopNum).NumControllers > 0 && AirLoopControlInfo(AirLoopNum).AllowWarmRestartFlag && !FirstHVACIteration &&
        !state.dataGlobal->SysSizingCalc && AirLoopControlInfo(AirLoopNum).ConvergedFlag && !AirLoopControlInfo(AirLoopNum).LoopFlowRateSet &&
        !AirLoopControlInfo(AirLoopNum).NewFlowRateFlag;

    if (!state.dataSimAirServingZones->DoWarmRestartFlagSAL) {
        // Solve controllers with cold start using default initial values
        SolveAirLoopControllers(state,
                                FirstHVACIteration,
                                AirLoopNum,
                                state.dataSimAirServingZones->AirLoopConvergedFlagSAL,
                                state.dataSimAirServingZones->IterMaxSAL2,
                                state.dataSimAirServingZones->IterTotSAL2,
                                state.dataSimAirServingZones->NumCallsSAL2);

        // Update air loop trackers
        state.dataSimAirServingZones->WarmRestartStatusSAL = ControllerWarmRestart::None;
        AirLoopNumCalls += state.dataSimAirServingZones->NumCallsSAL2;
        AirLoopIterMax = max(AirLoopIterMax, state.dataSimAirServingZones->IterMaxSAL2);
        AirLoopIterTot += state.dataSimAirServingZones->IterTotSAL2;
    } else {
        // First try with speculative warm restart using previous solution
        ReSolveAirLoopControllers(state,
                                  FirstHVACIteration,
                                  AirLoopNum,
                                  state.dataSimAirServingZones->AirLoopConvergedFlagSAL,
                                  state.dataSimAirServingZones->IterMaxSAL2,
                                  state.dataSimAirServingZones->IterTotSAL2,
                                  state.dataSimAirServingZones->NumCallsSAL2);

        // Update air loop trackers
        state.dataSimAirServingZones->WarmRestartStatusSAL = ControllerWarmRestart::Success;
        AirLoopNumCalls += state.dataSimAirServingZones->NumCallsSAL2;
        AirLoopIterMax = max(AirLoopIterMax, state.dataSimAirServingZones->IterMaxSAL2);
        AirLoopIterTot += state.dataSimAirServingZones->IterTotSAL2;

        // Retry with cold start using default initial values if speculative warm restart did not work
        if (!state.dataSimAirServingZones->AirLoopConvergedFlagSAL) {
            SolveAirLoopControllers(state,
                                    FirstHVACIteration,
                                    AirLoopNum,
                                    state.dataSimAirServingZones->AirLoopConvergedFlagSAL,
                                    state.dataSimAirServingZones->IterMaxSAL2,
                                    state.dataSimAirServingZones->IterTotSAL2,
                                    state.dataSimAirServingZones->NumCallsSAL2);

            // Update air loop trackers
            state.dataSimAirServingZones->WarmRestartStatusSAL = DataHVACControllers::ControllerWarmRestart::Fail;
            AirLoopNumCalls += state.dataSimAirServingZones->NumCallsSAL2;
            AirLoopIterMax = max(AirLoopIterMax, state.dataSimAirServingZones->IterMaxSAL2);
            AirLoopIterTot += state.dataSimAirServingZones->IterTotSAL2;
        }
    }

    // Updates air loop statistics
    // To enable runtime statistics tracking for each air loop, define the environment variable
    // TRACK_AIRLOOP=YES or TRACK_AIRLOOP=Y
    if (state.dataSysVars->TrackAirLoopEnvFlag) {
        TrackAirLoopControllers(
            state, AirLoopNum, state.dataSimAirServingZones->WarmRestartStatusSAL, AirLoopIterMax, AirLoopIterTot, AirLoopNumCalls);
    }

    // Generate trace for all controllers on this air loop
    // To enable generating a trace file with the converged solution for all controllers on each air loop,
    // define the environment variable TRACE_AIRLOOP=YES or TRACE_AIRLOOP=Y.
    if (state.dataSysVars->TraceAirLoopEnvFlag) {
        TraceAirLoopControllers(
            state, FirstHVACIteration, AirLoopNum, AirLoopPass, state.dataSimAirServingZones->AirLoopConvergedFlagSAL, AirLoopNumCalls);
    }

    // When there is more than 1 controller on an air loop, each controller sensing
    // different nodes with potentially different setpoints, it is likely that
    // AirLoopConvergedFlag will be false as the individual setpoints will not
    // be satisfied once all the controllers have been simulated. Typically, this could
    // happen if
    // If this is the case then we do not want to try a warm restart as it is very
    // unlikely to succeed.
    AirLoopControlInfo(AirLoopNum).ConvergedFlag = state.dataSimAirServingZones->AirLoopConvergedFlagSAL;
}

void SolveAirLoopControllers(
    EnergyPlusData &state, bool const FirstHVACIteration, int const AirLoopNum, bool &AirLoopConvergedFlag, int &IterMax, int &IterTot, int &NumCalls)
{

    // SUBROUTINE INFORMATION
    //             AUTHOR:  Dimitri Curtil (LBNL)
    //       DATE WRITTEN:  Feb 2006
    //      RE-ENGINEERED:  This is reengineered code that used to be in SimAirLoops()

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine solves for the controllers on the specfied air loop assuming a cold start.

    // METHODOLOGY EMPLOYED:
    // For the specified primary air system:
    // (1) each component in the system is simulated in natural order, beginning at
    //     the return air inlet and progressing to the supply air outlets. Node data
    //     is passed in the same direction.
    // (2) The controllers and their actions are simulated.
    // (3) Steps 2 and 3 are repeated until the control criteria are satisfied.

    // Using/Aliasing
    using namespace DataHVACControllers;
    using General::CreateSysTimeIntervalString;
    using HVACControllers::ManageControllers;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:
    // TRUE if first full HVAC iteration in an HVAC timestep
    // DO loop index; there are 2 passes the 2nd is done only if mass balance fails
    // Index of the air loop being simulated
    // TRUE when primary air system & controllers simulation has converged;
    // Max number of iterations performed by controllers across this air loop
    // Aggregated number of iterations across all controllers on this air loop
    // Total number of times SimAirLoopComponents() has been invoked

    // SUBROUTINE PARAMETER DEFINITIONS:
    // Maximum iterations of an air system/controllers simulation sequence
    int constexpr MaxIter(50);

    // SUBROUTINE LOCAL VARIABLE DEFINITIONS
    // TRUE if controller supports speculative warm restart
    bool AllowWarmRestartFlag;
    // TRUE when controller has converged
    bool ControllerConvergedFlag;
    // TRUE when air loop has been evaluated with latest actuated variables
    bool IsUpToDateFlag;

    auto &PrimaryAirSystems = state.dataAirSystemsData->PrimaryAirSystems;
    auto &AirLoopControlInfo = state.dataAirLoop->AirLoopControlInfo;

    // To track number of calls to SimAirLoopComponents() for each air loop
    // Represents the most computationally expensive operation in the iteration.
    // Best metric to use to assess the runtime performance of air loop simulation
    NumCalls = 0;
    IterMax = 0;
    IterTot = 0;

    AirLoopConvergedFlag = true;
    state.dataSimAirServingZones->BypassOAControllerSALC = true; // don't simulate OA contollers at this time (see SolveWaterCoilController)
    IsUpToDateFlag = false;
    PrimaryAirSystems(AirLoopNum).ControlConverged = false;

    AllowWarmRestartFlag = true;
    AirLoopControlInfo(AirLoopNum).AllowWarmRestartFlag = true;

    if (PrimaryAirSystems(AirLoopNum).SizeAirloopCoil) { // one time flag to initialize controller index and size coils if needed
        // Loop through the controllers first to set the controller index in the PrimaryAirSystem array.
        // Need to actaully simulate controller to get controller index.
        for (int AirLoopControlNum = 1; AirLoopControlNum <= PrimaryAirSystems(AirLoopNum).NumControllers; ++AirLoopControlNum) {
            PrimaryAirSystems(AirLoopNum).ControllerIndex(AirLoopControlNum) =
                HVACControllers::GetControllerIndex(state, PrimaryAirSystems(AirLoopNum).ControllerName(AirLoopControlNum));
            state.dataHVACControllers->ControllerProps(PrimaryAirSystems(AirLoopNum).ControllerIndex(AirLoopControlNum)).AirLoopControllerIndex =
                AirLoopControlNum;
        }
        // When using controllers, size air loop coils so ControllerProps (e.g., Min/Max Actuated) can be set
        if (PrimaryAirSystems(AirLoopNum).NumControllers > 0) {
            SimAirLoopComponents(state, AirLoopNum, FirstHVACIteration);
        }
        PrimaryAirSystems(AirLoopNum).SizeAirloopCoil = false;
    }

    // This call to ManageControllers reinitializes the controllers actuated variables to zero
    // E.g., actuator inlet water flow
    for (int AirLoopControlNum = 1; AirLoopControlNum <= PrimaryAirSystems(AirLoopNum).NumControllers; ++AirLoopControlNum) {

        // BypassOAController is true here since we do not want to simulate the controller if it has already been simulated in the OA
        // system ControllerConvergedFlag is returned true here for water coils in OA system
        ManageControllers(state,
                          PrimaryAirSystems(AirLoopNum).ControllerName(AirLoopControlNum),
                          PrimaryAirSystems(AirLoopNum).ControllerIndex(AirLoopControlNum),
                          FirstHVACIteration,
                          AirLoopNum,
                          ControllerOperation::ColdStart,
                          ControllerConvergedFlag,
                          IsUpToDateFlag,
                          state.dataSimAirServingZones->BypassOAControllerSALC,
                          AllowWarmRestartFlag);
        // Detect whether the speculative warm restart feature is supported by each controller
        // on this air loop.
        AirLoopControlInfo(AirLoopNum).AllowWarmRestartFlag = AirLoopControlInfo(AirLoopNum).AllowWarmRestartFlag && AllowWarmRestartFlag;
    }

    // Evaluate air loop components with new actuated variables
    ++NumCalls;
    SimAirLoopComponents(state, AirLoopNum, FirstHVACIteration);
    IsUpToDateFlag = true;

    // Loop over the air sys controllers until convergence or MaxIter iterations
    for (int AirLoopControlNum = 1; AirLoopControlNum <= PrimaryAirSystems(AirLoopNum).NumControllers; ++AirLoopControlNum) {

        state.dataSimAirServingZones->IterSALC = 0;
        ControllerConvergedFlag = false;
        // if the controller can be locked out by the economizer operation and the economizer is active, leave the controller inactive
        if (AirLoopControlInfo(AirLoopNum).EconoActive) {
            // nesting this next if to try and speed this up. If economizer is not active, it doesn't matter if CanBeLockedOutByEcono =
            // true
            if (PrimaryAirSystems(AirLoopNum).CanBeLockedOutByEcono(AirLoopControlNum)) {
                ControllerConvergedFlag = true;
                continue;
            }
        }

        // For each controller in sequence, iterate until convergence
        while (!ControllerConvergedFlag) {

            ++state.dataSimAirServingZones->IterSALC;

            ManageControllers(state,
                              PrimaryAirSystems(AirLoopNum).ControllerName(AirLoopControlNum),
                              PrimaryAirSystems(AirLoopNum).ControllerIndex(AirLoopControlNum),
                              FirstHVACIteration,
                              AirLoopNum,
                              ControllerOperation::Iterate,
                              ControllerConvergedFlag,
                              IsUpToDateFlag,
                              state.dataSimAirServingZones->BypassOAControllerSALC);

            PrimaryAirSystems(AirLoopNum).ControlConverged(AirLoopControlNum) = ControllerConvergedFlag;

            if (!ControllerConvergedFlag) {
                // Only check abnormal termination if not yet converged
                // The iteration counter has been exceeded.
                if (state.dataSimAirServingZones->IterSALC > MaxIter) {
                    // Indicate that this air loop is not converged
                    AirLoopConvergedFlag = false;

                    // The warning message will be suppressed during the warm up days.
                    if (!state.dataGlobal->WarmupFlag) {
                        ++state.dataSimAirServingZones->ErrCountSALC;
                        if (state.dataSimAirServingZones->ErrCountSALC < 15) {
                            state.dataSimAirServingZones->ErrEnvironmentName = state.dataEnvrn->EnvironmentName;
                            const std::string CharErrOut = fmt::to_string(MaxIter);
                            ShowWarningError(state,
                                             format("SolveAirLoopControllers: Maximum iterations ({}) exceeded for {}, {}, at {}, {} {}",
                                                    CharErrOut,
                                                    PrimaryAirSystems(AirLoopNum).Name,
                                                    PrimaryAirSystems(AirLoopNum).ControllerName(AirLoopControlNum),
                                                    state.dataEnvrn->EnvironmentName,
                                                    state.dataEnvrn->CurMnDy,
                                                    CreateSysTimeIntervalString(state)));
                        } else {
                            if (state.dataEnvrn->EnvironmentName != state.dataSimAirServingZones->ErrEnvironmentName) {
                                state.dataSimAirServingZones->MaxErrCountSALC = 0;
                                state.dataSimAirServingZones->ErrEnvironmentName = state.dataEnvrn->EnvironmentName;
                            }
                            ShowRecurringWarningErrorAtEnd(state,
                                                           "SolveAirLoopControllers: Exceeding Maximum iterations for " +
                                                               PrimaryAirSystems(AirLoopNum).Name + " during " + state.dataEnvrn->EnvironmentName +
                                                               " continues",
                                                           state.dataSimAirServingZones->MaxErrCountSALC);
                        }
                    }

                    // It is necessary to execute this statement anytime, even if the warning message is suppressed.
                    // To continue the simulation it must be able to goto the Exit statement
                    break; // It will not converge this time
                }

                // Re-evaluate air loop components with new actuated variables
                ++NumCalls;
                // this call to SimAirLoopComponents will simulate the OA system and set the PrimaryAirSystem( AirLoopNum
                // ).ControlConverged( AirLoopControlNum ) flag for controllers of water coils in the OA system for controllers not in the
                // OA system, this flag is set above in this function
                SimAirLoopComponents(state, AirLoopNum, FirstHVACIteration);
                // pass convergence flag from OA system water coils (i.e., SolveWaterCoilController) back to this loop
                // for future reference, the PrimaryAirSystem().ControlConverged flag is set while managing OA system water coils.
                // If convergence is not achieved with OA system water coils, suspect how this flag is passed back here or why OA system
                // coils do not converge
                ControllerConvergedFlag = PrimaryAirSystems(AirLoopNum).ControlConverged(AirLoopControlNum);
                IsUpToDateFlag = true;
            }

        } // End of the Convergence Iteration

        // Update tracker for max iteration counter across all controllers on this air loops
        IterMax = max(IterMax, state.dataSimAirServingZones->IterSALC);
        // Update tracker for aggregated counter of air loop inner iterations across controllers
        // on this air loop
        IterTot += state.dataSimAirServingZones->IterSALC;

    } // End of controller loop

    // Once the controllers are converged then need to simulate the components once
    // more to ensure that they are simulated with the latest values.
    if (!IsUpToDateFlag || !AirLoopConvergedFlag) {
        ++NumCalls;
        SimAirLoopComponents(state, AirLoopNum, FirstHVACIteration);
        IsUpToDateFlag = true;
    }

    // Check that all active controllers are still convergence
    for (int AirLoopControlNum = 1; AirLoopControlNum <= PrimaryAirSystems(AirLoopNum).NumControllers; ++AirLoopControlNum) {

        ControllerConvergedFlag = false;

        ManageControllers(state,
                          PrimaryAirSystems(AirLoopNum).ControllerName(AirLoopControlNum),
                          PrimaryAirSystems(AirLoopNum).ControllerIndex(AirLoopControlNum),
                          FirstHVACIteration,
                          AirLoopNum,
                          ControllerOperation::End,
                          ControllerConvergedFlag,
                          IsUpToDateFlag,
                          state.dataSimAirServingZones->BypassOAControllerSALC);

        PrimaryAirSystems(AirLoopNum).ControlConverged(AirLoopControlNum) = ControllerConvergedFlag;

        AirLoopConvergedFlag = AirLoopConvergedFlag && ControllerConvergedFlag;
    }
}

void SolveWaterCoilController(EnergyPlusData &state,
                              bool const FirstHVACIteration,
                              int const AirLoopNum,
                              std::string const &CompName,
                              int &CompIndex,
                              std::string const &ControllerName,
                              int ControllerIndex,
                              bool const HXAssistedWaterCoil)
{

    // SUBROUTINE INFORMATION
    //       AUTHOR:  Richard Raustad (FSEC)
    //       DATE WRITTEN:  July 2017

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine solves for the controllers in the specfied air loop OA system.

    // METHODOLOGY EMPLOYED:
    // For the specified primary air system:
    // (1) the specific component in the OA system is simulated
    // (2) The controllers and their actions are simulated
    // (3) Steps 2 and 3 are repeated until the control criteria are satisfied
    // (4) convergence is passed back to SolveAirLoopControllers via PrimaryAirSystem( AirLoopNum ).ControlConverged( ControllerIndex )

    // REFERENCES: None

    // Using/Aliasing
    using namespace DataHVACControllers;
    using General::CreateSysTimeIntervalString;
    using HVACControllers::ManageControllers;
    using HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil;
    using WaterCoils::SimulateWaterCoilComponents;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:
    // TRUE if first full HVAC iteration in an HVAC timestep
    // DO loop index; there are 2 passes the 2nd is done only if mass balance fails
    // Index of the air loop being simulated
    // TRUE when primary air system & controllers simulation has converged;
    // Max number of iterations performed by controllers across this air loop
    // Aggregated number of iterations across all controllers on this air loop
    // Total number of times SimAirLoopComponents() has been invoked

    // SUBROUTINE PARAMETER DEFINITIONS:
    // Maximum iterations of an air system/controllers simulation sequence
    constexpr int MaxIter(50);

    // SUBROUTINE LOCAL VARIABLE DEFINITIONS
    // TRUE if controller supports speculative warm restart
    bool AllowWarmRestartFlag;
    // TRUE when controller has converged
    bool ControllerConvergedFlag;
    // TRUE when air loop has been evaluated with latest actuated variables
    bool IsUpToDateFlag;

    // A character string equivalent of ErrCount

    auto &PrimaryAirSystems = state.dataAirSystemsData->PrimaryAirSystems;
    auto &AirLoopControlInfo = state.dataAirLoop->AirLoopControlInfo;

    bool AirLoopCheck = false;
    if (AirLoopNum > 0) {
        AirLoopCheck = true;
    }
    state.dataSimAirServingZones->BypassOAControllerSWCC = false; // simulate OA water coil controllers
    if (AirLoopCheck) {
        state.dataSimAirServingZones->AirLoopPassSWCC = AirLoopControlInfo(AirLoopNum).AirLoopPass;
    }
    IsUpToDateFlag = false;
    if (AirLoopCheck) {
        PrimaryAirSystems(AirLoopNum).ControlConverged = false;
    }

    AllowWarmRestartFlag = true;
    if (AirLoopCheck) {
        AirLoopControlInfo(AirLoopNum).AllowWarmRestartFlag = true;
    }

    // This call to ManageControllers reinitializes the controllers actuated variables to zero

    // BypassOAController is false here since we want to simulate the controller
    ManageControllers(state,
                      ControllerName,
                      ControllerIndex,
                      FirstHVACIteration,
                      AirLoopNum,
                      ControllerOperation::ColdStart,
                      ControllerConvergedFlag,
                      IsUpToDateFlag,
                      state.dataSimAirServingZones->BypassOAControllerSWCC,
                      AllowWarmRestartFlag);

    // Detect whether the speculative warm restart feature is supported by each controller on this air loop.
    if (AirLoopCheck) {
        AirLoopControlInfo(AirLoopNum).AllowWarmRestartFlag = AirLoopControlInfo(AirLoopNum).AllowWarmRestartFlag && AllowWarmRestartFlag;
    }

    // Evaluate water coils with new actuated variables
    if (HXAssistedWaterCoil) {
        SimHXAssistedCoolingCoil(state, CompName, FirstHVACIteration, HVAC::CompressorOp::On, 0.0, CompIndex, HVAC::FanOp::Continuous);
    } else {
        SimulateWaterCoilComponents(state, CompName, FirstHVACIteration, CompIndex);
    }
    IsUpToDateFlag = true;

    // Loop over the air sys controllers until convergence or MaxIter iterations
    state.dataSimAirServingZones->IterSWCC = 0;
    ControllerConvergedFlag = false;
    // if the controller can be locked out by the economizer operation and the economizer is active, leave the controller inactive
    if (AirLoopCheck) {
        if (AirLoopControlInfo(AirLoopNum).EconoActive) {
            if (PrimaryAirSystems(AirLoopNum)
                    .CanBeLockedOutByEcono(state.dataHVACControllers->ControllerProps(ControllerIndex).AirLoopControllerIndex)) {
                ControllerConvergedFlag = true;
            }
        }
    }

    // For this controller, iterate until convergence
    while (!ControllerConvergedFlag) {

        ++state.dataSimAirServingZones->IterSWCC;

        ManageControllers(state,
                          ControllerName,
                          ControllerIndex,
                          FirstHVACIteration,
                          AirLoopNum,
                          ControllerOperation::Iterate,
                          ControllerConvergedFlag,
                          IsUpToDateFlag,
                          state.dataSimAirServingZones->BypassOAControllerSWCC);

        if (AirLoopCheck) {
            PrimaryAirSystems(AirLoopNum).ControlConverged(state.dataHVACControllers->ControllerProps(ControllerIndex).AirLoopControllerIndex) =
                ControllerConvergedFlag;
        }

        if (!ControllerConvergedFlag) {
            // Only check abnormal termination if not yet converged
            // The iteration counter has been exceeded.
            if (state.dataSimAirServingZones->IterSWCC > MaxIter) {

                // The warning message will be suppressed during the warm up days.
                if (!state.dataGlobal->WarmupFlag) {
                    ++state.dataSimAirServingZones->ErrCountSWCC;
                    if (state.dataSimAirServingZones->ErrCountSWCC < 15) {
                        state.dataSimAirServingZones->ErrEnvironmentNameSolveWaterCoilController = state.dataEnvrn->EnvironmentName;
                        const std::string CharErrOut = fmt::to_string(MaxIter);
                        ShowWarningError(state,
                                         format("SolveAirLoopControllers: Maximum iterations ({}) exceeded for {}:{}, at {}, {} {}",
                                                CharErrOut,
                                                PrimaryAirSystems(AirLoopNum).Name,
                                                ControllerName,
                                                state.dataEnvrn->EnvironmentName,
                                                state.dataEnvrn->CurMnDy,
                                                CreateSysTimeIntervalString(state)));
                    } else {
                        if (state.dataEnvrn->EnvironmentName != state.dataSimAirServingZones->ErrEnvironmentNameSolveWaterCoilController) {
                            state.dataSimAirServingZones->MaxErrCountSWCC = 0;
                            state.dataSimAirServingZones->ErrEnvironmentNameSolveWaterCoilController = state.dataEnvrn->EnvironmentName;
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       "SolveAirLoopControllers: Exceeding Maximum iterations for " +
                                                           PrimaryAirSystems(AirLoopNum).Name + " during " + state.dataEnvrn->EnvironmentName +
                                                           " continues",
                                                       state.dataSimAirServingZones->MaxErrCountSWCC);
                    }
                }

                // It is necessary to execute this statement anytime, even if the warning message is suppressed.
                // To continue the simulation it must be able to goto the Exit statement
                break; // It will not converge this time
            }

            // Re-evaluate air loop components with new actuated variables
            if (HXAssistedWaterCoil) {
                SimHXAssistedCoolingCoil(state, CompName, FirstHVACIteration, HVAC::CompressorOp::On, 0.0, CompIndex, HVAC::FanOp::Continuous);
            } else {
                SimulateWaterCoilComponents(state, CompName, FirstHVACIteration, CompIndex);
            }
            IsUpToDateFlag = true;
        }

    } // End of the Convergence Iteration

    IsUpToDateFlag = true;

    // Check that this controller is still converged

    ControllerConvergedFlag = false;

    ManageControllers(state,
                      ControllerName,
                      ControllerIndex,
                      FirstHVACIteration,
                      AirLoopNum,
                      ControllerOperation::End,
                      ControllerConvergedFlag,
                      IsUpToDateFlag,
                      state.dataSimAirServingZones->BypassOAControllerSWCC);

    // pass convergence of OA system water coils back to SolveAirLoopControllers via PrimaryAirSystem().ControlConverged flag
    if (AirLoopCheck) {
        PrimaryAirSystems(AirLoopNum).ControlConverged(state.dataHVACControllers->ControllerProps(ControllerIndex).AirLoopControllerIndex) =
            ControllerConvergedFlag;
        AirLoopControlInfo(AirLoopNum).ConvergedFlag = AirLoopControlInfo(AirLoopNum).ConvergedFlag && ControllerConvergedFlag;
    }
}

void ReSolveAirLoopControllers(
    EnergyPlusData &state, bool const FirstHVACIteration, int const AirLoopNum, bool &AirLoopConvergedFlag, int &IterMax, int &IterTot, int &NumCalls)
{

    // SUBROUTINE INFORMATION
    //             AUTHOR:  Dimitri Curtil (LBNL)
    //       DATE WRITTEN:  Feb 2006
    //      RE-ENGINEERED:  This is new code

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine solves for the controllers on the specfied air loop by reusing
    // the solution from the previous HVAC iteration.
    // It is used in the context of the optimization technique referred to as
    // speculative warm restart.

    // METHODOLOGY EMPLOYED:
    // For the specified primary air system:
    // (1) each component in the system is simulated in natural order, beginning at
    //     the return air inlet and progressing to the supply air outlets. Node data
    //     is passed in the same direction.
    // (2) The controllers and their actions are simulated.

    // REFERENCES: None

    // Using/Aliasing
    using namespace DataHVACControllers;
    using HVACControllers::ManageControllers;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:
    // TRUE if first full HVAC iteration in an HVAC timestep
    // DO loop index; there are 2 passes the 2nd is done only if mass balance fails
    // TRUE when primary air system & controllers simulation has converged;
    // Max number of iterations performed by controllers across all air loops
    // Aggregated number of iterations across all air loops
    // Total number of times SimAirLoopComponents() has been invoked

    // SUBROUTINE LOCAL VARIABLE DEFINITIONS
    // Controller DO loop index
    int AirLoopControlNum;
    // TRUE when controller has converged
    bool ControllerConvergedFlag;
    // TRUE when air loop needs to be refreshed.
    // Note that it is not used by ManageControllers() in the WARM_RESTART mode.
    bool IsUpToDateFlag;

    auto &primaryAirSystems = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum);

    // To track number of calls to SimAirLoopComponents() for each air loop
    // Represents the most computationally expensive operation in the iteration.
    // Best metric to use to assess the runtime performance of air loop simulation
    NumCalls = 0;
    IterMax = 0;
    IterTot = 0;

    AirLoopConvergedFlag = true;
    state.dataSimAirServingZones->BypassOAControllerRSALC =
        false; // not exactly sure of this but it seems all controllers need to be simulated -- don't bypass
    IsUpToDateFlag = false;
    primaryAirSystems.ControlConverged = false;

    // This call to ManageControllers reinitializes the controllers actuated variables to zero
    // E.g., actuator inlet water flow
    for (AirLoopControlNum = 1; AirLoopControlNum <= primaryAirSystems.NumControllers; ++AirLoopControlNum) {

        // BypassOAController is false here since we want to simulate the controller during ReSolveAirLoopControllers calls ?
        ManageControllers(state,
                          primaryAirSystems.ControllerName(AirLoopControlNum),
                          primaryAirSystems.ControllerIndex(AirLoopControlNum),
                          FirstHVACIteration,
                          AirLoopNum,
                          ControllerOperation::WarmRestart,
                          ControllerConvergedFlag,
                          IsUpToDateFlag,
                          state.dataSimAirServingZones->BypassOAControllerRSALC);
    }

    // Evaluate air loop components with new actuated variables
    ++NumCalls;
    SimAirLoopComponents(state, AirLoopNum, FirstHVACIteration);
    IsUpToDateFlag = true;

    // Check that all active controllers are still convergence
    // Check that actuated variables are within min/max constraints
    for (AirLoopControlNum = 1; AirLoopControlNum <= primaryAirSystems.NumControllers; ++AirLoopControlNum) {

        ControllerConvergedFlag = false;

        ManageControllers(state,
                          primaryAirSystems.ControllerName(AirLoopControlNum),
                          primaryAirSystems.ControllerIndex(AirLoopControlNum),
                          FirstHVACIteration,
                          AirLoopNum,
                          ControllerOperation::End,
                          ControllerConvergedFlag,
                          IsUpToDateFlag,
                          state.dataSimAirServingZones->BypassOAControllerRSALC);

        primaryAirSystems.ControlConverged(AirLoopControlNum) = ControllerConvergedFlag;

        AirLoopConvergedFlag = AirLoopConvergedFlag && ControllerConvergedFlag;

        // Update tracker for max iteration counter across all controllers on all air loops
        IterMax = max(IterMax, 0);
        // Update tracker for aggregated counter of air loop inner iterations across all controllers
        IterTot += 0;

    } // end of controller loop
}

void SimAirLoopComponents(EnergyPlusData &state,
                          int const AirLoopNum,         // Index of the air loop being currently simulated
                          bool const FirstHVACIteration // TRUE if first full HVAC iteration in an HVAC timestep
)
{
    // SUBROUTINE INFORMATION
    //             AUTHOR:  Dimitri Curtil (LBNL)
    //       DATE WRITTEN:  Feb 2006

    // PURPOSE OF THIS SUBROUTINE:
    // This simulates all components on a particular air loop in the primary air system.
    // This code used to appear in different places in SimAirLoops(). Now consolidated
    // into one subroutine called many times.

    // METHODOLOGY EMPLOYED:
    // For each branch in the air loop:
    // (1) update branch connection with (BeforeBranchSim)
    // (2) simulate each component
    // (3) update branch connection with (AfterBranchSim) to enforce continuity through splitter
    // Sets current branch number to CurBranchNum defined in MODULE DataSizing
    // Sets duct type of current branch to CurDuctType defined in MODULE DataSizing
    // Upon exiting, resets both counters to 0.

    auto &primaryAirSystems = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum);

    for (int BranchNum = 1; BranchNum <= primaryAirSystems.NumBranches; ++BranchNum) { // loop over all branches in air system

        UpdateBranchConnections(state, AirLoopNum, BranchNum, BeforeBranchSim);

        state.dataSize->CurBranchNum = BranchNum;
        state.dataSize->CurDuctType = primaryAirSystems.Branch(BranchNum).DuctType;

        // Loop over components in branch
        for (int CompNum = 1; CompNum <= primaryAirSystems.Branch(BranchNum).TotalComponents; ++CompNum) {
            // CompType = PrimaryAirSystem( AirLoopNum ).Branch( BranchNum ).Comp( CompNum ).TypeOf;
            // CompName = PrimaryAirSystem( AirLoopNum ).Branch( BranchNum ).Comp( CompNum ).Name;
            CompType CompType_Num = primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num;

            // Simulate each component on PrimaryAirSystem(AirLoopNum)%Branch(BranchNum)%Name
            SimAirLoopComponent(state,
                                primaryAirSystems.Branch(BranchNum).Comp(CompNum).Name,
                                CompType_Num,
                                FirstHVACIteration,
                                AirLoopNum,
                                primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompIndex,
                                primaryAirSystems.Branch(BranchNum).Comp(CompNum).compPointer,
                                AirLoopNum,
                                BranchNum,
                                CompNum);
        } // End of component loop

        // Enforce continuity through the splitter
        UpdateBranchConnections(state, AirLoopNum, BranchNum, AfterBranchSim);

    } // End of branch loop

    state.dataSize->CurBranchNum = 0;
    state.dataSize->CurDuctType = HVAC::AirDuctType::Invalid;
}

void SimAirLoopComponent(EnergyPlusData &state,
                         std::string const &CompName,   // the component Name
                         CompType const CompType_Num,   // numeric equivalent for component type
                         bool const FirstHVACIteration, // TRUE if first full HVAC iteration in an HVAC timestep
                         int const AirLoopNum,          // Primary air loop number
                         int &CompIndex,                // numeric pointer for CompType/CompName -- passed back from other routines
                         HVACSystemData *CompPointer,   // equipment actual pointer
                         int const airLoopNum,          // index to AirloopHVAC
                         int const branchNum,           // index to AirloopHVAC branch
                         int const compNum              // index to AirloopHVAC branch component
)
{

    // SUBROUTINE INFORMATION
    //             AUTHOR:  Russ Taylor, Dan Fisher, Fred Buhl
    //       DATE WRITTEN:  Oct 1997
    //           MODIFIED:  Dec 1997 Fred Buhl, Richard Raustad,FSEC Sept 2003
    //      RE-ENGINEERED:  This is new code, not reengineered

    // PURPOSE OF THIS SUBROUTINE:
    // Calls the individual air loop component simulation routines

    // METHODOLOGY EMPLOYED: None

    // REFERENCES: None

    // USE Statements
    // Using/Aliasing
    using DesiccantDehumidifiers::SimDesiccantDehumidifier;
    using EvaporativeCoolers::SimEvapCooler;
    using Furnaces::SimFurnace;
    using HeatingCoils::SimulateHeatingCoilComponents;
    using HeatRecovery::SimHeatRecovery;
    using Humidifiers::SimHumidifier;
    using HVACDuct::SimDuct;
    using HVACDXHeatPumpSystem::SimDXHeatPumpSystem;
    using HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil;
    using HVACMultiSpeedHeatPump::SimMSHeatPump;
    using HVACUnitaryBypassVAV::SimUnitaryBypassVAV;
    using MixedAir::ManageOutsideAirSystem;
    using SteamCoils::SimulateSteamCoilComponents;
    using UserDefinedComponents::SimCoilUserDefined;
    using WaterCoils::SimulateWaterCoilComponents;

    // SUBROUTINE LOCAL VARIABLE DEFINITIONS:
    Real64 QActual;
    int OAUnitNum = 0;           // used only for UnitarySystem call
    Real64 OAUCoilOutTemp = 0.0; // used only for UnitarySystem call
    bool ZoneEquipFlag = false;  // used only for UnitarySystem call
    bool CoolingActive = false;
    bool HeatingActive = false;

    auto &airLoopControlInfo = state.dataAirLoop->AirLoopControlInfo(AirLoopNum);

    switch (CompType_Num) {
    case CompType::OAMixer_Num: { // 'OUTSIDE AIR SYSTEM'
        ManageOutsideAirSystem(state, CompName, FirstHVACIteration, AirLoopNum, CompIndex);
        // Fan Types for the air sys simulation
    } break;
    case CompType::Fan_Simple_CV:        // 'Fan:ConstantVolume'
    case CompType::Fan_Simple_VAV:       // 'Fan:VariableVolume'
    case CompType::Fan_ComponentModel: { // 'Fan:ComponentModel'
        state.dataFans->fans(CompIndex)->simulate(state, FirstHVACIteration);
    } break;

    case CompType::Fan_System_Object: { // "Fan:SystemModel" new for V8.6
        // if the fan is here, it can't (yet) really be cycling fan operation, set this ugly global in the event that there are dx coils
        // involved but the fan should really run like constant volume and not cycle with compressor
        state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
        state.dataFans->fans(CompIndex)->simulate(state, FirstHVACIteration, _, _); // vector is 0 based, but CompIndex is 1 based so shift
    } break;

    case CompType::WaterCoil_CoolingHXAsst: { // 'CoilSystem:Cooling:Water:HeatExchangerAssisted'
        SimHXAssistedCoolingCoil(state,
                                 CompName,
                                 FirstHVACIteration,
                                 HVAC::CompressorOp::On,
                                 DataPrecisionGlobals::constant_zero,
                                 CompIndex,
                                 HVAC::FanOp::Continuous,
                                 _,
                                 _,
                                 _,
                                 QActual);
        if (QActual > 0.0) {
            CoolingActive = true; // determine if coil is ON
        }
    } break;
    case CompType::WaterCoil_SimpleHeat: { // 'Coil:Heating:Water'
        SimulateWaterCoilComponents(state, CompName, FirstHVACIteration, CompIndex, QActual);
        if (QActual > 0.0) {
            HeatingActive = true; // determine if coil is ON
        }
    } break;
    case CompType::SteamCoil_AirHeat: { // 'Coil:Heating:Steam'
        SimulateSteamCoilComponents(state, CompName, FirstHVACIteration, CompIndex, DataPrecisionGlobals::constant_zero, QActual);
        if (QActual > 0.0) {
            HeatingActive = true; // determine if coil is ON
        }
    } break;
    case CompType::WaterCoil_DetailedCool: { // 'Coil:Cooling:Water:DetailedGeometry'
        SimulateWaterCoilComponents(state, CompName, FirstHVACIteration, CompIndex, QActual);
        if (QActual > 0.0) {
            CoolingActive = true; // determine if coil is ON
        }
    } break;
    case CompType::WaterCoil_Cooling: { // 'Coil:Cooling:Water'
        SimulateWaterCoilComponents(state, CompName, FirstHVACIteration, CompIndex, QActual);
        if (QActual > 0.0) {
            CoolingActive = true; // determine if coil is ON
        }
        // stand-alone coils are temperature controlled (do not pass QCoilReq in argument list, QCoilReq overrides temp SP)
    } break;
    case CompType::Coil_ElectricHeat: { // 'Coil:Heating:Electric'
        SimulateHeatingCoilComponents(state, CompName, FirstHVACIteration, _, CompIndex, QActual);
        if (QActual > 0.0) {
            HeatingActive = true; // determine if coil is ON
        }
        // stand-alone coils are temperature controlled (do not pass QCoilReq in argument list, QCoilReq overrides temp SP)
    } break;
    case CompType::Coil_GasHeat: { // 'Coil:Heating:Fuel'
        SimulateHeatingCoilComponents(state, CompName, FirstHVACIteration, _, CompIndex, QActual);
        if (QActual > 0.0) {
            HeatingActive = true; // determine if coil is ON
        }
        // stand-alone coils are temperature controlled (do not pass QCoilReq in argument list, QCoilReq overrides temp SP)
    } break;
    case CompType::Coil_DeSuperHeat: { // 'Coil:Heating:Desuperheater' - heat reclaim
        SimulateHeatingCoilComponents(state, CompName, FirstHVACIteration, _, CompIndex, QActual);
        if (QActual > 0.0) {
            HeatingActive = true; // determine if coil is ON
        }
    } break;
    case CompType::DXSystem: { // CoilSystem:Cooling:DX  old 'AirLoopHVAC:UnitaryCoolOnly'
        if (CompPointer == nullptr) {
            UnitarySystems::UnitarySys thisSys;
            CompPointer = thisSys.factory(state, HVAC::UnitarySysType::Unitary_AnyCoilType, CompName, false, 0);
            // temporary fix for saving pointer, eventually apply to UnitarySystem 25 lines down
            state.dataAirSystemsData->PrimaryAirSystems(airLoopNum).Branch(branchNum).Comp(compNum).compPointer = CompPointer;
        }
        Real64 sensOut = 0.0;
        Real64 latOut = 0.0;
        CompPointer->simulate(state,
                              CompName,
                              FirstHVACIteration,
                              AirLoopNum,
                              CompIndex,
                              HeatingActive,
                              CoolingActive,
                              OAUnitNum,
                              OAUCoilOutTemp,
                              ZoneEquipFlag,
                              sensOut,
                              latOut);

    } break;
    case CompType::DXHeatPumpSystem: { // 'CoilSystem:Heating:DX'
        SimDXHeatPumpSystem(state, CompName, FirstHVACIteration, AirLoopNum, CompIndex, _, _, QActual);
        if (QActual > 0.0) {
            HeatingActive = true; // determine if coil is ON
        }
    } break;
    case CompType::CoilUserDefined: { // Coil:UserDefined
        SimCoilUserDefined(state, CompName, CompIndex, AirLoopNum, HeatingActive, CoolingActive);
    } break;
    case CompType::UnitarySystemModel: { // 'AirLoopHVAC:UnitarySystem'
        Real64 sensOut = 0.0;
        Real64 latOut = 0.0;
        CompPointer->simulate(state,
                              CompName,
                              FirstHVACIteration,
                              AirLoopNum,
                              CompIndex,
                              HeatingActive,
                              CoolingActive,
                              OAUnitNum,
                              OAUCoilOutTemp,
                              ZoneEquipFlag,
                              sensOut,
                              latOut);
    } break;
    case CompType::CoilSystemWater: { // 'CoilSystemCooling:Water'
        if (CompPointer == nullptr) {
            UnitarySystems::UnitarySys thisSys;
            CompPointer = thisSys.factory(state, HVAC::UnitarySysType::Unitary_AnyCoilType, CompName, false, 0);
            // temporary fix for saving pointer, eventually apply to UnitarySystem 16 lines above
            state.dataAirSystemsData->PrimaryAirSystems(airLoopNum).Branch(branchNum).Comp(compNum).compPointer = CompPointer;
        }
        Real64 sensOut = 0.0;
        Real64 latOut = 0.0;
        CompPointer->simulate(state,
                              CompName,
                              FirstHVACIteration,
                              AirLoopNum,
                              CompIndex,
                              HeatingActive,
                              CoolingActive,
                              OAUnitNum,
                              OAUCoilOutTemp,
                              ZoneEquipFlag,
                              sensOut,
                              latOut);
    } break;
    case CompType::Furnace_UnitarySys_HeatOnly:
    case CompType::Furnace_UnitarySys_HeatCool: {
        // 'AirLoopHVAC:Unitary:Furnace:HeatOnly', 'AirLoopHVAC:Unitary:Furnace:HeatCool',
        // 'AirLoopHVAC:UnitaryHeatOnly', 'AirLoopHVAC:UnitaryHeatCool'
        // 'AirLoopHVAC:UnitaryHeatPump:AirToAir', 'AirLoopHVAC:UnitaryHeatPump:WaterToAir'
        SimFurnace(state, CompName, FirstHVACIteration, AirLoopNum, CompIndex);
    } break;
    case CompType::UnitarySystem_BypassVAVSys: { // 'AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass'
        SimUnitaryBypassVAV(state, CompName, FirstHVACIteration, AirLoopNum, CompIndex);
    } break;
    case CompType::UnitarySystem_MSHeatPump: { // 'AirLoopHVAC:UnitaryHeatPump:AirToAir:Multispeed'
        SimMSHeatPump(state, CompName, FirstHVACIteration, AirLoopNum, CompIndex);
        // Humidifier Types for the air system simulation
    } break;
    case CompType::Humidifier: { // 'Humidifier:Steam:Electric' and 'Humidifier:Steam:Gas'
        SimHumidifier(state, CompName, FirstHVACIteration, CompIndex);
        // Evap Cooler Types for the air system simulation
    } break;
    case CompType::EvapCooler: { // 'EvaporativeCooler:Direct:CelDekPad', 'EvaporativeCooler:Indirect:CelDekPad'
        // 'EvaporativeCooler:Indirect:WetCoil', 'EvaporativeCooler:Indirect:ResearchSpecial'
        SimEvapCooler(state, CompName, CompIndex, state.dataAirLoop->AirLoopFlow(AirLoopNum).FanPLR);
        // Desiccant Dehumidifier Types for the air system simulation
    } break;
    case CompType::Desiccant: { // 'Dehumidifier:Desiccant:NoFans', 'Dehumidifier:Desiccant:System'
        SimDesiccantDehumidifier(state, CompName, FirstHVACIteration, CompIndex);
        // Heat recovery
    } break;
    case CompType::HeatXchngr: { // 'HeatExchanger:AirToAir:FlatPlate', 'HeatExchanger:AirToAir:SensibleAndLatent'
        // 'HeatExchanger:Desiccant:BalancedFlow'
        SimHeatRecovery(state,
                        CompName,
                        FirstHVACIteration,
                        CompIndex,
                        airLoopControlInfo.fanOp,
                        state.dataAirLoop->AirLoopFlow(AirLoopNum).FanPLR,
                        _,
                        _,
                        _,
                        airLoopControlInfo.EconoActive,
                        airLoopControlInfo.HighHumCtrlActive);

        // Ducts
    } break;
    case CompType::ZoneVRFasAirLoopEquip: { // 'ZoneHVAC:TerminalUnit:VariableRefrigerantFlow'
        int ControlledZoneNum = 0;
        int constexpr OAUnitNumLocal = 0;
        Real64 constexpr OAUCoilOutTempLocal = 0.0;
        bool constexpr ZoneEquipment = false;
        Real64 sysOut = 0.0;
        Real64 latOut = 0.0;
        HVACVariableRefrigerantFlow::SimulateVRF(state,
                                                 CompName,
                                                 FirstHVACIteration,
                                                 ControlledZoneNum,
                                                 CompIndex,
                                                 HeatingActive,
                                                 CoolingActive,
                                                 OAUnitNumLocal,
                                                 OAUCoilOutTempLocal,
                                                 ZoneEquipment,
                                                 sysOut,
                                                 latOut);

    } break;
    case CompType::Duct: { // 'Duct'
        SimDuct(state, CompName, FirstHVACIteration, CompIndex);
    } break;
    default:
        break;
    }

    // Set AirLoopControlInfo flag to identify coil operation for "Air Loop Coils"
    // Any coil operation from multiple coils causes flag to be TRUE
    // Flag is reset at beginning of each iteration (Subroutine SimHVAC)
    airLoopControlInfo.CoolingActiveFlag = airLoopControlInfo.CoolingActiveFlag || CoolingActive;
    airLoopControlInfo.HeatingActiveFlag = airLoopControlInfo.HeatingActiveFlag || HeatingActive;
}

void UpdateBranchConnections(EnergyPlusData &state,
                             int const AirLoopNum, // primary air system number
                             int const BranchNum,  // branch reference number
                             int const Update      // 1=BeforeBranchSim; 2=AfterBranchSim
)
{

    // SUBROUTINE INFORMATION
    //             AUTHOR:  Fred Buhl
    //       DATE WRITTEN:  Nov 1999
    //           MODIFIED:
    //      RE-ENGINEERED:  This is new code, not reengineered

    // PURPOSE OF THIS SUBROUTINE:
    // This routine passes node data from a branch exit node through a
    // splitter.

    // METHODOLOGY EMPLOYED:
    // Temperature, humidity ratio, and enthalpy are passed through from
    // the inlet to the outlets. The mass flow is divided among the outlets
    // according to the required mass flows established by the zone equipment
    // simulation. The required mass flows are were stored in the node data
    // as MassFlowRateSetPoints in the InitAirLoops routine.

    // Using/Aliasing
    using Psychrometrics::PsyTdbFnHW;

    int OutletNum;                  // splitter outlet DO loop index
    int InletNum;                   // mixer inlet DO loop index
    int InletNodeNum;               // node number of splitter inlet node
    int OutletNodeNum;              // node number of a splitter outlet node
    int RABNodeNum;                 // splitter outlet RAB node
    int NonRABNodeNum;              // splitter outlet nonRAB node
    Real64 MassFlowRateSetSum;      // sum of mass flow rate setpoints for splitter outlet nodes
    Real64 MassFlowRateOut;         // outlet mass flow rate of mixer
    Real64 MassFlowRateMinAvailOut; // outlet minimum available mass flow rate
    Real64 OutletHumRat;            // outlet humidity ratio of mixer
    Real64 OutletEnthalpy;          // outlet enthalpy of mixer
    Real64 OutletPress;
    Real64 OutletCO2; // outlet CO2 of mixer
    Real64 OutletGC;  // outlet generic contaminant of mixer
    MassFlowRateSetSum = 0.0;
    MassFlowRateOut = 0.0;
    MassFlowRateMinAvailOut = 0.0;
    OutletHumRat = 0.0;
    OutletEnthalpy = 0.0;
    OutletPress = 0.0;
    RABNodeNum = 0;
    NonRABNodeNum = 0;
    OutletCO2 = 0.0;
    OutletGC = 0.0;

    auto &PrimaryAirSystems = state.dataAirSystemsData->PrimaryAirSystems;
    auto &AirLoopControlInfo = state.dataAirLoop->AirLoopControlInfo;

    if (PrimaryAirSystems(AirLoopNum).Splitter.Exists && Update == AfterBranchSim) {
        // if we are at an inlet branch, pass data through the splitter
        if (PrimaryAirSystems(AirLoopNum).Splitter.BranchNumIn == BranchNum) {
            InletNodeNum = PrimaryAirSystems(AirLoopNum).Splitter.NodeNumIn;
            // Pass node data through the splitter
            for (OutletNum = 1; OutletNum <= PrimaryAirSystems(AirLoopNum).Splitter.TotalOutletNodes; ++OutletNum) {
                OutletNodeNum = PrimaryAirSystems(AirLoopNum).Splitter.NodeNumOut(OutletNum);
                state.dataLoopNodes->Node(OutletNodeNum).Temp = state.dataLoopNodes->Node(InletNodeNum).Temp;
                state.dataLoopNodes->Node(OutletNodeNum).HumRat = state.dataLoopNodes->Node(InletNodeNum).HumRat;
                state.dataLoopNodes->Node(OutletNodeNum).Enthalpy = state.dataLoopNodes->Node(InletNodeNum).Enthalpy;
                state.dataLoopNodes->Node(OutletNodeNum).Press = state.dataLoopNodes->Node(InletNodeNum).Press;
                MassFlowRateSetSum +=
                    min(state.dataLoopNodes->Node(OutletNodeNum).MassFlowRateSetPoint, state.dataLoopNodes->Node(OutletNodeNum).MassFlowRateMaxAvail);
                if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                    state.dataLoopNodes->Node(OutletNodeNum).CO2 = state.dataLoopNodes->Node(InletNodeNum).CO2;
                }
                if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                    state.dataLoopNodes->Node(OutletNodeNum).GenContam = state.dataLoopNodes->Node(InletNodeNum).GenContam;
                }
            }
            if (!PrimaryAirSystems(AirLoopNum).RABExists) {
                // set the outlet mass flows
                for (OutletNum = 1; OutletNum <= PrimaryAirSystems(AirLoopNum).Splitter.TotalOutletNodes; ++OutletNum) {
                    OutletNodeNum = PrimaryAirSystems(AirLoopNum).Splitter.NodeNumOut(OutletNum);
                    if (MassFlowRateSetSum < HVAC::SmallMassFlow || state.dataLoopNodes->Node(InletNodeNum).MassFlowRate < HVAC::SmallMassFlow) {
                        state.dataLoopNodes->Node(OutletNodeNum).MassFlowRate = 0.0;
                    } else {
                        state.dataLoopNodes->Node(OutletNodeNum).MassFlowRate = state.dataLoopNodes->Node(InletNodeNum).MassFlowRate *
                                                                                (min(state.dataLoopNodes->Node(OutletNodeNum).MassFlowRateSetPoint,
                                                                                     state.dataLoopNodes->Node(OutletNodeNum).MassFlowRateMaxAvail) /
                                                                                 MassFlowRateSetSum);
                    }
                    state.dataLoopNodes->Node(OutletNodeNum).MassFlowRateMaxAvail = state.dataLoopNodes->Node(InletNodeNum).MassFlowRateMaxAvail;
                    state.dataLoopNodes->Node(OutletNodeNum).MassFlowRateMinAvail = 0.0;
                }
            } else { // set the RAB flow rates
                RABNodeNum = PrimaryAirSystems(AirLoopNum).RABSplitOutNode;
                NonRABNodeNum = PrimaryAirSystems(AirLoopNum).OtherSplitOutNode;
                if (AirLoopControlInfo(AirLoopNum).EconoActive) {
                    state.dataLoopNodes->Node(RABNodeNum).MassFlowRate = 0.0;
                    state.dataLoopNodes->Node(NonRABNodeNum).MassFlowRate = state.dataLoopNodes->Node(InletNodeNum).MassFlowRate;
                } else {
                    state.dataLoopNodes->Node(RABNodeNum).MassFlowRate = state.dataLoopNodes->Node(RABNodeNum).MassFlowRateSetPoint;
                    state.dataLoopNodes->Node(NonRABNodeNum).MassFlowRate =
                        state.dataLoopNodes->Node(InletNodeNum).MassFlowRate - state.dataLoopNodes->Node(RABNodeNum).MassFlowRate;
                    if (state.dataLoopNodes->Node(NonRABNodeNum).MassFlowRate <= state.dataAirLoop->AirLoopFlow(AirLoopNum).MinOutAir) {
                        state.dataLoopNodes->Node(NonRABNodeNum).MassFlowRate =
                            min(state.dataAirLoop->AirLoopFlow(AirLoopNum).MinOutAir, state.dataLoopNodes->Node(InletNodeNum).MassFlowRate);
                        state.dataLoopNodes->Node(RABNodeNum).MassFlowRate =
                            state.dataLoopNodes->Node(InletNodeNum).MassFlowRate - state.dataLoopNodes->Node(NonRABNodeNum).MassFlowRate;
                    }
                }
            }
        }
    }

    if (PrimaryAirSystems(AirLoopNum).Mixer.Exists && Update == BeforeBranchSim) {
        // if we are at a mixer outlet branch, calculate the outlet branch conditions
        if (PrimaryAirSystems(AirLoopNum).Mixer.BranchNumOut == BranchNum) {
            OutletNodeNum = PrimaryAirSystems(AirLoopNum).Mixer.NodeNumOut;
            // get the outlet mass flow rate and the outlet minavail mass flow rate
            for (InletNum = 1; InletNum <= PrimaryAirSystems(AirLoopNum).Mixer.TotalInletNodes; ++InletNum) {
                InletNodeNum = PrimaryAirSystems(AirLoopNum).Mixer.NodeNumIn(InletNum);
                MassFlowRateOut += state.dataLoopNodes->Node(InletNodeNum).MassFlowRate;
                MassFlowRateMinAvailOut += state.dataLoopNodes->Node(InletNodeNum).MassFlowRateMinAvail;
            }
            // set the outlet mass flow
            state.dataLoopNodes->Node(OutletNodeNum).MassFlowRate = MassFlowRateOut;
            state.dataLoopNodes->Node(OutletNodeNum).MassFlowRateMinAvail = MassFlowRateMinAvailOut;
            state.dataLoopNodes->Node(OutletNodeNum).MassFlowRateMaxAvail = state.dataLoopNodes->Node(OutletNodeNum).MassFlowRateMax;
            // calculate the outlet humidity ratio and enthalpy and pressure
            if (MassFlowRateOut > 0.0) {
                for (InletNum = 1; InletNum <= PrimaryAirSystems(AirLoopNum).Mixer.TotalInletNodes; ++InletNum) {
                    InletNodeNum = PrimaryAirSystems(AirLoopNum).Mixer.NodeNumIn(InletNum);
                    OutletHumRat +=
                        (state.dataLoopNodes->Node(InletNodeNum).MassFlowRate * state.dataLoopNodes->Node(InletNodeNum).HumRat) / MassFlowRateOut;
                    OutletEnthalpy +=
                        (state.dataLoopNodes->Node(InletNodeNum).MassFlowRate * state.dataLoopNodes->Node(InletNodeNum).Enthalpy) / MassFlowRateOut;
                    OutletPress +=
                        (state.dataLoopNodes->Node(InletNodeNum).MassFlowRate * state.dataLoopNodes->Node(InletNodeNum).Press) / MassFlowRateOut;
                    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        OutletCO2 +=
                            (state.dataLoopNodes->Node(InletNodeNum).MassFlowRate * state.dataLoopNodes->Node(InletNodeNum).CO2) / MassFlowRateOut;
                    }
                    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        OutletGC += (state.dataLoopNodes->Node(InletNodeNum).MassFlowRate * state.dataLoopNodes->Node(InletNodeNum).GenContam) /
                                    MassFlowRateOut;
                    }
                }
            } else {
                InletNodeNum = PrimaryAirSystems(AirLoopNum).Mixer.NodeNumIn(1);
                OutletHumRat = state.dataLoopNodes->Node(InletNodeNum).HumRat;
                OutletEnthalpy = state.dataLoopNodes->Node(InletNodeNum).Enthalpy;
                OutletPress = state.dataLoopNodes->Node(InletNodeNum).Press;
                if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                    OutletCO2 = state.dataLoopNodes->Node(InletNodeNum).CO2;
                }
                if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                    OutletGC = state.dataLoopNodes->Node(InletNodeNum).GenContam;
                }
            }
            state.dataLoopNodes->Node(OutletNodeNum).HumRat = OutletHumRat;
            state.dataLoopNodes->Node(OutletNodeNum).Enthalpy = OutletEnthalpy;
            state.dataLoopNodes->Node(OutletNodeNum).Press = OutletPress;
            // calculate the outlet temperature
            state.dataLoopNodes->Node(OutletNodeNum).Temp = PsyTdbFnHW(OutletEnthalpy, OutletHumRat);
            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                state.dataLoopNodes->Node(OutletNodeNum).CO2 = OutletCO2;
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                state.dataLoopNodes->Node(OutletNodeNum).GenContam = OutletGC;
            }
        }
    }
}

void ResolveSysFlow(EnergyPlusData &state,
                    int const SysNum, // the primary air system number
                    bool &SysReSim    // Set to TRUE if mass balance fails and resimulation is needed
)
{

    // SUBROUTINE INFORMATION
    //             AUTHOR:  Fred Buhl
    //       DATE WRITTEN:  Dec 1999
    //           MODIFIED:
    //      RE-ENGINEERED:  This is new code, not reengineered

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutines checks for mass flow balance in all air system branches
    // and across all connections. If there is a failure of mass flow
    // balance, mass flows are imposed to achieve mass flow balance and
    // the resimulate flag SysReSim is set to true.

    // METHODOLOGY EMPLOYED:
    // Node()%MassFlowRateMaxAvail for every node is set to the minimum
    // Node()%MassFlowRateMaxAvail on each branch.  Mass balance is imposed
    // at the branch connections. System inlet mass flows are forced to
    // be less than or equal to the resulting inlet MassFlowRateMaxAvails.

    int BranchNum;                 // branch DO loop index
    int NodeIndex;                 // node on branch DO loop index
    Real64 MassFlowRateOutSum;     // sum of splitter outlet mass flow rates (imposed)
    Real64 BranchMassFlowMaxAvail; // branch level maximum flow rate possible
    int OutletNum;                 // splitter outlet DO loop index
    int OutletNodeNum;             // a splitter outlet node number
    int InletNodeNum;              // splitter inlet node number
    int NodeNum;                   // a node number
    int NodeNumNext;               // node number of next node on a branch
    int InNodeNum;                 // air system inlet node
    int InBranchNum;               // air system inlet branch number
    int InBranchIndex;             // air sys inlet branch DO loop index

    auto &PrimaryAirSystems = state.dataAirSystemsData->PrimaryAirSystems;
    auto &AirLoopControlInfo = state.dataAirLoop->AirLoopControlInfo;

    // Find the minimum MassFlowMaxAvail for each branch in the system and store it on the branch inlet node.
    // Check for mass flow conservation on each branch. Set SysReSim to TRUE is mass flow not conserved.
    for (BranchNum = 1; BranchNum <= PrimaryAirSystems(SysNum).NumBranches; ++BranchNum) { // loop over branches in system
        // Initialize branch max avail mass flow to max avail mass flow at outlet node
        BranchMassFlowMaxAvail = state.dataLoopNodes->Node(PrimaryAirSystems(SysNum).Branch(BranchNum).NodeNumOut).MassFlowRateMaxAvail;
        for (NodeIndex = 1; NodeIndex <= PrimaryAirSystems(SysNum).Branch(BranchNum).TotalNodes; ++NodeIndex) { // loop over nodes on branch
            // Get the new smallest max avail mass flow
            NodeNum = PrimaryAirSystems(SysNum).Branch(BranchNum).NodeNum(NodeIndex);
            BranchMassFlowMaxAvail = min(BranchMassFlowMaxAvail, state.dataLoopNodes->Node(NodeNum).MassFlowRateMaxAvail);
            // Check for mass flow conservation on the branch
            if (NodeIndex < PrimaryAirSystems(SysNum).Branch(BranchNum).TotalNodes) {
                // Set ReSim flag to TRUE if mass flow not conserved on this branch
                NodeNumNext = PrimaryAirSystems(SysNum).Branch(BranchNum).NodeNum(NodeIndex + 1);
                if (NodeNum == PrimaryAirSystems(SysNum).OASysInletNodeNum) {
                    continue; // don't enforce mass balance across OA Sys
                }
                // Changeover bypass system connected to a plenum or mixer will need to include the bypass flow rate
                if (std::abs(state.dataLoopNodes->Node(NodeNum).MassFlowRate - state.dataLoopNodes->Node(NodeNumNext).MassFlowRate -
                             state.dataAirLoop->AirLoopFlow(SysNum).BypassMassFlow) > HVAC::SmallMassFlow) {
                    SysReSim = true;
                }
            }
        } // end node loop
        // Store the minimum MassFlowMaxAvail for this branch on the branch inlet node (AirloopHVAC supply inlet node)
        state.dataLoopNodes->Node(PrimaryAirSystems(SysNum).Branch(BranchNum).NodeNumIn).MassFlowRateMaxAvail = BranchMassFlowMaxAvail;
    } // end branch loop
    // force resimulation for fan-cycling, nonsimple systems
    if (!AirLoopControlInfo(SysNum).Simple && AirLoopControlInfo(SysNum).CyclingFan) {
        SysReSim = true;
    }

    // If mass flow conserved on each branch, check for mass balance across splitter
    if (!SysReSim && PrimaryAirSystems(SysNum).Splitter.Exists) {
        MassFlowRateOutSum = 0.0;
        InletNodeNum = PrimaryAirSystems(SysNum).Splitter.NodeNumIn;
        // Get sum of splitter outlet mass flows
        for (OutletNum = 1; OutletNum <= PrimaryAirSystems(SysNum).Splitter.TotalOutletNodes; ++OutletNum) {
            OutletNodeNum = PrimaryAirSystems(SysNum).Splitter.NodeNumOut(OutletNum);
            MassFlowRateOutSum += state.dataLoopNodes->Node(OutletNodeNum).MassFlowRate;
        }
        // Check whether sum of splitter outlet mass flows equals splitter inlet flow.
        if (std::abs(MassFlowRateOutSum - state.dataLoopNodes->Node(InletNodeNum).MassFlowRate) > HVAC::SmallMassFlow) {
            SysReSim = true;
        }
    }

    //// Resimulate if the zone air mass flow conservation convergence critreon is not met
    if (state.dataHVACGlobal->ZoneMassBalanceHVACReSim) {
        SysReSim = true;
    }

    // If mass balance failed, resimulation is needed. Impose a mass balance for the new simulation.
    if (SysReSim) {
        // Set the MassFlowRateMaxAvail on each node to the minimum MassFlowRateMaxAvail for the branch.
        for (BranchNum = 1; BranchNum <= PrimaryAirSystems(SysNum).NumBranches; ++BranchNum) {                      // loop over branches in system
            for (NodeIndex = 2; NodeIndex <= PrimaryAirSystems(SysNum).Branch(BranchNum).TotalNodes; ++NodeIndex) { // loop over nodes on branch
                NodeNum = PrimaryAirSystems(SysNum).Branch(BranchNum).NodeNum(NodeIndex);
                state.dataLoopNodes->Node(NodeNum).MassFlowRateMaxAvail =
                    state.dataLoopNodes->Node(PrimaryAirSystems(SysNum).Branch(BranchNum).NodeNumIn).MassFlowRateMaxAvail;
            }
        }

        // Impose mass balance at splitter
        if (PrimaryAirSystems(SysNum).Splitter.Exists) {
            InBranchNum = PrimaryAirSystems(SysNum).Splitter.BranchNumIn;
            MassFlowRateOutSum = 0.0;
            InletNodeNum = PrimaryAirSystems(SysNum).Splitter.NodeNumIn;
            for (OutletNum = 1; OutletNum <= PrimaryAirSystems(SysNum).Splitter.TotalOutletNodes; ++OutletNum) {
                OutletNodeNum = PrimaryAirSystems(SysNum).Splitter.NodeNumOut(OutletNum);
                MassFlowRateOutSum +=
                    min(state.dataLoopNodes->Node(OutletNodeNum).MassFlowRateMaxAvail, state.dataLoopNodes->Node(OutletNodeNum).MassFlowRateSetPoint);
            }
            // set the splitter inlet Max Avail mass flow rate
            if (state.dataLoopNodes->Node(InletNodeNum).MassFlowRateMaxAvail > MassFlowRateOutSum + HVAC::SmallMassFlow) {
                state.dataLoopNodes->Node(InletNodeNum).MassFlowRateMaxAvail = MassFlowRateOutSum;
            }
            // Pass the splitter inlet Max Avail mass flow rate upstream to the mixed air node
            for (NodeIndex = PrimaryAirSystems(SysNum).Branch(InBranchNum).TotalNodes - 1; NodeIndex >= 1; --NodeIndex) {
                NodeNum = PrimaryAirSystems(SysNum).Branch(InBranchNum).NodeNum(NodeIndex);
                state.dataLoopNodes->Node(NodeNum).MassFlowRateMaxAvail = state.dataLoopNodes->Node(InletNodeNum).MassFlowRateMaxAvail;
                if (NodeNum == PrimaryAirSystems(SysNum).OASysOutletNodeNum) {
                    break;
                }
            }
        }

        // Make sure air system inlet nodes have flow consistent with MassFlowRateMaxAvail
        for (InBranchIndex = 1; InBranchIndex <= PrimaryAirSystems(SysNum).NumInletBranches; ++InBranchIndex) {
            InBranchNum = PrimaryAirSystems(SysNum).InletBranchNum[InBranchIndex - 1];
            InNodeNum = PrimaryAirSystems(SysNum).Branch(InBranchNum).NodeNumIn;
            state.dataLoopNodes->Node(InNodeNum).MassFlowRate =
                min(state.dataLoopNodes->Node(InNodeNum).MassFlowRate, state.dataLoopNodes->Node(InNodeNum).MassFlowRateMaxAvail);
        }
    }
}

void SizeAirLoops(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   February 2001
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Will perform central air system sizing simulations. Right now just
    // initializes system sizing arrays. Calculations based on System Sizing
    // input and the Zone Sizing simulations are done in UpdateSysSizing.

    // METHODOLOGY EMPLOYED:
    // Will run purchased hot and chilled water type simulations to determine
    // central plant flow rates. Right now just uses one time flag to call
    // SetUpSysSizingArrays.

    if (state.dataSimAirServingZones->SizeAirLoopsOneTimeFlag) {
        SetUpSysSizingArrays(state);
        state.dataSimAirServingZones->SizeAirLoopsOneTimeFlag = false;
    }
}

void SizeAirLoopBranches(EnergyPlusData &state, int const AirLoopNum, int const BranchNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   September 2001
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for sizing air loop branches for which flow rates have not been
    // specified in the input.

    // METHODOLOGY EMPLOYED:
    // Obtains flow rates from the zone or system sizing arrays.

    // Using/Aliasing
    using namespace DataSizing;
    using HVACHXAssistedCoolingCoil::GetHXCoilType;
    using HVACHXAssistedCoolingCoil::GetHXDXCoilName;
    using WaterCoils::SetCoilDesFlow;

    std::string CompType; // Component type
    std::string CompName; // Component name
    std::string CoilName;
    std::string CoilType;
    std::string ScalableSM;                    // scalable sizing methods label for reporting
    SimAirServingZones::CompType CompType_Num; // Numeric equivalent for CompType
    int CompNum;
    bool ErrorsFound;

    auto &FinalSysSizing = state.dataSize->FinalSysSizing;
    auto &PrimaryAirSystems = state.dataAirSystemsData->PrimaryAirSystems;

    ErrorsFound = false;

    if (BranchNum == 1) {

        if (PrimaryAirSystems(AirLoopNum).DesignVolFlowRate == AutoSize) {
            CheckSysSizing(state, "AirLoopHVAC", PrimaryAirSystems(AirLoopNum).Name);
            PrimaryAirSystems(AirLoopNum).DesignVolFlowRate = FinalSysSizing(AirLoopNum).DesMainVolFlow;
            switch (FinalSysSizing(AirLoopNum).ScaleCoolSAFMethod) {
            case FlowPerFloorArea: {
                ScalableSM = "User-Specified(scaled by flow / area) ";
            } break;
            case FractionOfAutosizedCoolingAirflow: {
                ScalableSM = "User-Specified(scaled by fractional multiplier) ";
            } break;
            case FlowPerCoolingCapacity: {
                ScalableSM = "User-Specified(scaled by flow / capacity) ";
            } break;
            default: {
                ScalableSM = "Design ";
            } break;
            }
            BaseSizer::reportSizerOutput(state,
                                         "AirLoopHVAC",
                                         PrimaryAirSystems(AirLoopNum).Name,
                                         ScalableSM + "Supply Air Flow Rate [m3/s]",
                                         PrimaryAirSystems(AirLoopNum).DesignVolFlowRate);
            // Initialize MaxOutAir for DOAS loops with no actual OASys, systems with an OA controller will overwrite this is
            // CalcOAController
            if (PrimaryAirSystems(AirLoopNum).isAllOA) {
                state.dataAirLoop->AirLoopFlow(AirLoopNum).MaxOutAir = PrimaryAirSystems(AirLoopNum).DesignVolFlowRate * state.dataEnvrn->StdRhoAir;
            }
        }

        if (allocated(FinalSysSizing) && FinalSysSizing(AirLoopNum).SysAirMinFlowRatWasAutoSized) {
            BaseSizer::reportSizerOutput(state,
                                         "AirLoopHVAC",
                                         PrimaryAirSystems(AirLoopNum).Name,
                                         "Central Heating Maximum System Air Flow Ratio",
                                         FinalSysSizing(AirLoopNum).SysAirMinFlowRat);
        }
        if (PrimaryAirSystems(AirLoopNum).DesignVolFlowRate < HVAC::SmallAirVolFlow) {
            ShowSevereError(state,
                            format("SizeAirLoopBranches: AirLoopHVAC {} has air flow less than {:.4R} m3/s.",
                                   PrimaryAirSystems(AirLoopNum).Name,
                                   HVAC::SmallAirVolFlow));
            ShowContinueError(state,
                              format("Primary air system volumetric flow rate = {:.4R} m3/s.", PrimaryAirSystems(AirLoopNum).DesignVolFlowRate));
            ShowContinueError(state, "Check flow rate inputs for components in this air loop and,");
            ShowContinueError(state, "if autosized, check Sizing:Zone and Sizing:System objects and related inputs.");
        }
    }

    // Loop over components in branch; pass the design air flow rate to the coil components that don't have
    // design air flow as an input
    for (CompNum = 1; CompNum <= PrimaryAirSystems(AirLoopNum).Branch(BranchNum).TotalComponents; ++CompNum) {
        CompType = PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).TypeOf;
        CompName = PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).Name;
        CompType_Num = PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).CompType_Num;
        if (CompType_Num == CompType::WaterCoil_DetailedCool || CompType_Num == CompType::WaterCoil_SimpleHeat ||
            CompType_Num == CompType::WaterCoil_CoolingHXAsst) {
            if (CompType_Num == CompType::WaterCoil_CoolingHXAsst) {
                CoilName = GetHXDXCoilName(state, CompType, CompName, ErrorsFound);
                CoilType = GetHXCoilType(state, CompType, CompName, ErrorsFound);
            } else {
                CoilName = CompName;
                CoilType = CompType;
            }
            SetCoilDesFlow(state, CoilType, CoilName, PrimaryAirSystems(AirLoopNum).DesignVolFlowRate, ErrorsFound);
        }
    } // End of component loop
    if (ErrorsFound) {
        ShowFatalError(state, "Preceding sizing errors cause program termination");
    }
}

void SetUpSysSizingArrays(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   February 2001

    // PURPOSE OF THIS SUBROUTINE:
    // Allocate and fill the SysSizing data array.

    // METHODOLOGY EMPLOYED:
    // Uses data from System Sizing input and the system to zone connection data
    // calculated in InitAirLoops and stored in AirToZoneNodeInfo in DataLoopNode..

    bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine
    int numAirTerminalUnits = state.dataSize->NumAirTerminalUnits;
    int numPrimaryAirSys = state.dataHVACGlobal->NumPrimaryAirSys;
    // have moved a large number of std 62.1 variables to DataSizing.hh so they can be used outside of this routine

    // allocate arrays used to store values for standard 62.1 tabular report
    if (!allocated(state.dataSize->VpzClgByZone)) {
        state.dataSize->VdzClgByZone.dimension(numAirTerminalUnits, 0.0);
        state.dataSize->VdzMinClgByZone.dimension(numAirTerminalUnits, 0.0);
        state.dataSize->VdzHtgByZone.dimension(numAirTerminalUnits, 0.0);
        state.dataSize->VdzMinHtgByZone.dimension(numAirTerminalUnits, 0.0);
        state.dataSize->ZdzClgByZone.dimension(numAirTerminalUnits, 0.0);
        state.dataSize->ZdzHtgByZone.dimension(numAirTerminalUnits, 0.0);
        state.dataSize->VpzClgByZone.dimension(numAirTerminalUnits, 0.0);
        state.dataSize->VpzMinClgByZone.dimension(numAirTerminalUnits, 0.0);
        state.dataSize->VpzHtgByZone.dimension(numAirTerminalUnits, 0.0);
        state.dataSize->VpzMinHtgByZone.dimension(numAirTerminalUnits, 0.0);
        state.dataSize->VbzByZone.dimension(numAirTerminalUnits, 0.0);
        state.dataSize->VpzClgSumBySys.dimension(numPrimaryAirSys, 0.0);
        state.dataSize->VpzHtgSumBySys.dimension(numPrimaryAirSys, 0.0);
        state.dataSize->PzSumBySys.dimension(numPrimaryAirSys, 0.0);
        state.dataSize->PsBySys.dimension(numPrimaryAirSys, 0.0);
        state.dataSize->DBySys.dimension(numPrimaryAirSys, 0.0);
        state.dataSize->SumRpxPzBySys.dimension(numPrimaryAirSys, 0.0);
        state.dataSize->SumRaxAzBySys.dimension(numPrimaryAirSys, 0.0);
        state.dataSize->PeakPsOccurrenceDateTimeStringBySys.dimension(numPrimaryAirSys, "");
        state.dataSize->PeakPsOccurrenceEnvironmentStringBySys.dimension(numPrimaryAirSys, "");
        state.dataSize->VouBySys.dimension(numPrimaryAirSys, 0.0);
        state.dataSize->VpsClgBySys.dimension(numPrimaryAirSys, 0.0);
        state.dataSize->VpsHtgBySys.dimension(numPrimaryAirSys, 0.0);
    }

    for (int SysSizIndex = 1; SysSizIndex <= state.dataSize->NumSysSizInput; ++SysSizIndex) {
        auto &sysSizInput = state.dataSize->SysSizInput(SysSizIndex);
        sysSizInput.AirLoopNum = Util::FindItemInList(sysSizInput.AirPriLoopName, state.dataAirSystemsData->PrimaryAirSystems);
        if (sysSizInput.AirLoopNum == 0) {
            ShowSevereError(state, format("Sizing:System: {} references unknown AirLoopHVAC", sysSizInput.AirPriLoopName));
            ErrorsFound = true;
        }
    }
    if (ErrorsFound) {
        ShowFatalError(state, "Errors found in Sizing:System input");
    }

    state.dataSize->SysSizing.allocate(state.dataEnvrn->TotDesDays + state.dataEnvrn->TotRunDesPersDays, numPrimaryAirSys);
    state.dataSize->FinalSysSizing.allocate(numPrimaryAirSys);
    state.dataSize->CalcSysSizing.allocate(numPrimaryAirSys);
    state.dataSize->SysSizPeakDDNum.allocate(numPrimaryAirSys);

    for (int AirLoopNum = 1; AirLoopNum <= numPrimaryAirSys; ++AirLoopNum) {
        auto &primaryAirSystems = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum);
        int SysSizNum = Util::FindItemInList(primaryAirSystems.Name, state.dataSize->SysSizInput, &SystemSizingInputData::AirPriLoopName);
        if (SysSizNum <= 0) {
            SysSizNum = 1;
            ShowWarningError(
                state,
                format(
                    "SetUpSysSizingArrays: Sizing for System (HVACAirLoop)=\" {}\" will use Sizing:System specifications listed for System=\" {}\".",
                    primaryAirSystems.Name,
                    state.dataSize->SysSizInput(1).AirPriLoopName));
        }
        auto &sysSizInput = state.dataSize->SysSizInput(SysSizNum);
        for (int DesDayEnvrnNum = 1; DesDayEnvrnNum <= state.dataEnvrn->TotDesDays + state.dataEnvrn->TotRunDesPersDays; ++DesDayEnvrnNum) {
            auto &sysSizing = state.dataSize->SysSizing(DesDayEnvrnNum, AirLoopNum);
            // move data from system sizing input
            sysSizing.AirPriLoopName = primaryAirSystems.Name;
            sysSizing.loadSizingType = sysSizInput.loadSizingType;
            sysSizing.coolingPeakLoad = sysSizInput.coolingPeakLoad;
            sysSizing.CoolCapControl = sysSizInput.CoolCapControl;
            sysSizing.heatCoilSizingMethod = sysSizInput.heatCoilSizingMethod;
            sysSizing.maxHeatCoilToCoolingLoadSizingRatio = sysSizInput.maxHeatCoilToCoolingLoadSizingRatio;
            sysSizing.DesOutAirVolFlow = sysSizInput.DesOutAirVolFlow;
            sysSizing.SysAirMinFlowRat = sysSizInput.SysAirMinFlowRat;
            sysSizing.SysAirMinFlowRatWasAutoSized = sysSizInput.SysAirMinFlowRatWasAutoSized;
            sysSizing.PreheatTemp = sysSizInput.PreheatTemp;
            sysSizing.PreheatHumRat = sysSizInput.PreheatHumRat;
            sysSizing.PrecoolTemp = sysSizInput.PrecoolTemp;
            sysSizing.PrecoolHumRat = sysSizInput.PrecoolHumRat;
            sysSizing.CoolSupTemp = sysSizInput.CoolSupTemp;
            sysSizing.HeatSupTemp = sysSizInput.HeatSupTemp;
            sysSizing.CoolSupHumRat = sysSizInput.CoolSupHumRat;
            sysSizing.HeatSupHumRat = sysSizInput.HeatSupHumRat;
            sysSizing.SizingOption = sysSizInput.SizingOption;
            if (primaryAirSystems.isAllOA) {
                sysSizing.CoolOAOption = OAControl::AllOA;
                sysSizing.HeatOAOption = OAControl::AllOA;
            } else {
                sysSizing.CoolOAOption = sysSizInput.CoolOAOption;
                sysSizing.HeatOAOption = sysSizInput.HeatOAOption;
            }
            sysSizing.CoolAirDesMethod = sysSizInput.CoolAirDesMethod;
            sysSizing.HeatAirDesMethod = sysSizInput.HeatAirDesMethod;
            sysSizing.ScaleCoolSAFMethod = sysSizInput.ScaleCoolSAFMethod;
            sysSizing.ScaleHeatSAFMethod = sysSizInput.ScaleHeatSAFMethod;
            sysSizing.CoolingCapMethod = sysSizInput.CoolingCapMethod;
            sysSizing.HeatingCapMethod = sysSizInput.HeatingCapMethod;
            sysSizing.InpDesCoolAirFlow = sysSizInput.DesCoolAirFlow;
            sysSizing.InpDesHeatAirFlow = sysSizInput.DesHeatAirFlow;
            sysSizing.MaxZoneOAFraction = sysSizInput.MaxZoneOAFraction;
            sysSizing.OAAutoSized = sysSizInput.OAAutoSized;

            sysSizing.HeatFlowSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.SumZoneHeatLoadSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.CoolFlowSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.SumZoneCoolLoadSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.CoolZoneAvgTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.HeatZoneAvgTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.SensCoolCapSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.TotCoolCapSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.HeatCapSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.PreheatCapSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.SysCoolRetTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.SysCoolRetHumRatSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.SysHeatRetTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.SysHeatRetHumRatSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.SysCoolOutTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.SysCoolOutHumRatSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.SysHeatOutTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.SysHeatOutHumRatSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.SysDOASHeatAddSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
            sysSizing.SysDOASLatAddSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        } // end the design day loop

        auto &finalSysSizing = state.dataSize->FinalSysSizing(AirLoopNum);
        auto &calcSysSizing = state.dataSize->CalcSysSizing(AirLoopNum);
        finalSysSizing.AirPriLoopName = primaryAirSystems.Name;
        calcSysSizing.AirPriLoopName = primaryAirSystems.Name;

        // move data from system sizing input
        finalSysSizing.loadSizingType = sysSizInput.loadSizingType;
        finalSysSizing.coolingPeakLoad = sysSizInput.coolingPeakLoad;
        finalSysSizing.CoolCapControl = sysSizInput.CoolCapControl;
        finalSysSizing.heatCoilSizingMethod = sysSizInput.heatCoilSizingMethod;
        finalSysSizing.maxHeatCoilToCoolingLoadSizingRatio = sysSizInput.maxHeatCoilToCoolingLoadSizingRatio;
        finalSysSizing.DesOutAirVolFlow = sysSizInput.DesOutAirVolFlow;
        finalSysSizing.SysAirMinFlowRat = sysSizInput.SysAirMinFlowRat;
        finalSysSizing.SysAirMinFlowRatWasAutoSized = sysSizInput.SysAirMinFlowRatWasAutoSized;
        finalSysSizing.PreheatTemp = sysSizInput.PreheatTemp;
        finalSysSizing.PreheatHumRat = sysSizInput.PreheatHumRat;
        finalSysSizing.PrecoolTemp = sysSizInput.PrecoolTemp;
        finalSysSizing.PrecoolHumRat = sysSizInput.PrecoolHumRat;
        finalSysSizing.CoolSupTemp = sysSizInput.CoolSupTemp;
        finalSysSizing.HeatSupTemp = sysSizInput.HeatSupTemp;
        finalSysSizing.CoolSupHumRat = sysSizInput.CoolSupHumRat;
        finalSysSizing.HeatSupHumRat = sysSizInput.HeatSupHumRat;
        finalSysSizing.SizingOption = sysSizInput.SizingOption;
        finalSysSizing.CoolAirDesMethod = sysSizInput.CoolAirDesMethod;
        finalSysSizing.HeatAirDesMethod = sysSizInput.HeatAirDesMethod;
        finalSysSizing.ScaleCoolSAFMethod = sysSizInput.ScaleCoolSAFMethod;
        finalSysSizing.ScaleHeatSAFMethod = sysSizInput.ScaleHeatSAFMethod;
        finalSysSizing.CoolingCapMethod = sysSizInput.CoolingCapMethod;
        finalSysSizing.HeatingCapMethod = sysSizInput.HeatingCapMethod;
        finalSysSizing.ScaledCoolingCapacity = sysSizInput.ScaledCoolingCapacity;
        finalSysSizing.ScaledHeatingCapacity = sysSizInput.ScaledHeatingCapacity;
        finalSysSizing.InpDesCoolAirFlow = sysSizInput.DesCoolAirFlow;
        finalSysSizing.InpDesHeatAirFlow = sysSizInput.DesHeatAirFlow;
        finalSysSizing.SystemOAMethod = sysSizInput.SystemOAMethod;
        finalSysSizing.MaxZoneOAFraction = sysSizInput.MaxZoneOAFraction;
        finalSysSizing.OAAutoSized = sysSizInput.OAAutoSized;
        finalSysSizing.FlowPerFloorAreaCooled = sysSizInput.FlowPerFloorAreaCooled;
        finalSysSizing.FlowPerFloorAreaHeated = sysSizInput.FlowPerFloorAreaHeated;
        finalSysSizing.FractionOfAutosizedCoolingAirflow = sysSizInput.FractionOfAutosizedCoolingAirflow;
        finalSysSizing.FractionOfAutosizedHeatingAirflow = sysSizInput.FractionOfAutosizedHeatingAirflow;
        finalSysSizing.FlowPerCoolingCapacity = sysSizInput.FlowPerCoolingCapacity;
        finalSysSizing.FlowPerHeatingCapacity = sysSizInput.FlowPerHeatingCapacity;

        if (primaryAirSystems.isAllOA) {
            finalSysSizing.CoolOAOption = DataSizing::OAControl::AllOA;
            finalSysSizing.HeatOAOption = DataSizing::OAControl::AllOA;
            calcSysSizing.CoolOAOption = DataSizing::OAControl::AllOA;
            calcSysSizing.HeatOAOption = DataSizing::OAControl::AllOA;
        } else {
            finalSysSizing.CoolOAOption = sysSizInput.CoolOAOption;
            finalSysSizing.HeatOAOption = sysSizInput.HeatOAOption;
            calcSysSizing.CoolOAOption = sysSizInput.CoolOAOption;
            calcSysSizing.HeatOAOption = sysSizInput.HeatOAOption;
        }

        calcSysSizing.loadSizingType = sysSizInput.loadSizingType;
        calcSysSizing.coolingPeakLoad = sysSizInput.coolingPeakLoad;
        calcSysSizing.CoolCapControl = sysSizInput.CoolCapControl;
        calcSysSizing.heatCoilSizingMethod = sysSizInput.heatCoilSizingMethod;
        calcSysSizing.maxHeatCoilToCoolingLoadSizingRatio = sysSizInput.maxHeatCoilToCoolingLoadSizingRatio;
        calcSysSizing.DesOutAirVolFlow = sysSizInput.DesOutAirVolFlow;
        calcSysSizing.SysAirMinFlowRat = sysSizInput.SysAirMinFlowRat;
        calcSysSizing.SysAirMinFlowRatWasAutoSized = sysSizInput.SysAirMinFlowRatWasAutoSized;
        calcSysSizing.PreheatTemp = sysSizInput.PreheatTemp;
        calcSysSizing.PreheatHumRat = sysSizInput.PreheatHumRat;
        calcSysSizing.PrecoolTemp = sysSizInput.PrecoolTemp;
        calcSysSizing.PrecoolHumRat = sysSizInput.PrecoolHumRat;
        calcSysSizing.CoolSupTemp = sysSizInput.CoolSupTemp;
        calcSysSizing.HeatSupTemp = sysSizInput.HeatSupTemp;
        calcSysSizing.CoolSupHumRat = sysSizInput.CoolSupHumRat;
        calcSysSizing.HeatSupHumRat = sysSizInput.HeatSupHumRat;
        calcSysSizing.SizingOption = sysSizInput.SizingOption;
        calcSysSizing.CoolAirDesMethod = sysSizInput.CoolAirDesMethod;
        calcSysSizing.HeatAirDesMethod = sysSizInput.HeatAirDesMethod;
        calcSysSizing.ScaleCoolSAFMethod = sysSizInput.ScaleCoolSAFMethod;
        calcSysSizing.ScaleHeatSAFMethod = sysSizInput.ScaleHeatSAFMethod;
        calcSysSizing.CoolingCapMethod = sysSizInput.CoolingCapMethod;
        calcSysSizing.HeatingCapMethod = sysSizInput.HeatingCapMethod;
        calcSysSizing.ScaledCoolingCapacity = sysSizInput.ScaledCoolingCapacity;
        calcSysSizing.ScaledHeatingCapacity = sysSizInput.ScaledHeatingCapacity;
        calcSysSizing.InpDesCoolAirFlow = sysSizInput.DesCoolAirFlow;
        calcSysSizing.InpDesHeatAirFlow = sysSizInput.DesHeatAirFlow;
        calcSysSizing.SystemOAMethod = sysSizInput.SystemOAMethod;
        calcSysSizing.MaxZoneOAFraction = sysSizInput.MaxZoneOAFraction;
        calcSysSizing.OAAutoSized = sysSizInput.OAAutoSized;
        calcSysSizing.FlowPerFloorAreaCooled = sysSizInput.FlowPerFloorAreaCooled;
        calcSysSizing.FlowPerFloorAreaHeated = sysSizInput.FlowPerFloorAreaHeated;
        calcSysSizing.FractionOfAutosizedCoolingAirflow = sysSizInput.FractionOfAutosizedCoolingAirflow;
        calcSysSizing.FractionOfAutosizedHeatingAirflow = sysSizInput.FractionOfAutosizedHeatingAirflow;
        calcSysSizing.FlowPerCoolingCapacity = sysSizInput.FlowPerCoolingCapacity;
        calcSysSizing.FlowPerHeatingCapacity = sysSizInput.FlowPerHeatingCapacity;

        finalSysSizing.HeatFlowSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.SumZoneHeatLoadSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.CoolFlowSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.SumZoneCoolLoadSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.CoolZoneAvgTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.HeatZoneAvgTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.SensCoolCapSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.TotCoolCapSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.HeatCapSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.PreheatCapSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.SysCoolRetTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.SysCoolRetHumRatSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.SysHeatRetTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.SysHeatRetHumRatSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.SysCoolOutTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.SysCoolOutHumRatSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.SysHeatOutTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.SysHeatOutHumRatSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.SysDOASHeatAddSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.SysDOASLatAddSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        finalSysSizing.FloorAreaOnAirLoopCooled = 0.0;
        finalSysSizing.FloorAreaOnAirLoopHeated = 0.0;
        calcSysSizing.HeatFlowSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.SumZoneHeatLoadSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.CoolFlowSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.SumZoneCoolLoadSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.CoolZoneAvgTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.HeatZoneAvgTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.SensCoolCapSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.TotCoolCapSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.HeatCapSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.PreheatCapSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.SysCoolRetTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.SysCoolRetHumRatSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.SysHeatRetTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.SysHeatRetHumRatSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.SysCoolOutTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.SysCoolOutHumRatSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.SysHeatOutTempSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.SysHeatOutHumRatSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.SysDOASHeatAddSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.SysDOASLatAddSeq.dimension(state.dataSimAirServingZones->NumOfTimeStepInDay, 0.0);
        calcSysSizing.FloorAreaOnAirLoopCooled = 0.0;
        calcSysSizing.FloorAreaOnAirLoopHeated = 0.0;

        auto &sysSizePeakDDNum = state.dataSize->SysSizPeakDDNum(AirLoopNum);
        sysSizePeakDDNum.TimeStepAtSensCoolPk.dimension(state.dataEnvrn->TotDesDays + state.dataEnvrn->TotRunDesPersDays, 0);
        sysSizePeakDDNum.TimeStepAtTotCoolPk.dimension(state.dataEnvrn->TotDesDays + state.dataEnvrn->TotRunDesPersDays, 0);
        sysSizePeakDDNum.TimeStepAtCoolFlowPk.dimension(state.dataEnvrn->TotDesDays + state.dataEnvrn->TotRunDesPersDays, 0);
        sysSizePeakDDNum.TimeStepAtHeatPk.dimension(state.dataEnvrn->TotDesDays + state.dataEnvrn->TotRunDesPersDays, 0);

        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {

            SetupEMSInternalVariable(state,
                                     "Intermediate Air System Main Supply Volume Flow Rate",
                                     finalSysSizing.AirPriLoopName,
                                     "[m3/s]",
                                     finalSysSizing.DesMainVolFlow);
            SetupEMSActuator(state,
                             "Sizing:System",
                             finalSysSizing.AirPriLoopName,
                             "Main Supply Volume Flow Rate",
                             "[m3/s]",
                             finalSysSizing.EMSOverrideDesMainVolFlowOn,
                             finalSysSizing.EMSValueDesMainVolFlow);

            SetupEMSInternalVariable(state,
                                     "Intermediate Air System Coincident Peak Cooling Mass Flow Rate",
                                     finalSysSizing.AirPriLoopName,
                                     "[kg/s]",
                                     finalSysSizing.CoinCoolMassFlow);
            SetupEMSActuator(state,
                             "Sizing:System",
                             finalSysSizing.AirPriLoopName,
                             "Main Supply Coincident Peak Cooling Mass Flow Rate",
                             "[kg/s]",
                             finalSysSizing.EMSOverrideCoinCoolMassFlowOn,
                             finalSysSizing.EMSValueCoinCoolMassFlow);

            SetupEMSInternalVariable(state,
                                     "Intermediate Air System Coincident Peak Heating Mass Flow Rate",
                                     finalSysSizing.AirPriLoopName,
                                     "[kg/s]",
                                     finalSysSizing.CoinHeatMassFlow);
            SetupEMSActuator(state,
                             "Sizing:System",
                             finalSysSizing.AirPriLoopName,
                             "Main Supply Coincident Peak Heating Mass Flow Rate",
                             "[kg/s]",
                             finalSysSizing.EMSOverrideCoinHeatMassFlowOn,
                             finalSysSizing.EMSValueCoinHeatMassFlow);

            SetupEMSInternalVariable(state,
                                     "Intermediate Air System Noncoincident Peak Cooling Mass Flow Rate",
                                     finalSysSizing.AirPriLoopName,
                                     "[kg/s]",
                                     finalSysSizing.NonCoinCoolMassFlow);
            SetupEMSActuator(state,
                             "Sizing:System",
                             finalSysSizing.AirPriLoopName,
                             "Main Supply Noncoincident Peak Cooling Mass Flow Rate",
                             "[kg/s]",
                             finalSysSizing.EMSOverrideNonCoinCoolMassFlowOn,
                             finalSysSizing.EMSValueNonCoinCoolMassFlow);
            SetupEMSInternalVariable(state,
                                     "Intermediate Air System Noncoincident Peak Heating Mass Flow Rate",
                                     finalSysSizing.AirPriLoopName,
                                     "[kg/s]",
                                     finalSysSizing.NonCoinHeatMassFlow);
            SetupEMSActuator(state,
                             "Sizing:System",
                             finalSysSizing.AirPriLoopName,
                             "Main Supply Noncoincident Peak Heating Mass Flow Rate",
                             "[kg/s]",
                             finalSysSizing.EMSOverrideNonCoinHeatMassFlowOn,
                             finalSysSizing.EMSValueNonCoinHeatMassFlow);

            SetupEMSInternalVariable(
                state, "Intermediate Air System Heating Volume Flow Rate", finalSysSizing.AirPriLoopName, "[m3/s]", finalSysSizing.DesHeatVolFlow);
            SetupEMSActuator(state,
                             "Sizing:System",
                             finalSysSizing.AirPriLoopName,
                             "Main Heating Volume Flow Rate",
                             "[m3/s]",
                             finalSysSizing.EMSOverrideDesHeatVolFlowOn,
                             finalSysSizing.EMSValueDesHeatVolFlow);

            SetupEMSInternalVariable(
                state, "Intermediate Air System Cooling Volume Flow Rate", finalSysSizing.AirPriLoopName, "[m3/s]", finalSysSizing.DesCoolVolFlow);
            SetupEMSActuator(state,
                             "Sizing:System",
                             finalSysSizing.AirPriLoopName,
                             "Main Cooling Volume Flow Rate",
                             "[m3/s]",
                             finalSysSizing.EMSOverrideDesCoolVolFlowOn,
                             finalSysSizing.EMSValueDesCoolVolFlow);
            // internal variables useful for sizing air system component models
            SetupEMSInternalVariable(
                state, "Air System Cooling Design Sensible Capacity", finalSysSizing.AirPriLoopName, "[W]", finalSysSizing.SensCoolCap);
            SetupEMSInternalVariable(
                state, "Air System Cooling Design Total Capacity", finalSysSizing.AirPriLoopName, "[W]", finalSysSizing.TotCoolCap);
            SetupEMSInternalVariable(
                state, "Air System Heating Design Sensible Capacity", finalSysSizing.AirPriLoopName, "[W]", finalSysSizing.HeatCap);
            SetupEMSInternalVariable(
                state, "Air System Preheating Design Sensible Capacity", finalSysSizing.AirPriLoopName, "[W]", finalSysSizing.PreheatCap);

            SetupEMSInternalVariable(
                state, "Air System Outdoor Air Design Volume Flow Rate", finalSysSizing.AirPriLoopName, "[m3/s]", finalSysSizing.DesOutAirVolFlow);

            SetupEMSInternalVariable(
                state, "Air System Cooling Design Mixed Air Temperature", finalSysSizing.AirPriLoopName, "[C]", finalSysSizing.MixTempAtCoolPeak);
            SetupEMSInternalVariable(state,
                                     "Air System Cooling Design Mixed Air Humidity Ratio",
                                     finalSysSizing.AirPriLoopName,
                                     "[kgWater/kgDryAir]",
                                     finalSysSizing.MixHumRatAtCoolPeak);
            SetupEMSInternalVariable(
                state, "Air System Cooling Design Return Air Temperature", finalSysSizing.AirPriLoopName, "[C]", finalSysSizing.RetTempAtCoolPeak);
            SetupEMSInternalVariable(state,
                                     "Air System Cooling Design Return Air Humidity Ratio",
                                     finalSysSizing.AirPriLoopName,
                                     "[kgWater/kgDryAir]",
                                     finalSysSizing.RetHumRatAtCoolPeak);
            SetupEMSInternalVariable(
                state, "Air System Cooling Design Outdoor Air Temperature", finalSysSizing.AirPriLoopName, "[C]", finalSysSizing.OutTempAtCoolPeak);
            SetupEMSInternalVariable(state,
                                     "Air System Cooling Design Outdoor Air Humidity Ratio",
                                     finalSysSizing.AirPriLoopName,
                                     "[kgWater/kgDryAir]",
                                     finalSysSizing.OutHumRatAtCoolPeak);

            SetupEMSInternalVariable(
                state, "Air System Heating Design Mixed Air Temperature", finalSysSizing.AirPriLoopName, "[C]", finalSysSizing.HeatMixTemp);
            SetupEMSInternalVariable(state,
                                     "Air System Heating Design Mixed Air Humidity Ratio",
                                     finalSysSizing.AirPriLoopName,
                                     "[kgWater/kgDryAir]",
                                     finalSysSizing.HeatMixHumRat);
            SetupEMSInternalVariable(
                state, "Air System Heating Design Return Air Temperature", finalSysSizing.AirPriLoopName, "[C]", finalSysSizing.HeatRetTemp);
            SetupEMSInternalVariable(state,
                                     "Air System Heating Design Return Air Humidity Ratio",
                                     finalSysSizing.AirPriLoopName,
                                     "[kgWater/kgDryAir]",
                                     finalSysSizing.HeatRetHumRat);
            SetupEMSInternalVariable(
                state, "Air System Heating Design Outdoor Air Temperature", finalSysSizing.AirPriLoopName, "[C]", finalSysSizing.HeatOutTemp);
            SetupEMSInternalVariable(state,
                                     "Air System Heating Design Outdoor Air Humidity Ratio",
                                     finalSysSizing.AirPriLoopName,
                                     "[kgWater/kgDryAir]",
                                     finalSysSizing.HeatOutHumRat);
        }

    } // end the primary air system loop
}

void SizeSysOutdoorAir(EnergyPlusData &state)
{

    using namespace OutputReportPredefined;

    Real64 MinOAFlow;                // design minimum outside air flow for a system
    Real64 ZoneOAFracCooling;        // zone OA fraction for cooling design air flow
    Real64 ZoneOAFracHeating;        // zone OA fraction for heating design air flow
    Real64 ZoneSA;                   // Zone supply air flow rate
    Real64 ZonePA;                   // Zone primary air flow rate
    Real64 ClgSupplyAirAdjustFactor; // temporary variable
    Real64 HtgSupplyAirAdjustFactor; // temporary variable
    Real64 SysOAUnc;                 // uncorrected system OA summing up people and area based OA for all zones for VRP
    Real64 ZoneOAUnc;                // uncorrected zone OA summing up people and area based OA for each zone

    // begin system OA calcs, this is the first pass, std 62.1 calcs are redone after adjustments and zone units are set up
    // call refactored routine for Pz, Ps and D
    SizingManager::DetermineSystemPopulationDiversity(state);

    // If the system design minimum outside air flow rate is autosized, calculate it from the zone data
    // Note that all TermUnitFinalZoneSizing values have already been scaled by air terminal sizing factors
    for (int AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) {
        auto &finalSysSizing = state.dataSize->FinalSysSizing(AirLoopNum);
        auto &airToZoneNodeInfo = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum);
        MinOAFlow = 0.0;
        SysOAUnc = 0.0;
        ClgSupplyAirAdjustFactor = 1.0;
        HtgSupplyAirAdjustFactor = 1.0;
        int SysSizNum = Util::FindItemInList(finalSysSizing.AirPriLoopName, state.dataSize->SysSizInput, &SystemSizingInputData::AirPriLoopName);
        if (SysSizNum == 0) {
            SysSizNum = 1; // use first when none applicable
        }
        if (finalSysSizing.OAAutoSized) {
            int NumZonesCooled = airToZoneNodeInfo.NumZonesCooled;

            // people related code removed, see SizingManager::DetermineSystemPopulationDiversity

            for (int ZonesCooledNum = 1; ZonesCooledNum <= NumZonesCooled; ++ZonesCooledNum) {
                int TermUnitSizingIndex = airToZoneNodeInfo.TermUnitCoolSizingIndex(ZonesCooledNum);
                if (TermUnitSizingIndex == 0) {
                    ShowSevereError(state,
                                    format("SetUpSysSizingArray: TermUnitSizingIndex = 0 for AirLoop={}, Zone ={}",
                                           airToZoneNodeInfo.AirLoopName,
                                           state.dataHeatBal->Zone(airToZoneNodeInfo.CoolCtrlZoneNums(ZonesCooledNum)).Name));
                    ShowFatalError(state, "This is a defect. Please report this issue.");
                }
                auto &termUnitSizing = state.dataSize->TermUnitSizing(TermUnitSizingIndex);
                auto &termUnitFinalZoneSizing = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex);
                if (SysSizNum > 0) {
                    ZoneOAUnc =
                        termUnitFinalZoneSizing.TotalOAFromPeople +
                        termUnitFinalZoneSizing.TotalOAFromArea; // should not have diversity at this point (no should have diversity in Vou if VRP)
                    if (state.dataSize->SysSizInput(SysSizNum).SystemOAMethod == SysOAMethod::ZoneSum) { // ZoneSum Method
                        SysOAUnc += ZoneOAUnc;
                    } else if (state.dataSize->SysSizInput(SysSizNum).SystemOAMethod == SysOAMethod::VRP ||
                               state.dataSize->SysSizInput(SysSizNum).SystemOAMethod == SysOAMethod::SP) { // Ventilation Rate Procedure
                        SysOAUnc += termUnitFinalZoneSizing.TotalOAFromPeople * state.dataSize->DBySys(AirLoopNum) +
                                    termUnitFinalZoneSizing.TotalOAFromArea; // apply D to people term
                    }
                    state.dataSize->SumRpxPzBySys(AirLoopNum) += termUnitFinalZoneSizing.TotalOAFromPeople;
                    state.dataSize->SumRaxAzBySys(AirLoopNum) += termUnitFinalZoneSizing.TotalOAFromArea;

                    // save for Standard 62 tabular report
                    state.dataSize->VbzByZone(TermUnitSizingIndex) = ZoneOAUnc; // fixed now, previously RHS already had Ez factored in.
                    // Save Std 62.1 cooling ventilation required by zone
                    if (termUnitFinalZoneSizing.ZoneADEffCooling > 0.0) {
                        termUnitFinalZoneSizing.VozClgByZone = ZoneOAUnc / termUnitFinalZoneSizing.ZoneADEffCooling;
                    } else {
                        termUnitFinalZoneSizing.VozClgByZone = ZoneOAUnc;
                    }

                    if (state.dataSize->SysSizInput(SysSizNum).SystemOAMethod == SysOAMethod::ZoneSum) { // ZoneSum Method
                        MinOAFlow += termUnitFinalZoneSizing.MinOA;
                        if (termUnitFinalZoneSizing.DesCoolVolFlow > 0.0) {
                            ZoneOAFracCooling = termUnitFinalZoneSizing.VozClgByZone /
                                                termUnitFinalZoneSizing.DesCoolVolFlow; // calculate anyway for use with zone OA max fraction below
                        } else {
                            ZoneOAFracCooling = 0.0;
                        }
                    } else if (state.dataSize->SysSizInput(SysSizNum).SystemOAMethod == SysOAMethod::VRP ||
                               state.dataSize->SysSizInput(SysSizNum).SystemOAMethod == SysOAMethod::SP) { // Ventilation Rate Procedure
                        // CR 8872 - check to see if uncorrected OA is calculated to be greater than 0
                        if (!(ZoneOAUnc > 0.0)) {
                            ShowSevereError(
                                state, format("Sizing:System - The system outdoor air method is set to VRP in {}", finalSysSizing.AirPriLoopName));
                            ShowContinueError(
                                state,
                                format("But zone \"{}\" associated with system does not have OA flow/person", termUnitFinalZoneSizing.ZoneName));
                            ShowContinueError(state,
                                              "or flow/area values specified in DesignSpecification:OutdoorAir object associated with the zone");
                        }

                        // Save Std 62.1 cooling ventilation required by zone
                        MinOAFlow += termUnitFinalZoneSizing.VozClgByZone; // Don't include D

                        if (termUnitFinalZoneSizing.DesCoolVolFlow > 0.0) {
                            if (termUnitFinalZoneSizing.ZoneSecondaryRecirculation > 0.0 || termUnitFinalZoneSizing.DesCoolVolFlowMin <= 0) {
                                // multi-path system or VAV Minimum not defined
                                ZoneOAFracCooling =
                                    termUnitFinalZoneSizing.VozClgByZone /
                                    termUnitFinalZoneSizing.DesCoolVolFlow; // this should be based on final atu flows, not sizing design

                            } else {
                                // Single path; Use VAV Minimum as the Vpz in the Zp = Voz / Vpz equations
                                ZoneOAFracCooling =
                                    termUnitFinalZoneSizing.VozClgByZone /
                                    termUnitFinalZoneSizing.DesCoolVolFlowMin; // this should be based on final atu flows, not sizing design
                            }
                        } else {
                            ZoneOAFracCooling = 0.0;
                        }
                    } else { // error
                    }
                } else { // ZoneSum Method
                    MinOAFlow += termUnitFinalZoneSizing.MinOA;
                    ZoneOAFracCooling = 0.0;
                }

                // Calc maximum zone OA fraction and supply air adjustment factor based on
                // user entered max allowed OA fraction
                if (finalSysSizing.MaxZoneOAFraction > 0 && ZoneOAFracCooling > finalSysSizing.MaxZoneOAFraction) {
                    if (finalSysSizing.CoolAirDesMethod == DataSizing::AirflowSizingMethod::FromDDCalc) { // DesignDay Method
                        ClgSupplyAirAdjustFactor = ZoneOAFracCooling / finalSysSizing.MaxZoneOAFraction;
                        if (termUnitFinalZoneSizing.ZoneSecondaryRecirculation > 0.0 || termUnitFinalZoneSizing.DesCoolVolFlowMin <= 0) {
                            // multi-path system or VAV Minimum not defined
                            termUnitFinalZoneSizing.DesCoolVolFlow *= ClgSupplyAirAdjustFactor;
                        } else {
                            // Single path; Use VAV Minimum as the Vpz in the Zp = Voz / Vpz equations
                            termUnitFinalZoneSizing.DesCoolVolFlowMin *=
                                ClgSupplyAirAdjustFactor; // from code inspection value set here is used above, before being set.

                            // Don't allow the design cooling airflow to be less than the VAV minimum airflow
                            termUnitFinalZoneSizing.DesCoolVolFlow =
                                max(termUnitFinalZoneSizing.DesCoolVolFlow, termUnitFinalZoneSizing.DesCoolVolFlowMin);
                        }
                        // Don't allow the design terminal airflow to be less than the design cooling airflow
                        termUnitSizing.AirVolFlow = max(termUnitSizing.AirVolFlow, termUnitFinalZoneSizing.DesCoolVolFlow);
                        ZoneOAFracCooling = finalSysSizing.MaxZoneOAFraction;
                    } else {
                        ClgSupplyAirAdjustFactor = 1.0;
                    }
                } else {
                    ClgSupplyAirAdjustFactor = 1.0;
                }

                ZoneSA = 0.0;
                ZonePA = 0.0;
                state.dataSimAirServingZones->EpSSOA = 1.0;
                if (termUnitFinalZoneSizing.ZoneSecondaryRecirculation > 0.0) { // multi-path system
                    // Vpz: "Primary" supply air from main air handler served by an oa mixer
                    ZonePA = termUnitFinalZoneSizing.DesCoolVolFlow;
                    // Vdz: "Discharge" supply air delivered to zone by terminal unit
                    ZoneSA = max(termUnitSizing.AirVolFlow, ZonePA);

                    // For re-circulation systems, Vpz used to determine Zpz is the design terminal airflow
                    // Std 62.1-2010, section 6.2.5.1: "Vpz (used to determin Zpz) is the primary airflow rate
                    // rate to the ventilation zone from the air handler, including outdoor air and recirculated air.
                    // MJW - Not sure this is correct, seems like it should be ZonePA - above comments contradict each other
                    state.dataSize->VpzMinClgByZone(TermUnitSizingIndex) = ZoneSA;

                } else { // single path system
                    // Vdz: "Discharge" supply air delivered to zone by terminal unit
                    ZonePA = termUnitFinalZoneSizing.DesCoolVolFlow;
                    // Vpz: "Primary" supply air from main air handler served by an oa mixer
                    ZoneSA = termUnitFinalZoneSizing.DesCoolVolFlow;

                    // Save VpzMin in case this is a single path VAV system.
                    // Std 62.1-2010, section 6.2.5.1: "For VAV-system design purposes, Vpz is the lowest zone primary
                    // airflow value expected at the design condition analyzed."
                    state.dataSize->VpzMinClgByZone(TermUnitSizingIndex) =
                        termUnitFinalZoneSizing.DesCoolVolFlowMin; // this may be getting used before it gets filled ??

                    // In case for some reason the VAV minimum has not been defined, use the design primary airflow
                    if (termUnitFinalZoneSizing.DesCoolVolFlowMin <= 0) {
                        state.dataSize->VpzMinClgByZone(TermUnitSizingIndex) = ZonePA;
                    }
                }

                // save zone discharge supply airflow
                state.dataSize->VdzClgByZone(TermUnitSizingIndex) = ZoneSA;

                // save Vpz zone primary airflow for standard 62.1 report
                state.dataSize->VpzClgByZone(TermUnitSizingIndex) = ZonePA;
                state.dataSize->VpzClgSumBySys(AirLoopNum) += ZonePA;

                // Fraction of required zone ventilation to minimum primary airflow expected at condition analyzed
                termUnitFinalZoneSizing.ZpzClgByZone = 0.0;
                if (state.dataSize->VpzMinClgByZone(TermUnitSizingIndex) > 0) {
                    termUnitFinalZoneSizing.ZpzClgByZone =
                        min(1.0, termUnitFinalZoneSizing.VozClgByZone / state.dataSize->VpzMinClgByZone(TermUnitSizingIndex));
                }

                // calc zone primary air fraction
                if (ZoneSA > 0.0) {
                    state.dataSimAirServingZones->EpSSOA = ZonePA / ZoneSA;
                }
                if (state.dataSimAirServingZones->EpSSOA > 1.0) {
                    state.dataSimAirServingZones->EpSSOA = 1.0;
                }
                termUnitFinalZoneSizing.ZonePrimaryAirFraction = state.dataSimAirServingZones->EpSSOA;
                termUnitFinalZoneSizing.ZoneOAFracCooling = ZoneOAFracCooling;

                // determined cooled zone floor area in an airloop
                finalSysSizing.FloorAreaOnAirLoopCooled += termUnitFinalZoneSizing.TotalZoneFloorArea;

                termUnitFinalZoneSizing.SupplyAirAdjustFactor = max(ClgSupplyAirAdjustFactor, HtgSupplyAirAdjustFactor);
            }

            int NumZonesHeated = airToZoneNodeInfo.NumZonesHeated;
            if (NumZonesHeated > 0) {
                for (int ZonesHeatedNum = 1; ZonesHeatedNum <= NumZonesHeated; ++ZonesHeatedNum) {
                    int TermUnitSizingIndex = airToZoneNodeInfo.TermUnitHeatSizingIndex(ZonesHeatedNum);
                    if (TermUnitSizingIndex == 0) {
                        ShowSevereError(state,
                                        format("SetUpSysSizingArray: TermUnitSizingIndex = 0 for AirLoop={}, Zone ={}",
                                               airToZoneNodeInfo.AirLoopName,
                                               state.dataHeatBal->Zone(airToZoneNodeInfo.HeatCtrlZoneNums(ZonesHeatedNum)).Name));
                        ShowFatalError(state, "This is a defect. Please report this issue.");
                    }
                    auto &termUnitSizing = state.dataSize->TermUnitSizing(TermUnitSizingIndex);
                    auto &termUnitFinalZoneSizing = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex);
                    int MatchingCooledZoneNum =
                        General::FindNumberInList(TermUnitSizingIndex, airToZoneNodeInfo.TermUnitCoolSizingIndex, NumZonesCooled);
                    if (MatchingCooledZoneNum == 0) {
                        if (SysSizNum > 0) {
                            ZoneOAUnc = termUnitFinalZoneSizing.TotalOAFromPeople +
                                        termUnitFinalZoneSizing.TotalOAFromArea; // should not have diversity at this point
                            if (state.dataSize->SysSizInput(SysSizNum).SystemOAMethod == SysOAMethod::ZoneSum) { // ZoneSum Method
                                SysOAUnc += ZoneOAUnc;
                            } else if (state.dataSize->SysSizInput(SysSizNum).SystemOAMethod == SysOAMethod::VRP ||
                                       state.dataSize->SysSizInput(SysSizNum).SystemOAMethod ==
                                           SysOAMethod::SP) { // Ventilation Rate and Simplified Procedure
                                SysOAUnc += termUnitFinalZoneSizing.TotalOAFromPeople * state.dataSize->DBySys(AirLoopNum) +
                                            termUnitFinalZoneSizing.TotalOAFromArea; // apply D to people term
                            }
                            state.dataSize->SumRpxPzBySys(AirLoopNum) += termUnitFinalZoneSizing.TotalOAFromPeople;
                            state.dataSize->SumRaxAzBySys(AirLoopNum) += termUnitFinalZoneSizing.TotalOAFromArea;
                            // save for Standard 62 tabular report
                            state.dataSize->VbzByZone(TermUnitSizingIndex) = ZoneOAUnc; // fixed now, previously RHS already had Ez factored in.
                            // Save Std 62.1 heating ventilation required by zone
                            if (termUnitFinalZoneSizing.ZoneADEffHeating > 0.0) {
                                termUnitFinalZoneSizing.VozHtgByZone = ZoneOAUnc / termUnitFinalZoneSizing.ZoneADEffHeating;
                            } else {
                                termUnitFinalZoneSizing.VozHtgByZone = ZoneOAUnc;
                            }

                            if (state.dataSize->SysSizInput(SysSizNum).SystemOAMethod == SysOAMethod::ZoneSum) { // ZoneSum Method
                                MinOAFlow += termUnitFinalZoneSizing.MinOA;
                                if (termUnitFinalZoneSizing.DesHeatVolFlow > 0.0) {
                                    ZoneOAFracHeating =
                                        termUnitFinalZoneSizing.VozHtgByZone /
                                        termUnitFinalZoneSizing.DesHeatVolFlow; // calculate anyway for use with zone OA max fraction below
                                } else {
                                    ZoneOAFracHeating = 0.0;
                                }

                            } else if (state.dataSize->SysSizInput(SysSizNum).SystemOAMethod == SysOAMethod::VRP ||
                                       state.dataSize->SysSizInput(SysSizNum).SystemOAMethod ==
                                           SysOAMethod::SP) { // Ventilation Rate and Simplified Procedure
                                // CR 8872 - check to see if uncorrected OA is calculated to be greater than 0
                                if (!(ZoneOAUnc > 0.0)) {
                                    ShowSevereError(
                                        state,
                                        format("Sizing:System - The system outdoor air method is set to VRP in {}", finalSysSizing.AirPriLoopName));
                                    ShowContinueError(state,
                                                      format("But zone \"{}\" associated with system does not have OA flow/person",
                                                             termUnitFinalZoneSizing.ZoneName));
                                    ShowContinueError(state,
                                                      "or flow/area values specified in DesignSpecification:OutdoorAir object associated "
                                                      "with the zone");
                                }

                                // Save Std 62.1 heating ventilation required by zone
                                MinOAFlow += termUnitFinalZoneSizing.VozHtgByZone; // Don't include D

                                if (termUnitFinalZoneSizing.DesHeatVolFlow > 0.0) {
                                    if (termUnitFinalZoneSizing.ZoneSecondaryRecirculation > 0.0) { // multi-path system
                                        // multi-path system
                                        ZoneOAFracHeating = termUnitFinalZoneSizing.VozHtgByZone / termUnitSizing.AirVolFlow;
                                    } else {
                                        // Single path system
                                        ZoneOAFracHeating = termUnitFinalZoneSizing.VozHtgByZone / termUnitFinalZoneSizing.DesHeatVolFlow;
                                    }
                                } else {
                                    ZoneOAFracHeating = 0.0;
                                }
                            } else { // would be error
                            }
                        } else { // ZoneSum Method
                            MinOAFlow += termUnitFinalZoneSizing.MinOA;
                            ZoneOAFracHeating = 0.0;
                        }
                    } else { // matching cooled zone > 0
                        //?? so what happens if zone is both heated and cooled this makes little sense?  Don't want to double count in
                        // MinOAFlow
                        // but still need to do std62.1 heating calcs ??
                        ZoneOAFracHeating = 0.0;
                    }

                    // Calc maximum zone OA fraction and supply air adjustment factor based
                    // on user entered max allowed OA fraction
                    if (finalSysSizing.MaxZoneOAFraction > 0 && ZoneOAFracHeating > finalSysSizing.MaxZoneOAFraction) {
                        if (finalSysSizing.CoolAirDesMethod == AirflowSizingMethod::FromDDCalc) { // DesignDay Method
                            HtgSupplyAirAdjustFactor = ZoneOAFracHeating / finalSysSizing.MaxZoneOAFraction;
                            if (termUnitFinalZoneSizing.ZoneSecondaryRecirculation > 0.0 || termUnitFinalZoneSizing.DesCoolVolFlowMin <= 0) {
                                // multi-path system or VAV Heating airflow max not defined
                                termUnitSizing.AirVolFlow *= HtgSupplyAirAdjustFactor;
                            } else {
                                // Single path; Use VAV Heating airflow max as the Vpz in the Zp = Voz / Vpz equations
                                termUnitFinalZoneSizing.DesHeatVolFlow *= HtgSupplyAirAdjustFactor;
                                // Don't allow the design terminal airflow to be less than the design heating airflow
                                termUnitSizing.AirVolFlow = max(termUnitSizing.AirVolFlow, termUnitFinalZoneSizing.DesHeatVolFlow);
                            }
                            ZoneOAFracHeating = finalSysSizing.MaxZoneOAFraction;
                        } else {
                            HtgSupplyAirAdjustFactor = 1.0;
                        }
                    } else {
                        HtgSupplyAirAdjustFactor = 1.0;
                    }

                    ZoneSA = 0.0;
                    ZonePA = 0.0;
                    state.dataSimAirServingZones->EpSSOA = 1.0;
                    if (termUnitFinalZoneSizing.ZoneSecondaryRecirculation > 0.0) { // multi-path system
                        // Vpz: "Primary" supply air from main air handler served by an oa mixer
                        ZonePA = termUnitFinalZoneSizing.DesHeatVolFlow;
                        // Vdz: "Discharge" supply air delivered to zone by terminal unit
                        ZoneSA = max(termUnitSizing.AirVolFlow, ZonePA);

                        // For re-circulation systems, Vpz used to determine Zpz is the design terminal airflow
                        // Std 62.1-2010, section 6.2.5.1: "Vpz (used to determin Zpz) is the primary airflow rate
                        // rate to the ventilation zone from the air handler, including outdoor air and recirculated air.
                        state.dataSize->VpzMinHtgByZone(TermUnitSizingIndex) = ZoneSA;

                    } else { // single path system

                        ZonePA = termUnitFinalZoneSizing.DesHeatVolFlow;
                        ZoneSA = termUnitFinalZoneSizing.DesHeatVolFlow;

                        // We do not use the cooling VAV min for heating because the VAV-box heating maximum may be larger.
                        state.dataSize->VpzMinHtgByZone(TermUnitSizingIndex) = ZoneSA;
                    }

                    // save Vdz zone discharge supply airflow for standard 62.1 report
                    state.dataSize->VdzHtgByZone(TermUnitSizingIndex) = ZoneSA;

                    // save Vpz zone primary airflow for standard 62.1 report
                    state.dataSize->VpzHtgByZone(TermUnitSizingIndex) = ZonePA;
                    state.dataSize->VpzHtgSumBySys(AirLoopNum) += ZonePA;

                    // Fraction of required zone ventilation to minimum primary airflow expected at condition analyzed
                    termUnitFinalZoneSizing.ZpzHtgByZone = 0.0;
                    if (state.dataSize->VpzMinHtgByZone(TermUnitSizingIndex) > 0) {
                        termUnitFinalZoneSizing.ZpzHtgByZone =
                            min(1.0, termUnitFinalZoneSizing.VozHtgByZone / state.dataSize->VpzMinHtgByZone(TermUnitSizingIndex));
                    }

                    // calc zone primary air fraction
                    if (ZoneSA > 0.0) {
                        state.dataSimAirServingZones->EpSSOA = ZonePA / ZoneSA;
                    }
                    if (state.dataSimAirServingZones->EpSSOA > 1.0) {
                        state.dataSimAirServingZones->EpSSOA = 1.0;
                    }
                    termUnitFinalZoneSizing.ZonePrimaryAirFractionHtg = state.dataSimAirServingZones->EpSSOA;
                    termUnitFinalZoneSizing.ZoneOAFracHeating = ZoneOAFracHeating;

                    // determined heated zone floor area in an airloop
                    finalSysSizing.FloorAreaOnAirLoopHeated += termUnitFinalZoneSizing.TotalZoneFloorArea;

                    termUnitFinalZoneSizing.SupplyAirAdjustFactor = max(ClgSupplyAirAdjustFactor, HtgSupplyAirAdjustFactor);

                } // end for loop of heated zones

            } else { // getting heating flow based values for Std 62.1 report for single path systems
                ZoneOAFracHeating = 0.0;
                for (int ZonesHeatedNum = 1; ZonesHeatedNum <= NumZonesCooled; ++ZonesHeatedNum) {
                    int TermUnitSizingIndex = airToZoneNodeInfo.TermUnitCoolSizingIndex(ZonesHeatedNum);
                    if (TermUnitSizingIndex == 0) {
                        ShowSevereError(state,
                                        format("SetUpSysSizingArray: TermUnitSizingIndex = 0 for AirLoop={}, Zone ={}",
                                               airToZoneNodeInfo.AirLoopName,
                                               state.dataHeatBal->Zone(airToZoneNodeInfo.CoolCtrlZoneNums(ZonesHeatedNum)).Name));
                        ShowFatalError(state, "This is a defect. Please report this issue.");
                    }
                    auto &termUnitSizing = state.dataSize->TermUnitSizing(TermUnitSizingIndex);
                    auto &termUnitFinalZoneSizing = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex);
                    // Save Std 62.1 heating ventilation required by zone
                    if (termUnitFinalZoneSizing.ZoneADEffHeating > 0.0) {
                        termUnitFinalZoneSizing.VozHtgByZone =
                            state.dataSize->VbzByZone(TermUnitSizingIndex) / termUnitFinalZoneSizing.ZoneADEffHeating;
                    } else {
                        termUnitFinalZoneSizing.VozHtgByZone = state.dataSize->VbzByZone(TermUnitSizingIndex);
                    }

                    if (termUnitFinalZoneSizing.DesHeatVolFlow > 0.0) {
                        if (termUnitFinalZoneSizing.ZoneSecondaryRecirculation > 0.0) { // multi-path system
                            // multi-path system
                            if (termUnitSizing.AirVolFlow != 0) {
                                ZoneOAFracHeating = termUnitFinalZoneSizing.VozHtgByZone / termUnitSizing.AirVolFlow;
                            }
                        } else {
                            // Single path system
                            ZoneOAFracHeating = termUnitFinalZoneSizing.VozHtgByZone / termUnitFinalZoneSizing.DesHeatVolFlow;
                        }
                    } else {
                        ZoneOAFracHeating = 0.0;
                    }

                    // Calc maximum zone OA fraction and supply air adjustment factor based
                    // on user entered max allowed OA fraction - a TRACE feature
                    if (finalSysSizing.MaxZoneOAFraction > 0 && ZoneOAFracHeating > finalSysSizing.MaxZoneOAFraction) {
                        if (finalSysSizing.HeatAirDesMethod == AirflowSizingMethod::FromDDCalc) { // DesignDay Method
                            HtgSupplyAirAdjustFactor = ZoneOAFracHeating / finalSysSizing.MaxZoneOAFraction;
                            if (termUnitFinalZoneSizing.ZoneSecondaryRecirculation > 0.0 || termUnitFinalZoneSizing.DesCoolVolFlowMin <= 0) {
                                // multi-path system or VAV Heating airflow max not defined
                                termUnitSizing.AirVolFlow *= HtgSupplyAirAdjustFactor;
                            } else {
                                // Single path; Use VAV Heating airflow max as the Vpz in the Zp = Voz / Vpz equations
                                termUnitFinalZoneSizing.DesHeatVolFlow *= HtgSupplyAirAdjustFactor;
                                // Don't allow the design terminal airflow to be less than the design heating airflow
                                termUnitSizing.AirVolFlow = max(termUnitSizing.AirVolFlow, termUnitFinalZoneSizing.DesHeatVolFlow);
                            }
                            ZoneOAFracHeating = finalSysSizing.MaxZoneOAFraction;
                        }
                    }
                    ZonePA = termUnitFinalZoneSizing.DesHeatVolFlow;
                    ZoneSA = termUnitFinalZoneSizing.DesHeatVolFlow;
                    // save Vdz zone discharge airflow for standard 62.1 report
                    state.dataSize->VdzHtgByZone(TermUnitSizingIndex) = ZoneSA;
                    // save Vpz zone primary airflow for standard 62.1 report
                    state.dataSize->VpzHtgByZone(TermUnitSizingIndex) = ZonePA;
                    state.dataSize->VpzHtgSumBySys(AirLoopNum) += ZonePA;

                    // We do not use the cooling VAV min for heating because the VAV-box heating maximum may be larger.
                    state.dataSize->VpzMinHtgByZone(TermUnitSizingIndex) = ZoneSA;

                    // Fraction of required zone ventilation to minimum primary airflow expected at condition analyzed
                    termUnitFinalZoneSizing.ZpzHtgByZone = 0.0;
                    if (state.dataSize->VpzMinHtgByZone(TermUnitSizingIndex) > 0) {
                        termUnitFinalZoneSizing.ZpzHtgByZone =
                            termUnitFinalZoneSizing.VozHtgByZone / state.dataSize->VpzMinHtgByZone(TermUnitSizingIndex);
                    }

                    // calc zone primary air fraction
                    state.dataSimAirServingZones->EpSSOA = 1.0;
                    if (ZoneSA > 0.0) {
                        state.dataSimAirServingZones->EpSSOA = ZonePA / ZoneSA;
                    }
                    if (state.dataSimAirServingZones->EpSSOA > 1.0) {
                        state.dataSimAirServingZones->EpSSOA = 1.0;
                    }
                    termUnitFinalZoneSizing.ZonePrimaryAirFractionHtg = state.dataSimAirServingZones->EpSSOA;
                    termUnitFinalZoneSizing.ZoneOAFracHeating = ZoneOAFracHeating;
                    termUnitFinalZoneSizing.SupplyAirAdjustFactor = max(ClgSupplyAirAdjustFactor, HtgSupplyAirAdjustFactor);

                } // end for loop over cooled zones (for htg calcs though)
                finalSysSizing.FloorAreaOnAirLoopHeated = finalSysSizing.FloorAreaOnAirLoopCooled;
            }

            finalSysSizing.SysUncOA = SysOAUnc;
            state.dataSize->CalcSysSizing(AirLoopNum).SysUncOA = SysOAUnc;
            state.dataSize->VouBySys(AirLoopNum) = SysOAUnc;

            finalSysSizing.DesOutAirVolFlow = MinOAFlow;
            state.dataSize->CalcSysSizing(AirLoopNum).DesOutAirVolFlow = MinOAFlow;

            for (int DesDayEnvrnNum = 1; DesDayEnvrnNum <= state.dataEnvrn->TotDesDays + state.dataEnvrn->TotRunDesPersDays; ++DesDayEnvrnNum) {
                state.dataSize->SysSizing(DesDayEnvrnNum, AirLoopNum).DesOutAirVolFlow = finalSysSizing.DesOutAirVolFlow;
            }
        }
    }

    // END SYSTEM OA CALCS

    // have moved std 62.1 table report writing to ManageSystemVentilationAdjustments in SizingManager
}

void UpdateSysSizing(EnergyPlusData &state, Constant::CallIndicator const CallIndicator)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   February 2001

    // PURPOSE OF THIS SUBROUTINE:
    // Update the result variables of the zone sizing calculation

    // METHODOLOGY EMPLOYED:
    // CallIndicator = 1 (BeginDay) zero the result arrays
    // CallIndicator = 2 (DuringDay) fill arrays, averaging over 1 zone time step
    // CallIndicator = 3 (EndDay) calculate daily maxima
    // CallIndicator = 5 (EndSysSizingCalc) write out results

    // Using/Aliasing
    using EMSManager::ManageEMS;
    using General::FindNumberInList;
    using Psychrometrics::PsyCpAirFnW;
    using Psychrometrics::PsyHFnTdbW;
    using Psychrometrics::PsyRhoAirFnPbTdbW;
    using namespace OutputReportPredefined;
    using namespace DataSizing;

    // Locals
    int numOfTimeStepInDay; // number of zone time steps in a day

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int AirLoopNum;                // primary air system index
    int TimeStepInDay;             // zone time step in day
    int TimeStepIndex;             // zone time step index
    int I;                         // write statement index
    int J;                         // write statement index
    Real64 SysCoolRetTemp;         // system cooling return temperature for a time step [C]
    Real64 SysHeatRetTemp;         // system heating return temperature for a time step [C]
    Real64 RhoAir;                 // density of air kg/m3
    Real64 OutAirFrac;             // outside air fraction
    Real64 SysCoolMixTemp;         // system cooling mixed air temperature [C]
    Real64 SysHeatMixTemp;         // system heating mixed air temperature [C]
    Real64 SysSensCoolCap;         // system sensible cooling capacity [W]
    Real64 SysTotCoolCap;          // system total cooling capacity [W]
    Real64 SysCoolZoneAvgTemp;     // system cooling zone average temperature [C]
    Real64 SysHeatZoneAvgTemp;     // system heating zone average temperature [C]
    Real64 SysHeatCap;             // system heating capacity [W]
    int HourCounter;               // Hour Counter
    int TimeStepCounter;           // Time Step Counter
    int Minutes;                   // Current Minutes Counter
    int HourPrint;                 // Hour to print (timestamp)
    int DDNum;                     // design day index
    int CoolDDNum;                 // design day index of a peak cooling day
    int HeatDDNum;                 // design day index of a peak cooling day
    int CoolTimeStepNum;           // time step index (in day) of a cooling peak
    int HeatTimeStepNum;           // time step index (in day) of a cooling peak
    Real64 OutAirTemp;             // outside air temperature
    Real64 OutAirHumRat;           // outside air humifity ratio
    Real64 SysCoolMixHumRat;       // system cooling mixed air humidity ratio [kg water/kg dry air]
    Real64 SysCoolRetHumRat;       // system coolingreturn air humifity ratio [kg water/kg dry air]
    Real64 SysHeatMixHumRat;       // system heating mixed air humidity ratio [kg water/kg dry air]
    Real64 SysHeatRetHumRat;       // system heatingreturn air humifity ratio [kg water/kg dry air]
    Real64 SysCoolOutTemp;         // system cooling outside air temperature [C]
    Real64 SysCoolOutHumRat;       // system cooling outside air humidity ratio [kg water/kg dry air]
    Real64 SysHeatOutTemp;         // system heating outside air temperature [C]
    Real64 SysHeatOutHumRat;       // system heating outside air humidity ratio [kg water/kg dry air]
    Real64 SysDOASHeatAdd;         // system DOAS heat addition rate [W]
    Real64 SysDOASLatAdd;          // system DOAS latent heat addition rate [W]
    Real64 SysCoolSizingRat;       // ratio of user input design flow for cooling divided by calculated design cooling flow
    Real64 SysHeatSizingRat;       // ratio of user input design flow for heating divided by calculated design heating flow
    Real64 ZoneOARatio;            // ratio of zone OA flow to zone design cooling or heating flow
    Real64 RetTempRise;            // difference between zone return temperature and zone temperature [delta K]
    Real64 SysCoolingEv;           // System level ventilation effectiveness for cooling mode
    Real64 SysHeatingEv;           // System level ventilation effectiveness for heating mode
    Real64 SysHtgPeakAirflow;      // Peak heating airflow
    int MatchingCooledZoneNum;     // temporary variable
    Real64 termunitsizingtempfrac; // 1.0/(1.0+termunitsizing(ctrlzone)%inducrat)
    Real64 termunitsizingtemp;     // (1.0+termunitsizing(ctrlzone)%inducrat)
    Real64 VozClg(0.0);            // corrected (for ventilation efficiency) zone outside air flow rate [m3/s]

    numOfTimeStepInDay = state.dataGlobal->TimeStepsInHour * Constant::iHoursInDay;

    // allocate scratch arrays
    if (!allocated(state.dataSize->SensCoolCapTemp)) {
        state.dataSize->SensCoolCapTemp.dimension(state.dataHVACGlobal->NumPrimaryAirSys, 0.0);
        state.dataSize->TotCoolCapTemp.dimension(state.dataHVACGlobal->NumPrimaryAirSys, 0.0);
    }

    // allocate arrays used to store values for standard 62.1 tabular report
    if (!allocated(state.dataSize->FaByZoneCool)) {
        state.dataSize->FaByZoneCool.dimension(state.dataSize->NumAirTerminalUnits, 0.0);
        state.dataSize->FaByZoneHeat.dimension(state.dataSize->NumAirTerminalUnits, 0.0);
        state.dataSize->FbByZoneCool.dimension(state.dataSize->NumAirTerminalUnits, 0.0);
        state.dataSize->FbByZoneHeat.dimension(state.dataSize->NumAirTerminalUnits, 0.0);
        state.dataSize->FcByZoneCool.dimension(state.dataSize->NumAirTerminalUnits, 0.0);
        state.dataSize->FcByZoneHeat.dimension(state.dataSize->NumAirTerminalUnits, 0.0);
        state.dataSimAirServingZones->EvBySysCool.dimension(state.dataHVACGlobal->NumPrimaryAirSys, 1.0);
        state.dataSimAirServingZones->EvBySysHeat.dimension(state.dataHVACGlobal->NumPrimaryAirSys, 1.0);
        state.dataSize->XsBySysCool.dimension(state.dataHVACGlobal->NumPrimaryAirSys, 1.0);
        state.dataSize->XsBySysHeat.dimension(state.dataHVACGlobal->NumPrimaryAirSys, 1.0);
        state.dataSize->EvzByZoneCool.dimension(state.dataSize->NumAirTerminalUnits, 1.0);
        state.dataSize->EvzByZoneCoolPrev.dimension(state.dataSize->NumAirTerminalUnits, 1.0);
        state.dataSize->EvzByZoneHeat.dimension(state.dataSize->NumAirTerminalUnits, 1.0);
        state.dataSize->EvzByZoneHeatPrev.dimension(state.dataSize->NumAirTerminalUnits, 1.0);
        state.dataSize->EvzMinBySysCool.dimension(state.dataHVACGlobal->NumPrimaryAirSys, 1.0);
        state.dataSize->EvzMinBySysHeat.dimension(state.dataHVACGlobal->NumPrimaryAirSys, 1.0);
        state.dataSize->VotClgBySys.dimension(state.dataHVACGlobal->NumPrimaryAirSys, 0.0);
        state.dataSize->VotHtgBySys.dimension(state.dataHVACGlobal->NumPrimaryAirSys, 0.0);
        state.dataSize->VozSumClgBySys.dimension(state.dataHVACGlobal->NumPrimaryAirSys, 0.0);
        state.dataSize->VozSumHtgBySys.dimension(state.dataHVACGlobal->NumPrimaryAirSys, 0.0);
    }

    switch (CallIndicator) {
    case Constant::CallIndicator::BeginDay: {
        // Correct the zone return temperature in ZoneSizing for the case of induction units. The calc in
        // ZoneEquipmentManager assumes all the air entering the zone goes into the return node.
        for (int CtrlZoneNum = 1; CtrlZoneNum <= state.dataGlobal->NumOfZones; ++CtrlZoneNum) {
            auto &zoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum);
            auto &zoneSizing = state.dataSize->ZoneSizing(state.dataSize->CurOverallSimDay, CtrlZoneNum);
            if (!zoneEquipConfig.IsControlled) {
                continue;
            }
            // Use first non-zero airdistunit for now
            int TermUnitSizingIndex = 0;
            for (int InletNode = 1; InletNode <= zoneEquipConfig.NumInletNodes; ++InletNode) {
                TermUnitSizingIndex = zoneEquipConfig.AirDistUnitCool(InletNode).TermUnitSizingIndex;
                if (TermUnitSizingIndex == 0) {
                    continue;
                }
                termunitsizingtemp = (1.0 + state.dataSize->TermUnitSizing(TermUnitSizingIndex).InducRat);
                termunitsizingtempfrac = (1.0 / termunitsizingtemp);
                if (TermUnitSizingIndex > 0) {
                    break;
                }
            }
            if (TermUnitSizingIndex == 0) {
                continue; // Skip this if there are no terminal units
            }
            RetTempRise = zoneSizing.ZoneRetTempAtCoolPeak - zoneSizing.ZoneTempAtCoolPeak;
            if (RetTempRise > 0.01) {
                zoneSizing.ZoneRetTempAtCoolPeak = zoneSizing.ZoneTempAtCoolPeak + RetTempRise * termunitsizingtempfrac;
            }
            RetTempRise = zoneSizing.ZoneRetTempAtHeatPeak - zoneSizing.ZoneTempAtHeatPeak;
            if (RetTempRise > 0.01) {
                zoneSizing.ZoneRetTempAtHeatPeak = zoneSizing.ZoneTempAtHeatPeak + RetTempRise * termunitsizingtempfrac;
            }
        }

        for (AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) { // start of begin day loop over primary air systems
            auto &airToZoneNodeInfo = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum);
            int NumZonesCooled = airToZoneNodeInfo.NumZonesCooled;
            int NumZonesHeated = airToZoneNodeInfo.NumZonesHeated;
            state.dataSize->SysSizing(state.dataSize->CurOverallSimDay, AirLoopNum).CoolDesDay = state.dataEnvrn->EnvironmentName;
            state.dataSize->SysSizing(state.dataSize->CurOverallSimDay, AirLoopNum).HeatDesDay = state.dataEnvrn->EnvironmentName;
            state.dataSize->SensCoolCapTemp(AirLoopNum) = 0.0;
            state.dataSize->TotCoolCapTemp(AirLoopNum) = 0.0;

            for (int ZonesCooledNum = 1; ZonesCooledNum <= NumZonesCooled; ++ZonesCooledNum) { // loop over cooled zones
                int CtrlZoneNum = airToZoneNodeInfo.CoolCtrlZoneNums(ZonesCooledNum);
                int TermUnitSizingIndex = airToZoneNodeInfo.TermUnitCoolSizingIndex(ZonesCooledNum);
                Real64 adjCoolMassFlow =
                    state.dataSize->TermUnitSizing(TermUnitSizingIndex)
                        .applyTermUnitSizingCoolFlow(state.dataSize->ZoneSizing(state.dataSize->CurOverallSimDay, CtrlZoneNum).DesCoolMassFlow,
                                                     state.dataSize->ZoneSizing(state.dataSize->CurOverallSimDay, CtrlZoneNum).DesCoolMassFlowNoOA);
                state.dataSize->SysSizing(state.dataSize->CurOverallSimDay, AirLoopNum).NonCoinCoolMassFlow +=
                    adjCoolMassFlow / (1.0 + state.dataSize->TermUnitSizing(TermUnitSizingIndex).InducRat);
                if (state.dataSize->SysSizing(state.dataSize->CurOverallSimDay, AirLoopNum).loadSizingType == DataSizing::LoadSizing::Latent &&
                    !state.dataSize->FinalZoneSizing.empty()) {
                    if (!state.dataSize->FinalZoneSizing(CtrlZoneNum).zoneLatentSizing && state.dataSize->CurOverallSimDay == 1) {
                        ShowWarningError(state,
                                         format("Latent Sizing for AirLoop = {} requires latent sizing in Sizing:Zone object for Zone = {}",
                                                airToZoneNodeInfo.AirLoopName,
                                                state.dataSize->FinalZoneSizing(CtrlZoneNum).ZoneName));
                    }
                } else if (!state.dataSize->FinalZoneSizing.empty()) { // not latent sizing for air loop
                    if (state.dataSize->FinalZoneSizing(CtrlZoneNum).zoneLatentSizing && state.dataSize->CurOverallSimDay == 1 &&
                        state.dataSize->FinalZoneSizing(CtrlZoneNum).heatCoilSizingMethod == DataSizing::HeatCoilSizMethod::None) {
                        ShowWarningError(state,
                                         format("Sizing for AirLoop = {} includes latent sizing in Sizing:Zone object for Zone = {}",
                                                airToZoneNodeInfo.AirLoopName,
                                                state.dataSize->FinalZoneSizing(CtrlZoneNum).ZoneName));
                    }
                }
            } // end of loop over cooled zones

            if (NumZonesHeated > 0) {                                                              // if there are zones supplied with central hot air
                for (int ZonesHeatedNum = 1; ZonesHeatedNum <= NumZonesHeated; ++ZonesHeatedNum) { // loop over heated zones
                    int CtrlZoneNum = airToZoneNodeInfo.HeatCtrlZoneNums(ZonesHeatedNum);
                    int TermUnitSizingIndex = airToZoneNodeInfo.TermUnitHeatSizingIndex(ZonesHeatedNum);
                    Real64 adjHeatMassFlow = state.dataSize->TermUnitSizing(TermUnitSizingIndex)
                                                 .applyTermUnitSizingHeatFlow(
                                                     state.dataSize->ZoneSizing(state.dataSize->CurOverallSimDay, CtrlZoneNum).DesHeatMassFlow,
                                                     state.dataSize->ZoneSizing(state.dataSize->CurOverallSimDay, CtrlZoneNum).DesHeatMassFlowNoOA);
                    state.dataSize->SysSizing(state.dataSize->CurOverallSimDay, AirLoopNum).NonCoinHeatMassFlow +=
                        adjHeatMassFlow / (1.0 + state.dataSize->TermUnitSizing(TermUnitSizingIndex).InducRat);
                } // end of loop over heated zones
            } else {                                                                               // otherwise use cool supply zones
                for (int ZonesCooledNum = 1; ZonesCooledNum <= NumZonesCooled; ++ZonesCooledNum) { // loop over cooled zones
                    int CtrlZoneNum = airToZoneNodeInfo.CoolCtrlZoneNums(ZonesCooledNum);
                    int TermUnitSizingIndex = airToZoneNodeInfo.TermUnitCoolSizingIndex(ZonesCooledNum);
                    Real64 adjHeatMassFlow = state.dataSize->TermUnitSizing(TermUnitSizingIndex)
                                                 .applyTermUnitSizingHeatFlow(
                                                     state.dataSize->ZoneSizing(state.dataSize->CurOverallSimDay, CtrlZoneNum).DesHeatMassFlow,
                                                     state.dataSize->ZoneSizing(state.dataSize->CurOverallSimDay, CtrlZoneNum).DesHeatMassFlowNoOA);
                    state.dataSize->SysSizing(state.dataSize->CurOverallSimDay, AirLoopNum).NonCoinHeatMassFlow +=
                        adjHeatMassFlow / (1.0 + state.dataSize->TermUnitSizing(TermUnitSizingIndex).InducRat);
                } // end of loop over cooled zones
            } // End of heat / cool zone if - else

        } // End of begin day loop over primary air systems
    } break;
    case Constant::CallIndicator::DuringDay: {
        TimeStepInDay = (state.dataGlobal->HourOfDay - 1) * state.dataGlobal->TimeStepsInHour +
                        state.dataGlobal->TimeStep; // calculate current zone time step index

        // Correct the zone return temperature in ZoneSizing for the case of induction units. The calc in
        // ZoneEquipmentManager assumes all the air entering the zone goes into the return node.
        for (int CtrlZoneNum = 1; CtrlZoneNum <= state.dataGlobal->NumOfZones; ++CtrlZoneNum) {
            if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).IsControlled) {
                continue;
            }
            // Use first non-zero airdistunit for now, if there is one
            termunitsizingtempfrac = 1.0;
            int TermUnitSizingIndex = 0;
            for (int InletNode = 1; InletNode <= state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).NumInletNodes; ++InletNode) {
                TermUnitSizingIndex = state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(InletNode).TermUnitSizingIndex;
                if (TermUnitSizingIndex == 0) {
                    continue;
                }
                termunitsizingtemp = (1.0 + state.dataSize->TermUnitSizing(TermUnitSizingIndex).InducRat);
                termunitsizingtempfrac = (1.0 / termunitsizingtemp);
                if (TermUnitSizingIndex > 0) {
                    break;
                }
            }
            if (TermUnitSizingIndex == 0) {
                continue; // Skip this if there are no terminal units
            }
            auto &zoneSizing = state.dataSize->ZoneSizing(state.dataSize->CurOverallSimDay, CtrlZoneNum);
            RetTempRise = zoneSizing.CoolZoneRetTempSeq(TimeStepInDay) - zoneSizing.CoolZoneTempSeq(TimeStepInDay);
            if (RetTempRise > 0.01) {
                zoneSizing.CoolZoneRetTempSeq(TimeStepInDay) = zoneSizing.CoolZoneTempSeq(TimeStepInDay) + RetTempRise * termunitsizingtempfrac;
            }
            RetTempRise = zoneSizing.HeatZoneRetTempSeq(TimeStepInDay) - zoneSizing.HeatZoneTempSeq(TimeStepInDay);
            if (RetTempRise > 0.01) {
                zoneSizing.HeatZoneRetTempSeq(TimeStepInDay) = zoneSizing.HeatZoneTempSeq(TimeStepInDay) + RetTempRise * termunitsizingtempfrac;
            }
        }
        // start of zone time step loop over primary air systems
        for (AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) {

            int NumZonesCooled = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesCooled;
            int NumZonesHeated = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesHeated;

            SysCoolRetTemp = 0.0;
            OutAirFrac = 0.0;
            SysCoolMixTemp = 0.0;
            SysSensCoolCap = 0.0;
            SysCoolRetHumRat = 0.0;
            SysCoolMixHumRat = 0.0;
            SysCoolZoneAvgTemp = 0.0;
            SysHeatZoneAvgTemp = 0.0;
            SysTotCoolCap = 0.0;
            SysDOASHeatAdd = 0.0;
            SysDOASLatAdd = 0.0;
            Real64 SysLatCoolHumRat = 0.0;

            for (int ZonesCooledNum = 1; ZonesCooledNum <= NumZonesCooled; ++ZonesCooledNum) { // loop over zones cooled by central system
                int CtrlZoneNum = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).CoolCtrlZoneNums(ZonesCooledNum);
                int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesCooledNum);
                auto &termUnitSizing = state.dataSize->TermUnitSizing(TermUnitSizingIndex);
                auto &zoneSizing = state.dataSize->ZoneSizing(state.dataSize->CurOverallSimDay, CtrlZoneNum);
                // sum up the system mass flow rate for this time step
                Real64 adjCoolFlowSeq =
                    termUnitSizing.applyTermUnitSizingCoolFlow(zoneSizing.CoolFlowSeq(TimeStepInDay), zoneSizing.CoolFlowSeqNoOA(TimeStepInDay));
                Real64 adjustedFlow = adjCoolFlowSeq / (1.0 + termUnitSizing.InducRat);
                state.dataSize->SysSizing(state.dataSize->CurOverallSimDay, AirLoopNum).CoolFlowSeq(TimeStepInDay) += adjustedFlow;
                // sum up the zone cooling load to be met by this system for this time step
                state.dataSize->SysSizing(state.dataSize->CurOverallSimDay, AirLoopNum).SumZoneCoolLoadSeq(TimeStepInDay) +=
                    termUnitSizing.applyTermUnitSizingCoolLoad(zoneSizing.CoolLoadSeq(TimeStepInDay));
                // calculate the return air temperature for this time step
                SysCoolRetTemp += zoneSizing.CoolZoneRetTempSeq(TimeStepInDay) * adjustedFlow;
                SysCoolRetHumRat += zoneSizing.CoolZoneHumRatSeq(TimeStepInDay) * adjustedFlow;
                SysCoolZoneAvgTemp += zoneSizing.CoolZoneTempSeq(TimeStepInDay) * adjustedFlow;
                SysDOASHeatAdd += zoneSizing.DOASHeatAddSeq(TimeStepInDay) * adjustedFlow;
                SysDOASLatAdd += zoneSizing.DOASLatAddSeq(TimeStepInDay) * adjustedFlow;
                SysLatCoolHumRat += zoneSizing.CoolDesHumRat * adjustedFlow;
            } // end of loop over zones cooled by central system
            // Get peak system cooling load with coincident
            auto &sysSizing = state.dataSize->SysSizing(state.dataSize->CurOverallSimDay, AirLoopNum);
            if (sysSizing.SysDesCoolLoad < sysSizing.SumZoneCoolLoadSeq(TimeStepInDay)) {
                sysSizing.SysDesCoolLoad = sysSizing.SumZoneCoolLoadSeq(TimeStepInDay);
                sysSizing.SysCoolLoadTimeStepPk = TimeStepInDay;
            }
            // check that there is system mass flow
            if (sysSizing.CoolFlowSeq(TimeStepInDay) > 0.0) {
                // complete return air temp calc
                SysLatCoolHumRat /= sysSizing.CoolFlowSeq(TimeStepInDay);
                SysCoolRetTemp /= sysSizing.CoolFlowSeq(TimeStepInDay);
                SysCoolRetHumRat /= sysSizing.CoolFlowSeq(TimeStepInDay);
                SysCoolZoneAvgTemp /= sysSizing.CoolFlowSeq(TimeStepInDay);
                sysSizing.SysCoolRetTempSeq(TimeStepInDay) = SysCoolRetTemp;
                sysSizing.SysCoolRetHumRatSeq(TimeStepInDay) = SysCoolRetHumRat;
                sysSizing.CoolZoneAvgTempSeq(TimeStepInDay) = SysCoolZoneAvgTemp;
                // calculate the outside air fraction for this time step
                RhoAir = state.dataEnvrn->StdRhoAir;
                if (sysSizing.CoolOAOption == OAControl::MinOA) {
                    OutAirFrac = RhoAir * sysSizing.DesOutAirVolFlow / sysSizing.CoolFlowSeq(TimeStepInDay);
                    OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                } else {
                    OutAirFrac = 1.0;
                }
                // now calculate the mixed air temperature
                SysCoolMixTemp = state.dataEnvrn->OutDryBulbTemp * OutAirFrac + SysCoolRetTemp * (1.0 - OutAirFrac);
                SysCoolMixHumRat = state.dataEnvrn->OutHumRat * OutAirFrac + SysCoolRetHumRat * (1.0 - OutAirFrac);
                sysSizing.SysCoolOutTempSeq(TimeStepInDay) = state.dataEnvrn->OutDryBulbTemp;
                sysSizing.SysCoolOutHumRatSeq(TimeStepInDay) = state.dataEnvrn->OutHumRat;
                // adjust supply air humidity ratio to meet latent load
                if (sysSizing.loadSizingType == DataSizing::LoadSizing::Latent) {
                    if (state.dataHeatBal->isAnyLatentLoad) {
                        sysSizing.CoolSupHumRat = std::min(SysLatCoolHumRat, sysSizing.CoolSupHumRat);
                        state.dataSize->FinalSysSizing(AirLoopNum).CoolSupHumRat = sysSizing.CoolSupHumRat;
                    } else {
                        // switch back to sensible load if all latent zone loads are smaller than sensible load
                        sysSizing.coolingPeakLoad = DataSizing::PeakLoad::SensibleCooling;
                        state.dataSize->FinalSysSizing(AirLoopNum).coolingPeakLoad = DataSizing::PeakLoad::SensibleCooling;
                    }
                }
                // From the mixed air temp, system design supply air temp, and the mass flow rate
                // calculate the system sensible cooling capacity
                SysSensCoolCap = PsyCpAirFnW(DataPrecisionGlobals::constant_zero) * sysSizing.CoolFlowSeq(TimeStepInDay) *
                                 (SysCoolMixTemp - sysSizing.CoolSupTemp);
                SysSensCoolCap = max(0.0, SysSensCoolCap);
                // calculate the system total cooling capacity
                SysTotCoolCap = sysSizing.CoolFlowSeq(TimeStepInDay) *
                                (PsyHFnTdbW(SysCoolMixTemp, SysCoolMixHumRat) - PsyHFnTdbW(sysSizing.CoolSupTemp, sysSizing.CoolSupHumRat));
                SysTotCoolCap = max(0.0, SysTotCoolCap);
                // Save the sens cool cap for this time step
                sysSizing.SensCoolCapSeq(TimeStepInDay) = SysSensCoolCap;
                // Save the tot cool cap for this time step
                sysSizing.TotCoolCapSeq(TimeStepInDay) = SysTotCoolCap;
                // Save the DOAS flows
                sysSizing.SysDOASHeatAddSeq(TimeStepInDay) = SysDOASHeatAdd;
                sysSizing.SysDOASLatAddSeq(TimeStepInDay) = SysDOASLatAdd;
            } // end of system mass flow check

            // get the maximum system sensible cooling capacity
            if (SysSensCoolCap > state.dataSize->SensCoolCapTemp(AirLoopNum)) {
                state.dataSize->SysSizPeakDDNum(AirLoopNum).TimeStepAtSensCoolPk(state.dataSize->CurOverallSimDay) = TimeStepInDay;
                state.dataSize->SensCoolCapTemp(AirLoopNum) = SysSensCoolCap;
                if (sysSizing.coolingPeakLoad == DataSizing::PeakLoad::SensibleCooling) {
                    sysSizing.SensCoolCap = SysSensCoolCap;
                    sysSizing.TotCoolCap = SysTotCoolCap;
                    sysSizing.MixTempAtCoolPeak = SysCoolMixTemp;
                    sysSizing.MixHumRatAtCoolPeak = SysCoolMixHumRat;
                    sysSizing.RetTempAtCoolPeak = SysCoolRetTemp;
                    sysSizing.RetHumRatAtCoolPeak = SysCoolRetHumRat;
                    sysSizing.OutTempAtCoolPeak = state.dataEnvrn->OutDryBulbTemp;
                    sysSizing.OutHumRatAtCoolPeak = state.dataEnvrn->OutHumRat;
                    sysSizing.MassFlowAtCoolPeak = sysSizing.CoolFlowSeq(TimeStepInDay);
                }
            }
            // get the maximum system total cooling capacity
            if (SysTotCoolCap > state.dataSize->TotCoolCapTemp(AirLoopNum)) {
                state.dataSize->SysSizPeakDDNum(AirLoopNum).TimeStepAtTotCoolPk(state.dataSize->CurOverallSimDay) = TimeStepInDay;
                state.dataSize->TotCoolCapTemp(AirLoopNum) = SysTotCoolCap;
                if (sysSizing.coolingPeakLoad == DataSizing::PeakLoad::TotalCooling) {
                    sysSizing.SensCoolCap = SysSensCoolCap;
                    sysSizing.TotCoolCap = SysTotCoolCap;
                    sysSizing.MixTempAtCoolPeak = SysCoolMixTemp;
                    sysSizing.MixHumRatAtCoolPeak = SysCoolMixHumRat;
                    sysSizing.RetTempAtCoolPeak = SysCoolRetTemp;
                    sysSizing.RetHumRatAtCoolPeak = SysCoolRetHumRat;
                    sysSizing.OutTempAtCoolPeak = state.dataEnvrn->OutDryBulbTemp;
                    sysSizing.OutHumRatAtCoolPeak = state.dataEnvrn->OutHumRat;
                    sysSizing.MassFlowAtCoolPeak = sysSizing.CoolFlowSeq(TimeStepInDay);
                }
                sysSizing.SysCoolCoinSpaceSens = 0.0;
                for (int zonesCoolLoop = 1; zonesCoolLoop <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesCooled; ++zonesCoolLoop) {
                    int zoneNum = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).CoolCtrlZoneNums(zonesCoolLoop);
                    sysSizing.SysCoolCoinSpaceSens +=
                        state.dataSize->CalcZoneSizing(state.dataSize->CurOverallSimDay, zoneNum).CoolLoadSeq(TimeStepInDay);
                }
            }
            // get the maximum cooling mass flow rate
            if (sysSizing.CoolFlowSeq(TimeStepInDay) > sysSizing.CoinCoolMassFlow) {
                sysSizing.CoinCoolMassFlow = sysSizing.CoolFlowSeq(TimeStepInDay);
                state.dataSize->SysSizPeakDDNum(AirLoopNum).TimeStepAtCoolFlowPk(state.dataSize->CurOverallSimDay) = TimeStepInDay;
            }
            SysHeatRetTemp = 0.0;
            OutAirFrac = 0.0;
            SysHeatMixTemp = 0.0;
            SysHeatCap = 0.0;
            SysHeatRetHumRat = 0.0;
            SysHeatMixHumRat = 0.0;

            if (NumZonesHeated > 0) { // IF there are centrally heated zones

                for (int ZonesHeatedNum = 1; ZonesHeatedNum <= NumZonesHeated; ++ZonesHeatedNum) { // loop over the heated zones
                    int CtrlZoneNum = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).HeatCtrlZoneNums(ZonesHeatedNum);
                    int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitHeatSizingIndex(ZonesHeatedNum);
                    auto &termUnitSizing = state.dataSize->TermUnitSizing(TermUnitSizingIndex);
                    auto &zoneSizing = state.dataSize->ZoneSizing(state.dataSize->CurOverallSimDay, CtrlZoneNum);
                    // sum up the heating mass flow rate for this time step
                    Real64 adjHeatFlowSeq =
                        termUnitSizing.applyTermUnitSizingHeatFlow(zoneSizing.HeatFlowSeq(TimeStepInDay), zoneSizing.HeatFlowSeqNoOA(TimeStepInDay));
                    Real64 adjustedFlow = adjHeatFlowSeq / (1.0 + termUnitSizing.InducRat);
                    sysSizing.HeatFlowSeq(TimeStepInDay) += adjustedFlow;
                    // sum up the zone heating load to be met by this system for this time step
                    sysSizing.SumZoneHeatLoadSeq(TimeStepInDay) +=
                        termUnitSizing.applyTermUnitSizingHeatLoad(zoneSizing.HeatLoadSeq(TimeStepInDay) / (1.0 + termUnitSizing.InducRat));
                    // calculate the return air temperature for this time step
                    SysHeatRetTemp += zoneSizing.HeatZoneRetTempSeq(TimeStepInDay) * adjustedFlow;
                    SysHeatRetHumRat += zoneSizing.HeatZoneHumRatSeq(TimeStepInDay) * adjustedFlow;
                    SysHeatZoneAvgTemp += zoneSizing.HeatZoneTempSeq(TimeStepInDay) * adjustedFlow;
                } // end heated zones loop
                // Get peak system heating load with coincident
                if (std::abs(sysSizing.SysDesHeatLoad) > std::abs(sysSizing.SumZoneHeatLoadSeq(TimeStepInDay))) {
                    sysSizing.SysDesHeatLoad = sysSizing.SumZoneHeatLoadSeq(TimeStepInDay);
                    sysSizing.SysHeatLoadTimeStepPk = TimeStepInDay;
                }
                // check that the system flow rate is nonzero
                if (sysSizing.HeatFlowSeq(TimeStepInDay) > 0.0) {
                    // complete return air temp calc
                    SysHeatRetTemp /= sysSizing.HeatFlowSeq(TimeStepInDay);
                    SysHeatRetHumRat /= sysSizing.HeatFlowSeq(TimeStepInDay);
                    SysHeatZoneAvgTemp /= sysSizing.HeatFlowSeq(TimeStepInDay);
                    sysSizing.SysHeatRetTempSeq(TimeStepInDay) = SysHeatRetTemp;
                    sysSizing.SysHeatRetHumRatSeq(TimeStepInDay) = SysHeatRetHumRat;
                    sysSizing.HeatZoneAvgTempSeq(TimeStepInDay) = SysHeatZoneAvgTemp;
                    // calculate the outside air fraction for this time step
                    RhoAir = state.dataEnvrn->StdRhoAir;
                    if (sysSizing.HeatOAOption == DataSizing::OAControl::MinOA) {
                        OutAirFrac = RhoAir * sysSizing.DesOutAirVolFlow / sysSizing.HeatFlowSeq(TimeStepInDay);
                        OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                    } else {
                        OutAirFrac = 1.0;
                    }
                    // calculate the mixed air temperature
                    SysHeatMixTemp = state.dataEnvrn->OutDryBulbTemp * OutAirFrac + SysHeatRetTemp * (1.0 - OutAirFrac);
                    SysHeatMixHumRat = state.dataEnvrn->OutHumRat * OutAirFrac + SysHeatRetHumRat * (1.0 - OutAirFrac);
                    sysSizing.SysHeatOutTempSeq(TimeStepInDay) = state.dataEnvrn->OutDryBulbTemp;
                    sysSizing.SysHeatOutHumRatSeq(TimeStepInDay) = state.dataEnvrn->OutHumRat;
                    // From the mixed air temp, heating supply air temp, and mass flow rate calculate the system heating capacity
                    SysHeatCap = PsyCpAirFnW(DataPrecisionGlobals::constant_zero) * sysSizing.HeatFlowSeq(TimeStepInDay) *
                                 (sysSizing.HeatSupTemp - SysHeatMixTemp);
                    SysHeatCap = max(0.0, SysHeatCap);
                    // save the system heating capacity for the time step
                    sysSizing.HeatCapSeq(TimeStepInDay) = SysHeatCap;
                } // end system flow rate IF

                // Get the maximum system heating capacity
                if (SysHeatCap > sysSizing.HeatCap) {
                    state.dataSize->SysSizPeakDDNum(AirLoopNum).TimeStepAtHeatPk(state.dataSize->CurOverallSimDay) = TimeStepInDay;
                    sysSizing.HeatCap = SysHeatCap;
                    sysSizing.HeatMixTemp = SysHeatMixTemp;
                    sysSizing.HeatMixHumRat = SysHeatMixHumRat;
                    sysSizing.HeatRetTemp = SysHeatRetTemp;
                    sysSizing.HeatRetHumRat = SysHeatRetHumRat;
                    sysSizing.HeatOutTemp = state.dataEnvrn->OutDryBulbTemp;
                    sysSizing.HeatOutHumRat = state.dataEnvrn->OutHumRat;
                    // save time of system coincident heating coil peak
                    sysSizing.SysHeatCoilTimeStepPk = TimeStepInDay;
                    sysSizing.SysHeatCoinSpaceSens = 0.0;
                    if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesHeated > 0) {
                        for (int zonesHeatLoop = 1; zonesHeatLoop <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesHeated;
                             ++zonesHeatLoop) {
                            int zoneNum = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).HeatCtrlZoneNums(zonesHeatLoop);
                            sysSizing.SysHeatCoinSpaceSens +=
                                state.dataSize->CalcZoneSizing(state.dataSize->CurOverallSimDay, zoneNum).HeatLoadSeq(TimeStepInDay);
                        }
                    }
                }
                //! save time of system coincident heating airflow peak
                if (sysSizing.HeatFlowSeq(TimeStepInDay) > sysSizing.CoinHeatMassFlow) {
                    sysSizing.SysHeatAirTimeStepPk = TimeStepInDay;
                }

                // Get the maximum system heating flow rate
                sysSizing.CoinHeatMassFlow = max(sysSizing.CoinHeatMassFlow, sysSizing.HeatFlowSeq(TimeStepInDay));

            } else { // No centrally heated zones: use cooled zones

                for (int ZonesCooledNum = 1; ZonesCooledNum <= NumZonesCooled; ++ZonesCooledNum) { // loop over the cooled zones
                    int CtrlZoneNum = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).CoolCtrlZoneNums(ZonesCooledNum);
                    auto &termUnitSizing =
                        state.dataSize->TermUnitSizing(state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesCooledNum));
                    auto &zoneSizing = state.dataSize->ZoneSizing(state.dataSize->CurOverallSimDay, CtrlZoneNum);
                    // sum up the heating mass flow rate for this time step
                    Real64 adjHeatFlowSeq =
                        termUnitSizing.applyTermUnitSizingHeatFlow(zoneSizing.HeatFlowSeq(TimeStepInDay), zoneSizing.HeatFlowSeqNoOA(TimeStepInDay));
                    Real64 adjustedFlow = adjHeatFlowSeq / (1.0 + termUnitSizing.InducRat);
                    sysSizing.HeatFlowSeq(TimeStepInDay) += adjustedFlow;
                    // sum up the zone heating load to be met by this system for this time step
                    sysSizing.SumZoneHeatLoadSeq(TimeStepInDay) +=
                        termUnitSizing.applyTermUnitSizingHeatLoad(zoneSizing.HeatLoadSeq(TimeStepInDay) / (1.0 + termUnitSizing.InducRat));
                    // calculate the return air temperature for this time step
                    SysHeatRetTemp += zoneSizing.HeatZoneRetTempSeq(TimeStepInDay) * adjustedFlow;
                    SysHeatRetHumRat += zoneSizing.HeatZoneHumRatSeq(TimeStepInDay) * adjustedFlow;
                    SysHeatZoneAvgTemp += zoneSizing.HeatZoneTempSeq(TimeStepInDay) * adjustedFlow;
                } // end of cooled zones loop
                // Get peak system heating load with coincident
                if (fabs(sysSizing.SysDesHeatLoad) < fabs(sysSizing.SumZoneHeatLoadSeq(TimeStepInDay))) {
                    sysSizing.SysDesHeatLoad = sysSizing.SumZoneHeatLoadSeq(TimeStepInDay);
                    sysSizing.SysHeatLoadTimeStepPk = TimeStepInDay;
                }

                if (sysSizing.HeatFlowSeq(TimeStepInDay) > 0.0) {
                    // complete return air temp calc
                    SysHeatRetTemp /= sysSizing.HeatFlowSeq(TimeStepInDay);
                    SysHeatRetHumRat /= sysSizing.HeatFlowSeq(TimeStepInDay);
                    SysHeatZoneAvgTemp /= sysSizing.HeatFlowSeq(TimeStepInDay);
                    sysSizing.SysHeatRetTempSeq(TimeStepInDay) = SysHeatRetTemp;
                    sysSizing.SysHeatRetHumRatSeq(TimeStepInDay) = SysHeatRetHumRat;
                    sysSizing.HeatZoneAvgTempSeq(TimeStepInDay) = SysHeatZoneAvgTemp;
                    // calculate the outside air fraction for this time step
                    RhoAir = state.dataEnvrn->StdRhoAir;
                    if (sysSizing.HeatOAOption == DataSizing::OAControl::MinOA) {
                        OutAirFrac = RhoAir * sysSizing.DesOutAirVolFlow / sysSizing.HeatFlowSeq(TimeStepInDay);
                        OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                    } else {
                        OutAirFrac = 1.0;
                    }
                    // calculate the mixed air temperature
                    SysHeatMixTemp = state.dataEnvrn->OutDryBulbTemp * OutAirFrac + SysHeatRetTemp * (1.0 - OutAirFrac);
                    SysHeatMixHumRat = state.dataEnvrn->OutHumRat * OutAirFrac + SysHeatRetHumRat * (1.0 - OutAirFrac);
                    sysSizing.SysHeatOutTempSeq(TimeStepInDay) = state.dataEnvrn->OutDryBulbTemp;
                    sysSizing.SysHeatOutHumRatSeq(TimeStepInDay) = state.dataEnvrn->OutHumRat;
                    // From the mixed air temp, heating supply air temp, and mass flow rate calculate the system heating capacity
                    SysHeatCap = PsyCpAirFnW(DataPrecisionGlobals::constant_zero) * sysSizing.HeatFlowSeq(TimeStepInDay) *
                                 (sysSizing.HeatSupTemp - SysHeatMixTemp);
                    SysHeatCap = max(0.0, SysHeatCap);
                    // save the system heating capacity for the time step
                    sysSizing.HeatCapSeq(TimeStepInDay) = SysHeatCap;
                } // end system flow rate IF

                // Get the maximum system heating capacity
                if (SysHeatCap > sysSizing.HeatCap) {
                    state.dataSize->SysSizPeakDDNum(AirLoopNum).TimeStepAtHeatPk(state.dataSize->CurOverallSimDay) = TimeStepInDay;
                    sysSizing.HeatCap = SysHeatCap;
                    sysSizing.HeatMixTemp = SysHeatMixTemp;
                    sysSizing.HeatMixHumRat = SysHeatMixHumRat;
                    sysSizing.HeatRetTemp = SysHeatRetTemp;
                    sysSizing.HeatRetHumRat = SysHeatRetHumRat;
                    sysSizing.HeatOutTemp = state.dataEnvrn->OutDryBulbTemp;
                    sysSizing.HeatOutHumRat = state.dataEnvrn->OutHumRat;
                    // save time of system coincident heating coil peak
                    sysSizing.SysHeatCoilTimeStepPk = TimeStepInDay;

                    sysSizing.SysHeatCoinSpaceSens = 0.0;
                    for (int zonesCoolLoop = 1; zonesCoolLoop <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesCooled; ++zonesCoolLoop) {
                        int zoneNum = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).CoolCtrlZoneNums(zonesCoolLoop);
                        sysSizing.SysHeatCoinSpaceSens +=
                            state.dataSize->CalcZoneSizing(state.dataSize->CurOverallSimDay, zoneNum).HeatLoadSeq(TimeStepInDay);
                    }
                } // Get the maximum system heating flow rate
                // save time of system coincident heating airflow peak
                if (sysSizing.HeatFlowSeq(TimeStepInDay) > sysSizing.CoinHeatMassFlow) {
                    sysSizing.SysHeatAirTimeStepPk = TimeStepInDay;
                }

                sysSizing.CoinHeatMassFlow = max(sysSizing.CoinHeatMassFlow, sysSizing.HeatFlowSeq(TimeStepInDay));
            }

        } // end of loop over primary air systems
    } break;
    case Constant::CallIndicator::EndDay: {
        // the entire set of std. 62.1 code here seems misplaced, should have been placed in EndSysSizCalc block
        // Get design flows
        SysCoolingEv = 1.0;
        SysHeatingEv = 1.0;
        for (AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) {
            auto &finalSysSizing = state.dataSize->FinalSysSizing(AirLoopNum);
            int NumZonesCooled = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesCooled;
            int NumZonesHeated = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesHeated;
            auto &sysSizing = state.dataSize->SysSizing(state.dataSize->CurOverallSimDay, AirLoopNum);

            switch (sysSizing.SizingOption) {
            case DataSizing::SizingConcurrence::Coincident: {
                if (finalSysSizing.SystemOAMethod == SysOAMethod::ZoneSum) {
                    sysSizing.DesCoolVolFlow = sysSizing.CoinCoolMassFlow / state.dataEnvrn->StdRhoAir;
                    sysSizing.DesHeatVolFlow = sysSizing.CoinHeatMassFlow / state.dataEnvrn->StdRhoAir;
                    state.dataSize->VotClgBySys(AirLoopNum) = finalSysSizing.SysUncOA;
                    state.dataSize->VotHtgBySys(AirLoopNum) = finalSysSizing.SysUncOA;
                    for (int ZonesCooledNum = 1; ZonesCooledNum <= NumZonesCooled; ++ZonesCooledNum) {
                        int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesCooledNum);
                        if (state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffCooling <
                            state.dataSize->EvzMinBySysCool(AirLoopNum)) {
                            state.dataSize->EvzMinBySysCool(AirLoopNum) =
                                state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffCooling;
                        }
                        if (state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffHeating <
                            state.dataSize->EvzMinBySysHeat(AirLoopNum)) {
                            state.dataSize->EvzMinBySysHeat(AirLoopNum) =
                                state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffHeating;
                        }
                    }
                    for (int ZonesHeatedNum = 1; ZonesHeatedNum <= NumZonesHeated; ++ZonesHeatedNum) {
                        int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitHeatSizingIndex(ZonesHeatedNum);
                        if (state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffCooling <
                            state.dataSize->EvzMinBySysCool(AirLoopNum)) {
                            state.dataSize->EvzMinBySysCool(AirLoopNum) =
                                state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffCooling;
                        }
                        if (state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffHeating <
                            state.dataSize->EvzMinBySysHeat(AirLoopNum)) {
                            state.dataSize->EvzMinBySysHeat(AirLoopNum) =
                                state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffHeating;
                        }
                    }
                    if (sysSizing.DesCoolVolFlow > 0) {
                        state.dataSize->XsBySysCool(AirLoopNum) = min(1.0, finalSysSizing.SysUncOA / sysSizing.DesCoolVolFlow);
                    } else {
                        state.dataSize->XsBySysCool(AirLoopNum) = 0.0;
                    }
                    if (sysSizing.DesHeatVolFlow > 0) {
                        state.dataSize->XsBySysHeat(AirLoopNum) = min(1.0, finalSysSizing.SysUncOA / sysSizing.DesHeatVolFlow);
                    } else {
                        state.dataSize->XsBySysHeat(AirLoopNum) = 0.0;
                    }
                } else if (finalSysSizing.SystemOAMethod == SysOAMethod::VRP ||
                           finalSysSizing.SystemOAMethod == SysOAMethod::SP) { // Ventilation Rate and Simplified Procedure
                    // cooling
                    sysSizing.DesCoolVolFlow = sysSizing.CoinCoolMassFlow / state.dataEnvrn->StdRhoAir;
                    if (sysSizing.DesCoolVolFlow > 0) {
                        OutAirFrac = sysSizing.DesOutAirVolFlow / sysSizing.DesCoolVolFlow;
                    } else {
                        OutAirFrac = 0.0;
                    }
                    OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                    if (sysSizing.DesCoolVolFlow > 0) {
                        state.dataSimAirServingZones->Xs = min(1.0, finalSysSizing.SysUncOA / sysSizing.DesCoolVolFlow);
                    } else {
                        state.dataSimAirServingZones->Xs = 0.0;
                    }
                    if (finalSysSizing.OAAutoSized && sysSizing.DesCoolVolFlow > 0) {
                        int numZonesCooled = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesCooled;
                        state.dataSimAirServingZones->MinCoolingEvz = 1.0;
                        state.dataSize->VozSumClgBySys(AirLoopNum) = 0.0;
                        for (int ZonesCooledNum = 1; ZonesCooledNum <= numZonesCooled; ++ZonesCooledNum) {
                            int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesCooledNum);

                            // Zone air secondary recirculation fraction
                            state.dataSimAirServingZones->Er =
                                state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneSecondaryRecirculation;
                            state.dataSimAirServingZones->Ep = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZonePrimaryAirFraction;
                            state.dataSimAirServingZones->ZoneOAFrac = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZpzClgByZone;
                            state.dataSimAirServingZones->ZoneEz = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffCooling;
                            VozClg = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).VozClgByZone;
                            if (finalSysSizing.SystemOAMethod == SysOAMethod::SP) { // 62.1 simplified procedure
                                if (state.dataSize->DBySys(AirLoopNum) < 0.60) {
                                    state.dataSize->EvzByZoneCool(TermUnitSizingIndex) = 0.88 * state.dataSize->DBySys(AirLoopNum) + 0.22;
                                } else {
                                    state.dataSize->EvzByZoneCool(TermUnitSizingIndex) = 0.75;
                                }
                                state.dataSimAirServingZones->MinCoolingEvz = state.dataSize->EvzByZoneCool(TermUnitSizingIndex);
                            } else {
                                if (state.dataSimAirServingZones->Er > 0.0) {
                                    // multi-path ventilation system using VRP
                                    state.dataSimAirServingZones->Fa = state.dataSimAirServingZones->Ep +
                                                                       (1.0 - state.dataSimAirServingZones->Ep) * state.dataSimAirServingZones->Er;
                                    state.dataSimAirServingZones->Fb = state.dataSimAirServingZones->Ep;
                                    state.dataSimAirServingZones->Fc = 1.0 - (1.0 - state.dataSimAirServingZones->ZoneEz) *
                                                                                 (1.0 - state.dataSimAirServingZones->Er) *
                                                                                 (1.0 - state.dataSimAirServingZones->Ep);
                                    // save Fa Fb and Fc for standard 62.1 report
                                    state.dataSize->FaByZoneCool(TermUnitSizingIndex) = state.dataSimAirServingZones->Fa;
                                    state.dataSize->FbByZoneCool(TermUnitSizingIndex) = state.dataSimAirServingZones->Fb;
                                    state.dataSize->FcByZoneCool(TermUnitSizingIndex) = state.dataSimAirServingZones->Fc;

                                    // Calc zone ventilation efficiency
                                    if (state.dataSimAirServingZones->Fa > 0.0) {
                                        SysCoolingEv =
                                            1.0 +
                                            state.dataSimAirServingZones->Xs * state.dataSimAirServingZones->Fb / state.dataSimAirServingZones->Fa -
                                            state.dataSimAirServingZones->ZoneOAFrac * state.dataSimAirServingZones->Ep *
                                                state.dataSimAirServingZones->Fc / state.dataSimAirServingZones->Fa;
                                    } else {
                                        SysCoolingEv = 1.0;
                                    }

                                } else {
                                    // single-path ventilation system
                                    SysCoolingEv = 1.0 + state.dataSimAirServingZones->Xs - state.dataSimAirServingZones->ZoneOAFrac;
                                    // Apply ventilation efficiency limit; reset SysCoolingEv if necessary
                                    LimitZoneVentEff(state, state.dataSimAirServingZones->Xs, VozClg, TermUnitSizingIndex, SysCoolingEv);
                                }
                                if (SysCoolingEv < state.dataSimAirServingZones->MinCoolingEvz) {
                                    state.dataSimAirServingZones->MinCoolingEvz = SysCoolingEv;
                                }
                                state.dataSize->EvzByZoneCoolPrev(TermUnitSizingIndex) =
                                    state.dataSize->EvzByZoneCool(TermUnitSizingIndex); // Save previous EvzByZoneCool
                                state.dataSize->EvzByZoneCool(TermUnitSizingIndex) = SysCoolingEv;
                            }
                            state.dataSize->VozSumClgBySys(AirLoopNum) += state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).VozClgByZone;
                        }

                        if (state.dataSimAirServingZones->MinCoolingEvz > 0) {
                            // (However, I don't think people diversity can be done correctly in E+ Sizing so assuming D=1 in this
                            // equation
                            // Vou = Diversity*(Rp*Pz) + Ra*Az
                            state.dataSimAirServingZones->Vou = finalSysSizing.SysUncOA;
                            state.dataSimAirServingZones->Vot = state.dataSimAirServingZones->Vou / state.dataSimAirServingZones->MinCoolingEvz;
                            if (state.dataSimAirServingZones->Vot > state.dataSize->VotClgBySys(AirLoopNum)) {
                                // This might be the cooling design day so only update if Vot is larger than the previous
                                state.dataSize->VotClgBySys(AirLoopNum) = state.dataSimAirServingZones->Vot;
                                state.dataSize->XsBySysCool(AirLoopNum) = state.dataSimAirServingZones->Xs;
                                state.dataSize->EvzMinBySysCool(AirLoopNum) = state.dataSimAirServingZones->MinCoolingEvz;
                            } else {
                                // Restore EvzByZoneCool() since it was reset by the current (but not highest Vot) design day
                                for (int ZonesCooledNum = 1; ZonesCooledNum <= numZonesCooled; ++ZonesCooledNum) {
                                    int TermUnitSizingIndex =
                                        state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesCooledNum);
                                    state.dataSize->EvzByZoneCool(TermUnitSizingIndex) = state.dataSize->EvzByZoneCoolPrev(TermUnitSizingIndex);
                                }
                            }
                        }
                    }

                    // heating
                    sysSizing.DesHeatVolFlow = sysSizing.CoinHeatMassFlow / state.dataEnvrn->StdRhoAir;
                    if (sysSizing.DesHeatVolFlow > 0) {
                        OutAirFrac = sysSizing.DesOutAirVolFlow / sysSizing.DesHeatVolFlow;
                    } else {
                        OutAirFrac = 0.0;
                    }
                    OutAirFrac = min(1.0, max(0.0, OutAirFrac));

                    // This is a bit of a cludge. If the design zone heating airflows were increased due to
                    // the MaxZoneOaFraction, then the SysSizing(AirLoopNum,state.dataSize->CurOverallSimDay)%DesHeatVolFlow
                    // variable will be out of sync with the
                    if (finalSysSizing.MaxZoneOAFraction > 0 && finalSysSizing.HeatAirDesMethod == AirflowSizingMethod::FromDDCalc) {
                        SysHtgPeakAirflow = 0.0;
                        if (NumZonesHeated > 0) {
                            for (int ZonesHeatedNum = 1; ZonesHeatedNum <= NumZonesHeated; ++ZonesHeatedNum) {
                                int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitHeatSizingIndex(ZonesHeatedNum);
                                SysHtgPeakAirflow += state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).DesHeatVolFlow;
                            }
                        } else {
                            for (int ZonesHeatedNum = 1; ZonesHeatedNum <= NumZonesCooled; ++ZonesHeatedNum) {
                                int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesHeatedNum);
                                SysHtgPeakAirflow += state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).DesHeatVolFlow;
                            }
                        }
                    } else {
                        SysHtgPeakAirflow = sysSizing.DesHeatVolFlow;
                    }

                    if (sysSizing.DesHeatVolFlow > 0) {
                        // SysSizing(AirLoopNum,state.dataSize->CurOverallSimDay)%DesHeatVolFlow may be out of sync with
                        // FinalZoneSizing(CtrlZoneNum)%DesHeatVolFlow
                        state.dataSimAirServingZones->Xs = min(1.0, finalSysSizing.SysUncOA / max(sysSizing.DesHeatVolFlow, SysHtgPeakAirflow));
                    } else {
                        state.dataSimAirServingZones->Xs = 0.0;
                    }

                    if (finalSysSizing.OAAutoSized && sysSizing.DesHeatVolFlow > 0) {
                        int numZonesHeated = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesHeated;
                        state.dataSimAirServingZones->MinHeatingEvz = 1.0;
                        state.dataSize->VozSumHtgBySys(AirLoopNum) = 0.0;
                        if (numZonesHeated > 0) {
                            for (int ZonesHeatedNum = 1; ZonesHeatedNum <= numZonesHeated; ++ZonesHeatedNum) {
                                int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitHeatSizingIndex(ZonesHeatedNum);
                                MatchingCooledZoneNum = FindNumberInList(
                                    TermUnitSizingIndex, state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex, NumZonesCooled);
                                if (MatchingCooledZoneNum == 0) {
                                    // Zone air secondary recirculation fraction
                                    state.dataSimAirServingZones->Er =
                                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneSecondaryRecirculation;
                                    state.dataSimAirServingZones->Ep =
                                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZonePrimaryAirFractionHtg;
                                    state.dataSimAirServingZones->ZoneOAFrac =
                                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZpzHtgByZone;
                                    state.dataSimAirServingZones->ZoneEz =
                                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffHeating;
                                    if (finalSysSizing.SystemOAMethod == SysOAMethod::SP) { // 62.1 simplified procedure
                                        if (state.dataSize->DBySys(AirLoopNum) < 0.60) {
                                            state.dataSize->EvzByZoneHeat(TermUnitSizingIndex) = 0.88 * state.dataSize->DBySys(AirLoopNum) + 0.22;
                                        } else {
                                            state.dataSize->EvzByZoneHeat(TermUnitSizingIndex) = 0.75;
                                        }
                                        state.dataSimAirServingZones->MinHeatingEvz = state.dataSize->EvzByZoneHeat(TermUnitSizingIndex);
                                    } else {
                                        if (state.dataSimAirServingZones->Er > 0.0) {
                                            // multi-path ventilation system using VRP
                                            state.dataSimAirServingZones->Fa =
                                                state.dataSimAirServingZones->Ep +
                                                (1.0 - state.dataSimAirServingZones->Ep) * state.dataSimAirServingZones->Er;
                                            state.dataSimAirServingZones->Fb = state.dataSimAirServingZones->Ep;
                                            state.dataSimAirServingZones->Fc = 1.0 - (1.0 - state.dataSimAirServingZones->ZoneEz) *
                                                                                         (1.0 - state.dataSimAirServingZones->Er) *
                                                                                         (1.0 - state.dataSimAirServingZones->Ep);
                                            // save Fa Fb and Fc for standard 62.1 report
                                            state.dataSize->FaByZoneHeat(TermUnitSizingIndex) = state.dataSimAirServingZones->Fa;
                                            state.dataSize->FbByZoneHeat(TermUnitSizingIndex) = state.dataSimAirServingZones->Fb;
                                            state.dataSize->FcByZoneHeat(TermUnitSizingIndex) = state.dataSimAirServingZones->Fc;

                                            // Calc zone ventilation efficiency
                                            if (state.dataSimAirServingZones->Fa > 0.0) {
                                                SysHeatingEv = 1.0 +
                                                               state.dataSimAirServingZones->Xs * state.dataSimAirServingZones->Fb /
                                                                   state.dataSimAirServingZones->Fa -
                                                               state.dataSimAirServingZones->ZoneOAFrac * state.dataSimAirServingZones->Ep *
                                                                   state.dataSimAirServingZones->Fc / state.dataSimAirServingZones->Fa;
                                            } else {
                                                SysHeatingEv = 1.0;
                                            }
                                        } else {
                                            // single-path ventilation system
                                            SysHeatingEv = 1.0 + state.dataSimAirServingZones->Xs - state.dataSimAirServingZones->ZoneOAFrac;
                                        }
                                        if (SysHeatingEv < state.dataSimAirServingZones->MinHeatingEvz) {
                                            state.dataSimAirServingZones->MinHeatingEvz = SysHeatingEv;
                                        }
                                        state.dataSize->EvzByZoneHeatPrev(TermUnitSizingIndex) =
                                            state.dataSize->EvzByZoneHeat(TermUnitSizingIndex); // Save previous EvzByZoneHeat
                                        state.dataSize->EvzByZoneHeat(TermUnitSizingIndex) = SysHeatingEv;
                                    }
                                    state.dataSize->VozSumHtgBySys(AirLoopNum) +=
                                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).VozHtgByZone;
                                }
                            }
                        } else {
                            for (int ZonesHeatedNum = 1; ZonesHeatedNum <= NumZonesCooled; ++ZonesHeatedNum) {
                                int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesHeatedNum);
                                // Zone air secondary recirculation fraction
                                state.dataSimAirServingZones->Er =
                                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneSecondaryRecirculation;
                                state.dataSimAirServingZones->Ep =
                                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZonePrimaryAirFractionHtg;
                                state.dataSimAirServingZones->ZoneOAFrac = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZpzHtgByZone;
                                state.dataSimAirServingZones->ZoneEz = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffHeating;
                                if (finalSysSizing.SystemOAMethod == SysOAMethod::SP) { // 62.1 simplified procedure
                                    if (state.dataSize->DBySys(AirLoopNum) < 0.60) {
                                        state.dataSize->EvzByZoneHeat(TermUnitSizingIndex) = 0.88 * state.dataSize->DBySys(AirLoopNum) + 0.22;
                                    } else {
                                        state.dataSize->EvzByZoneHeat(TermUnitSizingIndex) = 0.75;
                                    }
                                    state.dataSimAirServingZones->MinHeatingEvz = state.dataSize->EvzByZoneHeat(TermUnitSizingIndex);
                                } else {
                                    if (state.dataSimAirServingZones->Er > 0.0) {
                                        // multi-path ventilation system using VRP
                                        state.dataSimAirServingZones->Fa =
                                            state.dataSimAirServingZones->Ep +
                                            (1.0 - state.dataSimAirServingZones->Ep) * state.dataSimAirServingZones->Er;
                                        state.dataSimAirServingZones->Fb = state.dataSimAirServingZones->Ep;
                                        state.dataSimAirServingZones->Fc = 1.0 - (1.0 - state.dataSimAirServingZones->ZoneEz) *
                                                                                     (1.0 - state.dataSimAirServingZones->Er) *
                                                                                     (1.0 - state.dataSimAirServingZones->Ep);
                                        // save Fa Fb and Fc for standard 62.1 report
                                        state.dataSize->FaByZoneHeat(TermUnitSizingIndex) = state.dataSimAirServingZones->Fa;
                                        state.dataSize->FbByZoneHeat(TermUnitSizingIndex) = state.dataSimAirServingZones->Fb;
                                        state.dataSize->FcByZoneHeat(TermUnitSizingIndex) = state.dataSimAirServingZones->Fc;

                                        // Calc zone ventilation efficiency
                                        if (state.dataSimAirServingZones->Fa > 0.0) {
                                            SysHeatingEv = 1.0 +
                                                           state.dataSimAirServingZones->Xs * state.dataSimAirServingZones->Fb /
                                                               state.dataSimAirServingZones->Fa -
                                                           state.dataSimAirServingZones->ZoneOAFrac * state.dataSimAirServingZones->Ep *
                                                               state.dataSimAirServingZones->Fc / state.dataSimAirServingZones->Fa;
                                        } else {
                                            SysHeatingEv = 1.0;
                                        }
                                    } else {
                                        // single-path ventilation system
                                        SysHeatingEv = 1.0 + state.dataSimAirServingZones->Xs - state.dataSimAirServingZones->ZoneOAFrac;
                                    }
                                    if (SysHeatingEv < state.dataSimAirServingZones->MinHeatingEvz) {
                                        state.dataSimAirServingZones->MinHeatingEvz = SysHeatingEv;
                                    }
                                    state.dataSize->EvzByZoneHeatPrev(TermUnitSizingIndex) =
                                        state.dataSize->EvzByZoneHeat(TermUnitSizingIndex); // Save previous EvzByZoneHeat
                                    state.dataSize->EvzByZoneHeat(TermUnitSizingIndex) = SysHeatingEv;
                                    state.dataSize->VozSumHtgBySys(AirLoopNum) +=
                                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).VozHtgByZone;
                                }
                            }
                        }

                        if (state.dataSimAirServingZones->MinHeatingEvz > 0) {
                            // Std 62.1-2010, section 6.2.5.4: Eq. 6.6
                            // (However, I don't think people diversity can be done correctly in E+ Sizing so assuming D=1 in this
                            // equation
                            // Vou = Diversity*(Rp*Pz) + Ra*Az
                            state.dataSimAirServingZones->Vou = finalSysSizing.SysUncOA;
                            state.dataSimAirServingZones->Vot = state.dataSimAirServingZones->Vou / state.dataSimAirServingZones->MinHeatingEvz;
                            if (state.dataSimAirServingZones->Vot > state.dataSize->VotHtgBySys(AirLoopNum)) {
                                // This might be the cooling design day so only update if Vot is larger than the previous
                                state.dataSize->VotHtgBySys(AirLoopNum) = state.dataSimAirServingZones->Vot;
                                state.dataSize->XsBySysHeat(AirLoopNum) = state.dataSimAirServingZones->Xs;
                                state.dataSize->EvzMinBySysHeat(AirLoopNum) = state.dataSimAirServingZones->MinHeatingEvz;
                            } else {
                                // Restore EvzByZoneHeat() since it was reset by the current (but not highest Vot) design day
                                // This kludge is probably because inside EndDay block and code gets called for each design day.
                                if (numZonesHeated > 0) {
                                    for (int ZonesHeatedNum = 1; ZonesHeatedNum <= numZonesHeated; ++ZonesHeatedNum) {
                                        int TermUnitSizingIndex =
                                            state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitHeatSizingIndex(ZonesHeatedNum);
                                        state.dataSize->EvzByZoneHeat(TermUnitSizingIndex) = state.dataSize->EvzByZoneHeatPrev(TermUnitSizingIndex);
                                    }
                                } else {
                                    for (int ZonesHeatedNum = 1; ZonesHeatedNum <= NumZonesCooled; ++ZonesHeatedNum) {
                                        int TermUnitSizingIndex =
                                            state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesHeatedNum);
                                        state.dataSize->EvzByZoneHeat(TermUnitSizingIndex) = state.dataSize->EvzByZoneHeatPrev(TermUnitSizingIndex);
                                    }
                                }
                            }
                        }
                    }
                } else { // error
                }
                sysSizing.DesMainVolFlow = max(sysSizing.DesCoolVolFlow, sysSizing.DesHeatVolFlow);
                // this should also be as least as big as is needed for Vot
            } break;
            case DataSizing::SizingConcurrence::NonCoincident: {
                if (finalSysSizing.SystemOAMethod == SysOAMethod::ZoneSum) {
                    sysSizing.DesCoolVolFlow = sysSizing.NonCoinCoolMassFlow / state.dataEnvrn->StdRhoAir;
                    sysSizing.DesHeatVolFlow = sysSizing.NonCoinHeatMassFlow / state.dataEnvrn->StdRhoAir;
                    state.dataSize->VotClgBySys(AirLoopNum) = finalSysSizing.SysUncOA;
                    state.dataSize->VotHtgBySys(AirLoopNum) = finalSysSizing.SysUncOA;
                    for (int ZonesCooledNum = 1; ZonesCooledNum <= NumZonesCooled; ++ZonesCooledNum) {
                        int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesCooledNum);
                        if (state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffCooling <
                            state.dataSize->EvzMinBySysCool(AirLoopNum)) {
                            state.dataSize->EvzMinBySysCool(AirLoopNum) =
                                state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffCooling;
                        }
                        if (state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffHeating <
                            state.dataSize->EvzMinBySysHeat(AirLoopNum)) {
                            state.dataSize->EvzMinBySysHeat(AirLoopNum) =
                                state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffHeating;
                        }
                    }
                    for (int ZonesHeatedNum = 1; ZonesHeatedNum <= NumZonesHeated; ++ZonesHeatedNum) {
                        int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitHeatSizingIndex(ZonesHeatedNum);
                        if (state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffCooling <
                            state.dataSize->EvzMinBySysCool(AirLoopNum)) {
                            state.dataSize->EvzMinBySysCool(AirLoopNum) =
                                state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffCooling;
                        }
                        if (state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffHeating <
                            state.dataSize->EvzMinBySysHeat(AirLoopNum)) {
                            state.dataSize->EvzMinBySysHeat(AirLoopNum) =
                                state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffHeating;
                        }
                    }
                    if (sysSizing.DesCoolVolFlow > 0) {
                        state.dataSize->XsBySysCool(AirLoopNum) = min(1.0, finalSysSizing.SysUncOA / sysSizing.DesCoolVolFlow);
                    } else {
                        state.dataSize->XsBySysCool(AirLoopNum) = 0.0;
                    }
                    if (sysSizing.DesHeatVolFlow > 0) {
                        state.dataSize->XsBySysHeat(AirLoopNum) = min(1.0, finalSysSizing.SysUncOA / sysSizing.DesHeatVolFlow);
                    } else {
                        state.dataSize->XsBySysHeat(AirLoopNum) = 0.0;
                    }
                } else if (finalSysSizing.SystemOAMethod == SysOAMethod::VRP ||
                           finalSysSizing.SystemOAMethod == SysOAMethod::SP) { // Ventilation Rate and Simplified Procedure
                    // cooling
                    sysSizing.DesCoolVolFlow = sysSizing.NonCoinCoolMassFlow / state.dataEnvrn->StdRhoAir;
                    if (sysSizing.DesCoolVolFlow > 0) {
                        OutAirFrac = sysSizing.DesOutAirVolFlow / sysSizing.DesCoolVolFlow;
                    } else {
                        OutAirFrac = 0.0;
                    }
                    OutAirFrac = min(1.0, max(0.0, OutAirFrac));

                    if (sysSizing.DesCoolVolFlow > 0) {
                        state.dataSimAirServingZones->Xs = min(1.0, finalSysSizing.SysUncOA / sysSizing.DesCoolVolFlow);
                    } else {
                        state.dataSimAirServingZones->Xs = 0.0;
                    }
                    if (finalSysSizing.OAAutoSized && sysSizing.DesCoolVolFlow > 0) {
                        int numZonesCooled = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesCooled;
                        state.dataSimAirServingZones->MinCoolingEvz = 1.0;
                        state.dataSize->VozSumClgBySys(AirLoopNum) = 0.0;
                        for (int ZonesCooledNum = 1; ZonesCooledNum <= numZonesCooled; ++ZonesCooledNum) {
                            int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesCooledNum);

                            // Zone air secondary recirculation fraction
                            state.dataSimAirServingZones->Er =
                                state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneSecondaryRecirculation;
                            state.dataSimAirServingZones->Ep = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZonePrimaryAirFraction;
                            state.dataSimAirServingZones->ZoneOAFrac = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZpzClgByZone;
                            state.dataSimAirServingZones->ZoneEz = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffCooling;
                            VozClg = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).VozClgByZone;
                            if (finalSysSizing.SystemOAMethod == SysOAMethod::SP) { // 62.1 simplified procedure
                                if (state.dataSize->DBySys(AirLoopNum) < 0.60) {
                                    state.dataSize->EvzByZoneCool(TermUnitSizingIndex) = 0.88 * state.dataSize->DBySys(AirLoopNum) + 0.22;
                                } else {
                                    state.dataSize->EvzByZoneCool(TermUnitSizingIndex) = 0.75;
                                }
                                state.dataSimAirServingZones->MinCoolingEvz = state.dataSize->EvzByZoneCool(TermUnitSizingIndex);
                            } else {
                                if (state.dataSimAirServingZones->Er > 0.0) {
                                    // multi-path ventilation system using VRP
                                    state.dataSimAirServingZones->Fa = state.dataSimAirServingZones->Ep +
                                                                       (1.0 - state.dataSimAirServingZones->Ep) * state.dataSimAirServingZones->Er;
                                    state.dataSimAirServingZones->Fb = state.dataSimAirServingZones->Ep;
                                    state.dataSimAirServingZones->Fc = 1.0 - (1.0 - state.dataSimAirServingZones->ZoneEz) *
                                                                                 (1.0 - state.dataSimAirServingZones->Er) *
                                                                                 (1.0 - state.dataSimAirServingZones->Ep);
                                    // save Fa Fb and Fc for standard 62.1 report
                                    state.dataSize->FaByZoneCool(TermUnitSizingIndex) = state.dataSimAirServingZones->Fa;
                                    state.dataSize->FbByZoneCool(TermUnitSizingIndex) = state.dataSimAirServingZones->Fb;
                                    state.dataSize->FcByZoneCool(TermUnitSizingIndex) = state.dataSimAirServingZones->Fc;

                                    // Calc zone ventilation efficiency
                                    if (state.dataSimAirServingZones->Fa > 0.0) {
                                        SysCoolingEv =
                                            1.0 +
                                            state.dataSimAirServingZones->Xs * state.dataSimAirServingZones->Fb / state.dataSimAirServingZones->Fa -
                                            state.dataSimAirServingZones->ZoneOAFrac * state.dataSimAirServingZones->Ep *
                                                state.dataSimAirServingZones->Fc / state.dataSimAirServingZones->Fa;
                                    } else {
                                        SysCoolingEv = 1.0;
                                    }
                                } else {
                                    // single-path ventilation system
                                    SysCoolingEv = 1.0 + state.dataSimAirServingZones->Xs - state.dataSimAirServingZones->ZoneOAFrac;
                                    // Apply ventilation efficiency limit; reset SysCoolingEv if necessary
                                    LimitZoneVentEff(state, state.dataSimAirServingZones->Xs, VozClg, TermUnitSizingIndex, SysCoolingEv);
                                }
                                if (SysCoolingEv < state.dataSimAirServingZones->MinCoolingEvz) {
                                    state.dataSimAirServingZones->MinCoolingEvz = SysCoolingEv;
                                }
                                state.dataSize->EvzByZoneCoolPrev(TermUnitSizingIndex) = state.dataSize->EvzByZoneCool(TermUnitSizingIndex);
                                state.dataSize->EvzByZoneCool(TermUnitSizingIndex) = SysCoolingEv;
                                state.dataSize->VozSumClgBySys(AirLoopNum) +=
                                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).VozClgByZone;
                            }
                            state.dataSize->VozSumClgBySys(AirLoopNum) += state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).VozClgByZone;
                        }

                        if (state.dataSimAirServingZones->MinCoolingEvz > 0) {
                            // Std 62.1-2010, section 6.2.5.4: Eq. 6.6
                            // (However, I don't think people diversity can be done correctly in E+ Sizing so assuming D=1 in this
                            // equation
                            // Vou = Diversity*(Rp*Pz) + Ra*Az
                            state.dataSimAirServingZones->Vou = finalSysSizing.SysUncOA;
                            state.dataSimAirServingZones->Vot = state.dataSimAirServingZones->Vou / state.dataSimAirServingZones->MinCoolingEvz;
                            if (state.dataSimAirServingZones->Vot > state.dataSize->VotClgBySys(AirLoopNum)) {
                                // This might be the cooling design day so only update if Vot is larger than the previous
                                state.dataSize->VotClgBySys(AirLoopNum) = state.dataSimAirServingZones->Vot;
                                state.dataSize->XsBySysCool(AirLoopNum) = state.dataSimAirServingZones->Xs;
                                state.dataSize->EvzMinBySysCool(AirLoopNum) = state.dataSimAirServingZones->MinCoolingEvz;
                            } else {
                                // Restore EvzByZoneCool() since it was reset by the current (but not highest Vot) design day
                                for (int ZonesCooledNum = 1; ZonesCooledNum <= numZonesCooled; ++ZonesCooledNum) {
                                    int TermUnitSizingIndex =
                                        state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesCooledNum);
                                    state.dataSize->EvzByZoneCool(TermUnitSizingIndex) = state.dataSize->EvzByZoneCoolPrev(TermUnitSizingIndex);
                                }
                            }
                        }
                    }

                    // heating
                    sysSizing.DesHeatVolFlow = sysSizing.NonCoinHeatMassFlow / state.dataEnvrn->StdRhoAir;
                    if (sysSizing.DesHeatVolFlow > 0) {
                        OutAirFrac = sysSizing.DesOutAirVolFlow / sysSizing.DesHeatVolFlow;
                    } else {
                        OutAirFrac = 0.0;
                    }
                    OutAirFrac = min(1.0, max(0.0, OutAirFrac));

                    if (sysSizing.DesHeatVolFlow > 0) {
                        state.dataSimAirServingZones->Xs = min(1.0, finalSysSizing.SysUncOA / sysSizing.DesHeatVolFlow);
                    } else {
                        state.dataSimAirServingZones->Xs = 0.0;
                    }
                    if (finalSysSizing.OAAutoSized && sysSizing.DesHeatVolFlow > 0) {
                        int numZonesHeated = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesHeated;
                        state.dataSimAirServingZones->MinHeatingEvz = 1.0;
                        state.dataSize->VozSumHtgBySys(AirLoopNum) = 0.0;
                        if (numZonesHeated > 0) {
                            for (int ZonesHeatedNum = 1; ZonesHeatedNum <= numZonesHeated; ++ZonesHeatedNum) {
                                int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitHeatSizingIndex(ZonesHeatedNum);
                                MatchingCooledZoneNum = FindNumberInList(
                                    TermUnitSizingIndex, state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex, NumZonesCooled);
                                if (MatchingCooledZoneNum == 0) {
                                    // Zone air secondary recirculation fraction
                                    state.dataSimAirServingZones->Er =
                                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneSecondaryRecirculation;
                                    state.dataSimAirServingZones->Ep =
                                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZonePrimaryAirFractionHtg;
                                    state.dataSimAirServingZones->ZoneOAFrac =
                                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZpzHtgByZone;
                                    state.dataSimAirServingZones->ZoneEz =
                                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffHeating;
                                    if (finalSysSizing.SystemOAMethod == SysOAMethod::SP) { // 62.1 simplified procedure
                                        if (state.dataSize->DBySys(AirLoopNum) < 0.60) {
                                            state.dataSize->EvzByZoneHeat(TermUnitSizingIndex) = 0.88 * state.dataSize->DBySys(AirLoopNum) + 0.22;
                                        } else {
                                            state.dataSize->EvzByZoneHeat(TermUnitSizingIndex) = 0.75;
                                        }
                                        state.dataSimAirServingZones->MinHeatingEvz = state.dataSize->EvzByZoneHeat(TermUnitSizingIndex);
                                    } else {
                                        if (state.dataSimAirServingZones->Er > 0.0) {
                                            // multi-path ventilation system using VRP
                                            state.dataSimAirServingZones->Fa =
                                                state.dataSimAirServingZones->Ep +
                                                (1.0 - state.dataSimAirServingZones->Ep) * state.dataSimAirServingZones->Er;
                                            state.dataSimAirServingZones->Fb = state.dataSimAirServingZones->Ep;
                                            state.dataSimAirServingZones->Fc = 1.0 - (1.0 - state.dataSimAirServingZones->ZoneEz) *
                                                                                         (1.0 - state.dataSimAirServingZones->Er) *
                                                                                         (1.0 - state.dataSimAirServingZones->Ep);

                                            // Calc zone ventilation efficiency
                                            if (state.dataSimAirServingZones->Fa > 0.0) {
                                                SysHeatingEv = 1.0 +
                                                               state.dataSimAirServingZones->Xs * state.dataSimAirServingZones->Fb /
                                                                   state.dataSimAirServingZones->Fa -
                                                               state.dataSimAirServingZones->ZoneOAFrac * state.dataSimAirServingZones->Ep *
                                                                   state.dataSimAirServingZones->Fc / state.dataSimAirServingZones->Fa;
                                            } else {
                                                SysHeatingEv = 1.0;
                                            }
                                        } else {
                                            // single-path ventilation system
                                            SysHeatingEv = 1.0 + state.dataSimAirServingZones->Xs - state.dataSimAirServingZones->ZoneOAFrac;
                                        }
                                    }
                                }
                                if (finalSysSizing.SystemOAMethod == SysOAMethod::SP) { // 62.1 simplified procedure
                                    state.dataSize->VozSumHtgBySys(AirLoopNum) +=
                                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).VozHtgByZone;
                                } else {
                                    if (SysHeatingEv < state.dataSimAirServingZones->MinHeatingEvz) {
                                        state.dataSimAirServingZones->MinHeatingEvz = SysHeatingEv;
                                    }
                                    state.dataSize->EvzByZoneHeatPrev(TermUnitSizingIndex) =
                                        state.dataSize->EvzByZoneHeat(TermUnitSizingIndex); // Save previous EvzByZoneHeat
                                    state.dataSize->EvzByZoneHeat(TermUnitSizingIndex) = SysHeatingEv;
                                    state.dataSize->VozSumHtgBySys(AirLoopNum) +=
                                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).VozHtgByZone;
                                }
                            }
                        } else {
                            int numZonesCooled = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesCooled;
                            for (int ZonesHeatedNum = 1; ZonesHeatedNum <= numZonesCooled; ++ZonesHeatedNum) {
                                int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesHeatedNum);
                                // Zone air secondary recirculation fraction
                                state.dataSimAirServingZones->Er =
                                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneSecondaryRecirculation;
                                state.dataSimAirServingZones->Ep =
                                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZonePrimaryAirFractionHtg;
                                state.dataSimAirServingZones->ZoneOAFrac = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZpzHtgByZone;
                                state.dataSimAirServingZones->ZoneEz = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneADEffHeating;
                                if (finalSysSizing.SystemOAMethod == SysOAMethod::SP) { // 62.1 simplified procedure
                                    if (state.dataSize->DBySys(AirLoopNum) < 0.60) {
                                        state.dataSize->EvzByZoneCool(TermUnitSizingIndex) = 0.88 * state.dataSize->DBySys(AirLoopNum) + 0.22;
                                    } else {
                                        state.dataSize->EvzByZoneCool(TermUnitSizingIndex) = 0.75;
                                    }
                                    state.dataSimAirServingZones->MinCoolingEvz = state.dataSize->EvzByZoneCool(TermUnitSizingIndex);
                                } else {
                                    if (state.dataSimAirServingZones->Er > 0.0) {
                                        // multi-path ventilation system using VRP
                                        state.dataSimAirServingZones->Fa =
                                            state.dataSimAirServingZones->Ep +
                                            (1.0 - state.dataSimAirServingZones->Ep) * state.dataSimAirServingZones->Er;
                                        state.dataSimAirServingZones->Fb = state.dataSimAirServingZones->Ep;
                                        state.dataSimAirServingZones->Fc = 1.0 - (1.0 - state.dataSimAirServingZones->ZoneEz) *
                                                                                     (1.0 - state.dataSimAirServingZones->Er) *
                                                                                     (1.0 - state.dataSimAirServingZones->Ep);

                                        // Calc zone ventilation efficiency
                                        if (state.dataSimAirServingZones->Fa > 0.0) {
                                            SysHeatingEv = 1.0 +
                                                           state.dataSimAirServingZones->Xs * state.dataSimAirServingZones->Fb /
                                                               state.dataSimAirServingZones->Fa -
                                                           state.dataSimAirServingZones->ZoneOAFrac * state.dataSimAirServingZones->Ep *
                                                               state.dataSimAirServingZones->Fc / state.dataSimAirServingZones->Fa;
                                        } else {
                                            SysHeatingEv = 1.0;
                                        }
                                    } else {
                                        // single-path ventilation system
                                        SysHeatingEv = 1.0 + state.dataSimAirServingZones->Xs - state.dataSimAirServingZones->ZoneOAFrac;
                                    }
                                    if (SysHeatingEv < state.dataSimAirServingZones->MinHeatingEvz) {
                                        state.dataSimAirServingZones->MinHeatingEvz = SysHeatingEv;
                                    }
                                    state.dataSize->EvzByZoneHeatPrev(TermUnitSizingIndex) =
                                        state.dataSize->EvzByZoneHeat(TermUnitSizingIndex); // Save previous EvzByZoneHeat
                                    state.dataSize->EvzByZoneHeat(TermUnitSizingIndex) = SysHeatingEv;
                                }
                                state.dataSize->VozSumHtgBySys(AirLoopNum) +=
                                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).VozHtgByZone;
                            }
                        }

                        if (state.dataSimAirServingZones->MinHeatingEvz > 0) {
                            // Std 62.1-2010, section 6.2.5.4: Eq. 6.6
                            // (However, I don't think people diversity can be done correctly in E+ Sizing so assuming D=1 in this
                            // equation
                            // Vou = Diversity*(Rp*Pz) + Ra*Az
                            state.dataSimAirServingZones->Vou = finalSysSizing.SysUncOA;
                            state.dataSimAirServingZones->Vot = state.dataSimAirServingZones->Vou / state.dataSimAirServingZones->MinHeatingEvz;
                            if (state.dataSimAirServingZones->Vot > state.dataSize->VotHtgBySys(AirLoopNum)) {
                                // This might be the cooling design day so only update if Vot is larger than the previous
                                state.dataSize->VotHtgBySys(AirLoopNum) = state.dataSimAirServingZones->Vot;
                                state.dataSize->XsBySysHeat(AirLoopNum) = state.dataSimAirServingZones->Xs;
                                state.dataSize->EvzMinBySysHeat(AirLoopNum) = state.dataSimAirServingZones->MinHeatingEvz;
                            } else {
                                // Restore EvzByZoneHeat() since it was just reset by the current (but not highest Vot) design day
                                // This kludge is probably because inside EndDay block and code gets called for each design day.
                                if (numZonesHeated > 0) {
                                    for (int ZonesHeatedNum = 1; ZonesHeatedNum <= numZonesHeated; ++ZonesHeatedNum) {
                                        int TermUnitSizingIndex =
                                            state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitHeatSizingIndex(ZonesHeatedNum);
                                        state.dataSize->EvzByZoneHeat(TermUnitSizingIndex) = state.dataSize->EvzByZoneHeatPrev(TermUnitSizingIndex);
                                    }
                                } else {
                                    for (int ZonesHeatedNum = 1; ZonesHeatedNum <= NumZonesCooled; ++ZonesHeatedNum) {
                                        int TermUnitSizingIndex =
                                            state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesHeatedNum);
                                        state.dataSize->EvzByZoneHeat(TermUnitSizingIndex) = state.dataSize->EvzByZoneHeatPrev(TermUnitSizingIndex);
                                    }
                                }
                            }
                        }
                    }
                } else { // error
                }

                sysSizing.DesMainVolFlow = max(sysSizing.DesCoolVolFlow, sysSizing.DesHeatVolFlow);
                // this should also be as least as big as is needed for Vot
            } break;
            default:
                break;
            }

            // If the ventilation was autosized using the ASHRAE VRP method, then the design zone and system ventilation values
            // must be based on the larger of the cooling or heating OA
            if (finalSysSizing.OAAutoSized &&
                (finalSysSizing.SystemOAMethod == SysOAMethod::VRP || finalSysSizing.SystemOAMethod == SysOAMethod::SP)) {
                Real64 VotMax = max(state.dataSize->VotClgBySys(AirLoopNum), state.dataSize->VotHtgBySys(AirLoopNum));

                // Reset the system level ventilation to the larger of the system-level cooling or heating Vot
                finalSysSizing.DesOutAirVolFlow = VotMax;
                state.dataSize->CalcSysSizing(AirLoopNum).DesOutAirVolFlow = VotMax;

                // Reset the zone level ventilation to the larger of the zone-level cooling or heating Voz
                // Loop through cooled zones and heated zones - ok if there's overlap
                for (int zoneNum = 1; zoneNum <= NumZonesCooled; ++zoneNum) {
                    int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(zoneNum);
                    Real64 VozMax = max(state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).VozClgByZone,
                                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).VozHtgByZone);
                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).MinOA = VozMax;
                }
                for (int zoneNum = 1; zoneNum <= NumZonesHeated; ++zoneNum) {
                    int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitHeatSizingIndex(zoneNum);
                    Real64 VozMax = max(state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).VozClgByZone,
                                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).VozHtgByZone);
                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).MinOA = VozMax;
                }
            }
        }
    } break;
    case Constant::CallIndicator::EndSysSizingCalc: {
        // Correct the zone return temperature in FinalZoneSizing for the case of induction units. The calc in
        // ZoneEquipmentManager assumes all the air entering the zone goes into the return node.
        for (int CtrlZoneNum = 1; CtrlZoneNum <= state.dataGlobal->NumOfZones; ++CtrlZoneNum) {
            if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).IsControlled) {
                continue;
            }
            // Use first non-zero airdistunit for now, if there is one
            termunitsizingtempfrac = 1.0;
            int TermUnitSizingIndex = 0;
            for (int InletNode = 1; InletNode <= state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).NumInletNodes; ++InletNode) {
                TermUnitSizingIndex = state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(InletNode).TermUnitSizingIndex;
                if (TermUnitSizingIndex == 0) {
                    continue;
                }
                termunitsizingtemp = (1.0 + state.dataSize->TermUnitSizing(TermUnitSizingIndex).InducRat);
                termunitsizingtempfrac = (1.0 / termunitsizingtemp);
                if (TermUnitSizingIndex > 0) {
                    break;
                }
            }
            if (TermUnitSizingIndex == 0) {
                continue; // Skip this if there are no terminal units
            }
            RetTempRise = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneRetTempAtCoolPeak -
                          state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneTempAtCoolPeak;
            if (RetTempRise > 0.01) {
                // avoid possible compiler bug
                //          FinalZoneSizing(CtrlZoneNum)%ZoneRetTempAtCoolPeak = &
                //            FinalZoneSizing(CtrlZoneNum)%ZoneTempAtCoolPeak + RetTempRise * &
                //           (1.0d0/(1.0d0+TermUnitSizing(CtrlZoneNum)%InducRat))
                state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneRetTempAtCoolPeak =
                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneTempAtCoolPeak + RetTempRise * termunitsizingtempfrac;
            }
            RetTempRise = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneRetTempAtHeatPeak -
                          state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneTempAtHeatPeak;
            if (RetTempRise > 0.01) {
                // avoid possible compiler bug
                //          FinalZoneSizing(CtrlZoneNum)%ZoneRetTempAtHeatPeak = &
                //            FinalZoneSizing(CtrlZoneNum)%ZoneTempAtHeatPeak + RetTempRise * &
                //            (1.0d0/(1.0d0+TermUnitSizing(CtrlZoneNum)%InducRat))
                state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneRetTempAtHeatPeak =
                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneTempAtHeatPeak + RetTempRise * termunitsizingtempfrac;
            }
            for (TimeStepIndex = 1; TimeStepIndex <= numOfTimeStepInDay; ++TimeStepIndex) {
                RetTempRise = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).CoolZoneRetTempSeq(TimeStepIndex) -
                              state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).CoolZoneTempSeq(TimeStepIndex);
                if (RetTempRise > 0.01) {
                    // avoid possible compiler bug
                    //            FinalZoneSizing(CtrlZoneNum)%CoolZoneRetTempSeq(TimeStepIndex) = &
                    //              FinalZoneSizing(CtrlZoneNum)%CoolZoneTempSeq(TimeStepIndex) + RetTempRise * &
                    //             (1.0d0/(1.0d0+TermUnitSizing(CtrlZoneNum)%InducRat))
                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).CoolZoneRetTempSeq(TimeStepIndex) =
                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).CoolZoneTempSeq(TimeStepIndex) +
                        RetTempRise * termunitsizingtempfrac;
                }
                RetTempRise = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).HeatZoneRetTempSeq(TimeStepIndex) -
                              state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).HeatZoneTempSeq(TimeStepIndex);
                if (RetTempRise > 0.01) {
                    // avoid possible compiler bug
                    //            FinalZoneSizing(CtrlZoneNum)%HeatZoneRetTempSeq(TimeStepIndex) = &
                    //              FinalZoneSizing(CtrlZoneNum)%HeatZoneTempSeq(TimeStepIndex) + RetTempRise * &
                    //             (1.0d0/(1.0d0+TermUnitSizing(CtrlZoneNum)%InducRat))
                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).HeatZoneRetTempSeq(TimeStepIndex) =
                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).HeatZoneTempSeq(TimeStepIndex) +
                        RetTempRise * termunitsizingtempfrac;
                }
            }
        }

        // Get final design flows
        for (AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) {
            state.dataSize->SensCoolCapTemp(AirLoopNum) = 0.0;
            state.dataSize->TotCoolCapTemp(AirLoopNum) = 0.0;

            // For coincident sizing, loop over design days and pick out the largest central heating amd
            // cooling flow rates and associated data

            for (DDNum = 1; DDNum <= state.dataEnvrn->TotDesDays + state.dataEnvrn->TotRunDesPersDays; ++DDNum) {
                auto const &sysSizing = state.dataSize->SysSizing(DDNum, AirLoopNum);
                if (sysSizing.SensCoolCap > state.dataSize->SensCoolCapTemp(AirLoopNum)) {
                    state.dataSize->SysSizPeakDDNum(AirLoopNum).SensCoolPeakDD = DDNum;
                    state.dataSize->SysSizPeakDDNum(AirLoopNum).cSensCoolPeakDDDate = state.dataSize->DesDayWeath(DDNum).DateString;
                    state.dataSize->SensCoolCapTemp(AirLoopNum) = sysSizing.SensCoolCap;
                    if (sysSizing.coolingPeakLoad == DataSizing::PeakLoad::SensibleCooling) {
                        state.dataSize->CalcSysSizing(AirLoopNum).DesCoolVolFlow = sysSizing.DesCoolVolFlow;
                        state.dataSize->CalcSysSizing(AirLoopNum).CoolDesDay = sysSizing.CoolDesDay;
                        // state.dataSize->CalcSysSizing( AirLoopNum ).CoinCoolMassFlow = SysSizing( DDNum, AirLoopNum ).CoinCoolMassFlow;
                        state.dataSize->CalcSysSizing(AirLoopNum).MassFlowAtCoolPeak = sysSizing.MassFlowAtCoolPeak;
                        state.dataSize->CalcSysSizing(AirLoopNum).SensCoolCap = sysSizing.SensCoolCap;
                        state.dataSize->CalcSysSizing(AirLoopNum).TotCoolCap = sysSizing.TotCoolCap;
                        state.dataSize->CalcSysSizing(AirLoopNum).CoolFlowSeq = sysSizing.CoolFlowSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).SumZoneCoolLoadSeq = sysSizing.SumZoneCoolLoadSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).CoolZoneAvgTempSeq = sysSizing.CoolZoneAvgTempSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).SensCoolCapSeq = sysSizing.SensCoolCapSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).TotCoolCapSeq = sysSizing.TotCoolCapSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).MixTempAtCoolPeak = sysSizing.MixTempAtCoolPeak;
                        state.dataSize->CalcSysSizing(AirLoopNum).RetTempAtCoolPeak = sysSizing.RetTempAtCoolPeak;
                        state.dataSize->CalcSysSizing(AirLoopNum).MixHumRatAtCoolPeak = sysSizing.MixHumRatAtCoolPeak;
                        state.dataSize->CalcSysSizing(AirLoopNum).RetHumRatAtCoolPeak = sysSizing.RetHumRatAtCoolPeak;
                        state.dataSize->CalcSysSizing(AirLoopNum).OutTempAtCoolPeak = sysSizing.OutTempAtCoolPeak;
                        state.dataSize->CalcSysSizing(AirLoopNum).OutHumRatAtCoolPeak = sysSizing.OutHumRatAtCoolPeak;
                        state.dataSize->CalcSysSizing(AirLoopNum).SysCoolRetTempSeq = sysSizing.SysCoolRetTempSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).SysCoolRetHumRatSeq = sysSizing.SysCoolRetHumRatSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).SysCoolOutTempSeq = sysSizing.SysCoolOutTempSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).SysCoolOutHumRatSeq = sysSizing.SysCoolOutHumRatSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).SysDOASHeatAddSeq = sysSizing.SysDOASHeatAddSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).SysDOASLatAddSeq = sysSizing.SysDOASLatAddSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).SysCoolCoinSpaceSens = sysSizing.SysCoolCoinSpaceSens;
                        state.dataSize->CalcSysSizing(AirLoopNum).SysDesCoolLoad = sysSizing.SysDesCoolLoad;
                        state.dataSize->CalcSysSizing(AirLoopNum).SysCoolLoadTimeStepPk = sysSizing.SysCoolLoadTimeStepPk;
                    }
                }

                if (sysSizing.TotCoolCap > state.dataSize->TotCoolCapTemp(AirLoopNum)) {
                    state.dataSize->SysSizPeakDDNum(AirLoopNum).TotCoolPeakDD = DDNum;
                    state.dataSize->SysSizPeakDDNum(AirLoopNum).cTotCoolPeakDDDate = state.dataSize->DesDayWeath(DDNum).DateString;
                    state.dataSize->TotCoolCapTemp(AirLoopNum) = sysSizing.TotCoolCap;
                    if (sysSizing.coolingPeakLoad == DataSizing::PeakLoad::TotalCooling) {
                        state.dataSize->CalcSysSizing(AirLoopNum).DesCoolVolFlow = sysSizing.DesCoolVolFlow;
                        state.dataSize->CalcSysSizing(AirLoopNum).CoolDesDay = sysSizing.CoolDesDay;
                        // state.dataSize->CalcSysSizing( AirLoopNum ).CoinCoolMassFlow = SysSizing( DDNum, AirLoopNum ).CoinCoolMassFlow;
                        state.dataSize->CalcSysSizing(AirLoopNum).MassFlowAtCoolPeak = sysSizing.MassFlowAtCoolPeak;
                        state.dataSize->CalcSysSizing(AirLoopNum).SensCoolCap = sysSizing.SensCoolCap;
                        state.dataSize->CalcSysSizing(AirLoopNum).TotCoolCap = sysSizing.TotCoolCap;
                        state.dataSize->CalcSysSizing(AirLoopNum).CoolFlowSeq = sysSizing.CoolFlowSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).SumZoneCoolLoadSeq = sysSizing.SumZoneCoolLoadSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).CoolZoneAvgTempSeq = sysSizing.CoolZoneAvgTempSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).SensCoolCapSeq = sysSizing.SensCoolCapSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).TotCoolCapSeq = sysSizing.TotCoolCapSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).MixTempAtCoolPeak = sysSizing.MixTempAtCoolPeak;
                        state.dataSize->CalcSysSizing(AirLoopNum).RetTempAtCoolPeak = sysSizing.RetTempAtCoolPeak;
                        state.dataSize->CalcSysSizing(AirLoopNum).MixHumRatAtCoolPeak = sysSizing.MixHumRatAtCoolPeak;
                        state.dataSize->CalcSysSizing(AirLoopNum).RetHumRatAtCoolPeak = sysSizing.RetHumRatAtCoolPeak;
                        state.dataSize->CalcSysSizing(AirLoopNum).OutTempAtCoolPeak = sysSizing.OutTempAtCoolPeak;
                        state.dataSize->CalcSysSizing(AirLoopNum).OutHumRatAtCoolPeak = sysSizing.OutHumRatAtCoolPeak;
                        state.dataSize->CalcSysSizing(AirLoopNum).SysCoolRetTempSeq = sysSizing.SysCoolRetTempSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).SysCoolRetHumRatSeq = sysSizing.SysCoolRetHumRatSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).SysCoolOutTempSeq = sysSizing.SysCoolOutTempSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).SysCoolOutHumRatSeq = sysSizing.SysCoolOutHumRatSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).SysDOASHeatAddSeq = sysSizing.SysDOASHeatAddSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).SysDOASLatAddSeq = sysSizing.SysDOASLatAddSeq;
                        state.dataSize->CalcSysSizing(AirLoopNum).SysDesCoolLoad = sysSizing.SysDesCoolLoad;
                        state.dataSize->CalcSysSizing(AirLoopNum).SysCoolLoadTimeStepPk = sysSizing.SysCoolLoadTimeStepPk;
                    }
                    state.dataSize->CalcSysSizing(AirLoopNum).SysCoolCoinSpaceSens = sysSizing.SysCoolCoinSpaceSens;
                }

                if (sysSizing.CoinCoolMassFlow > state.dataSize->CalcSysSizing(AirLoopNum).CoinCoolMassFlow) {
                    state.dataSize->CalcSysSizing(AirLoopNum).CoinCoolMassFlow = sysSizing.CoinCoolMassFlow;
                    state.dataSize->SysSizPeakDDNum(AirLoopNum).CoolFlowPeakDD = DDNum;
                    state.dataSize->SysSizPeakDDNum(AirLoopNum).cCoolFlowPeakDDDate = state.dataSize->DesDayWeath(DDNum).DateString;
                }

                if (sysSizing.HeatCap > state.dataSize->CalcSysSizing(AirLoopNum).HeatCap) {
                    state.dataSize->SysSizPeakDDNum(AirLoopNum).HeatPeakDD = DDNum;
                    state.dataSize->SysSizPeakDDNum(AirLoopNum).cHeatPeakDDDate = state.dataSize->DesDayWeath(DDNum).DateString;
                    state.dataSize->CalcSysSizing(AirLoopNum).DesHeatVolFlow = sysSizing.DesHeatVolFlow;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatDesDay = sysSizing.HeatDesDay;
                    state.dataSize->CalcSysSizing(AirLoopNum).CoinHeatMassFlow = sysSizing.CoinHeatMassFlow;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatCap = sysSizing.HeatCap;
                    state.dataSize->CalcSysSizing(AirLoopNum).PreheatCap = sysSizing.PreheatCap;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatFlowSeq = sysSizing.HeatFlowSeq;
                    state.dataSize->CalcSysSizing(AirLoopNum).SumZoneHeatLoadSeq = sysSizing.SumZoneHeatLoadSeq;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatCapSeq = sysSizing.HeatCapSeq;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatZoneAvgTempSeq = sysSizing.HeatZoneAvgTempSeq;
                    state.dataSize->CalcSysSizing(AirLoopNum).PreheatCapSeq = sysSizing.PreheatCapSeq;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatMixTemp = sysSizing.HeatMixTemp;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatRetTemp = sysSizing.HeatRetTemp;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatMixHumRat = sysSizing.HeatMixHumRat;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatRetHumRat = sysSizing.HeatRetHumRat;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatOutTemp = sysSizing.HeatOutTemp;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatOutHumRat = sysSizing.HeatOutHumRat;
                    state.dataSize->CalcSysSizing(AirLoopNum).SysHeatRetTempSeq = sysSizing.SysHeatRetTempSeq;
                    state.dataSize->CalcSysSizing(AirLoopNum).SysHeatRetHumRatSeq = sysSizing.SysHeatRetHumRatSeq;
                    state.dataSize->CalcSysSizing(AirLoopNum).SysHeatOutTempSeq = sysSizing.SysHeatOutTempSeq;
                    state.dataSize->CalcSysSizing(AirLoopNum).SysHeatOutHumRatSeq = sysSizing.SysHeatOutHumRatSeq;

                    state.dataSize->CalcSysSizing(AirLoopNum).SysHeatCoilTimeStepPk = sysSizing.SysHeatCoilTimeStepPk;

                    state.dataSize->CalcSysSizing(AirLoopNum).SysHeatAirTimeStepPk = sysSizing.SysHeatAirTimeStepPk;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatDDNum = DDNum;
                    state.dataSize->CalcSysSizing(AirLoopNum).SysHeatCoinSpaceSens = sysSizing.SysHeatCoinSpaceSens;
                    state.dataSize->CalcSysSizing(AirLoopNum).SysDesHeatLoad = sysSizing.SysDesHeatLoad;
                    state.dataSize->CalcSysSizing(AirLoopNum).SysHeatLoadTimeStepPk = sysSizing.SysHeatLoadTimeStepPk;
                }
            }

            state.dataSize->CalcSysSizing(AirLoopNum).DesMainVolFlow =
                max(state.dataSize->CalcSysSizing(AirLoopNum).DesCoolVolFlow, state.dataSize->CalcSysSizing(AirLoopNum).DesHeatVolFlow);

            // For noncoincident sizing, find the max heat and cool mass flow for each zone over all the
            // design days. Then calculate the associated heating and cooling capacities.

            int NumZonesCooled = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesCooled;
            int NumZonesHeated = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesHeated;
            SysCoolRetTemp = 0.0;
            OutAirFrac = 0.0;
            SysCoolMixTemp = 0.0;
            SysSensCoolCap = 0.0;
            SysTotCoolCap = 0.0;
            CoolTimeStepNum = 0;
            CoolDDNum = 0;
            OutAirTemp = 0.0;
            OutAirHumRat = 0.0;
            SysCoolMixHumRat = 0.0;
            SysCoolRetHumRat = 0.0;
            SysCoolOutTemp = 0.0;
            SysCoolOutHumRat = 0.0;

            for (int ZonesCooledNum = 1; ZonesCooledNum <= NumZonesCooled; ++ZonesCooledNum) { // loop over cooled zones
                int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesCooledNum);
                auto const &termUnitSizing = state.dataSize->TermUnitSizing(TermUnitSizingIndex);
                // save the system cooling supply air temp
                state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).DesCoolCoilInTempTU =
                    state.dataSize->CalcSysSizing(AirLoopNum).CoolSupTemp;
                // save the system cooling supply air hum rat
                state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).DesCoolCoilInHumRatTU =
                    state.dataSize->CalcSysSizing(AirLoopNum).CoolSupHumRat;
                if (state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).DesCoolMassFlow <= 0.0) {
                    continue;
                }
                Real64 coolMassFlow = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex)
                                          .DesCoolMassFlow; // already scaled for term unit sizing in Updatestate.dataSize->TermUnitFinalZoneSizing
                state.dataSize->CalcSysSizing(AirLoopNum).NonCoinCoolMassFlow += coolMassFlow / (1.0 + termUnitSizing.InducRat);
                SysCoolRetTemp += state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneRetTempAtCoolPeak * coolMassFlow /
                                  (1.0 + termUnitSizing.InducRat);
                SysCoolRetHumRat += state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneHumRatAtCoolPeak * coolMassFlow /
                                    (1.0 + termUnitSizing.InducRat);
                CoolDDNum = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).CoolDDNum;
                CoolTimeStepNum = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).TimeStepNumAtCoolMax;
                if (CoolDDNum == 0) {
                    auto const &zoneCFS = state.dataSize->CalcFinalZoneSizing(state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneNum);
                    OutAirTemp += zoneCFS.CoolOutTemp * coolMassFlow / (1.0 + termUnitSizing.InducRat);
                    OutAirHumRat += zoneCFS.CoolOutHumRat * coolMassFlow / (1.0 + termUnitSizing.InducRat);
                } else {
                    OutAirTemp += state.dataSize->DesDayWeath(CoolDDNum).Temp(CoolTimeStepNum) * coolMassFlow / (1.0 + termUnitSizing.InducRat);
                    OutAirHumRat += state.dataSize->DesDayWeath(CoolDDNum).HumRat(CoolTimeStepNum) * coolMassFlow / (1.0 + termUnitSizing.InducRat);
                }
            }
            if (state.dataSize->CalcSysSizing(AirLoopNum).NonCoinCoolMassFlow > 0.0) {
                SysCoolRetTemp /= state.dataSize->CalcSysSizing(AirLoopNum).NonCoinCoolMassFlow;
                SysCoolRetHumRat /= state.dataSize->CalcSysSizing(AirLoopNum).NonCoinCoolMassFlow;
                OutAirTemp /= state.dataSize->CalcSysSizing(AirLoopNum).NonCoinCoolMassFlow;
                OutAirHumRat /= state.dataSize->CalcSysSizing(AirLoopNum).NonCoinCoolMassFlow;
                SysCoolOutTemp = OutAirTemp;
                SysCoolOutHumRat = OutAirHumRat;
                RhoAir = state.dataEnvrn->StdRhoAir;
                if (state.dataSize->CalcSysSizing(AirLoopNum).CoolOAOption == OAControl::MinOA) {
                    OutAirFrac = RhoAir * state.dataSize->CalcSysSizing(AirLoopNum).DesOutAirVolFlow /
                                 state.dataSize->CalcSysSizing(AirLoopNum).NonCoinCoolMassFlow;
                    OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                } else {
                    OutAirFrac = 1.0;
                }
                SysCoolMixTemp = OutAirTemp * OutAirFrac + SysCoolRetTemp * (1.0 - OutAirFrac);
                SysCoolMixHumRat = OutAirHumRat * OutAirFrac + SysCoolRetHumRat * (1.0 - OutAirFrac);
                SysSensCoolCap = PsyCpAirFnW(DataPrecisionGlobals::constant_zero) * state.dataSize->CalcSysSizing(AirLoopNum).NonCoinCoolMassFlow *
                                 (SysCoolMixTemp - state.dataSize->CalcSysSizing(AirLoopNum).CoolSupTemp);
                SysSensCoolCap = max(0.0, SysSensCoolCap);
                SysTotCoolCap = state.dataSize->CalcSysSizing(AirLoopNum).NonCoinCoolMassFlow *
                                (PsyHFnTdbW(SysCoolMixTemp, SysCoolMixHumRat) - PsyHFnTdbW(state.dataSize->CalcSysSizing(AirLoopNum).CoolSupTemp,
                                                                                           state.dataSize->CalcSysSizing(AirLoopNum).CoolSupHumRat));
                SysTotCoolCap = max(0.0, SysTotCoolCap);
            }

            SysHeatRetTemp = 0.0;
            OutAirFrac = 0.0;
            SysHeatMixTemp = 0.0;
            SysHeatCap = 0.0;
            HeatTimeStepNum = 0;
            HeatDDNum = 0;
            OutAirTemp = 0.0;
            OutAirHumRat = 0.0;
            SysHeatMixHumRat = 0.0;
            SysHeatRetHumRat = 0.0;
            SysHeatOutTemp = 0.0;
            SysHeatOutHumRat = 0.0;

            if (NumZonesHeated > 0) { // IF there are centrally heated zones

                for (int ZonesHeatedNum = 1; ZonesHeatedNum <= NumZonesHeated; ++ZonesHeatedNum) { // loop over the heated zones
                    int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitHeatSizingIndex(ZonesHeatedNum);
                    auto const &termUnitSizing = state.dataSize->TermUnitSizing(TermUnitSizingIndex);
                    // save the system heating supply air temp
                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).DesHeatCoilInTempTU =
                        state.dataSize->CalcSysSizing(AirLoopNum).HeatSupTemp;
                    // save the system heating supply air hum rat
                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).DesHeatCoilInHumRatTU =
                        state.dataSize->CalcSysSizing(AirLoopNum).HeatSupHumRat;
                    if (state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).DesHeatMassFlow <= 0.0) {
                        continue;
                    }
                    Real64 heatMassFlow =
                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex)
                            .DesHeatMassFlow; // already scaled for term unit sizing in Updatestate.dataSize->TermUnitFinalZoneSizing
                    state.dataSize->CalcSysSizing(AirLoopNum).NonCoinHeatMassFlow += heatMassFlow / (1.0 + termUnitSizing.InducRat);
                    SysHeatRetTemp += state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneRetTempAtHeatPeak * heatMassFlow /
                                      (1.0 + termUnitSizing.InducRat);
                    SysHeatRetHumRat += state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneHumRatAtHeatPeak * heatMassFlow /
                                        (1.0 + termUnitSizing.InducRat);
                    HeatDDNum = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).HeatDDNum;
                    HeatTimeStepNum = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).TimeStepNumAtHeatMax;
                    if (HeatDDNum == 0) {
                        auto const &zoneCFS =
                            state.dataSize->CalcFinalZoneSizing(state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneNum);
                        OutAirTemp += zoneCFS.HeatOutTemp * heatMassFlow / (1.0 + termUnitSizing.InducRat);
                        OutAirHumRat += zoneCFS.HeatOutHumRat * heatMassFlow / (1.0 + termUnitSizing.InducRat);
                    } else {
                        OutAirTemp += state.dataSize->DesDayWeath(HeatDDNum).Temp(HeatTimeStepNum) * heatMassFlow / (1.0 + termUnitSizing.InducRat);
                        OutAirHumRat +=
                            state.dataSize->DesDayWeath(HeatDDNum).HumRat(HeatTimeStepNum) * heatMassFlow / (1.0 + termUnitSizing.InducRat);
                    }
                }
                if (state.dataSize->CalcSysSizing(AirLoopNum).NonCoinHeatMassFlow > 0.0) {
                    SysHeatRetTemp /= state.dataSize->CalcSysSizing(AirLoopNum).NonCoinHeatMassFlow;
                    SysHeatRetHumRat /= state.dataSize->CalcSysSizing(AirLoopNum).NonCoinHeatMassFlow;
                    OutAirTemp /= state.dataSize->CalcSysSizing(AirLoopNum).NonCoinHeatMassFlow;
                    OutAirHumRat /= state.dataSize->CalcSysSizing(AirLoopNum).NonCoinHeatMassFlow;
                    SysHeatOutTemp = OutAirTemp;
                    SysHeatOutHumRat = OutAirHumRat;
                    RhoAir = state.dataEnvrn->StdRhoAir;
                    if (state.dataSize->CalcSysSizing(AirLoopNum).HeatOAOption == DataSizing::OAControl::MinOA) {
                        OutAirFrac = RhoAir * state.dataSize->CalcSysSizing(AirLoopNum).DesOutAirVolFlow /
                                     state.dataSize->CalcSysSizing(AirLoopNum).NonCoinHeatMassFlow;
                        OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                    } else {
                        OutAirFrac = 1.0;
                    }
                    SysHeatMixTemp = OutAirTemp * OutAirFrac + SysHeatRetTemp * (1.0 - OutAirFrac);
                    SysHeatMixHumRat = OutAirHumRat * OutAirFrac + SysHeatRetHumRat * (1.0 - OutAirFrac);
                    SysHeatCap = PsyCpAirFnW(DataPrecisionGlobals::constant_zero) * state.dataSize->CalcSysSizing(AirLoopNum).NonCoinHeatMassFlow *
                                 (state.dataSize->CalcSysSizing(AirLoopNum).HeatSupTemp - SysHeatMixTemp);
                    SysHeatCap = max(0.0, SysHeatCap);
                }

            } else { // No centrally heated zones: use cooled zones

                for (int ZonesCooledNum = 1; ZonesCooledNum <= NumZonesCooled; ++ZonesCooledNum) { // loop over the cooled zones
                    int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesCooledNum);
                    auto const &termUnitSizing = state.dataSize->TermUnitSizing(TermUnitSizingIndex);
                    // save the system heating supply air temp
                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).DesHeatCoilInTempTU =
                        state.dataSize->CalcSysSizing(AirLoopNum).HeatSupTemp;
                    // save the system heating supply air hum rat
                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).DesHeatCoilInHumRatTU =
                        state.dataSize->CalcSysSizing(AirLoopNum).HeatSupHumRat;
                    if (state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).DesHeatMassFlow <= 0.0) {
                        continue;
                    }
                    Real64 heatMassFlow =
                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex)
                            .DesHeatMassFlow; // already scaled for term unit sizing in Updatestate.dataSize->TermUnitFinalZoneSizing
                    state.dataSize->CalcSysSizing(AirLoopNum).NonCoinHeatMassFlow += heatMassFlow / (1.0 + termUnitSizing.InducRat);
                    SysHeatRetTemp += state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneRetTempAtHeatPeak * heatMassFlow /
                                      (1.0 + termUnitSizing.InducRat);
                    SysHeatRetHumRat += state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneHumRatAtHeatPeak * heatMassFlow /
                                        (1.0 + termUnitSizing.InducRat);
                    HeatDDNum = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).HeatDDNum;
                    HeatTimeStepNum = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).TimeStepNumAtHeatMax;
                    if (HeatDDNum == 0) {
                        auto const &zoneCFS =
                            state.dataSize->CalcFinalZoneSizing(state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).ZoneNum);
                        OutAirTemp += zoneCFS.HeatOutTemp * heatMassFlow / (1.0 + termUnitSizing.InducRat);
                        OutAirHumRat += zoneCFS.HeatOutHumRat * heatMassFlow / (1.0 + termUnitSizing.InducRat);
                    } else {
                        OutAirTemp += state.dataSize->DesDayWeath(HeatDDNum).Temp(HeatTimeStepNum) * heatMassFlow / (1.0 + termUnitSizing.InducRat);
                        OutAirHumRat +=
                            state.dataSize->DesDayWeath(HeatDDNum).HumRat(HeatTimeStepNum) * heatMassFlow / (1.0 + termUnitSizing.InducRat);
                    }
                }
                if (state.dataSize->CalcSysSizing(AirLoopNum).NonCoinHeatMassFlow > 0.0) {
                    SysHeatRetTemp /= state.dataSize->CalcSysSizing(AirLoopNum).NonCoinHeatMassFlow;
                    SysHeatRetHumRat /= state.dataSize->CalcSysSizing(AirLoopNum).NonCoinHeatMassFlow;
                    OutAirTemp /= state.dataSize->CalcSysSizing(AirLoopNum).NonCoinHeatMassFlow;
                    OutAirHumRat /= state.dataSize->CalcSysSizing(AirLoopNum).NonCoinHeatMassFlow;
                    SysHeatOutTemp = OutAirTemp;
                    SysHeatOutHumRat = OutAirHumRat;
                    RhoAir = state.dataEnvrn->StdRhoAir;
                    if (state.dataSize->CalcSysSizing(AirLoopNum).HeatOAOption == DataSizing::OAControl::MinOA) {
                        OutAirFrac = RhoAir * state.dataSize->CalcSysSizing(AirLoopNum).DesOutAirVolFlow /
                                     state.dataSize->CalcSysSizing(AirLoopNum).NonCoinHeatMassFlow;
                        OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                    } else {
                        OutAirFrac = 1.0;
                    }
                    SysHeatMixTemp = OutAirTemp * OutAirFrac + SysHeatRetTemp * (1.0 - OutAirFrac);
                    SysHeatMixHumRat = OutAirHumRat * OutAirFrac + SysHeatRetHumRat * (1.0 - OutAirFrac);
                    SysHeatCap = PsyCpAirFnW(DataPrecisionGlobals::constant_zero) * state.dataSize->CalcSysSizing(AirLoopNum).NonCoinHeatMassFlow *
                                 (state.dataSize->CalcSysSizing(AirLoopNum).HeatSupTemp - SysHeatMixTemp);
                    SysHeatCap = max(0.0, SysHeatCap);
                }
            }

            // move the noncoincident results into the system sizing array
            if (state.dataSize->CalcSysSizing(AirLoopNum).SizingOption == DataSizing::SizingConcurrence::NonCoincident) {
                // But first check to see if the noncoincident result is actually bigger than the coincident (for 100% outside air)
                if (!(state.dataSize->FinalSysSizing(AirLoopNum).CoolOAOption == OAControl::AllOA &&
                      SysSensCoolCap <= 0.0)) { // CoolOAOption = Yes 100% OA
                    state.dataSize->CalcSysSizing(AirLoopNum).SensCoolCap = SysSensCoolCap;
                    state.dataSize->CalcSysSizing(AirLoopNum).TotCoolCap = SysTotCoolCap;
                    state.dataSize->CalcSysSizing(AirLoopNum).MixTempAtCoolPeak = SysCoolMixTemp;
                    state.dataSize->CalcSysSizing(AirLoopNum).RetTempAtCoolPeak = SysCoolRetTemp;
                    state.dataSize->CalcSysSizing(AirLoopNum).MixHumRatAtCoolPeak = SysCoolMixHumRat;
                    state.dataSize->CalcSysSizing(AirLoopNum).RetHumRatAtCoolPeak = SysCoolRetHumRat;
                    state.dataSize->CalcSysSizing(AirLoopNum).OutTempAtCoolPeak = SysCoolOutTemp;
                    state.dataSize->CalcSysSizing(AirLoopNum).OutHumRatAtCoolPeak = SysCoolOutHumRat;
                }
                // check to see if the noncoincident result is actually bigger than the coincident (for 100% outside air)
                // why is this < 0.0 ? SysHeatCap cannot be < 0 ?? this code will always get executed
                if (!(state.dataSize->FinalSysSizing(AirLoopNum).HeatOAOption == OAControl::AllOA &&
                      SysHeatCap < 0.0)) { // HeatOAOption = Yes 100% OA
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatCap = SysHeatCap;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatMixTemp = SysHeatMixTemp;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatRetTemp = SysHeatRetTemp;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatMixHumRat = SysHeatMixHumRat;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatRetHumRat = SysHeatRetHumRat;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatOutTemp = SysHeatOutTemp;
                    state.dataSize->CalcSysSizing(AirLoopNum).HeatOutHumRat = SysHeatOutHumRat;
                }
                state.dataSize->CalcSysSizing(AirLoopNum).DesCoolVolFlow =
                    state.dataSize->CalcSysSizing(AirLoopNum).NonCoinCoolMassFlow / state.dataEnvrn->StdRhoAir;
                state.dataSize->CalcSysSizing(AirLoopNum).DesHeatVolFlow =
                    state.dataSize->CalcSysSizing(AirLoopNum).NonCoinHeatMassFlow / state.dataEnvrn->StdRhoAir;
                state.dataSize->CalcSysSizing(AirLoopNum).DesMainVolFlow =
                    max(state.dataSize->CalcSysSizing(AirLoopNum).DesCoolVolFlow, state.dataSize->CalcSysSizing(AirLoopNum).DesHeatVolFlow);
            }
        }

        // Move final system design data (calculated from zone data) to user design array
        for (std::size_t i = 0; i < state.dataSize->FinalSysSizing.size(); ++i) {
            auto &z = state.dataSize->FinalSysSizing[i];
            auto &c = state.dataSize->CalcSysSizing[i];
            z.CoolDesDay = c.CoolDesDay;
            z.HeatDesDay = c.HeatDesDay;
            z.CoinCoolMassFlow = c.CoinCoolMassFlow;
            z.CoinHeatMassFlow = c.CoinHeatMassFlow;
            z.NonCoinCoolMassFlow = c.NonCoinCoolMassFlow;
            z.NonCoinHeatMassFlow = c.NonCoinHeatMassFlow;
            z.DesMainVolFlow = c.DesMainVolFlow;
            z.DesHeatVolFlow = c.DesHeatVolFlow;
            z.DesCoolVolFlow = c.DesCoolVolFlow;
            z.MassFlowAtCoolPeak = c.MassFlowAtCoolPeak;
            z.SensCoolCap = c.SensCoolCap;
            z.TotCoolCap = c.TotCoolCap;
            z.HeatCap = c.HeatCap;
            z.PreheatCap = c.PreheatCap;
            z.MixTempAtCoolPeak = c.MixTempAtCoolPeak;
            z.MixHumRatAtCoolPeak = c.MixHumRatAtCoolPeak;
            z.RetTempAtCoolPeak = c.RetTempAtCoolPeak;
            z.RetHumRatAtCoolPeak = c.RetHumRatAtCoolPeak;
            z.OutTempAtCoolPeak = c.OutTempAtCoolPeak;
            z.OutHumRatAtCoolPeak = c.OutHumRatAtCoolPeak;
            z.HeatMixTemp = c.HeatMixTemp;
            z.HeatMixHumRat = c.HeatMixHumRat;
            z.HeatRetTemp = c.HeatRetTemp;
            z.HeatRetHumRat = c.HeatRetHumRat;
            z.HeatOutTemp = c.HeatOutTemp;
            z.HeatOutHumRat = c.HeatOutHumRat;
            z.SysHeatCoilTimeStepPk = c.SysHeatCoilTimeStepPk;
            z.SysHeatAirTimeStepPk = c.SysHeatAirTimeStepPk;
            z.HeatDDNum = c.HeatDDNum;
            z.SysCoolCoinSpaceSens = c.SysCoolCoinSpaceSens;
            z.SysHeatCoinSpaceSens = c.SysHeatCoinSpaceSens;
            z.SysDesCoolLoad = c.SysDesCoolLoad;
            z.SysCoolLoadTimeStepPk = c.SysCoolLoadTimeStepPk;
            z.SysDesHeatLoad = c.SysDesHeatLoad;
            z.SysHeatLoadTimeStepPk = c.SysHeatLoadTimeStepPk;
        }

        for (AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) {
            auto &finalSysSizing = state.dataSize->FinalSysSizing(AirLoopNum);
            auto &calcSysSizing = state.dataSize->CalcSysSizing(AirLoopNum);
            for (TimeStepIndex = 1; TimeStepIndex <= numOfTimeStepInDay; ++TimeStepIndex) {
                finalSysSizing.HeatFlowSeq(TimeStepIndex) = calcSysSizing.HeatFlowSeq(TimeStepIndex);
                finalSysSizing.CoolFlowSeq(TimeStepIndex) = calcSysSizing.CoolFlowSeq(TimeStepIndex);
                finalSysSizing.SumZoneCoolLoadSeq(TimeStepIndex) = calcSysSizing.SumZoneCoolLoadSeq(TimeStepIndex);
                finalSysSizing.SumZoneHeatLoadSeq(TimeStepIndex) = calcSysSizing.SumZoneHeatLoadSeq(TimeStepIndex);
                finalSysSizing.CoolZoneAvgTempSeq(TimeStepIndex) = calcSysSizing.CoolZoneAvgTempSeq(TimeStepIndex);
                finalSysSizing.HeatZoneAvgTempSeq(TimeStepIndex) = calcSysSizing.HeatZoneAvgTempSeq(TimeStepIndex);
                finalSysSizing.SensCoolCapSeq(TimeStepIndex) = calcSysSizing.SensCoolCapSeq(TimeStepIndex);
                finalSysSizing.TotCoolCapSeq(TimeStepIndex) = calcSysSizing.TotCoolCapSeq(TimeStepIndex);
                finalSysSizing.HeatCapSeq(TimeStepIndex) = calcSysSizing.HeatCapSeq(TimeStepIndex);
                finalSysSizing.PreheatCapSeq(TimeStepIndex) = calcSysSizing.PreheatCapSeq(TimeStepIndex);
                finalSysSizing.SysCoolRetTempSeq(TimeStepIndex) = calcSysSizing.SysCoolRetTempSeq(TimeStepIndex);
                finalSysSizing.SysCoolRetHumRatSeq(TimeStepIndex) = calcSysSizing.SysCoolRetHumRatSeq(TimeStepIndex);
                finalSysSizing.SysHeatRetTempSeq(TimeStepIndex) = calcSysSizing.SysHeatRetTempSeq(TimeStepIndex);
                finalSysSizing.SysHeatRetHumRatSeq(TimeStepIndex) = calcSysSizing.SysHeatRetHumRatSeq(TimeStepIndex);
                finalSysSizing.SysCoolOutTempSeq(TimeStepIndex) = calcSysSizing.SysCoolOutTempSeq(TimeStepIndex);
                finalSysSizing.SysCoolOutHumRatSeq(TimeStepIndex) = calcSysSizing.SysCoolOutHumRatSeq(TimeStepIndex);
                finalSysSizing.SysHeatOutTempSeq(TimeStepIndex) = calcSysSizing.SysHeatOutTempSeq(TimeStepIndex);
                finalSysSizing.SysHeatOutHumRatSeq(TimeStepIndex) = calcSysSizing.SysHeatOutHumRatSeq(TimeStepIndex);
                finalSysSizing.SysDOASHeatAddSeq(TimeStepIndex) = calcSysSizing.SysDOASHeatAddSeq(TimeStepIndex);
                finalSysSizing.SysDOASLatAddSeq(TimeStepIndex) = calcSysSizing.SysDOASLatAddSeq(TimeStepIndex);
            }
        }

        // Check for user input design system flow rates. Set the sizing ratios.
        for (AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) {
            auto &calcSysSizing = state.dataSize->CalcSysSizing(AirLoopNum);
            auto &finalSysSizing = state.dataSize->FinalSysSizing(AirLoopNum);
            // adjust system sizing flow rates for scalable flows
            UpdateSysSizingForScalableInputs(state, AirLoopNum);

            int NumZonesCooled = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesCooled;
            int NumZonesHeated = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesHeated;
            RhoAir = state.dataEnvrn->StdRhoAir;
            SysCoolSizingRat = 0.0;
            if (calcSysSizing.InpDesCoolAirFlow > 0.0 && calcSysSizing.DesCoolVolFlow > 0.0 &&
                (calcSysSizing.CoolAirDesMethod == AirflowSizingMethod::InpDesAirFlow || calcSysSizing.ScaleCoolSAFMethod == FlowPerFloorArea ||
                 calcSysSizing.ScaleCoolSAFMethod == FractionOfAutosizedCoolingAirflow ||
                 calcSysSizing.ScaleCoolSAFMethod == FlowPerCoolingCapacity)) {
                SysCoolSizingRat = calcSysSizing.InpDesCoolAirFlow / calcSysSizing.DesCoolVolFlow;
            } else {
                SysCoolSizingRat = 1.0;
            }

            SysHeatSizingRat = 0.0;
            if (calcSysSizing.InpDesHeatAirFlow > 0.0 && calcSysSizing.DesHeatVolFlow > 0.0 &&
                (calcSysSizing.HeatAirDesMethod == AirflowSizingMethod::InpDesAirFlow || calcSysSizing.ScaleHeatSAFMethod == FlowPerFloorArea ||
                 calcSysSizing.ScaleHeatSAFMethod == FractionOfAutosizedHeatingAirflow ||
                 calcSysSizing.ScaleHeatSAFMethod == FractionOfAutosizedCoolingAirflow ||
                 calcSysSizing.ScaleHeatSAFMethod == FlowPerHeatingCapacity)) {
                SysHeatSizingRat = calcSysSizing.InpDesHeatAirFlow / calcSysSizing.DesHeatVolFlow;
            } else {
                SysHeatSizingRat = 1.0;
            }

            if (calcSysSizing.loadSizingType == DataSizing::LoadSizing::Ventilation && SysCoolSizingRat == 1.0) {
                if (calcSysSizing.DesCoolVolFlow > 0.0) {
                    SysCoolSizingRat = calcSysSizing.DesOutAirVolFlow / calcSysSizing.DesCoolVolFlow;
                    state.dataSize->VotClgBySys(AirLoopNum) = finalSysSizing.DesOutAirVolFlow;
                } else {
                    SysCoolSizingRat = 1.0;
                }
            }
            if (calcSysSizing.loadSizingType == DataSizing::LoadSizing::Ventilation && SysHeatSizingRat == 1.0) {
                if (calcSysSizing.DesHeatVolFlow > 0.0) {
                    SysHeatSizingRat = calcSysSizing.DesOutAirVolFlow / calcSysSizing.DesHeatVolFlow;
                    state.dataSize->VotHtgBySys(AirLoopNum) = finalSysSizing.DesOutAirVolFlow;
                } else {
                    SysHeatSizingRat = 1.0;
                }
            }

            // Calculate the new user modified system design quantities
            if (std::abs(SysCoolSizingRat - 1.0) > 0.00001) {

                finalSysSizing.CoinCoolMassFlow = SysCoolSizingRat * calcSysSizing.CoinCoolMassFlow;
                finalSysSizing.NonCoinCoolMassFlow = SysCoolSizingRat * calcSysSizing.NonCoinCoolMassFlow;
                finalSysSizing.DesCoolVolFlow = SysCoolSizingRat * calcSysSizing.DesCoolVolFlow;
                finalSysSizing.MassFlowAtCoolPeak = SysCoolSizingRat * calcSysSizing.MassFlowAtCoolPeak;

                if (finalSysSizing.DesCoolVolFlow > 0.0) {

                    for (TimeStepIndex = 1; TimeStepIndex <= numOfTimeStepInDay; ++TimeStepIndex) {

                        if (calcSysSizing.CoolFlowSeq(TimeStepIndex) > 0.0) {

                            finalSysSizing.CoolFlowSeq(TimeStepIndex) = SysCoolSizingRat * calcSysSizing.CoolFlowSeq(TimeStepIndex);
                            if (finalSysSizing.CoolOAOption == OAControl::MinOA) {
                                OutAirFrac = RhoAir * finalSysSizing.DesOutAirVolFlow / finalSysSizing.CoolFlowSeq(TimeStepIndex);
                                OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                            } else {
                                OutAirFrac = 1.0;
                            }
                            SysCoolMixTemp = finalSysSizing.SysCoolOutTempSeq(TimeStepIndex) * OutAirFrac +
                                             finalSysSizing.SysCoolRetTempSeq(TimeStepIndex) * (1.0 - OutAirFrac);
                            SysCoolMixHumRat = finalSysSizing.SysCoolOutHumRatSeq(TimeStepIndex) * OutAirFrac +
                                               finalSysSizing.SysCoolRetHumRatSeq(TimeStepIndex) * (1.0 - OutAirFrac);
                            SysSensCoolCap = PsyCpAirFnW(DataPrecisionGlobals::constant_zero) * finalSysSizing.CoolFlowSeq(TimeStepIndex) *
                                             (SysCoolMixTemp - finalSysSizing.CoolSupTemp);
                            SysSensCoolCap = max(0.0, SysSensCoolCap);
                            SysTotCoolCap =
                                finalSysSizing.CoolFlowSeq(TimeStepIndex) *
                                (PsyHFnTdbW(SysCoolMixTemp, SysCoolMixHumRat) - PsyHFnTdbW(finalSysSizing.CoolSupTemp, finalSysSizing.CoolSupHumRat));
                            SysTotCoolCap = max(0.0, SysTotCoolCap);
                            finalSysSizing.SensCoolCapSeq(TimeStepIndex) = SysSensCoolCap;
                            finalSysSizing.TotCoolCapSeq(TimeStepIndex) = SysTotCoolCap;
                        }
                    }

                    if (finalSysSizing.CoolOAOption == OAControl::MinOA) {
                        OutAirFrac = finalSysSizing.DesOutAirVolFlow / finalSysSizing.DesCoolVolFlow;
                        OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                    } else {
                        OutAirFrac = 1.0;
                    }
                    finalSysSizing.MixTempAtCoolPeak =
                        finalSysSizing.OutTempAtCoolPeak * OutAirFrac + finalSysSizing.RetTempAtCoolPeak * (1.0 - OutAirFrac);
                    finalSysSizing.MixHumRatAtCoolPeak =
                        finalSysSizing.OutHumRatAtCoolPeak * OutAirFrac + finalSysSizing.RetHumRatAtCoolPeak * (1.0 - OutAirFrac);
                    finalSysSizing.SensCoolCap = PsyCpAirFnW(DataPrecisionGlobals::constant_zero) * RhoAir * finalSysSizing.DesCoolVolFlow *
                                                 (finalSysSizing.MixTempAtCoolPeak - finalSysSizing.CoolSupTemp);
                    finalSysSizing.SensCoolCap = max(0.0, finalSysSizing.SensCoolCap);
                    finalSysSizing.TotCoolCap = RhoAir * finalSysSizing.DesCoolVolFlow *
                                                (PsyHFnTdbW(finalSysSizing.MixTempAtCoolPeak, finalSysSizing.MixHumRatAtCoolPeak) -
                                                 PsyHFnTdbW(finalSysSizing.CoolSupTemp, finalSysSizing.CoolSupHumRat));
                    finalSysSizing.TotCoolCap = max(0.0, finalSysSizing.TotCoolCap);
                }

                // take account of the user input system flow rates and alter the zone flow rates to match
                for (int ZonesCooledNum = 1; ZonesCooledNum <= NumZonesCooled; ++ZonesCooledNum) {
                    int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesCooledNum);
                    if ((SysCoolSizingRat != 1.0) && (finalSysSizing.loadSizingType == DataSizing::LoadSizing::Ventilation) &&
                        (state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).MinOA > 0.0)) {
                        // size on ventilation load
                        if (state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).MinOA > 0.0) {
                            ZoneOARatio = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).MinOA /
                                          max(state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).DesCoolVolFlow,
                                              state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).MinOA);
                            ZoneOARatio *= (1.0 + state.dataSize->TermUnitSizing(TermUnitSizingIndex).InducRat);
                        } else {
                            ZoneOARatio = 0.0;
                        }
                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).scaleZoneCooling(ZoneOARatio);
                    } else if ((SysCoolSizingRat > 1.0) ||
                               (SysCoolSizingRat < 1.0 && finalSysSizing.SizingOption == DataSizing::SizingConcurrence::NonCoincident)) {
                        // size on user input system design flows
                        state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).scaleZoneCooling(SysCoolSizingRat);
                    }
                }
            }

            if (std::abs(SysHeatSizingRat - 1.0) > 0.00001) {

                finalSysSizing.CoinHeatMassFlow = SysHeatSizingRat * calcSysSizing.CoinHeatMassFlow;
                finalSysSizing.NonCoinHeatMassFlow = SysHeatSizingRat * calcSysSizing.NonCoinHeatMassFlow;
                finalSysSizing.DesHeatVolFlow = SysHeatSizingRat * calcSysSizing.DesHeatVolFlow;

                if (finalSysSizing.DesHeatVolFlow > 0.0) {

                    for (TimeStepIndex = 1; TimeStepIndex <= numOfTimeStepInDay; ++TimeStepIndex) {

                        if (calcSysSizing.HeatFlowSeq(TimeStepIndex) > 0.0) {

                            finalSysSizing.HeatFlowSeq(TimeStepIndex) = SysHeatSizingRat * calcSysSizing.HeatFlowSeq(TimeStepIndex);
                            if (finalSysSizing.HeatOAOption == DataSizing::OAControl::MinOA) {
                                OutAirFrac = RhoAir * finalSysSizing.DesOutAirVolFlow / finalSysSizing.HeatFlowSeq(TimeStepIndex);
                                OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                            } else {
                                OutAirFrac = 1.0;
                            }
                            SysHeatMixTemp = finalSysSizing.SysHeatOutTempSeq(TimeStepIndex) * OutAirFrac +
                                             finalSysSizing.SysHeatRetTempSeq(TimeStepIndex) * (1.0 - OutAirFrac);
                            SysHeatMixHumRat = finalSysSizing.SysHeatOutHumRatSeq(TimeStepIndex) * OutAirFrac +
                                               finalSysSizing.SysHeatRetHumRatSeq(TimeStepIndex) * (1.0 - OutAirFrac);
                            SysHeatCap = PsyCpAirFnW(DataPrecisionGlobals::constant_zero) * finalSysSizing.HeatFlowSeq(TimeStepIndex) *
                                         (finalSysSizing.HeatSupTemp - SysHeatMixTemp);
                            SysHeatCap = max(0.0, SysHeatCap);
                            finalSysSizing.HeatCapSeq(TimeStepIndex) = SysHeatCap;
                        }
                    }

                    if (finalSysSizing.HeatOAOption == DataSizing::OAControl::MinOA) {
                        OutAirFrac = finalSysSizing.DesOutAirVolFlow / finalSysSizing.DesHeatVolFlow;
                        OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                    } else {
                        OutAirFrac = 1.0;
                    }
                    finalSysSizing.HeatMixTemp = finalSysSizing.HeatOutTemp * OutAirFrac + finalSysSizing.HeatRetTemp * (1.0 - OutAirFrac);
                    finalSysSizing.HeatMixHumRat = finalSysSizing.HeatOutHumRat * OutAirFrac + finalSysSizing.HeatRetHumRat * (1.0 - OutAirFrac);
                    finalSysSizing.HeatCap = PsyCpAirFnW(DataPrecisionGlobals::constant_zero) * RhoAir * finalSysSizing.DesHeatVolFlow *
                                             (finalSysSizing.HeatSupTemp - finalSysSizing.HeatMixTemp);
                    finalSysSizing.HeatCap = max(0.0, finalSysSizing.HeatCap);
                }
                // take account of the user input system flow rates and alter the zone flow rates to match (for terminal unit sizing)
                if (NumZonesHeated > 0) {                                                              // IF there are centrally heated zones
                    for (int ZonesHeatedNum = 1; ZonesHeatedNum <= NumZonesHeated; ++ZonesHeatedNum) { // loop over the heated zones
                        int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitHeatSizingIndex(ZonesHeatedNum);
                        auto &termUnitFinalZoneSizing = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex);
                        if ((SysHeatSizingRat != 1.0) && (finalSysSizing.loadSizingType == DataSizing::LoadSizing::Ventilation) &&
                            (termUnitFinalZoneSizing.MinOA > 0.0)) {
                            // size on ventilation load
                            ZoneOARatio = termUnitFinalZoneSizing.MinOA / max(termUnitFinalZoneSizing.DesHeatVolFlow, termUnitFinalZoneSizing.MinOA);
                            ZoneOARatio *= (1.0 + state.dataSize->TermUnitSizing(TermUnitSizingIndex).InducRat);
                            termUnitFinalZoneSizing.scaleZoneHeating(ZoneOARatio);
                        } else if ((SysHeatSizingRat > 1.0) ||
                                   (SysHeatSizingRat < 1.0 && finalSysSizing.SizingOption == DataSizing::SizingConcurrence::NonCoincident)) {
                            // size on user input system design flows
                            termUnitFinalZoneSizing.scaleZoneHeating(SysHeatSizingRat);
                        }
                    }
                } else {                                                                               // No centrally heated zones: use cooled zones
                    for (int ZonesCooledNum = 1; ZonesCooledNum <= NumZonesCooled; ++ZonesCooledNum) { // loop over the cooled zones
                        int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesCooledNum);
                        auto &termUnitFinalZoneSizing = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex);
                        if ((SysHeatSizingRat != 1.0) && (finalSysSizing.loadSizingType == DataSizing::LoadSizing::Ventilation) &&
                            (termUnitFinalZoneSizing.MinOA <= 0.0)) {
                            ShowWarningError(state,
                                             format("FinalSystemSizing: AirLoop=\"{}\", Requested sizing on Ventilation,",
                                                    state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).AirLoopName));
                            ShowContinueError(state, format("but Zone has no design OA Flow. Zone=\"{}\".", termUnitFinalZoneSizing.ZoneName));
                        }
                        if ((SysHeatSizingRat != 1.0) && (finalSysSizing.loadSizingType == DataSizing::LoadSizing::Ventilation) &&
                            (termUnitFinalZoneSizing.MinOA > 0.0)) {
                            // size on ventilation load
                            ZoneOARatio = termUnitFinalZoneSizing.MinOA / max(termUnitFinalZoneSizing.DesHeatVolFlow, termUnitFinalZoneSizing.MinOA);
                            ZoneOARatio *= (1.0 + state.dataSize->TermUnitSizing(TermUnitSizingIndex).InducRat);
                            termUnitFinalZoneSizing.scaleZoneHeating(ZoneOARatio);
                        } else if ((SysHeatSizingRat != 1.0) && (finalSysSizing.loadSizingType == DataSizing::LoadSizing::Ventilation) &&
                                   (termUnitFinalZoneSizing.MinOA > 0.0)) {
                            // size on user input system design flows
                            termUnitFinalZoneSizing.scaleZoneHeating(SysHeatSizingRat);
                        }
                    }
                }
            }

            finalSysSizing.DesMainVolFlow = max(finalSysSizing.DesCoolVolFlow, finalSysSizing.DesHeatVolFlow);

            // loop over the zones cooled by this system and sum up the min cooling flow rates to get the
            // min system cooling flow rate
            for (int ZonesCooledNum = 1; ZonesCooledNum <= NumZonesCooled; ++ZonesCooledNum) {
                int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolSizingIndex(ZonesCooledNum);
                finalSysSizing.DesCoolVolFlowMin += state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).DesCoolVolFlowMin;
            }
            if (finalSysSizing.DesCoolVolFlowMin <= 0.0) {
                finalSysSizing.DesCoolVolFlowMin = finalSysSizing.DesOutAirVolFlow;
            }
        }

        // Specify the heating supply air Temp/HumRat for different system configurations
        for (AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) {

            int NumZonesHeated = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesHeated;

            if (NumZonesHeated > 0) { // IF there are centrally heated zones
                for (int ZonesHeatedNum = 1; ZonesHeatedNum <= NumZonesHeated; ++ZonesHeatedNum) {
                    int TermUnitSizingIndex = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitHeatSizingIndex(ZonesHeatedNum);

                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).DesHeatCoilInTempTU = GetHeatingSATempForSizing(state, AirLoopNum);
                    state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex).DesHeatCoilInHumRatTU =
                        GetHeatingSATempHumRatForSizing(state, AirLoopNum);
                }
            }
        }

        // EMS calling point to customize system sizing results
        bool anyEMSRan;
        ManageEMS(state, EMSManager::EMSCallFrom::SystemSizing, anyEMSRan, ObjexxFCL::Optional_int_const());

        // EMS override point
        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
            for (AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) {
                auto &finalSysSizing = state.dataSize->FinalSysSizing(AirLoopNum);
                if (finalSysSizing.EMSOverrideCoinCoolMassFlowOn) {
                    finalSysSizing.CoinCoolMassFlow = finalSysSizing.EMSValueCoinCoolMassFlow;
                }
                if (finalSysSizing.EMSOverrideCoinHeatMassFlowOn) {
                    finalSysSizing.CoinHeatMassFlow = finalSysSizing.EMSValueCoinHeatMassFlow;
                }
                if (finalSysSizing.EMSOverrideNonCoinCoolMassFlowOn) {
                    finalSysSizing.NonCoinCoolMassFlow = finalSysSizing.EMSValueNonCoinCoolMassFlow;
                }
                if (finalSysSizing.EMSOverrideNonCoinHeatMassFlowOn) {
                    finalSysSizing.NonCoinHeatMassFlow = finalSysSizing.EMSValueNonCoinHeatMassFlow;
                }
                if (finalSysSizing.EMSOverrideDesMainVolFlowOn) {
                    finalSysSizing.DesMainVolFlow = finalSysSizing.EMSValueDesMainVolFlow;
                }
                if (finalSysSizing.EMSOverrideDesHeatVolFlowOn) {
                    finalSysSizing.DesHeatVolFlow = finalSysSizing.EMSValueDesHeatVolFlow;
                }
                if (finalSysSizing.EMSOverrideDesCoolVolFlowOn) {
                    finalSysSizing.DesCoolVolFlow = finalSysSizing.EMSValueDesCoolVolFlow;
                }

            } // over NumPrimaryAirSys
        }

        // determine if main design is from cooling or heating
        for (AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) {
            if (state.dataSize->FinalSysSizing(AirLoopNum).DesMainVolFlow == state.dataSize->FinalSysSizing(AirLoopNum).DesCoolVolFlow) {
                state.dataSize->FinalSysSizing(AirLoopNum).sysSizeCoolingDominant = true;
            } else if (state.dataSize->FinalSysSizing(AirLoopNum).DesMainVolFlow == state.dataSize->FinalSysSizing(AirLoopNum).DesHeatVolFlow) {
                state.dataSize->FinalSysSizing(AirLoopNum).sysSizeHeatingDominant = true;
            }
        }

        // write out the sys design calc results

        print(state.files.ssz, "Time");
        for (I = 1; I <= state.dataHVACGlobal->NumPrimaryAirSys; ++I) {
            for (J = 1; J <= state.dataEnvrn->TotDesDays + state.dataEnvrn->TotRunDesPersDays; ++J) {
                constexpr const char *SSizeFmt12("{}{}{}{:2}{}{}{}{}{:2}{}{}{}{}{:2}{}{}{}{}{:2}{}{}{}{}{:2}{}");
                print(state.files.ssz,
                      SSizeFmt12,
                      state.dataSize->SizingFileColSep,
                      state.dataSize->CalcSysSizing(I).AirPriLoopName,
                      ":DesPer",
                      J,
                      ":Des Heat Mass Flow [kg/s]",
                      state.dataSize->SizingFileColSep,
                      state.dataSize->CalcSysSizing(I).AirPriLoopName,
                      ":DesPer",
                      J,
                      ":Des Heat Cap [W]",
                      state.dataSize->SizingFileColSep,
                      state.dataSize->CalcSysSizing(I).AirPriLoopName,
                      ":DesPer",
                      J,
                      ":Des Cool Mass Flow [kg/s]",
                      state.dataSize->SizingFileColSep,
                      state.dataSize->CalcSysSizing(I).AirPriLoopName,
                      ":DesPer",
                      J,
                      ":Des Sens Cool Cap [W]",
                      state.dataSize->SizingFileColSep,
                      state.dataSize->CalcSysSizing(I).AirPriLoopName,
                      ":DesPer",
                      J,
                      ":Des Tot Cool Cap [W]");
            }
        }
        print(state.files.ssz, "\n");
        //      HourFrac = 0.0
        Minutes = 0;
        TimeStepIndex = 0;
        for (HourCounter = 1; HourCounter <= 24; ++HourCounter) {
            for (TimeStepCounter = 1; TimeStepCounter <= state.dataGlobal->TimeStepsInHour; ++TimeStepCounter) {
                ++TimeStepIndex;
                Minutes += state.dataGlobal->MinutesInTimeStep;
                if (Minutes == 60) {
                    Minutes = 0;
                    HourPrint = HourCounter;
                } else {
                    HourPrint = HourCounter - 1;
                }
                constexpr const char *SSizeFmt20("{:02}:{:02}:00");
                print(state.files.ssz, SSizeFmt20, HourPrint, Minutes);
                for (I = 1; I <= state.dataHVACGlobal->NumPrimaryAirSys; ++I) {
                    for (J = 1; J <= state.dataEnvrn->TotDesDays + state.dataEnvrn->TotRunDesPersDays; ++J) {
                        constexpr const char *SSizeFmt22("{}{:12.6E}{}{:12.6E}{}{:12.6E}{}{:12.6E}{}{:12.6E}");

                        print(state.files.ssz,
                              SSizeFmt22,
                              state.dataSize->SizingFileColSep,
                              state.dataSize->SysSizing(J, I).HeatFlowSeq(TimeStepIndex),
                              state.dataSize->SizingFileColSep,
                              state.dataSize->SysSizing(J, I).HeatCapSeq(TimeStepIndex),
                              state.dataSize->SizingFileColSep,
                              state.dataSize->SysSizing(J, I).CoolFlowSeq(TimeStepIndex),
                              state.dataSize->SizingFileColSep,
                              state.dataSize->SysSizing(J, I).SensCoolCapSeq(TimeStepIndex),
                              state.dataSize->SizingFileColSep,
                              state.dataSize->SysSizing(J, I).TotCoolCapSeq(TimeStepIndex));
                    }
                }
                print(state.files.ssz, "\n");
            }
        }

        constexpr const char *SSizeFmt31("{}{:12.6E}{}{:12.6E}{}{:12.6E}{}{:12.6E}");
        print(state.files.ssz, "Coinc Peak   ");
        for (I = 1; I <= state.dataHVACGlobal->NumPrimaryAirSys; ++I) {
            print(state.files.ssz,
                  SSizeFmt31,
                  state.dataSize->SizingFileColSep,
                  state.dataSize->CalcSysSizing(I).CoinHeatMassFlow,
                  state.dataSize->SizingFileColSep,
                  state.dataSize->CalcSysSizing(I).CoinCoolMassFlow,
                  state.dataSize->SizingFileColSep,
                  state.dataSize->CalcSysSizing(I).HeatCap,
                  state.dataSize->SizingFileColSep,
                  state.dataSize->CalcSysSizing(I).SensCoolCap);
        }
        print(state.files.ssz, "\n");

        print(state.files.ssz, "NonCoinc Peak");
        for (I = 1; I <= state.dataHVACGlobal->NumPrimaryAirSys; ++I) {
            print(state.files.ssz,
                  SSizeFmt31,
                  state.dataSize->SizingFileColSep,
                  state.dataSize->CalcSysSizing(I).NonCoinHeatMassFlow,
                  state.dataSize->SizingFileColSep,
                  state.dataSize->CalcSysSizing(I).NonCoinCoolMassFlow,
                  state.dataSize->SizingFileColSep,
                  state.dataSize->CalcSysSizing(I).HeatCap,
                  state.dataSize->SizingFileColSep,
                  state.dataSize->CalcSysSizing(I).SensCoolCap);
        }
        print(state.files.ssz, "\n");
        // have moved a big section to later in calling order, write predefined standard 62.1 report data
    } break;
    default:
        break;
    }
}

void UpdateSysSizingForScalableInputs(EnergyPlusData &state, int const AirLoopNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Bereket Nigusse
    //       DATE WRITTEN   Auguts 2014
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Modifies the design sizing flow rates for system scalable sizing method

    // Using/Aliasing
    using Psychrometrics::PsyCpAirFnW;
    using Psychrometrics::PsyHFnTdbW;

    // SUBROUTINE PARAMETER DEFINITIONS:

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 TempSize;           // autosized value
    Real64 CoilInTemp;         // entering coil air temperature [C]
    Real64 CoilInHumRat;       // entering coil air humidity ratio [kg/kg]
    Real64 CoilInEnth;         // entering coil air enthalpy [J/kg]
    Real64 CoilOutTemp;        // coil outlet air temperature [C]
    Real64 CoilOutHumRat;      // coil outlet air humidity ratio [kg/kg]
    Real64 CoilOutEnth;        // coil outlet air enthalpy [J/kg]
    Real64 OutAirFrac;         // outdoor air fraction [-]
    Real64 CpAirStd;           // specific heat of air at standard condition
    Real64 FractionOfAutosize; // user specified autosized fraction for capacity and supply air flow
    Real64 AutosizedCapacity;  // autosized heating and cooling capacity

    auto &FinalSysSizing = state.dataSize->FinalSysSizing;
    auto &CalcSysSizing = state.dataSize->CalcSysSizing;
    auto &PrimaryAirSystems = state.dataAirSystemsData->PrimaryAirSystems;

    state.dataSize->DataFracOfAutosizedCoolingCapacity = 1.0;
    state.dataSize->DataFracOfAutosizedHeatingCapacity = 1.0;

    if (AirLoopNum > 0) {

        TempSize = 0.0;
        FractionOfAutosize = 1.0;

        // scalable sizing option for cooling supply air flow rate
        switch (FinalSysSizing(AirLoopNum).ScaleCoolSAFMethod) {
        case FlowPerFloorArea: {
            TempSize = FinalSysSizing(AirLoopNum).FlowPerFloorAreaCooled * FinalSysSizing(AirLoopNum).FloorAreaOnAirLoopCooled;
            CalcSysSizing(AirLoopNum).InpDesCoolAirFlow = TempSize;
            FinalSysSizing(AirLoopNum).InpDesCoolAirFlow = TempSize;
        } break;
        case FractionOfAutosizedCoolingAirflow: {
            FractionOfAutosize = FinalSysSizing(AirLoopNum).FractionOfAutosizedCoolingAirflow;
            CalcSysSizing(AirLoopNum).InpDesCoolAirFlow = CalcSysSizing(AirLoopNum).DesCoolVolFlow * FractionOfAutosize;
            FinalSysSizing(AirLoopNum).InpDesCoolAirFlow = FinalSysSizing(AirLoopNum).DesCoolVolFlow * FractionOfAutosize;
        } break;
        case FlowPerCoolingCapacity: {
            if (FinalSysSizing(AirLoopNum).CoolingCapMethod == FractionOfAutosizedCoolingCapacity) {
                FractionOfAutosize = FinalSysSizing(AirLoopNum).ScaledCoolingCapacity;
                if (PrimaryAirSystems(AirLoopNum).NumOACoolCoils == 0) { // there is no precooling of the OA stream
                    CoilInTemp = FinalSysSizing(AirLoopNum).MixTempAtCoolPeak;
                    CoilInHumRat = FinalSysSizing(AirLoopNum).MixHumRatAtCoolPeak;
                } else { // there is precooling of OA stream
                    if (FinalSysSizing(AirLoopNum).DesCoolVolFlow > 0.0) {
                        OutAirFrac = FinalSysSizing(AirLoopNum).DesOutAirVolFlow / FinalSysSizing(AirLoopNum).DesCoolVolFlow;
                    } else {
                        OutAirFrac = 1.0;
                    }
                    OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                    CoilInTemp =
                        OutAirFrac * FinalSysSizing(AirLoopNum).PrecoolTemp + (1.0 - OutAirFrac) * FinalSysSizing(AirLoopNum).RetTempAtCoolPeak;
                    CoilInHumRat =
                        OutAirFrac * FinalSysSizing(AirLoopNum).PrecoolHumRat + (1.0 - OutAirFrac) * FinalSysSizing(AirLoopNum).RetHumRatAtCoolPeak;
                }
                CoilOutTemp = FinalSysSizing(AirLoopNum).CoolSupTemp;
                CoilOutHumRat = FinalSysSizing(AirLoopNum).CoolSupHumRat;
                CoilInEnth = PsyHFnTdbW(CoilInTemp, CoilInHumRat);
                CoilOutEnth = PsyHFnTdbW(CoilOutTemp, CoilOutHumRat);
                AutosizedCapacity = state.dataEnvrn->StdRhoAir * FinalSysSizing(AirLoopNum).DesCoolVolFlow * (CoilInEnth - CoilOutEnth);
                TempSize = FinalSysSizing(AirLoopNum).FlowPerCoolingCapacity * AutosizedCapacity * FractionOfAutosize;
            } else if (FinalSysSizing(AirLoopNum).CoolingCapMethod == CoolingDesignCapacity) {
                if (FinalSysSizing(AirLoopNum).ScaledCoolingCapacity == DataSizing::AutoSize) {
                    if (PrimaryAirSystems(AirLoopNum).NumOACoolCoils == 0) { // there is no precooling of the OA stream
                        CoilInTemp = FinalSysSizing(AirLoopNum).MixTempAtCoolPeak;
                        CoilInHumRat = FinalSysSizing(AirLoopNum).MixHumRatAtCoolPeak;
                    } else { // there is precooling of OA stream
                        if (FinalSysSizing(AirLoopNum).DesCoolVolFlow > 0.0) {
                            OutAirFrac = FinalSysSizing(AirLoopNum).DesOutAirVolFlow / FinalSysSizing(AirLoopNum).DesCoolVolFlow;
                        } else {
                            OutAirFrac = 1.0;
                        }
                        OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                        CoilInTemp =
                            OutAirFrac * FinalSysSizing(AirLoopNum).PrecoolTemp + (1.0 - OutAirFrac) * FinalSysSizing(AirLoopNum).RetTempAtCoolPeak;
                        CoilInHumRat = OutAirFrac * FinalSysSizing(AirLoopNum).PrecoolHumRat +
                                       (1.0 - OutAirFrac) * FinalSysSizing(AirLoopNum).RetHumRatAtCoolPeak;
                    }
                    CoilOutTemp = FinalSysSizing(AirLoopNum).CoolSupTemp;
                    CoilOutHumRat = FinalSysSizing(AirLoopNum).CoolSupHumRat;
                    CoilInEnth = PsyHFnTdbW(CoilInTemp, CoilInHumRat);
                    CoilOutEnth = PsyHFnTdbW(CoilOutTemp, CoilOutHumRat);
                    AutosizedCapacity = state.dataEnvrn->StdRhoAir * FinalSysSizing(AirLoopNum).DesCoolVolFlow * (CoilInEnth - CoilOutEnth);
                    TempSize = FinalSysSizing(AirLoopNum).FlowPerCoolingCapacity * AutosizedCapacity * FractionOfAutosize;
                } else {
                    TempSize = FinalSysSizing(AirLoopNum).FlowPerCoolingCapacity * FinalSysSizing(AirLoopNum).ScaledCoolingCapacity;
                }
            } else if (FinalSysSizing(AirLoopNum).CoolingCapMethod == CapacityPerFloorArea) {
                TempSize = FinalSysSizing(AirLoopNum).FlowPerCoolingCapacity * FinalSysSizing(AirLoopNum).ScaledCoolingCapacity *
                           FinalSysSizing(AirLoopNum).FloorAreaOnAirLoopCooled;
            }
            CalcSysSizing(AirLoopNum).InpDesCoolAirFlow = TempSize;
            FinalSysSizing(AirLoopNum).InpDesCoolAirFlow = TempSize;
        } break;
        default:
            break;
        }

        // scalable sizing option for heating supply air flow rate
        switch (FinalSysSizing(AirLoopNum).ScaleHeatSAFMethod) {
        case FlowPerFloorArea: {
            TempSize = FinalSysSizing(AirLoopNum).FlowPerFloorAreaHeated * FinalSysSizing(AirLoopNum).FloorAreaOnAirLoopHeated;
            CalcSysSizing(AirLoopNum).InpDesHeatAirFlow = TempSize;
            FinalSysSizing(AirLoopNum).InpDesHeatAirFlow = TempSize;
        } break;
        case FractionOfAutosizedHeatingAirflow: {
            FractionOfAutosize = FinalSysSizing(AirLoopNum).FractionOfAutosizedHeatingAirflow;
            CalcSysSizing(AirLoopNum).InpDesHeatAirFlow = CalcSysSizing(AirLoopNum).DesHeatVolFlow * FractionOfAutosize;
            FinalSysSizing(AirLoopNum).InpDesHeatAirFlow = FinalSysSizing(AirLoopNum).DesHeatVolFlow * FractionOfAutosize;
        } break;
        case FractionOfAutosizedCoolingAirflow: {
            FractionOfAutosize = FinalSysSizing(AirLoopNum).FractionOfAutosizedCoolingAirflow;
            CalcSysSizing(AirLoopNum).InpDesHeatAirFlow = CalcSysSizing(AirLoopNum).DesHeatVolFlow * FractionOfAutosize;
            FinalSysSizing(AirLoopNum).InpDesHeatAirFlow = FinalSysSizing(AirLoopNum).DesHeatVolFlow * FractionOfAutosize;
        } break;
        case FlowPerHeatingCapacity: {
            if (FinalSysSizing(AirLoopNum).HeatingCapMethod == FractionOfAutosizedHeatingCapacity) {
                FractionOfAutosize = FinalSysSizing(AirLoopNum).ScaledHeatingCapacity;
                if (FinalSysSizing(AirLoopNum).HeatOAOption == DataSizing::OAControl::MinOA) {
                    if (FinalSysSizing(AirLoopNum).DesHeatVolFlow > 0.0) {
                        OutAirFrac = FinalSysSizing(AirLoopNum).DesOutAirVolFlow / FinalSysSizing(AirLoopNum).DesHeatVolFlow;
                    } else {
                        OutAirFrac = 1.0;
                    }
                    OutAirFrac = std::min(1.0, std::max(0.0, OutAirFrac));
                } else {
                    OutAirFrac = 1.0;
                }
                if (state.dataSize->CurOASysNum == 0 && PrimaryAirSystems(AirLoopNum).NumOAHeatCoils > 0) {
                    CoilInTemp = OutAirFrac * FinalSysSizing(AirLoopNum).PreheatTemp + (1.0 - OutAirFrac) * FinalSysSizing(AirLoopNum).HeatRetTemp;
                } else {
                    CoilInTemp = OutAirFrac * FinalSysSizing(AirLoopNum).HeatOutTemp + (1.0 - OutAirFrac) * FinalSysSizing(AirLoopNum).HeatRetTemp;
                }
                CoilOutTemp = FinalSysSizing(AirLoopNum).HeatSupTemp;
                CpAirStd = PsyCpAirFnW(DataPrecisionGlobals::constant_zero);
                AutosizedCapacity = state.dataEnvrn->StdRhoAir * FinalSysSizing(AirLoopNum).DesHeatVolFlow * CpAirStd * (CoilOutTemp - CoilInTemp);
                TempSize = FinalSysSizing(AirLoopNum).FlowPerHeatingCapacity * AutosizedCapacity * FractionOfAutosize;
            } else if (FinalSysSizing(AirLoopNum).HeatingCapMethod == HeatingDesignCapacity) {
                if (FinalSysSizing(AirLoopNum).ScaledHeatingCapacity == DataSizing::AutoSize) {
                    if (FinalSysSizing(AirLoopNum).HeatOAOption == DataSizing::OAControl::MinOA) {
                        if (FinalSysSizing(AirLoopNum).DesHeatVolFlow > 0.0) {
                            OutAirFrac = FinalSysSizing(AirLoopNum).DesOutAirVolFlow / FinalSysSizing(AirLoopNum).DesHeatVolFlow;
                        } else {
                            OutAirFrac = 1.0;
                        }
                        OutAirFrac = std::min(1.0, std::max(0.0, OutAirFrac));
                    } else {
                        OutAirFrac = 1.0;
                    }
                    if (state.dataSize->CurOASysNum == 0 && PrimaryAirSystems(AirLoopNum).NumOAHeatCoils > 0) {
                        CoilInTemp =
                            OutAirFrac * FinalSysSizing(AirLoopNum).PreheatTemp + (1.0 - OutAirFrac) * FinalSysSizing(AirLoopNum).HeatRetTemp;
                    } else {
                        CoilInTemp =
                            OutAirFrac * FinalSysSizing(AirLoopNum).HeatOutTemp + (1.0 - OutAirFrac) * FinalSysSizing(AirLoopNum).HeatRetTemp;
                    }
                    CoilOutTemp = FinalSysSizing(AirLoopNum).HeatSupTemp;
                    CpAirStd = PsyCpAirFnW(DataPrecisionGlobals::constant_zero);
                    AutosizedCapacity =
                        state.dataEnvrn->StdRhoAir * FinalSysSizing(AirLoopNum).DesHeatVolFlow * CpAirStd * (CoilOutTemp - CoilInTemp);
                    TempSize = FinalSysSizing(AirLoopNum).FlowPerHeatingCapacity * AutosizedCapacity * FractionOfAutosize;
                } else {
                    TempSize = FinalSysSizing(AirLoopNum).FlowPerHeatingCapacity * FinalSysSizing(AirLoopNum).ScaledHeatingCapacity;
                }
            } else if (FinalSysSizing(AirLoopNum).HeatingCapMethod == CapacityPerFloorArea) {
                TempSize = FinalSysSizing(AirLoopNum).FlowPerHeatingCapacity * FinalSysSizing(AirLoopNum).ScaledHeatingCapacity *
                           FinalSysSizing(AirLoopNum).FloorAreaOnAirLoopCooled;
            }
            CalcSysSizing(AirLoopNum).InpDesHeatAirFlow = TempSize;
            FinalSysSizing(AirLoopNum).InpDesHeatAirFlow = TempSize;
        } break;
        default:
            break;
        }

        // save the total cooling capacity sizing data for scalable sizing
        switch (FinalSysSizing(AirLoopNum).CoolingCapMethod) {
        case CoolingDesignCapacity: {
            if (CalcSysSizing(AirLoopNum).ScaledCoolingCapacity > 0.0) {
                CalcSysSizing(AirLoopNum).CoolingTotalCapacity = CalcSysSizing(AirLoopNum).ScaledCoolingCapacity;
                FinalSysSizing(AirLoopNum).CoolingTotalCapacity = CalcSysSizing(AirLoopNum).ScaledCoolingCapacity;
            } else {
                FinalSysSizing(AirLoopNum).CoolingTotalCapacity = 0.0; // autosized, set to zero initially
            }
        } break;
        case CapacityPerFloorArea: {
            FinalSysSizing(AirLoopNum).CoolingTotalCapacity =
                CalcSysSizing(AirLoopNum).ScaledCoolingCapacity * FinalSysSizing(AirLoopNum).FloorAreaOnAirLoopCooled;
        } break;
        case FractionOfAutosizedCoolingCapacity: {
            CalcSysSizing(AirLoopNum).FractionOfAutosizedCoolingCapacity = CalcSysSizing(AirLoopNum).ScaledCoolingCapacity;
            FinalSysSizing(AirLoopNum).FractionOfAutosizedCoolingCapacity = CalcSysSizing(AirLoopNum).ScaledCoolingCapacity;
        } break;
        default:
            break;
        }

        // save the total heating capacity sizing data for scalable sizing
        switch (FinalSysSizing(AirLoopNum).HeatingCapMethod) {
        case HeatingDesignCapacity: {
            if (CalcSysSizing(AirLoopNum).ScaledHeatingCapacity > 0.0) {
                FinalSysSizing(AirLoopNum).HeatingTotalCapacity = CalcSysSizing(AirLoopNum).ScaledHeatingCapacity;
            } else {
                FinalSysSizing(AirLoopNum).HeatingTotalCapacity = 0.0; // autosized, set to zero initially
            }
        } break;
        case CapacityPerFloorArea: {
            // even for heating capacity we use cooled zones floor area ( *.FloorAreaOnAirLoopCooled ) served by the airloop
            FinalSysSizing(AirLoopNum).HeatingTotalCapacity =
                CalcSysSizing(AirLoopNum).ScaledHeatingCapacity * FinalSysSizing(AirLoopNum).FloorAreaOnAirLoopCooled;
        } break;
        case FractionOfAutosizedHeatingCapacity: {
            FinalSysSizing(AirLoopNum).FractionOfAutosizedHeatingCapacity = CalcSysSizing(AirLoopNum).ScaledHeatingCapacity;
        } break;
        default:
            break;
        }
    }
}

Real64 GetHeatingSATempForSizing(EnergyPlusData &state, int const IndexAirLoop // air loop index
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl, Rongpeng Zhang
    //       DATE WRITTEN   October 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine get the proper reheat coil inlet temperature for sizing, depending on
    // the system configurations:
    // (1) Central heating coils exist
    // (2) No central heating coils, but preheating coils or OA heat-exchangers exist
    // (3) No central heating coils; No preheating coils or OA heat-exchangers

    // Using/Aliasing
    using namespace DataSizing;
    using Psychrometrics::PsyHFnTdbW;
    using Psychrometrics::PsyTdbFnHW;

    // Locals
    Real64 ReheatCoilInTempForSizing;     // Dry bulb temperature of the reheat coil inlet air [C]
    Real64 ReheatCoilInHumRatForSizing;   // Humidity ratio of the reheat coil inlet air [kg/kg]
    Real64 ReheatCoilInEnthalpyForSizing; // Enthalpy of the reheat coil inlet air [J/kg]
    Real64 OutAirFrac;

    auto &CalcSysSizing = state.dataSize->CalcSysSizing;
    auto &FinalSysSizing = state.dataSize->FinalSysSizing;
    auto &PrimaryAirSystems = state.dataAirSystemsData->PrimaryAirSystems;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    if (PrimaryAirSystems(IndexAirLoop).CentralHeatCoilExists) {
        // Case: Central heating coils exist

        ReheatCoilInTempForSizing = CalcSysSizing(IndexAirLoop).HeatSupTemp;

    } else if ((PrimaryAirSystems(IndexAirLoop).NumOAHeatCoils > 0) || (PrimaryAirSystems(IndexAirLoop).NumOAHXs)) {
        // Case: No central heating coils, but preheating coils or OA heat-exchangers exist

        if (FinalSysSizing(IndexAirLoop).DesHeatVolFlow > 0) {
            OutAirFrac = FinalSysSizing(IndexAirLoop).DesOutAirVolFlow / FinalSysSizing(IndexAirLoop).DesHeatVolFlow;
            OutAirFrac = min(1.0, max(0.0, OutAirFrac));
        } else {
            OutAirFrac = 0.0;
        }

        // Mixed air humidity ratio and enthalpy
        ReheatCoilInHumRatForSizing =
            OutAirFrac * FinalSysSizing(IndexAirLoop).PreheatHumRat + (1 - OutAirFrac) * FinalSysSizing(IndexAirLoop).HeatRetHumRat;
        ReheatCoilInEnthalpyForSizing =
            OutAirFrac * PsyHFnTdbW(FinalSysSizing(IndexAirLoop).PreheatTemp, FinalSysSizing(IndexAirLoop).PreheatHumRat) +
            (1 - OutAirFrac) * PsyHFnTdbW(FinalSysSizing(IndexAirLoop).HeatRetTemp, FinalSysSizing(IndexAirLoop).HeatRetHumRat);

        // Mixed air dry bulb temperature
        ReheatCoilInTempForSizing = PsyTdbFnHW(ReheatCoilInEnthalpyForSizing, ReheatCoilInHumRatForSizing);

    } else {
        // Case: No central heating coils; No preheating coils or OA heat-exchangers

        ReheatCoilInTempForSizing = FinalSysSizing(IndexAirLoop).HeatMixTemp;
    }

    return ReheatCoilInTempForSizing;
}

Real64 GetHeatingSATempHumRatForSizing(EnergyPlusData &state, int const IndexAirLoop // air loop index
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl, Rongpeng Zhang
    //       DATE WRITTEN   October 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine get the proper reheat coil inlet humidity ratio for sizing, depending on
    // the system configurations:
    // (1) Central heating coils exist
    // (2) No central heating coils, but preheating coils or OA heat-exchangers exist
    // (3) No central heating coils; No preheating coils or OA heat-exchangers

    // Using/Aliasing
    using namespace DataSizing;

    // Locals
    Real64 ReheatCoilInHumRatForSizing;
    Real64 OutAirFrac;

    auto &FinalSysSizing = state.dataSize->FinalSysSizing;
    auto &PrimaryAirSystems = state.dataAirSystemsData->PrimaryAirSystems;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    if (PrimaryAirSystems(IndexAirLoop).CentralHeatCoilExists) {
        // Case: Central heating coils exist

        ReheatCoilInHumRatForSizing = state.dataSize->CalcSysSizing(IndexAirLoop).HeatSupHumRat;

    } else if ((PrimaryAirSystems(IndexAirLoop).NumOAHeatCoils > 0) || (PrimaryAirSystems(IndexAirLoop).NumOAHXs)) {
        // Case: No central heating coils, but preheating coils or OA heat-exchangers exist

        if (FinalSysSizing(IndexAirLoop).DesHeatVolFlow > 0) {
            OutAirFrac = FinalSysSizing(IndexAirLoop).DesOutAirVolFlow / FinalSysSizing(IndexAirLoop).DesHeatVolFlow;
            OutAirFrac = min(1.0, max(0.0, OutAirFrac));
        } else {
            OutAirFrac = 0.0;
        }

        ReheatCoilInHumRatForSizing =
            OutAirFrac * FinalSysSizing(IndexAirLoop).PreheatHumRat + (1 - OutAirFrac) * FinalSysSizing(IndexAirLoop).HeatRetHumRat;

    } else {
        // Case: No central heating coils; No preheating coils or OA heat-exchangers

        ReheatCoilInHumRatForSizing = FinalSysSizing(IndexAirLoop).HeatMixHumRat;
    }

    return ReheatCoilInHumRatForSizing;
}

void CheckWaterCoilIsOnAirLoop(EnergyPlusData &state,
                               SimAirServingZones::CompType const CompTypeNum,
                               std::string const &CompType,
                               std::string const &CompName,
                               bool &WaterCoilOnAirLoop)
{
    // PURPOSE OF THIS FUNCTION:
    // This function returns true if a water coil that has water controller is either on
    // primary air or outdoor air system branch. Searches for water coil name and type
    // that match components list in primary air and outside air systems.

    // Return value
    bool CheckWaterCoilIsOnAirLoop(false);

    CheckWaterCoilIsOnAirLoop = CheckWaterCoilOnPrimaryAirLoopBranch(state, CompTypeNum, CompName);
    if (!CheckWaterCoilIsOnAirLoop) {
        CheckWaterCoilIsOnAirLoop = CheckWaterCoilOnOASystem(state, CompTypeNum, CompName);
    }

    if (!CheckWaterCoilIsOnAirLoop) {
        CheckWaterCoilIsOnAirLoop = CheckWaterCoilSystemOnAirLoopOrOASystem(state, CompTypeNum, CompName);
    }
    if (!CheckWaterCoilIsOnAirLoop) {
        ShowSevereError(state, format("CheckWaterCoilIsOnAirLoop: = {} = {}.", CompType, CompName));
        ShowContinueError(state,
                          "The water coil or coil system is neither on primary air branch nor on outdoor air system hence does not require "
                          "'Controller:WaterCoil' object.");
    }
    WaterCoilOnAirLoop = CheckWaterCoilIsOnAirLoop;
}

bool CheckWaterCoilOnPrimaryAirLoopBranch(EnergyPlusData &state, SimAirServingZones::CompType const CompTypeNum, std::string const &CompName)
{
    // PURPOSE OF THIS FUNCTION:
    // This function returns true if a water coil that has water controller is on
    // primary air loop branch. Searches for water coil name and type that match
    // components list in primary air systems.

    if (state.dataSimAirServingZones->GetAirLoopInputFlag) { // First time subroutine has been entered
        GetAirPathData(state);                               // Get air loop descriptions from input file
        state.dataSimAirServingZones->GetAirLoopInputFlag = false;
    }

    if (state.dataHVACGlobal->NumPrimaryAirSys > 0) {
        for (int AirSysNum = 1; AirSysNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirSysNum) {
            auto const &primaryAirSystems = state.dataAirSystemsData->PrimaryAirSystems(AirSysNum);
            for (int BranchNum = 1; BranchNum <= primaryAirSystems.NumBranches; ++BranchNum) {
                for (int CompNum = 1; CompNum <= primaryAirSystems.Branch(BranchNum).TotalComponents; ++CompNum) {
                    if ((CompTypeNum == primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num) &&
                        Util::SameString(CompName, primaryAirSystems.Branch(BranchNum).Comp(CompNum).Name)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool CheckWaterCoilOnOASystem(EnergyPlusData &state, SimAirServingZones::CompType const CompTypeNum, std::string const &CompName)
{
    // PURPOSE OF THIS FUNCTION:
    // This function returns true if a water coil that has water controller is on
    // outdoor air system. Searches for water coil name and type that match
    // components list on outside air systems.

    // USE STATEMENTS:
    using MixedAir::GetNumOASystems;
    using MixedAir::GetOutsideAirSysInputs;

    if (state.dataMixedAir->GetOASysInputFlag) {
        GetOutsideAirSysInputs(state);
        state.dataMixedAir->GetOASysInputFlag = false;
    }
    int NumOASys = GetNumOASystems(state);
    if (NumOASys > 0) {
        for (int OASysNum = 1; OASysNum <= NumOASys; ++OASysNum) {
            auto &outsideAirSys = state.dataAirLoop->OutsideAirSys(OASysNum);
            for (int OACompNum = 1; OACompNum <= outsideAirSys.NumComponents; ++OACompNum) {
                if ((CompTypeNum == outsideAirSys.ComponentTypeEnum(OACompNum)) &&
                    (Util::SameString(CompName, outsideAirSys.ComponentName(OACompNum)))) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool CheckWaterCoilSystemOnAirLoopOrOASystem(EnergyPlusData &state, SimAirServingZones::CompType const CompTypeNum, std::string const &CompName)
{
    // PURPOSE OF THIS FUNCTION:
    // This function returns true if a water coil which is part of CoilSystem:Cooling:Water:HeatExchangerAssisted
    // and that has water controller is on primary air loop branch or outdoor air system. Searches for water coilsystem
    // type and name that match components list in primary air loop or outside air systems.

    // USE STATEMENTS:
    using HVACHXAssistedCoolingCoil::GetHXAssistedCoolingCoilInput;

    // Return value
    bool CheckWaterCoilSystemIsOnAirLoopOASystem(false);

    if (state.dataHVACAssistedCC->GetCoilsInputFlag) {
        // Get the HXAssistedCoolingCoil input
        GetHXAssistedCoolingCoilInput(state);
        state.dataHVACAssistedCC->GetCoilsInputFlag = false;
    }

    bool WaterCoilIsOnWaterCoilSystem = false;
    std::string CoilSystemName = CompName;
    CompType CoilSystemTypeNum = CompTypeNum;

    if (state.dataHVACAssistedCC->TotalNumHXAssistedCoils > 0) {
        // check if the water coil is placed on 'CoilSystem:Cooling:Water:HeatExchangerAssisted' object
        for (int HXASSCoilNum = 1; HXASSCoilNum <= state.dataHVACAssistedCC->TotalNumHXAssistedCoils; ++HXASSCoilNum) {
            std::string CompType = state.dataHVACAssistedCC->HXAssistedCoil(HXASSCoilNum).CoolingCoilType;
            if ((Util::SameString(CompType, "Coil:Cooling:Water") || Util::SameString(CompType, "Coil:Cooling:Water:DetailedGeometry")) &&
                Util::SameString(CompName, state.dataHVACAssistedCC->HXAssistedCoil(HXASSCoilNum).CoolingCoilName)) {
                CoilSystemName = state.dataHVACAssistedCC->HXAssistedCoil(HXASSCoilNum).Name;
                CoilSystemTypeNum = SimAirServingZones::CompType::WaterCoil_CoolingHXAsst;
                WaterCoilIsOnWaterCoilSystem = true;
                break;
            }
        }
    }

    // check if the CoilSystem object that contains the water coil is placed on air loop branch or OA system
    if (WaterCoilIsOnWaterCoilSystem) {
        CheckWaterCoilSystemIsOnAirLoopOASystem = CheckWaterCoilOnPrimaryAirLoopBranch(state, CoilSystemTypeNum, CoilSystemName);
        if (!CheckWaterCoilSystemIsOnAirLoopOASystem) {
            CheckWaterCoilSystemIsOnAirLoopOASystem = CheckWaterCoilOnOASystem(state, CoilSystemTypeNum, CoilSystemName);
        }
    }
    return CheckWaterCoilSystemIsOnAirLoopOASystem;
}
// namespace SimAirServingZones

// End Algorithm Section of the Module
// *****************************************************************************

// Beginning of Reporting subroutines for the SimAir Module
// *****************************************************************************

//        End of Reporting subroutines for the SimAir Module
// *****************************************************************************

//        Utility Subroutines for the SimAir Module
// *****************************************************************************

void LimitZoneVentEff(EnergyPlusData &state,
                      Real64 Xs,               // ratio of uncorrected system outdoor air flow rate to the design system supply flow rate
                      Real64 Voz,              // corrected (divided by distribution efficiency) zone outside air flow rate [m3/s]
                      int TermUnitSizingIndex, // terminal unit sizing index
                      Real64 &SystemCoolingEv  // system ventilation efficiency
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   November 2015

    // PURPOSE OF THIS FUNCTION:
    // Check that system ventilation eff is not less than input minimum system ventilation efficiency.
    // If it is, back calculate and reset ZpzClgByZone and DesCoolVolFlowMin and system ventilation efficiency
    // Also increase DesCoolVolFlow if needed to match the new DesCoolVolFlowMin
    // Why does this look only at cooling?  Shouldn't heating also be checked?

    // METHODOLOGY EMPLOYED:
    // Ventilation Rate Procedure for single pass system

    auto &TUFinalZoneSizing = state.dataSize->TermUnitFinalZoneSizing(TermUnitSizingIndex);

    if (SystemCoolingEv < TUFinalZoneSizing.ZoneVentilationEff) {
        Real64 ZoneOAFrac = 1.0 + Xs - TUFinalZoneSizing.ZoneVentilationEff; // ratio of Voz to available zone supply air flow
        Real64 AvailSAFlow = Voz / ZoneOAFrac;             // reset AvailSAFlow (which in this case is minimum cooling supply air flow rate)
        TUFinalZoneSizing.ZpzClgByZone = ZoneOAFrac;       // save ZoneOAFrac
        TUFinalZoneSizing.DesCoolVolFlowMin = AvailSAFlow; // save new (increased) minimum flow rate
        TUFinalZoneSizing.DesCoolVolFlow = max(AvailSAFlow, TUFinalZoneSizing.DesCoolVolFlow); // make sure max flow is >= the new minimum flow rate
        SystemCoolingEv = TUFinalZoneSizing.ZoneVentilationEff; // set the system ventilation efficiency to the user specified minimum

        // Vpz: "Primary" supply air from main air handler served by an oa mixer
        Real64 VpzClgByZone = TUFinalZoneSizing.DesCoolVolFlow;

        // Vdz: "Discharge" supply air delivered to zone by terminal unit
        Real64 VdzClgByZone = 0.0;
        // Taken from similar section in SetUpSysSizingArrays
        if (TUFinalZoneSizing.ZoneSecondaryRecirculation > 0.0) { // multi-path system
            VdzClgByZone = max(state.dataSize->TermUnitSizing(TermUnitSizingIndex).AirVolFlow, VpzClgByZone);
        } else { // single path system
            VdzClgByZone = TUFinalZoneSizing.DesCoolVolFlow;
        }

        // Update VRP table entries:
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchS62zcdVpz, TUFinalZoneSizing.ZoneName, VpzClgByZone, 4);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchS62zcdVdz, TUFinalZoneSizing.ZoneName, VdzClgByZone, 4);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchS62zcdVpzmin, TUFinalZoneSizing.ZoneName, TUFinalZoneSizing.DesCoolVolFlowMin, 4);
        // Zpz = Voz/Vpz
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchS62zcdZpz, TUFinalZoneSizing.ZoneName, TUFinalZoneSizing.ZpzClgByZone, 3);
    }
}

//        End of Utility subroutines for the SimAir Module
// *****************************************************************************

} // namespace EnergyPlus::SimAirServingZones
