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

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace OutAirNodeManager {
    // Module containing the routines that deal with the outside air nodes

    // MODULE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   September 1998
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and update the conditions for all the outside
    // air nodes in the problem.

    // METHODOLOGY EMPLOYED:
    // Outside air nodes provide the connection to outside conditions for the
    // EnergyPlus HVAC simulation. The  list of air nodes specified in the input
    // file will be read in. Each air node will be updated to the outside environmental
    // conditions at the start of each EnergyPlus main time step.

    // REFERENCES:

    // OTHER NOTES:

    // USE STATEMENTS:
    // Use statements for data only modules
    // Using/Aliasing
    using namespace DataLoopNode;
    using namespace DataEnvironment;

    void SetOutAirNodes(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   September 1998
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Make sure the outside air nodes are prepared for the HVAC simulation

        // METHODOLOGY EMPLOYED:
        // Use appropriate flag to check for needed action

        if (state.dataOutAirNodeMgr->GetOutAirNodesInputFlag) { // First time subroutine has been entered
            GetOutAirNodesInput(state);                         // Get OutAir Nodes data
            state.dataOutAirNodeMgr->GetOutAirNodesInputFlag = false;
        }
        InitOutAirNodes(state);
    }

    void GetOutAirNodesInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   September 1998
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE
        // Read in the list of outside air nodes & store in array OutAirInletNodeList

        // METHODOLOGY EMPLOYED:
        // Use the Get routines from the InputProcessor module.

        // Using/Aliasing
        using namespace NodeInputManager;

        // Locals
        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetOutAirNodesInput: "); // include trailing blank space
        static constexpr std::string_view routineName = "GetOutAirNodesInput";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumOutAirInletNodeLists;
        int NumOutsideAirNodeSingles;
        int NumNums;   // Number of REAL(r64) numbers returned by GetObjectItem
        int NumAlphas; // Number of alphanumerics returned by GetObjectItem
        int NumParams;
        Array1D_int NodeNums;
        int NumNodes;
        int IOStat;  // Status flag from GetObjectItem
        int NodeNum; // index into NodeNums
        //  INTEGER :: OutAirNodeNum ! index into OutAirInletNodeList
        int OutAirInletNodeListNum;  // OUTSIDE AIR INLET NODE LIST index
        int OutsideAirNodeSingleNum; // OUTSIDE AIR NODE index
        int AlphaNum;                // index into Alphas
        std::size_t ListSize;        // size of OutAirInletNodeList
        //  LOGICAL :: AlreadyInList ! flag used for checking for duplicate input
        bool ErrorsFound;
        bool ErrInList;
        std::size_t CurSize;
        int NextFluidStreamNum; // Fluid stream index (all outside air inlet nodes need a unique fluid stream number)
        Array1D_int TmpNums;
        std::string CurrentModuleObject; // Object type for getting and error messages
        Array1D_string Alphas;           // Alpha input items for object
        Array1D_string cAlphaFields;     // Alpha field names
        Array1D_string cNumericFields;   // Numeric field names
        Array1D<Real64> Numbers;         // Numeric input items for object
        Array1D_bool lAlphaBlanks;       // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;     // Logical array, numeric field input BLANK = .TRUE.
        int MaxNums(0);                  // Maximum number of numeric input fields
        int MaxAlphas(0);                // Maximum number of alpha input fields
        int TotalArgs(0);                // Total number of alpha and numeric arguments (max) for a

        NumOutAirInletNodeLists = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "OutdoorAir:NodeList");
        NumOutsideAirNodeSingles = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "OutdoorAir:Node");
        state.dataOutAirNodeMgr->NumOutsideAirNodes = 0;
        ErrorsFound = false;
        NextFluidStreamNum = 1;

        ListSize = 0;
        CurSize = 100;
        TmpNums.dimension(CurSize, 0);

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "NodeList", NumParams, NumAlphas, NumNums);
        NodeNums.dimension(NumParams, 0);

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "OutdoorAir:NodeList", TotalArgs, NumAlphas, NumNums);
        MaxNums = max(MaxNums, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "OutdoorAir:Node", TotalArgs, NumAlphas, NumNums);
        MaxNums = max(MaxNums, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        Alphas.allocate(MaxAlphas);
        cAlphaFields.allocate(MaxAlphas);
        cNumericFields.allocate(MaxNums);
        Numbers.dimension(MaxNums, 0.0);
        lAlphaBlanks.dimension(MaxAlphas, true);
        lNumericBlanks.dimension(MaxNums, true);

        if (NumOutAirInletNodeLists > 0) {
            // Loop over all outside air inlet nodes in the input and count them
            CurrentModuleObject = "OutdoorAir:NodeList";
            for (OutAirInletNodeListNum = 1; OutAirInletNodeListNum <= NumOutAirInletNodeLists; ++OutAirInletNodeListNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         OutAirInletNodeListNum,
                                                                         Alphas,
                                                                         NumAlphas,
                                                                         Numbers,
                                                                         NumNums,
                                                                         IOStat,
                                                                         lNumericBlanks,
                                                                         lAlphaBlanks,
                                                                         cAlphaFields,
                                                                         cNumericFields);

                for (AlphaNum = 1; AlphaNum <= NumAlphas; ++AlphaNum) {
                    ErrInList = false;
                    //  To support HVAC diagram, every outside inlet node must have a unique fluid stream number
                    //  GetNodeNums will increment the value across a node list, the starting value must be incremented
                    //  here across lists and across objects
                    GetNodeNums(state,
                                Alphas(AlphaNum),
                                NumNodes,
                                NodeNums,
                                ErrInList,
                                DataLoopNode::NodeFluidType::Air,
                                DataLoopNode::ConnectionObjectType::OutdoorAirNodeList,
                                CurrentModuleObject,
                                DataLoopNode::ConnectionType::OutsideAir,
                                static_cast<NodeInputManager::CompFluidStream>(NextFluidStreamNum),
                                ObjectIsNotParent,
                                IncrementFluidStreamYes,
                                cAlphaFields(AlphaNum));
                    NextFluidStreamNum += NumNodes;
                    if (ErrInList) {
                        ShowContinueError(state, format("Occurred in {}, {} = {}", CurrentModuleObject, cAlphaFields(AlphaNum), Alphas(AlphaNum)));
                        ErrorsFound = true;
                    }
                    for (NodeNum = 1; NodeNum <= NumNodes; ++NodeNum) {
                        // Duplicates here are not a problem, just ignore
                        if (!any_eq(TmpNums, NodeNums(NodeNum))) {
                            ++ListSize;
                            if (ListSize > CurSize) {
                                TmpNums.redimension(CurSize += 100, 0);
                            }
                            TmpNums(ListSize) = NodeNums(NodeNum);
                        }
                    }
                }
            }

            if (ErrorsFound) {
                ShowFatalError(state, format("{}Errors found in getting {} input.", RoutineName, CurrentModuleObject));
            }
        }

        if (NumOutsideAirNodeSingles > 0) {
            // Loop over all single outside air nodes in the input
            CurrentModuleObject = "OutdoorAir:Node";
            for (OutsideAirNodeSingleNum = 1; OutsideAirNodeSingleNum <= NumOutsideAirNodeSingles; ++OutsideAirNodeSingleNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         OutsideAirNodeSingleNum,
                                                                         Alphas,
                                                                         NumAlphas,
                                                                         Numbers,
                                                                         NumNums,
                                                                         IOStat,
                                                                         lNumericBlanks,
                                                                         lAlphaBlanks,
                                                                         cAlphaFields,
                                                                         cNumericFields);

                ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

                ErrInList = false;
                //  To support HVAC diagram, every outside inlet node must have a unique fluid stream number
                //  GetNodeNums will increment the value across a node list, the starting value must be incremented
                //  here across lists and across objects
                GetNodeNums(state,
                            Alphas(1),
                            NumNodes,
                            NodeNums,
                            ErrInList,
                            DataLoopNode::NodeFluidType::Air,
                            DataLoopNode::ConnectionObjectType::OutdoorAirNode,
                            CurrentModuleObject,
                            DataLoopNode::ConnectionType::OutsideAir,
                            static_cast<NodeInputManager::CompFluidStream>(NextFluidStreamNum),
                            ObjectIsNotParent,
                            IncrementFluidStreamYes,
                            cAlphaFields(1));
                NextFluidStreamNum += NumNodes;
                if (ErrInList) {
                    ShowContinueError(state, format("Occurred in {}, {} = {}", CurrentModuleObject, cAlphaFields(1), Alphas(1)));
                    ErrorsFound = true;
                }

                if (NumNodes > 1) {
                    ShowSevereError(state, format("{}, {} = {}", CurrentModuleObject, cAlphaFields(1), Alphas(1)));
                    ShowContinueError(state, "...appears to point to a node list, not a single node.");
                    ErrorsFound = true;
                    continue;
                }

                if (!any_eq(TmpNums, NodeNums(1))) {
                    ++ListSize;
                    if (ListSize > CurSize) {
                        TmpNums.redimension(CurSize += 100, 0);
                    }
                    TmpNums(ListSize) = NodeNums(1);
                } else { // Duplicates are a problem
                    ShowSevereError(state, format("{}, duplicate {} = {}", CurrentModuleObject, cAlphaFields(1), Alphas(1)));
                    ShowContinueError(state, format("Duplicate {} might be found in an OutdoorAir:NodeList.", cAlphaFields(1)));
                    ErrorsFound = true;
                    continue;
                }

                // Set additional node properties
                if (NumNums > 0) {
                    state.dataLoopNodes->Node(NodeNums(1)).Height = Numbers(1);
                }

                if (NumAlphas > 1) {
                    state.dataGlobal->AnyLocalEnvironmentsInModel = true;
                }

                if (NumAlphas <= 1 || lAlphaBlanks(2)) {
                } else if ((state.dataLoopNodes->Node(NodeNums(1)).outAirDryBulbSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                    ErrorsFound = true;
                }

                if (NumAlphas <= 2 || lAlphaBlanks(3)) {
                } else if ((state.dataLoopNodes->Node(NodeNums(1)).outAirWetBulbSched = Sched::GetSchedule(state, Alphas(3))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(3), Alphas(3));
                    ErrorsFound = true;
                }

                if (NumAlphas <= 3 || lAlphaBlanks(4)) {
                } else if ((state.dataLoopNodes->Node(NodeNums(1)).outAirWindSpeedSched = Sched::GetSchedule(state, Alphas(4))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(4), Alphas(4));
                    ErrorsFound = true;
                }

                if (NumAlphas <= 4 || lAlphaBlanks(5)) {
                } else if ((state.dataLoopNodes->Node(NodeNums(1)).outAirWindDirSched = Sched::GetSchedule(state, Alphas(5))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(5), Alphas(5));
                    ErrorsFound = true;
                }

                if (NumAlphas > 8) {
                    ShowSevereError(state, format("{}, {} = {}", CurrentModuleObject, cAlphaFields(1), Alphas(1)));
                    ShowContinueError(state, "Object Definition indicates more than 7 Alpha Objects.");
                    ErrorsFound = true;
                    continue;
                }
                if (state.dataLoopNodes->Node(NodeNums(1)).outAirDryBulbSched != nullptr ||
                    state.dataLoopNodes->Node(NodeNums(1)).outAirWetBulbSched != nullptr) {
                    state.dataLoopNodes->Node(NodeNums(1)).IsLocalNode = true;
                }
            }
            if (ErrorsFound) {
                ShowFatalError(state, format("{}Errors found in getting {} input.", RoutineName, CurrentModuleObject));
            }
        }

        if (ListSize > 0) {
            state.dataOutAirNodeMgr->NumOutsideAirNodes = ListSize;
            state.dataOutAirNodeMgr->OutsideAirNodeList = TmpNums({1, static_cast<int>(ListSize)});
        }
    }

    void InitOutAirNodes(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   Sept 1998
        //       MODIFIED       B. Griffith, added EMS override
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Initialize the outside air node data data.  In Particular,
        // set the outside air nodes to the outside conditions at the
        // start of every heat balance time step.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int OutsideAirNodeNum;
        int NodeNum;

        // Do the begin time step initialization
        for (OutsideAirNodeNum = 1; OutsideAirNodeNum <= state.dataOutAirNodeMgr->NumOutsideAirNodes; ++OutsideAirNodeNum) {
            NodeNum = state.dataOutAirNodeMgr->OutsideAirNodeList(OutsideAirNodeNum);
            SetOANodeValues(state, NodeNum, true);
        }
    }

    bool CheckOutAirNodeNumber(EnergyPlusData &state, int const NodeNumber) // Number of node to check to see if in Outside Air list
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   Feb 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Provide an entry into the OutAirNode List for checking from other routines.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Return value
        bool Okay; // True if found, false if not

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        // na

        if (state.dataOutAirNodeMgr->GetOutAirNodesInputFlag) { // First time subroutine has been entered
            GetOutAirNodesInput(state);                         // Get Out Air Nodes data
            state.dataOutAirNodeMgr->GetOutAirNodesInputFlag = false;
            SetOutAirNodes(state);
        }

        if (any_eq(state.dataOutAirNodeMgr->OutsideAirNodeList, NodeNumber)) {
            Okay = true;
        } else {
            Okay = false;
        }

        return Okay;
    }

    void CheckAndAddAirNodeNumber(EnergyPlusData &state,
                                  int const NodeNumber, // Number of node to check to see if in Outside Air list
                                  bool &Okay            // True if found, false if not
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   March 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // At the time of writing, some items (namely Chillers) have "made up" node
        // names for nodes that are "outside air nodes".  Rather than fatal out, add
        // this subroutine which will check and then add a outside air node, if necessary.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace NodeInputManager;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array1D_int TmpNums;
        int DummyNumber;

        if (state.dataOutAirNodeMgr->GetOutAirNodesInputFlag) { // First time subroutine has been entered
            GetOutAirNodesInput(state);                         // Get Out Air Nodes data
            state.dataOutAirNodeMgr->GetOutAirNodesInputFlag = false;
            SetOutAirNodes(state);
        }

        Okay = false;

        if (state.dataOutAirNodeMgr->NumOutsideAirNodes > 0) {
            if (any_eq(state.dataOutAirNodeMgr->OutsideAirNodeList, NodeNumber)) {
                Okay = true;
            } else {
                Okay = false;
            }
        } else {
            Okay = false;
        }

        if (NodeNumber > 0) {
            if (!Okay) { // Add new outside air node to list
                state.dataOutAirNodeMgr->OutsideAirNodeList.redimension(++state.dataOutAirNodeMgr->NumOutsideAirNodes);
                state.dataOutAirNodeMgr->OutsideAirNodeList(state.dataOutAirNodeMgr->NumOutsideAirNodes) = NodeNumber;
                TmpNums = state.dataOutAirNodeMgr->OutsideAirNodeList;
                bool errFlag(false);
                // register new node..
                GetNodeNums(state,
                            state.dataLoopNodes->NodeID(NodeNumber),
                            DummyNumber,
                            TmpNums,
                            errFlag,
                            DataLoopNode::NodeFluidType::Air,
                            DataLoopNode::ConnectionObjectType::OutdoorAirNode,
                            "OutdoorAir:Node",
                            DataLoopNode::ConnectionType::OutsideAir,
                            static_cast<NodeInputManager::CompFluidStream>(state.dataOutAirNodeMgr->NumOutsideAirNodes),
                            ObjectIsNotParent,
                            IncrementFluidStreamYes);
                SetOANodeValues(state, NodeNumber, false);
            }
        }
    }

    void SetOANodeValues(EnergyPlusData &state,
                         int const NodeNum, // Number of node to check to see if in Outside Air list
                         bool InitCall      // True if Init calls, false if CheckAndAddAirNodeNumber calls
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         L. Gu
        //       DATE WRITTEN   July 2018

        // PURPOSE OF THIS SUBROUTINE:
        // Consolidate a block from both CheckAndAddAirNodeNumber and InitOutAirNodes to set
        // up outdoor node values

        using Psychrometrics::PsyHFnTdbW;
        using Psychrometrics::PsyTwbFnTdbWPb;
        using Psychrometrics::PsyWFnTdbTwbPb;

        // Set node data to global values
        if (state.dataLoopNodes->Node(NodeNum).Height < 0.0) {
            // Note -- this setting is different than the DataEnvironment "AT" settings.
            state.dataLoopNodes->Node(NodeNum).OutAirDryBulb = state.dataEnvrn->OutDryBulbTemp;
            state.dataLoopNodes->Node(NodeNum).OutAirWetBulb = state.dataEnvrn->OutWetBulbTemp;
            if (InitCall) {
                state.dataLoopNodes->Node(NodeNum).OutAirWindSpeed = state.dataEnvrn->WindSpeed;
            }
        } else {
            state.dataLoopNodes->Node(NodeNum).OutAirDryBulb = OutDryBulbTempAt(state, state.dataLoopNodes->Node(NodeNum).Height);
            state.dataLoopNodes->Node(NodeNum).OutAirWetBulb = OutWetBulbTempAt(state, state.dataLoopNodes->Node(NodeNum).Height);
            if (InitCall) {
                state.dataLoopNodes->Node(NodeNum).OutAirWindSpeed = DataEnvironment::WindSpeedAt(state, state.dataLoopNodes->Node(NodeNum).Height);
            }
        }
        if (!InitCall) {
            state.dataLoopNodes->Node(NodeNum).OutAirWindSpeed = state.dataEnvrn->WindSpeed;
        }
        state.dataLoopNodes->Node(NodeNum).OutAirWindDir = state.dataEnvrn->WindDir;

        if (InitCall) {
            // Set node data to local air node values if defined
            if (state.dataLoopNodes->Node(NodeNum).outAirDryBulbSched != nullptr) {
                state.dataLoopNodes->Node(NodeNum).OutAirDryBulb = state.dataLoopNodes->Node(NodeNum).outAirDryBulbSched->getCurrentVal();
            }
            if (state.dataLoopNodes->Node(NodeNum).outAirWetBulbSched != nullptr) {
                state.dataLoopNodes->Node(NodeNum).OutAirWetBulb = state.dataLoopNodes->Node(NodeNum).outAirWetBulbSched->getCurrentVal();
            }
            if (state.dataLoopNodes->Node(NodeNum).outAirWindSpeedSched != nullptr) {
                state.dataLoopNodes->Node(NodeNum).OutAirWindSpeed = state.dataLoopNodes->Node(NodeNum).outAirWindSpeedSched->getCurrentVal();
            }
            if (state.dataLoopNodes->Node(NodeNum).outAirWindDirSched != nullptr) {
                state.dataLoopNodes->Node(NodeNum).OutAirWindDir = state.dataLoopNodes->Node(NodeNum).outAirWindDirSched->getCurrentVal();
            }

            // Set node data to EMS overwritten values if defined
            if (state.dataLoopNodes->Node(NodeNum).EMSOverrideOutAirDryBulb) {
                state.dataLoopNodes->Node(NodeNum).OutAirDryBulb = state.dataLoopNodes->Node(NodeNum).EMSValueForOutAirDryBulb;
            }
            if (state.dataLoopNodes->Node(NodeNum).EMSOverrideOutAirWetBulb) {
                state.dataLoopNodes->Node(NodeNum).OutAirWetBulb = state.dataLoopNodes->Node(NodeNum).EMSValueForOutAirWetBulb;
            }
            if (state.dataLoopNodes->Node(NodeNum).EMSOverrideOutAirWindSpeed) {
                state.dataLoopNodes->Node(NodeNum).OutAirWindSpeed = state.dataLoopNodes->Node(NodeNum).EMSValueForOutAirWindSpeed;
            }
            if (state.dataLoopNodes->Node(NodeNum).EMSOverrideOutAirWindDir) {
                state.dataLoopNodes->Node(NodeNum).OutAirWindDir = state.dataLoopNodes->Node(NodeNum).EMSValueForOutAirWindDir;
            }
        }

        state.dataLoopNodes->Node(NodeNum).Temp = state.dataLoopNodes->Node(NodeNum).OutAirDryBulb;
        if (state.dataLoopNodes->Node(NodeNum).IsLocalNode) {
            if (InitCall) {
                if (state.dataLoopNodes->Node(NodeNum).OutAirWetBulb > state.dataLoopNodes->Node(NodeNum).OutAirDryBulb) {
                    state.dataLoopNodes->Node(NodeNum).OutAirWetBulb = state.dataLoopNodes->Node(NodeNum).OutAirDryBulb;
                }
                if (state.dataLoopNodes->Node(NodeNum).outAirWetBulbSched == nullptr &&
                    !state.dataLoopNodes->Node(NodeNum).EMSOverrideOutAirWetBulb &&
                    (state.dataLoopNodes->Node(NodeNum).EMSOverrideOutAirDryBulb ||
                     state.dataLoopNodes->Node(NodeNum).outAirDryBulbSched != nullptr)) {
                    state.dataLoopNodes->Node(NodeNum).HumRat = state.dataEnvrn->OutHumRat;
                    state.dataLoopNodes->Node(NodeNum).OutAirWetBulb = PsyTwbFnTdbWPb(
                        state, state.dataLoopNodes->Node(NodeNum).OutAirDryBulb, state.dataEnvrn->OutHumRat, state.dataEnvrn->OutBaroPress);
                } else {
                    state.dataLoopNodes->Node(NodeNum).HumRat = PsyWFnTdbTwbPb(state,
                                                                               state.dataLoopNodes->Node(NodeNum).OutAirDryBulb,
                                                                               state.dataLoopNodes->Node(NodeNum).OutAirWetBulb,
                                                                               state.dataEnvrn->OutBaroPress);
                }
            } else {
                state.dataLoopNodes->Node(NodeNum).HumRat = PsyWFnTdbTwbPb(state,
                                                                           state.dataLoopNodes->Node(NodeNum).OutAirDryBulb,
                                                                           state.dataLoopNodes->Node(NodeNum).OutAirWetBulb,
                                                                           state.dataEnvrn->OutBaroPress);
            }
        } else {
            state.dataLoopNodes->Node(NodeNum).HumRat = state.dataEnvrn->OutHumRat;
        }
        state.dataLoopNodes->Node(NodeNum).Enthalpy =
            PsyHFnTdbW(state.dataLoopNodes->Node(NodeNum).OutAirDryBulb, state.dataLoopNodes->Node(NodeNum).HumRat);
        state.dataLoopNodes->Node(NodeNum).Press = state.dataEnvrn->OutBaroPress;
        state.dataLoopNodes->Node(NodeNum).Quality = 0.0;
        // Add contaminants
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            state.dataLoopNodes->Node(NodeNum).CO2 = state.dataContaminantBalance->OutdoorCO2;
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            state.dataLoopNodes->Node(NodeNum).GenContam = state.dataContaminantBalance->OutdoorGC;
        }
    }

} // namespace OutAirNodeManager

} // namespace EnergyPlus
