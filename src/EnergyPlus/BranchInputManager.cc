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
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchInputManager.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace BranchInputManager {

    // Module containing the routines dealing with the BRANCH and CONNECTOR
    // lists input.

    // MODULE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 1999
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To Get the IDD objects "BranchList", "Branch", "ConnectorList",
    // "Connector:Splitter", and "Connector:Mixer".  Also, to supply other modules/routines with
    // information about these objects.

    // Using/Aliasing
    using namespace DataLoopNode;
    using namespace DataBranchAirLoopPlant;
    using namespace NodeInputManager;
    using namespace BranchNodeConnections;

    // MODULE PARAMETER DEFINITIONS
    const char *cMIXER("Connector:Mixer");
    const char *cSPLITTER("Connector:Splitter");

    void ManageBranchInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   Nov 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is called from HVACManager to make sure that branch input is
        // gathered prior to need.

        if (state.dataBranchInputManager->GetBranchInputFlag) {
            GetBranchInput(state);
            if (state.dataBranchInputManager->GetBranchListInputFlag) {
                state.dataBranchInputManager->GetBranchListInputFlag = false;
                GetBranchListInput(state);
            }
            AuditBranches(state, false);
            state.dataBranchInputManager->GetBranchInputFlag = false;
        }
    }

    //==================================================================================
    //   Routines that "get" data from internal branch management structure
    //==================================================================================

    void GetBranchList(EnergyPlusData &state,
                       std::string const &LoopName,       // Name of Loop Branch List is on
                       std::string const &BranchListName, // Branch List Name from Input
                       int &NumBranchNames,               // Number of Branches for this Branch List
                       Array1D_string &BranchNames,       // Names of Branches on this Branch List
                       std::string const &LoopType        // Type of Loop Branch list is on
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   October 1999
        //       MODIFIED       October 2001, Automatic Extensibility
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine "gets" the branch list specified in a Plant or Condenser loop and
        // returns number and names to the outside calling routine.

        // Using/Aliasing

        int Found;     // Points to correct Branch List/Branch
        bool ErrFound; // True when error has occurred (cannot find Branch List)

        ErrFound = false;

        if (state.dataBranchInputManager->GetBranchListInputFlag) {
            state.dataBranchInputManager->GetBranchListInputFlag = false;
            GetBranchListInput(state);
        }

        //  Find this BranchList in the master BranchList Names
        Found = Util::FindItemInList(BranchListName, state.dataBranchInputManager->BranchList);
        if (Found == 0) {
            ShowFatalError(state, format("GetBranchList: BranchList Name not found={}", BranchListName));
        }

        // Set data
        if (state.dataBranchInputManager->BranchList(Found).LoopName.empty()) {
            state.dataBranchInputManager->BranchList(Found).LoopName = LoopName;
            state.dataBranchInputManager->BranchList(Found).LoopType = LoopType;
        } else if (state.dataBranchInputManager->BranchList(Found).LoopName != LoopName) {
            ShowSevereError(state, "GetBranchList: BranchList Loop Name already assigned");
            ShowContinueError(state,
                              format("BranchList={}, already assigned to loop={}",
                                     state.dataBranchInputManager->BranchList(Found).Name,
                                     state.dataBranchInputManager->BranchList(Found).LoopName));
            ShowContinueError(state, format("Now requesting assignment to Loop={}", LoopName));
            ErrFound = true;
        }

        // Return data
        NumBranchNames = state.dataBranchInputManager->BranchList(Found).NumOfBranchNames;
        if (isize(BranchNames) < NumBranchNames) {
            ShowSevereError(state, "GetBranchList: Branch Names array not big enough to hold Branch Names");
            ShowContinueError(state, format("Input BranchListName={}, in Loop={}", BranchListName, LoopName));
            ShowContinueError(state, fmt::format("BranchName Array size={}, but input size={}", size(BranchNames), (NumBranchNames)));
            ErrFound = true;
        } else {
            BranchNames = "";
            BranchNames({1, NumBranchNames}) = state.dataBranchInputManager->BranchList(Found).BranchNames({1, NumBranchNames});
        }

        if (ErrFound) {
            ShowFatalError(state, "GetBranchList: preceding condition(s) causes program termination.");
        }
    }

    int NumBranchesInBranchList(EnergyPlusData &state, std::string const &BranchListName)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   July 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function returns the number of branches in a branch list so that the calling
        // routine can allocate arrays before calling GetBranchList.

        // Return value
        int NumBranchesInBranchList;

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int Found;

        if (state.dataBranchInputManager->GetBranchListInputFlag) {
            state.dataBranchInputManager->GetBranchListInputFlag = false;
            GetBranchListInput(state);
        }

        //  Find this BranchList in the master BranchList Names
        Found = Util::FindItemInList(BranchListName, state.dataBranchInputManager->BranchList);
        if (Found == 0) {
            ShowFatalError(state, format("NumBranchesInBranchList: BranchList Name not found={}", BranchListName));
        }

        NumBranchesInBranchList = state.dataBranchInputManager->BranchList(Found).NumOfBranchNames;

        return NumBranchesInBranchList;
    }

    void GetBranchData(EnergyPlusData &state,
                       std::string const &LoopName,                               // Loop Name of this Branch
                       std::string const &BranchName,                             // Requested Branch Name
                       DataBranchAirLoopPlant::PressureCurveType &PressCurveType, // Index of a pressure curve object
                       int &PressCurveIndex,                                      // Index of a pressure curve object
                       int &NumComps,                                             // Number of Components on Branch
                       Array1D_string &CompType,                                  // Component Type for each item on Branch
                       Array1D_string &CompName,                                  // Component Name for each item on Branch
                       Array1D_string &CompInletNodeNames,                        // Component Inlet Node IDs for each item on Branch
                       Array1D_int &CompInletNodeNums,                            // Component Inlet Node Numbers for each item on Branch
                       Array1D_string &CompOutletNodeNames,                       // Component Outlet Node IDs for each item on Branch
                       Array1D_int &CompOutletNodeNums,                           // Component Outlet Node Numbers for each item on Branch
                       bool &ErrorsFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   October 1999
        //       MODIFIED       October 2001, Automatic Extensibility
        //                      September 2012, B. Griffith, removed component control types
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine gets the Branch Data (internal structure) for the requested
        // Branch Name and returns it in "list structure" to the calling routine.

        // Using/Aliasing

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Count; // Loop Counter
        int MinCompsAllowed;

        // NumComps now defined on input

        state.dataBranchInputManager->BComponents.allocate(NumComps);

        GetInternalBranchData(
            state, LoopName, BranchName, PressCurveType, PressCurveIndex, NumComps, state.dataBranchInputManager->BComponents, ErrorsFound);

        MinCompsAllowed = min(
            size(CompType), size(CompName), size(CompInletNodeNames), size(CompInletNodeNums), size(CompOutletNodeNames), size(CompOutletNodeNums));
        if (MinCompsAllowed < NumComps) {
            ShowSevereError(state, "GetBranchData: Component List arrays not big enough to hold Number of Components");
            ShowContinueError(state, format("Input BranchName={}, in Loop={}", BranchName, LoopName));
            ShowContinueError(state, fmt::format("Max Component Array size={}, but input size={}", MinCompsAllowed, NumComps));
            ShowFatalError(state, "Program terminates due to preceding conditions.");
        }

        for (Count = 1; Count <= NumComps; ++Count) {
            CompType(Count) = state.dataBranchInputManager->BComponents(Count).CType;
            CompName(Count) = state.dataBranchInputManager->BComponents(Count).Name;
            CompInletNodeNames(Count) = state.dataBranchInputManager->BComponents(Count).InletNodeName;
            CompInletNodeNums(Count) = state.dataBranchInputManager->BComponents(Count).InletNode;
            CompOutletNodeNames(Count) = state.dataBranchInputManager->BComponents(Count).OutletNodeName;
            CompOutletNodeNums(Count) = state.dataBranchInputManager->BComponents(Count).OutletNode;
        }
        state.dataBranchInputManager->BComponents.deallocate();
    }

    int NumCompsInBranch(EnergyPlusData &state, std::string const &BranchName)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   July 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function returns the number of components in a branch so that the calling
        // routine can allocate arrays before calling GetBranchData.

        // Return value
        int NumCompsInBranch;

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int Found;

        if (state.dataBranchInputManager->GetBranchInputFlag) {
            state.dataBranchInputManager->GetBranchInputFlag = false;
            GetBranchInput(state);
        }

        Found = Util::FindItemInList(BranchName, state.dataBranchInputManager->Branch);
        if (Found == 0) {
            ShowSevereError(state, format("NumCompsInBranch:  Branch not found={}", BranchName));
            NumCompsInBranch = 0;
        } else {
            NumCompsInBranch = state.dataBranchInputManager->Branch(Found).NumOfComponents;
        }

        return NumCompsInBranch;
    }

    int GetAirBranchIndex(EnergyPlusData &state, std::string const &CompType, std::string_view CompName)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   April 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function returns the branch index so that the calling
        // routine can search for a fan on this branch or use branch flow for sizing.

        // Return value
        int GetAirBranchIndex(0);

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int BranchNum;
        int CompNum;
        int NumBranches;

        if (state.dataBranchInputManager->GetBranchInputFlag) {
            state.dataBranchInputManager->GetBranchInputFlag = false;
            GetBranchInput(state);
        }

        NumBranches = size(state.dataBranchInputManager->Branch);

        if (NumBranches == 0) {
            ShowSevereError(state, format("GetAirBranchIndex:  Branch not found with component = {} \"{}\"", CompType, CompName));
        } else {
            for (BranchNum = 1; BranchNum <= NumBranches; ++BranchNum) {
                for (CompNum = 1; CompNum <= state.dataBranchInputManager->Branch(BranchNum).NumOfComponents; ++CompNum) {
                    if (Util::SameString(CompType, state.dataBranchInputManager->Branch(BranchNum).Component(CompNum).CType) &&
                        Util::SameString(CompName, state.dataBranchInputManager->Branch(BranchNum).Component(CompNum).Name)) {
                        GetAirBranchIndex = BranchNum;
                        goto BranchLoop_exit;
                    }
                }
            }
        BranchLoop_exit:;
        }

        return GetAirBranchIndex;
    }

    void GetBranchFanTypeName(EnergyPlusData &state,
                              int const BranchNum,
                              std::string &FanType,
                              std::string &FanName,
                              bool &ErrFound // Set to true if error found, false otherwise
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   April 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function returns the branch fan flow rate so that the calling
        // routine can either use this flow or use then branch flow for sizing.

        // Using/Aliasing

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int CompNum;
        int NumBranches;

        if (state.dataBranchInputManager->GetBranchInputFlag) {
            state.dataBranchInputManager->GetBranchInputFlag = false;
            GetBranchInput(state);
        }

        ErrFound = false;
        NumBranches = size(state.dataBranchInputManager->Branch);

        FanType = std::string();
        FanName = std::string();

        if (NumBranches == 0) {
            ShowSevereError(state, fmt::format("GetBranchFanTypeName:  Branch index not found = {}", BranchNum));
            ErrFound = true;
        } else {
            if (BranchNum > 0 && BranchNum <= NumBranches) {
                for (CompNum = 1; CompNum <= state.dataBranchInputManager->Branch(BranchNum).NumOfComponents; ++CompNum) {
                    if (Util::SameString("Fan:OnOff", state.dataBranchInputManager->Branch(BranchNum).Component(CompNum).CType) ||
                        Util::SameString("Fan:ConstantVolume", state.dataBranchInputManager->Branch(BranchNum).Component(CompNum).CType) ||
                        Util::SameString("Fan:VariableVolume", state.dataBranchInputManager->Branch(BranchNum).Component(CompNum).CType) ||
                        Util::SameString("Fan:SystemModel", state.dataBranchInputManager->Branch(BranchNum).Component(CompNum).CType)) {
                        FanType = state.dataBranchInputManager->Branch(BranchNum).Component(CompNum).CType;
                        FanName = state.dataBranchInputManager->Branch(BranchNum).Component(CompNum).Name;
                        break;
                    }
                }
                if (FanType.empty()) {
                    ErrFound = true;
                }
            } else {
                ShowSevereError(state, fmt::format("GetBranchFanTypeName:  Branch index not found = {}", BranchNum));
                ErrFound = true;
            }
        }
    }

    void GetInternalBranchData(EnergyPlusData &state,
                               std::string const &LoopName,                               // Loop Name for Branch
                               std::string const &BranchName,                             // Requested Branch Name
                               DataBranchAirLoopPlant::PressureCurveType &PressCurveType, // Index of pressure curve object
                               int &PressCurveIndex,                                      // Index of pressure curve object
                               int &NumComps,                                             // Number of Components on Branch
                               Array1D<ComponentData> const &BComponents,                 // Component data returned
                               bool &ErrorsFound // True when Loop Name is already assigned and this not same loop
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   October 1999
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine gets the Branch Data (internal structure) for the requested
        // Branch Name and returns it to the calling routine.  This is used internally
        // in the module.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Found; // Pointer to requested Branch Name

        if (state.dataBranchInputManager->GetBranchInputFlag) {
            GetBranchInput(state);
            state.dataBranchInputManager->GetBranchInputFlag = false;
        }

        Found = Util::FindItemInList(BranchName, state.dataBranchInputManager->Branch);
        if (Found == 0) {
            ShowSevereError(state, format("GetInternalBranchData:  Branch not found={}", BranchName));
            ErrorsFound = true;
            NumComps = 0;
        } else {
            if (state.dataBranchInputManager->Branch(Found).AssignedLoopName.empty()) {
                state.dataBranchInputManager->Branch(Found).AssignedLoopName = LoopName;
                PressCurveType = state.dataBranchInputManager->Branch(Found).PressureCurveType;
                PressCurveIndex = state.dataBranchInputManager->Branch(Found).PressureCurveIndex;
                NumComps = state.dataBranchInputManager->Branch(Found).NumOfComponents;
                BComponents({1, NumComps}) = state.dataBranchInputManager->Branch(Found).Component({1, NumComps});
            } else if (state.dataBranchInputManager->Branch(Found).AssignedLoopName != LoopName) {
                ShowSevereError(state, format("Attempt to assign branch to two different loops, Branch={}", BranchName));
                ShowContinueError(state, format("Branch already assigned to loop={}", state.dataBranchInputManager->Branch(Found).AssignedLoopName));
                ShowContinueError(state, format("New attempt to assign to loop={}", LoopName));
                ErrorsFound = true;
                NumComps = 0;
            } else {
                PressCurveType = state.dataBranchInputManager->Branch(Found).PressureCurveType;
                PressCurveIndex = state.dataBranchInputManager->Branch(Found).PressureCurveIndex;
                NumComps = state.dataBranchInputManager->Branch(Found).NumOfComponents;
                BComponents({1, NumComps}) = state.dataBranchInputManager->Branch(Found).Component({1, NumComps});
            }
        }
    }

    void GetNumSplitterMixerInConntrList(EnergyPlusData &state,
                                         std::string const &LoopName,          // Loop Name for this Splitter (used in error message)
                                         std::string const &ConnectorListName, // Requested Connector List Name
                                         int &numSplitters,                    // Number of splitters in the loop
                                         int &numMixers,                       // Number of mixers in the loop
                                         bool &ErrorsFound                     // if no connector list
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Sankaranarayanan K P
        //       DATE WRITTEN   April 2005
        //       MODIFIED       Linda Lawrie - September 2005
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine returns the number of splitter and mixers in a connector list item
        // The data is filled from the idd object 'ConnectorList'

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ConnNum;

        if (state.dataBranchInputManager->GetConnectorListInputFlag) {
            GetConnectorListInput(state);
            state.dataBranchInputManager->GetConnectorListInputFlag = false;
        }

        numSplitters = 0;
        numMixers = 0;
        ConnNum = Util::FindItemInList(ConnectorListName, state.dataBranchInputManager->ConnectorLists);

        if (ConnNum > 0) {
            numSplitters = state.dataBranchInputManager->ConnectorLists(ConnNum).NumOfSplitters;
            numMixers = state.dataBranchInputManager->ConnectorLists(ConnNum).NumOfMixers;
        } else {
            ShowSevereError(state, format("Ref: Loop={}, Connector List not found={}", LoopName, ConnectorListName));
            ErrorsFound = true;
        }
    }

    void GetConnectorList(EnergyPlusData &state,
                          std::string const &ConnectorListName,   // Requested Connector List
                          ConnectorData &Connectoid,              // Returned Connector Data
                          ObjexxFCL::Optional_int_const NumInList // Number of the current connector in the list of connectors
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   October 1999

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains connector data for requested connector list.  Also,
        // this subroutine gets the input for the following IDD structure:
        // ConnectorList,
        //         \memo only two connectors allowed per loop
        //         \memo if two entered, one must be Connector:Splitter and one must be Connector:Mixer
        //     A1, \field Name
        //         \required-field
        //         \reference ConnectorLists
        //     A2, \field Connector 1 Object Type
        //         \required-field
        //         \key Connector:Splitter
        //         \key Connector:Mixer
        //     A3, \field Connector 1 Name
        //         \required-field
        //     A4, \field Connector 2 Object Type
        //         \key Connector:Splitter
        //         \key Connector:Mixer
        //     A5; \field Connector 2 Name

        if (state.dataBranchInputManager->GetConnectorListInputFlag) {
            GetConnectorListInput(state);
            state.dataBranchInputManager->GetConnectorListInputFlag = false;
        }

        if (not_blank(ConnectorListName)) {
            int Count = Util::FindItemInList(ConnectorListName, state.dataBranchInputManager->ConnectorLists);
            if (Count == 0) {
                ShowFatalError(state, format("GetConnectorList: Connector List not found={}", ConnectorListName));
            }
            Connectoid = state.dataBranchInputManager->ConnectorLists(Count);
            if (present(NumInList)) {
                Connectoid.ConnectorType(1) = state.dataBranchInputManager->ConnectorLists(Count).ConnectorType(NumInList);
                Connectoid.ConnectorName(1) = state.dataBranchInputManager->ConnectorLists(Count).ConnectorName(NumInList);
                Connectoid.ConnectorType(2) = "";
                Connectoid.ConnectorName(2) = "";
            }
        } else {
            Connectoid.Name = "";
            Connectoid.NumOfConnectors = 0;
            Connectoid.ConnectorType(1) = "";
            Connectoid.ConnectorType(2) = "";
            Connectoid.ConnectorName(1) = "";
            Connectoid.ConnectorName(2) = "";
        }
    }

    void GetLoopMixer(EnergyPlusData &state,
                      std::string const &LoopName,          // Loop Name for Mixer
                      std::string const &ConnectorListName, // Requested Connector List Name
                      std::string &MixerName,               // Name of Mixer
                      bool &IsMixer,                        // True when Mixer is on this connector, false otherwise
                      std::string &OutletNodeName,          // Outlet Node ID
                      int &OutletNodeNum,                   // Outlet Node Number
                      int &NumInletNodes,                   // Number of Inlet Nodes
                      Array1D_string &InletNodeNames,       // Inlet Node IDs
                      Array1D_int &InletNodeNums,           // Inlet Node Numbers
                      bool &ErrorsFound,
                      ObjexxFCL::Optional_int_const ConnectorNumber, // number of the current item in connector list
                      ObjexxFCL::Optional_int MixerNumber            // Mixer number for this specific splitter
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   October 1999
        //       MODIFIED       October 2001, Automatic Extensibility

        // PURPOSE OF THIS SUBROUTINE:
        // This routine gets the data for the requested Connector List and returns values indicating
        // if this connector list name is a mixer or not.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Count; // Loop Counter
        DataBranchAirLoopPlant::PressureCurveType PressCurveType;

        // Object Data
        ConnectorData Connectoid;           // Connector Data
        Array1D<ComponentData> BComponents; // Branch Component Data

        if (state.dataBranchInputManager->GetMixerInputFlag) {
            GetMixerInput(state);
            state.dataBranchInputManager->GetMixerInputFlag = false;
        }

        GetConnectorList(state, ConnectorListName, Connectoid, ConnectorNumber);
        if (Util::SameString(Connectoid.ConnectorType(1), cMIXER)) {
            Count = Util::FindItemInList(Connectoid.ConnectorName(1), state.dataBranchInputManager->Mixers);
            if (present(MixerNumber)) {
                ++MixerNumber;
            }
            if (Count == 0) {
                ShowFatalError(state, format("GetLoopMixer: No Mixer Found={}", Connectoid.ConnectorName(1)));
            }
        } else if (Util::SameString(Connectoid.ConnectorType(2), cMIXER)) {
            Count = Util::FindItemInList(Connectoid.ConnectorName(2), state.dataBranchInputManager->Mixers);
            if (Count == 0) {
                ShowFatalError(state, format("GetLoopMixer: No Mixer Found={}", Connectoid.ConnectorName(2)));
            }
        } else {
            Count = 0;
        }

        // Set defaults for later error potential
        IsMixer = false;
        MixerName = std::string();
        OutletNodeName = std::string();
        OutletNodeNum = 0;
        NumInletNodes = 0;
        InletNodeNames = "";
        InletNodeNums = 0;

        if (Count != 0) { // Build up Output list(s). For each component(?)

            int NumParams;
            int NumAlphas;
            int NumNumbers;
            // The inlet nodes for the mixer will be the last "outlet" node of
            // each corresponding inlet branch.  The outlet node for the mixer
            // will be the first "inlet" node of the outlet branch since that
            // would be the first node on the branch.
            MixerName = state.dataBranchInputManager->Mixers(Count).Name;
            IsMixer = true;
            // The number of "components" on a Mixer is the number of branches.  This is the number of alpha arguments -1.
            state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "Branch", NumParams, NumAlphas, NumNumbers);
            BComponents.allocate(NumAlphas - 1);
            bool errFlag = false;
            int PressCurveIndex;
            int NumComps; // Number of Components on this Branch
            GetInternalBranchData(state,
                                  LoopName,
                                  state.dataBranchInputManager->Mixers(Count).OutletBranchName,
                                  PressCurveType,
                                  PressCurveIndex,
                                  NumComps,
                                  BComponents,
                                  errFlag);
            if (errFlag) {
                ShowContinueError(state, format("..occurs for Connector:Mixer Name={}", state.dataBranchInputManager->Mixers(Count).Name));
                ErrorsFound = true;
            }
            if (NumComps > 0) {
                OutletNodeName = BComponents(1).InletNodeName;
                OutletNodeNum = BComponents(1).InletNode;
                NumInletNodes = state.dataBranchInputManager->Mixers(Count).NumInletBranches;
                // Register this node connection because the mixer gets node information indirectly from the branch
                errFlag = false;
                RegisterNodeConnection(state,
                                       OutletNodeNum,
                                       state.dataLoopNodes->NodeID(OutletNodeNum),
                                       DataLoopNode::ConnectionObjectType::ConnectorMixer,
                                       MixerName,
                                       DataLoopNode::ConnectionType::Outlet,
                                       NodeInputManager::CompFluidStream::Primary,
                                       ObjectIsNotParent,
                                       errFlag);

                if (NumInletNodes > isize(InletNodeNames) || NumInletNodes > isize(InletNodeNums)) {
                    ShowSevereError(state, format("GetLoopMixer: Connector:Mixer={} contains too many inlets for size of Inlet Array.", MixerName));
                    ShowContinueError(state, fmt::format("Max array size={}, Mixer statement inlets={}", size(InletNodeNames), NumInletNodes));
                    ShowFatalError(state, "Program terminates due to preceding condition.");
                }
                InletNodeNums = 0;
                InletNodeNames = "";

                for (int Loop = 1; Loop <= state.dataBranchInputManager->Mixers(Count).NumInletBranches; ++Loop) {
                    GetInternalBranchData(state,
                                          LoopName,
                                          state.dataBranchInputManager->Mixers(Count).InletBranchNames(Loop),
                                          PressCurveType,
                                          PressCurveIndex,
                                          NumComps,
                                          BComponents,
                                          ErrorsFound);
                    if (NumComps > 0) {
                        InletNodeNames(Loop) = BComponents(NumComps).OutletNodeName;
                        InletNodeNums(Loop) = BComponents(NumComps).OutletNode;
                        // Register this node connection because the mixer gets node information indirectly from the branch
                        errFlag = false;
                        RegisterNodeConnection(state,
                                               InletNodeNums(Loop),
                                               state.dataLoopNodes->NodeID(InletNodeNums(Loop)),
                                               DataLoopNode::ConnectionObjectType::ConnectorMixer,
                                               MixerName,
                                               DataLoopNode::ConnectionType::Inlet,
                                               NodeInputManager::CompFluidStream::Primary,
                                               ObjectIsNotParent,
                                               errFlag);
                    }
                }
            } else {
                // Set so cascading errors don't happen?
                IsMixer = false;
            }
            BComponents.deallocate();
        }
    }

    void GetLoopSplitter(EnergyPlusData &state,
                         std::string const &LoopName,          // Loop Name for this Splitter
                         std::string const &ConnectorListName, // Requested Connector List Name
                         std::string &SplitterName,            // Name of Splitter
                         bool &IsSplitter,                     // True if splitter on this connector list, false otherwise
                         std::string &InletNodeName,           // Inlet Node ID
                         int &InletNodeNum,                    // Inlet Node Number
                         int &NumOutletNodes,                  // Number of Outlet Nodes
                         Array1D_string &OutletNodeNames,      // Outlet Node IDs
                         Array1D_int &OutletNodeNums,          // Outlet Node Numbers
                         bool &ErrorsFound,
                         ObjexxFCL::Optional_int_const ConnectorNumber, // number of the current item in connector list
                         ObjexxFCL::Optional_int SplitterNumber         // splitter number for this specific splitter
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   October 1999
        //       MODIFIED       October 2001, Automatic Extensibility

        // PURPOSE OF THIS SUBROUTINE:
        // This routine gets the data for the requested Connector List and returns values indicating
        // if this connector list name is a splitter or not.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Count; // Loop Counter
        DataBranchAirLoopPlant::PressureCurveType PressCurveType;

        // Object Data
        ConnectorData Connectoid;           // Connector Data
        Array1D<ComponentData> BComponents; // Branch Component Data

        if (state.dataBranchInputManager->GetSplitterInputFlag) {
            GetSplitterInput(state);
            state.dataBranchInputManager->GetSplitterInputFlag = false;
        }

        if (ConnectorListName.empty()) {
            ShowSevereError(state, format("GetLoopSplitter: ConnectorListName is blank.  LoopName={}", LoopName));
            ShowFatalError(state, "Program terminates due to previous condition.");
        }
        GetConnectorList(state, ConnectorListName, Connectoid, ConnectorNumber);
        if (Util::SameString(Connectoid.ConnectorType(1), cSPLITTER)) {
            Count = Util::FindItemInList(Connectoid.ConnectorName(1), state.dataBranchInputManager->Splitters);
            if (present(SplitterNumber)) {
                ++SplitterNumber;
            }
            if (Count == 0) {
                ShowFatalError(state, format("GetLoopSplitter: No Splitter Found={}", Connectoid.ConnectorName(1)));
            }
        } else if (Util::SameString(Connectoid.ConnectorType(2), cSPLITTER)) {
            Count = Util::FindItemInList(Connectoid.ConnectorName(2), state.dataBranchInputManager->Splitters);
            if (Count == 0) {
                ShowFatalError(state, format("GetLoopSplitter: No Splitter Found={}", Connectoid.ConnectorName(2)));
            }
        } else {
            Count = 0;
        }

        // Default for any errors
        SplitterName = std::string();
        IsSplitter = false;
        InletNodeName = std::string();
        InletNodeNum = 0;
        NumOutletNodes = 0;
        OutletNodeNames = "";
        OutletNodeNums = 0;

        if (Count != 0) { // Build up Output list(s). For each component(?)

            int NumComps; // Number of Components on this Branch
            int NumParams;
            int NumAlphas;
            int NumNumbers;
            int PressCurveIndex;

            // The inlet node for the splitter will be the last "outlet" node of the inlet
            // branch. The outlet nodes for the splitter will be the first "inlet" node of
            // each corresponding outlet branch since that would be the first node on the branch.

            SplitterName = state.dataBranchInputManager->Splitters(Count).Name;
            IsSplitter = true;
            // The number of "components" on a Splitter is the number of branches.  This is the number of alpha arguments -1.
            state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "Branch", NumParams, NumAlphas, NumNumbers);
            BComponents.allocate(NumAlphas - 1);
            bool errFlag = false;
            GetInternalBranchData(state,
                                  LoopName,
                                  state.dataBranchInputManager->Splitters(Count).InletBranchName,
                                  PressCurveType,
                                  PressCurveIndex,
                                  NumComps,
                                  BComponents,
                                  errFlag);
            if (errFlag) {
                ShowContinueError(state, format("..occurs for Splitter Name={}", state.dataBranchInputManager->Splitters(Count).Name));
                ErrorsFound = true;
            }
            if (NumComps > 0) {
                InletNodeName = BComponents(NumComps).OutletNodeName;
                InletNodeNum = BComponents(NumComps).OutletNode;
                NumOutletNodes = state.dataBranchInputManager->Splitters(Count).NumOutletBranches;
                // Register this node connection because the splitter gets node information indirectly from the branch
                errFlag = false;
                RegisterNodeConnection(state,
                                       InletNodeNum,
                                       state.dataLoopNodes->NodeID(InletNodeNum),
                                       DataLoopNode::ConnectionObjectType::ConnectorSplitter,
                                       SplitterName,
                                       DataLoopNode::ConnectionType::Inlet,
                                       NodeInputManager::CompFluidStream::Primary,
                                       ObjectIsNotParent,
                                       errFlag);

                if (NumOutletNodes > isize(OutletNodeNames) || NumOutletNodes > isize(OutletNodeNums)) {
                    ShowSevereError(
                        state, format("GetLoopSplitter: Connector:Splitter={} contains too many outlets for size of Outlet Array.", SplitterName));
                    ShowContinueError(state, fmt::format("Max array size={}, Splitter statement outlets={}", size(OutletNodeNames), NumOutletNodes));
                    ShowFatalError(state, "Program terminates due to preceding condition.");
                }
                OutletNodeNums = 0;
                OutletNodeNames = "";

                for (int Loop = 1; Loop <= state.dataBranchInputManager->Splitters(Count).NumOutletBranches; ++Loop) {
                    GetInternalBranchData(state,
                                          LoopName,
                                          state.dataBranchInputManager->Splitters(Count).OutletBranchNames(Loop),
                                          PressCurveType,
                                          PressCurveIndex,
                                          NumComps,
                                          BComponents,
                                          ErrorsFound);
                    if (NumComps > 0) {
                        OutletNodeNames(Loop) = BComponents(1).InletNodeName;
                        OutletNodeNums(Loop) = BComponents(1).InletNode;
                        // Register this node connection because the splitter gets node information indirectly from the branch
                        errFlag = false;
                        RegisterNodeConnection(state,
                                               OutletNodeNums(Loop),
                                               state.dataLoopNodes->NodeID(OutletNodeNums(Loop)),
                                               DataLoopNode::ConnectionObjectType::ConnectorSplitter,
                                               SplitterName,
                                               DataLoopNode::ConnectionType::Outlet,
                                               NodeInputManager::CompFluidStream::Primary,
                                               ObjectIsNotParent,
                                               errFlag);
                    }
                }
            } else {
                //  Set so cascading errors don't happen
                IsSplitter = false;
            }
            BComponents.deallocate();
        }
    }

    std::string GetFirstBranchInletNodeName(EnergyPlusData &state, std::string const &BranchListName) // Branch List name to search
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   November 2004

        // PURPOSE OF THIS FUNCTION:
        // This function uses the branch structure to obtain the inlet node
        // of the first branch from referenced Branch List.

        // Return value
        std::string InletNodeName; // Inlet node name of first branch in branch list

        if (state.dataBranchInputManager->GetBranchListInputFlag) {
            state.dataBranchInputManager->GetBranchListInputFlag = false;
            GetBranchListInput(state);
        }

        int Found1 = Util::FindItemInList(BranchListName, state.dataBranchInputManager->BranchList);
        if (Found1 == 0) {
            ShowSevereError(state, format("GetFirstBranchInletNodeName: BranchList=\"{}\", not a valid BranchList Name", BranchListName));
            InletNodeName = "Invalid Node Name";
        } else {
            int Found2 = Util::FindItemInList(state.dataBranchInputManager->BranchList(Found1).BranchNames(1), state.dataBranchInputManager->Branch);
            if (Found2 == 0) {
                ShowSevereError(state,
                                format("GetFirstBranchInletNodeName: BranchList=\"{}\", Branch=\"{}\" not a valid Branch Name",
                                       BranchListName,
                                       state.dataBranchInputManager->BranchList(Found1).BranchNames(1)));
                InletNodeName = "Invalid Node Name";
            } else {
                InletNodeName = state.dataBranchInputManager->Branch(Found2).Component(1).InletNodeName;
            }
        }

        return InletNodeName;
    }

    std::string GetLastBranchOutletNodeName(EnergyPlusData &state, std::string const &BranchListName) // Branch List name to search
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   August 2003

        // PURPOSE OF THIS FUNCTION:
        // This function uses the branch structure to obtain the outlet node
        // of the last branch from referenced Branch List.

        // Return value
        std::string OutletNodeName; // Outlet node name of last branch in branch list

        if (state.dataBranchInputManager->GetBranchListInputFlag) {
            state.dataBranchInputManager->GetBranchListInputFlag = false;
            GetBranchListInput(state);
        }

        int Found1 = Util::FindItemInList(BranchListName, state.dataBranchInputManager->BranchList);
        if (Found1 == 0) {
            ShowSevereError(state, format("GetLastBranchOutletNodeName: BranchList=\"{}\", not a valid BranchList Name", BranchListName));
            OutletNodeName = "Invalid Node Name";
        } else {
            int Found2 = Util::FindItemInList(
                state.dataBranchInputManager->BranchList(Found1).BranchNames(state.dataBranchInputManager->BranchList(Found1).NumOfBranchNames),
                state.dataBranchInputManager->Branch);
            if (Found2 == 0) {
                ShowSevereError(state,
                                format("GetLastBranchOutletNodeName: BranchList=\"{}\", Branch=\"{}\" not a valid Branch Name",
                                       BranchListName,
                                       state.dataBranchInputManager->BranchList(Found1).BranchNames(
                                           state.dataBranchInputManager->BranchList(Found1).NumOfBranchNames)));
                OutletNodeName = "Invalid Node Name";
            } else {
                OutletNodeName = state.dataBranchInputManager->Branch(Found2)
                                     .Component(state.dataBranchInputManager->Branch(Found2).NumOfComponents)
                                     .OutletNodeName;
            }
        }

        return OutletNodeName;
    }

    //==================================================================================
    //   Routines that get the input for the internal branch management structure
    //==================================================================================

    void GetBranchInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   October 1999
        //       MODIFIED       October 2001, Automatic Extensibility

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the input for the following IDD structure:
        // Branch,
        //         \extensible:4 Just duplicate last 4 fields and \ comments (changing numbering, please)
        //         \memo List components on the branch in simulation and connection order
        //         \memo Note: this should NOT include splitters or mixers which define
        //         \memo endpoints of branches
        //    A1,  \field Name
        //         \required-field
        //         \reference Branches
        //    N1, \field Maximum Flow Rate
        //         \default 0
        //         \units m3/s
        //         \minimum 0
        //         \autosizable
        //    A2, \field Pressure Curve Name
        //         \type object-list
        //         \reference AllCurves
        //    A3, \field Component 1 Object Type
        //         \required-field
        //    A4, \field Component 1 Name
        //         \required-field
        //    A5, \field Component 1 Inlet Node Name
        //         \required-field
        //    A6, \field Component 1 Outlet Node Name
        //         \required-field

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetBranchInput: ");

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        Array1D_string Alphas;   // Used to retrieve names from IDF
        Array1D_int NodeNums;    // Possible Array of Node Numbers (only 1 allowed)
        Array1D<Real64> Numbers; // Used to retrieve numbers from IDF
        Array1D_string cAlphaFields;
        Array1D_string cNumericFields;
        Array1D_bool lNumericBlanks;
        Array1D_bool lAlphaBlanks;

        if (state.dataBranchInputManager->GetBranchInputOneTimeFlag) {
            std::string CurrentModuleObject = "Branch";
            int NumOfBranches = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

            if (NumOfBranches > 0) {
                state.dataBranchInputManager->Branch.allocate(NumOfBranches);
                int NumNumbers; // Used to retrieve numbers from IDF
                int NumAlphas;  // Used to retrieve names from IDF
                int NumParams;
                state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "NodeList", NumParams, NumAlphas, NumNumbers);
                NodeNums.dimension(NumParams, 0);
                state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
                Alphas.allocate(NumAlphas);
                Numbers.dimension(NumNumbers, 0.0);
                cAlphaFields.allocate(NumAlphas);
                cNumericFields.allocate(NumNumbers);
                lAlphaBlanks.dimension(NumAlphas, true);
                lNumericBlanks.dimension(NumNumbers, true);
                int BCount = 0; // Actual Num of Branches
                int IOStat;     // Could be used in the Get Routines, not currently checked
                for (int Count = 1; Count <= NumOfBranches; ++Count) {
                    state.dataInputProcessing->inputProcessor->getObjectItem(state,
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
                    ++BCount;
                    GetSingleBranchInput(state, RoutineName, BCount, Alphas, cAlphaFields, NumAlphas, NodeNums, lAlphaBlanks);
                }

                NumOfBranches = BCount;
                NodeNums.deallocate();
                Alphas.deallocate();
                Numbers.deallocate();
                cAlphaFields.deallocate();
                cNumericFields.deallocate();
                lAlphaBlanks.deallocate();
                lNumericBlanks.deallocate();
                TestInletOutletNodes(state);
                state.dataBranchInputManager->GetBranchInputOneTimeFlag = false;
            }
        }
    }

    void GetSingleBranchInput(EnergyPlusData &state,
                              std::string_view const RoutineName,
                              int const BCount,
                              Array1D_string const &Alphas,
                              Array1D_string const &cAlphaFields,
                              int const NumAlphas,
                              Array1D_int &NodeNums,
                              Array1D_bool const &lAlphaBlanks)
    {
        // Locals
        PressureCurveType pressureCurveType;
        int PressureCurveIndex;
        bool ErrFound;                               // Flag for error detection
        int Comp;                                    // Loop Counter
        bool IsNotOK;                                // Flag to verify name
        int NumInComps;                              // Number of components actually verified (no SPLITTER or MIXER allowed)
        DataLoopNode::ConnectionType ConnectionType; // Used to pass variable node connection type to GetNodeNums
        int NumNodes;                                // Number of Nodes from NodeInputManager

        std::string CurrentModuleObject = "Branch";

        state.dataBranchInputManager->Branch(BCount).Name = Alphas(1);
        Curve::GetPressureCurveTypeAndIndex(state, Alphas(2), pressureCurveType, PressureCurveIndex);
        if (pressureCurveType == DataBranchAirLoopPlant::PressureCurveType::Invalid) {
            ShowSevereError(state, format("{}{}=\"{}\", invalid data.", RoutineName, CurrentModuleObject, Alphas(1)));
            ShowContinueError(state, format("..Invalid {}=\"{}\".", cAlphaFields(2), Alphas(2)));
            ShowContinueError(state, "This curve could not be found in the input deck.  Ensure that this curve has been entered");
            ShowContinueError(state, " as either a Curve:Functional:PressureDrop or one of Curve:{Linear,Quadratic,Cubic,Exponent}");
            ShowContinueError(state, "This error could be caused by a misspelled curve name");
            ErrFound = true;
        }
        state.dataBranchInputManager->Branch(BCount).PressureCurveType = pressureCurveType;
        state.dataBranchInputManager->Branch(BCount).PressureCurveIndex = PressureCurveIndex;
        state.dataBranchInputManager->Branch(BCount).NumOfComponents = (NumAlphas - 2) / 4;
        if (state.dataBranchInputManager->Branch(BCount).NumOfComponents * 4 != (NumAlphas - 2)) {
            ++state.dataBranchInputManager->Branch(BCount).NumOfComponents;
        }
        NumInComps = state.dataBranchInputManager->Branch(BCount).NumOfComponents;
        state.dataBranchInputManager->Branch(BCount).Component.allocate(state.dataBranchInputManager->Branch(BCount).NumOfComponents);
        Comp = 1;
        for (int Loop = 3; Loop <= NumAlphas; Loop += 4) {
            if (Util::SameString(Alphas(Loop), cSPLITTER) || Util::SameString(Alphas(Loop), cMIXER)) {
                ShowSevereError(state, format("{}{}=\"{}\", invalid data.", RoutineName, CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Connector:Splitter/Connector:Mixer not allowed in object {}", CurrentModuleObject));
                ErrFound = true;
                continue;
            }
            if (Comp > NumInComps) {
                ShowSevereError(state, format("{}{}=\"{}\", invalid data.", RoutineName, CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, fmt::format("...Number of Arguments indicate [{}], but count of fields indicates [{}]", NumInComps, Comp));
                ShowContinueError(state, format("...examine {} carefully.", CurrentModuleObject));
                continue;
            }
            state.dataBranchInputManager->Branch(BCount).Component(Comp).CType = Alphas(Loop);
            state.dataBranchInputManager->Branch(BCount).Component(Comp).Name = Alphas(Loop + 1);
            ValidateComponent(state, Alphas(Loop), Alphas(Loop + 1), IsNotOK, CurrentModuleObject);
            if (IsNotOK) {
                ShowContinueError(state, format("Occurs on {}={}", CurrentModuleObject, Alphas(1)));
                ErrFound = true;
            }
            state.dataBranchInputManager->Branch(BCount).Component(Comp).InletNodeName = Alphas(Loop + 2);
            // If first component on branch, then inlet node is inlet to branch, otherwise node is internal
            if (Loop == 3) {
                ConnectionType = DataLoopNode::ConnectionType::Inlet;
            } else {
                ConnectionType = DataLoopNode::ConnectionType::Internal;
            }
            if (!lAlphaBlanks(Loop + 2)) {
                GetNodeNums(state,
                            state.dataBranchInputManager->Branch(BCount).Component(Comp).InletNodeName,
                            NumNodes,
                            NodeNums,
                            ErrFound,
                            DataLoopNode::NodeFluidType::Blank,
                            DataLoopNode::ConnectionObjectType::Branch,
                            state.dataBranchInputManager->Branch(BCount).Name,
                            ConnectionType,
                            NodeInputManager::CompFluidStream::Primary,
                            ObjectIsParent,
                            false,
                            cAlphaFields(Loop + 2));
                if (NumNodes > 1) {
                    ShowSevereError(state, format("{}{}=\"{}\", invalid data.", RoutineName, CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state,
                                      format("..invalid {}=\"{}\" must be a single node - appears to be a list.",
                                             cAlphaFields(Loop + 2),
                                             state.dataBranchInputManager->Branch(BCount).Component(Comp).InletNodeName));
                    ShowContinueError(
                        state, format("Occurs on {}=\"{}\", {}=\"{}\".", cAlphaFields(Loop), Alphas(Loop), cAlphaFields(Loop + 1), Alphas(Loop + 1)));
                    ErrFound = true;
                } else {
                    state.dataBranchInputManager->Branch(BCount).Component(Comp).InletNode = NodeNums(1);
                }
            } else {
                ShowSevereError(state, format("{}{}=\"{}\", invalid data.", RoutineName, CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("blank required field: {}", cAlphaFields(Loop + 2)));
                ShowContinueError(
                    state, format("Occurs on {}=\"{}\", {}=\"{}\".", cAlphaFields(Loop), Alphas(Loop), cAlphaFields(Loop + 1), Alphas(Loop + 1)));
                ErrFound = true;
            }
            state.dataBranchInputManager->Branch(BCount).Component(Comp).OutletNodeName = Alphas(Loop + 3);
            // If last component on branch, then outlet node is outlet from branch, otherwise node is internal
            if (Loop == NumAlphas - 3) {
                ConnectionType = DataLoopNode::ConnectionType::Outlet;
            } else {
                ConnectionType = DataLoopNode::ConnectionType::Internal;
            }
            if (!lAlphaBlanks(Loop + 3)) {
                GetNodeNums(state,
                            state.dataBranchInputManager->Branch(BCount).Component(Comp).OutletNodeName,
                            NumNodes,
                            NodeNums,
                            ErrFound,
                            DataLoopNode::NodeFluidType::Blank,
                            DataLoopNode::ConnectionObjectType::Branch,
                            state.dataBranchInputManager->Branch(BCount).Name,
                            ConnectionType,
                            NodeInputManager::CompFluidStream::Primary,
                            ObjectIsParent,
                            false,
                            cAlphaFields(Loop + 3));
                if (NumNodes > 1) {
                    ShowSevereError(state, format("{}{}=\"{}\", invalid data.", RoutineName, CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state,
                                      format("..invalid {}=\"{}\" must be a single node - appears to be a list.",
                                             cAlphaFields(Loop + 2),
                                             state.dataBranchInputManager->Branch(BCount).Component(Comp).InletNodeName));
                    ShowContinueError(
                        state, format("Occurs on {}=\"{}\", {}=\"{}\".", cAlphaFields(Loop), Alphas(Loop), cAlphaFields(Loop + 1), Alphas(Loop + 1)));
                    ErrFound = true;
                } else {
                    state.dataBranchInputManager->Branch(BCount).Component(Comp).OutletNode = NodeNums(1);
                }
            } else {
                ShowSevereError(state, format("{}{}=\"{}\", invalid data.", RoutineName, CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("blank required field: {}", cAlphaFields(Loop + 3)));
                ShowContinueError(
                    state, format("Occurs on {}=\"{}\", {}=\"{}\".", cAlphaFields(Loop), Alphas(Loop), cAlphaFields(Loop + 1), Alphas(Loop + 1)));
                ErrFound = true;
            }

            if (!lAlphaBlanks(Loop) && !lAlphaBlanks(Loop + 1) && !lAlphaBlanks(Loop + 2) && !lAlphaBlanks(Loop + 3)) {
                SetUpCompSets(state,
                              CurrentModuleObject,
                              state.dataBranchInputManager->Branch(BCount).Name,
                              Alphas(Loop),
                              Alphas(Loop + 1),
                              Alphas(Loop + 2),
                              Alphas(Loop + 3)); // no blanks in required field set
            }

            ++Comp;
        }
        state.dataBranchInputManager->Branch(BCount).NumOfComponents = NumInComps;
    }

    void GetBranchListInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   July 2003

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the branch list input and fills up the structures for
        // branch lists.
        // This subroutine gets the input for the following IDD structure:
        // BRANCH LIST,
        //  \extensible:1 Just duplicate last field and \ comments (changing numbering, please)
        //  \memo Branches MUST be listed in flow order: inlet branch, then parallel branches, then outlet branch.
        //  \memo Branches are simulated in the order listed.  Branch names cannot be duplicated within a single branch list.
        //    A1, \field Branch List Name
        //        \required-field
        //        \reference BranchLists
        //    A2, \field Branch Name 1
        //        \required-field
        //        \type object-list
        //        \object-list Branches
        //    A3, \field Branch Name 2
        //        \type object-list
        //        \object-list Branches

        // Using/Aliasing

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetBranchListInput: ");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Count;     // Loop Counter
        int BCount;    // Actual Branch List Count
        int Loop;      // Loop Counter
        int Found;     // Points to correct Branch List/Branch
        bool ErrFound; // True when error has occurred (cannot find Branch List)
        // Following are needed because routine calls GetBranchInput
        // which would overwrite the module Alphas and NumAlphas
        int NumAlphas;         // Used to retrieve Branch list from IDF
        Array1D_string Alphas; // Used to retrieve names from IDF
        int NumNumbers;
        Array1D<Real64> Numbers; // Not used in this object
        Array1D_string cAlphaFields;
        Array1D_string cNumericFields;
        Array1D_bool lNumericBlanks;
        Array1D_bool lAlphaBlanks;
        int IOStat; // Could be used in the Get Routines, not currently checked
        int NumParams;
        std::string TestName;

        ErrFound = false;
        std::string CurrentModuleObject = "BranchList";
        int NumOfBranchLists = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataBranchInputManager->BranchList.allocate(NumOfBranchLists);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
        Alphas.allocate(NumAlphas);
        Numbers.dimension(NumNumbers, 0.0);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);

        if (NumNumbers > 0) {
            ShowSevereError(state,
                            format("{}{} Object definition contains numbers, cannot be decoded by GetBranchListInput routine.",
                                   RoutineName,
                                   CurrentModuleObject));
            ErrFound = true;
        }
        BCount = 0;
        for (Count = 1; Count <= NumOfBranchLists; ++Count) {
            CurrentModuleObject = "BranchList";
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
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

            ++BCount;
            state.dataBranchInputManager->BranchList(BCount).Name = Alphas(1);
            state.dataBranchInputManager->BranchList(BCount).NumOfBranchNames = NumAlphas - 1;
            state.dataBranchInputManager->BranchList(BCount).BranchNames.allocate(NumAlphas - 1);
            if (state.dataBranchInputManager->BranchList(BCount).NumOfBranchNames == 0) {
                ShowSevereError(state,
                                format("{}{}=\"{}\", No branch names entered.",
                                       RoutineName,
                                       CurrentModuleObject,
                                       state.dataBranchInputManager->BranchList(BCount).Name));
                ErrFound = true;
            } else {
                state.dataBranchInputManager->BranchList(BCount).BranchNames({1, NumAlphas - 1}) = Alphas({2, NumAlphas});
                for (Loop = 1; Loop <= state.dataBranchInputManager->BranchList(BCount).NumOfBranchNames; ++Loop) {
                    // If NumOfBranches = 0 then Branches haven't been read yet.
                    if ((int)state.dataBranchInputManager->Branch.size() == 0) {
                        GetBranchInput(state);
                    }
                    if (!state.dataBranchInputManager->BranchList(BCount).BranchNames(Loop).empty()) {
                        Found = Util::FindItemInList(state.dataBranchInputManager->BranchList(BCount).BranchNames(Loop),
                                                     state.dataBranchInputManager->Branch);
                        if (Found == 0) {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\", invalid data.",
                                                   RoutineName,
                                                   CurrentModuleObject,
                                                   state.dataBranchInputManager->BranchList(BCount).Name));
                            ShowContinueError(state,
                                              format("..invalid Branch Name not found=\"{}\".",
                                                     state.dataBranchInputManager->BranchList(BCount).BranchNames(Loop)));
                            ErrFound = true;
                        }
                    }
                }
            }
        }

        // Check for duplicate names specified in Branch Lists
        for (Count = 1; Count <= NumOfBranchLists; ++Count) {
            if (state.dataBranchInputManager->BranchList(Count).NumOfBranchNames == 0) {
                continue;
            }
            TestName = state.dataBranchInputManager->BranchList(Count).BranchNames(1);
            for (Loop = 2; Loop <= state.dataBranchInputManager->BranchList(Count).NumOfBranchNames; ++Loop) {
                if (TestName != state.dataBranchInputManager->BranchList(Count).BranchNames(Loop)) {
                    continue;
                }
                ShowSevereError(
                    state,
                    format("{}{}=\"{}\", invalid data.", RoutineName, CurrentModuleObject, state.dataBranchInputManager->BranchList(BCount).Name));
                ShowContinueError(state, "..invalid: duplicate branch name specified in the list.");
                ShowContinueError(state, format("..Branch Name={}", TestName));
                ShowContinueError(state, fmt::format("..Branch Name #{} is duplicate.", Loop));
                ErrFound = true;
            }
        }

        if (ErrFound) {
            ShowSevereError(state, format("{} Invalid Input -- preceding condition(s) will likely cause termination.", RoutineName));
        }
        NumOfBranchLists = BCount;
        Alphas.deallocate();
        Numbers.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();
    }

    void GetConnectorListInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   October 1999

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains connector list input from IDF.
        // ConnectorList,
        //         \memo only two connectors allowed per loop
        //         \memo if two entered, one must be Connector:Splitter and one must be Connector:Mixer
        //     A1, \field Name
        //         \required-field
        //         \reference ConnectorLists
        //     A2, \field Connector 1 Object Type
        //         \required-field
        //         \key Connector:Splitter
        //         \key Connector:Mixer
        //     A3, \field Connector 1 Name
        //         \required-field
        //     A4, \field Connector 2 Object Type
        //         \key Connector:Splitter
        //         \key Connector:Mixer
        //     A5; \field Connector 2 Name
        //  This is in the process of possibly being extended, thus the code herein.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Count;               // Loop Counter
        int NumAlphas;           // Used to retrieve names from IDF
        Array1D_string Alphas;   // Used to retrieve names from IDF
        int NumNumbers;          // Used to retrieve numbers from IDF
        Array1D<Real64> Numbers; // Used to retrieve numbers from IDF
        Array1D_string cAlphaFields;
        Array1D_string cNumericFields;
        Array1D_bool lNumericBlanks;
        Array1D_bool lAlphaBlanks;
        int IOStat; // Could be used in the Get Routines, not currently checked
        int NumParams;
        int Arg;
        int SplitNum;
        int MixerNum;
        Array1D_string BranchNames;
        int NumBranchNames;
        bool ErrorsFound;
        int Loop;
        int Loop1;
        int Loop2;
        bool CurMixer;
        bool CurSplitter;
        int TestNum;
        bool MatchFound;

        if (!state.dataBranchInputManager->GetConnectorListInputFlag) {
            return;
        }
        ErrorsFound = false;
        std::string CurrentModuleObject = "ConnectorList";
        int NumOfConnectorLists = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataBranchInputManager->ConnectorLists.allocate(NumOfConnectorLists);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
        if (NumAlphas != 5 || NumNumbers != 0) {
            ShowWarningError(state,
                             format("GetConnectorList: Illegal \"extension\" to {} object. Internal code does not support > 2 connectors "
                                    "(Connector:Splitter and Connector:Mixer)",
                                    CurrentModuleObject));
        }
        Alphas.allocate(NumAlphas);
        Numbers.dimension(NumNumbers, 0.0);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);
        for (Count = 1; Count <= NumOfConnectorLists; ++Count) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
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
            state.dataBranchInputManager->ConnectorLists(Count).Name = Alphas(1);
            int NumConnectors = (NumAlphas - 1) / 2; // potential problem if puts in type but not name
            if (mod(NumAlphas - 1, 2) != 0) {
                ++NumConnectors;
            }
            state.dataBranchInputManager->ConnectorLists(Count).NumOfConnectors = NumConnectors;
            state.dataBranchInputManager->ConnectorLists(Count).ConnectorType.allocate(NumConnectors);
            state.dataBranchInputManager->ConnectorLists(Count).ConnectorName.allocate(NumConnectors);
            state.dataBranchInputManager->ConnectorLists(Count).ConnectorMatchNo.allocate(NumConnectors);
            state.dataBranchInputManager->ConnectorLists(Count).ConnectorType = "UNKNOWN";
            state.dataBranchInputManager->ConnectorLists(Count).ConnectorName = "UNKNOWN";
            state.dataBranchInputManager->ConnectorLists(Count).ConnectorMatchNo = 0;
            state.dataBranchInputManager->ConnectorLists(Count).NumOfSplitters = 0;
            state.dataBranchInputManager->ConnectorLists(Count).NumOfMixers = 0;

            int CCount = 0;
            for (Arg = 2; Arg <= NumAlphas; Arg += 2) {
                ++CCount;
                if (Util::SameString(Alphas(Arg), cSPLITTER)) {
                    state.dataBranchInputManager->ConnectorLists(Count).ConnectorType(CCount) = Alphas(Arg).substr(0, 30);
                    ++state.dataBranchInputManager->ConnectorLists(Count).NumOfSplitters;
                } else if (Util::SameString(Alphas(Arg), cMIXER)) {
                    state.dataBranchInputManager->ConnectorLists(Count).ConnectorType(CCount) = Alphas(Arg).substr(0, 30);
                    ++state.dataBranchInputManager->ConnectorLists(Count).NumOfMixers;
                } else {
                    ShowWarningError(
                        state,
                        format("GetConnectorListInput: Invalid {}={} in {}={}", cAlphaFields(Arg), Alphas(Arg), CurrentModuleObject, Alphas(1)));
                }
                state.dataBranchInputManager->ConnectorLists(Count).ConnectorName(CCount) = Alphas(Arg + 1);
            }
        }
        state.dataBranchInputManager->GetConnectorListInputFlag = false;
        Alphas.deallocate();
        Numbers.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        // Validity checks on Connector Lists
        if (state.dataBranchInputManager->GetSplitterInputFlag) {
            GetSplitterInput(state);
            state.dataBranchInputManager->GetSplitterInputFlag = false;
        }
        if (state.dataBranchInputManager->GetMixerInputFlag) {
            GetMixerInput(state);
            state.dataBranchInputManager->GetMixerInputFlag = false;
        }

        SplitNum = 0;
        MixerNum = 0;
        for (Count = 1; Count <= NumOfConnectorLists; ++Count) {
            if (state.dataBranchInputManager->ConnectorLists(Count).NumOfConnectors <= 1) {
                continue; // Air Loop only has one.
            }
            if (state.dataBranchInputManager->ConnectorLists(Count).NumOfConnectors > 2) {
                continue; // Rules not clear for this case
            }
            for (Loop = 1; Loop <= state.dataBranchInputManager->ConnectorLists(Count).NumOfConnectors; ++Loop) {
                if (state.dataBranchInputManager->ConnectorLists(Count).ConnectorMatchNo(Loop) != 0) {
                    continue;
                }
                if (Util::SameString(state.dataBranchInputManager->ConnectorLists(Count).ConnectorType(Loop), cSPLITTER)) {
                    CurSplitter = true;
                    CurMixer = false;
                    SplitNum = Util::FindItemInList(state.dataBranchInputManager->ConnectorLists(Count).ConnectorName(Loop),
                                                    state.dataBranchInputManager->Splitters);
                    // Following code sets up branch names to be matched from Splitter/Mixer data structure
                    if (SplitNum == 0) {
                        ShowSevereError(state,
                                        format("Invalid Connector:Splitter(none)={}, referenced by {}={}",
                                               state.dataBranchInputManager->ConnectorLists(Count).ConnectorName(Loop),
                                               CurrentModuleObject,
                                               state.dataBranchInputManager->ConnectorLists(Count).Name));
                        ErrorsFound = true;
                        continue;
                    }
                    NumBranchNames = state.dataBranchInputManager->Splitters(SplitNum).NumOutletBranches;
                    BranchNames = state.dataBranchInputManager->Splitters(SplitNum).OutletBranchNames;
                } else if (Util::SameString(state.dataBranchInputManager->ConnectorLists(Count).ConnectorType(Loop), cMIXER)) {
                    CurSplitter = true;
                    CurMixer = false;
                    MixerNum = Util::FindItemInList(state.dataBranchInputManager->ConnectorLists(Count).ConnectorName(Loop),
                                                    state.dataBranchInputManager->Mixers);
                    if (MixerNum == 0) {
                        ShowSevereError(state,
                                        format("Invalid Connector:Mixer(none)={}, referenced by {}={}",
                                               state.dataBranchInputManager->ConnectorLists(Count).ConnectorName(Loop),
                                               CurrentModuleObject,
                                               state.dataBranchInputManager->ConnectorLists(Count).Name));
                        ErrorsFound = true;
                        continue;
                    }
                    NumBranchNames = state.dataBranchInputManager->Mixers(MixerNum).NumInletBranches;
                    BranchNames = state.dataBranchInputManager->Mixers(MixerNum).InletBranchNames;
                } else {
                    continue;
                }
                // Try to match mixer to splitter
                for (Loop1 = Loop + 1; Loop1 <= state.dataBranchInputManager->ConnectorLists(Count).NumOfConnectors; ++Loop1) {
                    if (CurMixer && !Util::SameString(state.dataBranchInputManager->ConnectorLists(Count).ConnectorType(Loop1), cSPLITTER)) {
                        continue;
                    }
                    if (CurSplitter && !Util::SameString(state.dataBranchInputManager->ConnectorLists(Count).ConnectorType(Loop1), cMIXER)) {
                        continue;
                    }
                    if (state.dataBranchInputManager->ConnectorLists(Count).ConnectorMatchNo(Loop1) != 0) {
                        continue;
                    }
                    {
                        if (CurSplitter) {
                            // Current "item" is a splitter, candidate is a mixer.
                            MixerNum = Util::FindItemInList(state.dataBranchInputManager->ConnectorLists(Count).ConnectorName(Loop1),
                                                            state.dataBranchInputManager->Mixers);
                            if (MixerNum == 0) {
                                continue;
                            }
                            if (state.dataBranchInputManager->Mixers(MixerNum).NumInletBranches != NumBranchNames) {
                                continue;
                            }
                            MatchFound = true;
                            for (Loop2 = 1; Loop2 <= state.dataBranchInputManager->Mixers(MixerNum).NumInletBranches; ++Loop2) {
                                TestNum = Util::FindItemInList(
                                    state.dataBranchInputManager->Mixers(MixerNum).InletBranchNames(Loop2), BranchNames, NumBranchNames);
                                if (TestNum == 0) {
                                    MatchFound = false;
                                    break;
                                }
                            }
                            if (MatchFound) {
                                state.dataBranchInputManager->ConnectorLists(Count).ConnectorMatchNo(Loop1) = MixerNum;
                                state.dataBranchInputManager->ConnectorLists(Count).ConnectorMatchNo(Loop) = SplitNum;
                            }
                        } else {
                            // Current "item" is a splitter, candidate is a mixer.
                            SplitNum = Util::FindItemInList(state.dataBranchInputManager->ConnectorLists(Count).ConnectorName(Loop1),
                                                            state.dataBranchInputManager->Splitters);
                            if (SplitNum == 0) {
                                continue;
                            }
                            if (state.dataBranchInputManager->Splitters(SplitNum).NumOutletBranches != NumBranchNames) {
                                continue;
                            }
                            MatchFound = true;
                            for (Loop2 = 1; Loop2 <= state.dataBranchInputManager->Splitters(SplitNum).NumOutletBranches; ++Loop2) {
                                TestNum = Util::FindItemInList(
                                    state.dataBranchInputManager->Splitters(SplitNum).OutletBranchNames(Loop2), BranchNames, NumBranchNames);
                                if (TestNum == 0) {
                                    MatchFound = false;
                                    break;
                                }
                            }
                            if (MatchFound) {
                                state.dataBranchInputManager->ConnectorLists(Count).ConnectorMatchNo(Loop1) = SplitNum;
                                state.dataBranchInputManager->ConnectorLists(Count).ConnectorMatchNo(Loop) = MixerNum;
                            }
                        }
                    }
                }
                BranchNames.deallocate();
            }
        }

        for (Count = 1; Count <= NumOfConnectorLists; ++Count) {
            if (state.dataBranchInputManager->ConnectorLists(Count).NumOfConnectors <= 1) {
                continue; // Air Loop only has one.
            }
            if (state.dataBranchInputManager->ConnectorLists(Count).NumOfConnectors > 2) {
                continue; // Rules not clear
            }
            for (Loop = 1; Loop <= state.dataBranchInputManager->ConnectorLists(Count).NumOfConnectors; ++Loop) {
                if (state.dataBranchInputManager->ConnectorLists(Count).ConnectorMatchNo(Loop) != 0) {
                    continue;
                }
                //  = 0, not matched.
                ShowSevereError(state, format("For {}={}", CurrentModuleObject, state.dataBranchInputManager->ConnectorLists(Count).Name));
                ShowContinueError(state,
                                  format("...Item={}, Type={} was not matched.",
                                         state.dataBranchInputManager->ConnectorLists(Count).ConnectorName(Loop),
                                         state.dataBranchInputManager->ConnectorLists(Count).ConnectorType(Loop)));
                if (Util::SameString(state.dataBranchInputManager->ConnectorLists(Count).ConnectorType(Loop), "Connector:Splitter")) {
                    ShowContinueError(
                        state, "The BranchList for this Connector:Splitter does not match the BranchList for its corresponding Connector:Mixer.");
                } else {
                    ShowContinueError(
                        state, "The BranchList for this Connector:Mixer does not match the BranchList for its corresponding Connector:Splitter.");
                }
                ErrorsFound = true;
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, "GetConnectorListInput: Program terminates for preceding conditions.");
        }
    }

    void GetSplitterInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   Sept 2005 (moved from GetLoopSplitter)

        // PURPOSE OF THIS SUBROUTINE:
        // Gets the Splitter data that is used in Loops.
        // IDD structure:
        // Connector:Splitter,
        //   \min-fields 3
        //        \extensible:1 Just duplicate last field and \ comments (changing numbering, please)
        //        \memo Split one air/water stream into N outlet streams.  Branch names cannot be duplicated
        //        \memo within a single Splitter list.
        //    A1, \field Name
        //         \required-field
        //    A2, \field Inlet Branch Name
        //         \required-field
        //         \type object-list
        //         \object-list Branches
        //    A3, \field Outlet Branch 1 Name
        //         \required-field
        //         \type object-list
        //         \object-list Branches
        //    A4, \field Outlet Branch 2 Name
        //         \type object-list
        //         \object-list Branches

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;           // Used to retrieve names from IDF
        Array1D_string Alphas;   // Used to retrieve names from IDF
        int NumNumbers;          // Used to retrieve numbers from IDF
        Array1D<Real64> Numbers; // Used to retrieve numbers from IDF
        Array1D_string cAlphaFields;
        Array1D_string cNumericFields;
        Array1D_bool lNumericBlanks;
        Array1D_bool lAlphaBlanks;
        int IOStat; // Could be used in the Get Routines, not currently checked
        int NumParams;
        int Loop;
        int Loop1;
        int Count;
        bool ErrorsFound(false);
        std::string TestName;
        std::string FoundSupplyDemandAir;
        std::string SaveSupplyDemandAir;
        std::string FoundLoop;
        std::string SaveLoop;
        bool MatchedLoop;

        if (!state.dataBranchInputManager->GetSplitterInputFlag) {
            return;
        }
        std::string CurrentModuleObject = cSPLITTER;
        int NumSplitters = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataBranchInputManager->Splitters.allocate(NumSplitters);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
        Alphas.allocate(NumAlphas);
        Numbers.dimension(NumNumbers, 0.0);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);
        for (Count = 1; Count <= NumSplitters; ++Count) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
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
            state.dataBranchInputManager->Splitters(Count).Name = Alphas(1);
            state.dataBranchInputManager->Splitters(Count).InletBranchName = Alphas(2);
            state.dataBranchInputManager->Splitters(Count).NumOutletBranches = NumAlphas - 2;
            state.dataBranchInputManager->Splitters(Count).OutletBranchNames.allocate(
                state.dataBranchInputManager->Splitters(Count).NumOutletBranches);
            for (Loop = 1; Loop <= state.dataBranchInputManager->Splitters(Count).NumOutletBranches; ++Loop) {
                state.dataBranchInputManager->Splitters(Count).OutletBranchNames(Loop) = Alphas(2 + Loop);
            }
        }
        state.dataBranchInputManager->GetSplitterInputFlag = false;
        Alphas.deallocate();
        Numbers.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        // More validity -- check splitter "names" against branches.
        if (!state.dataBranchInputManager->GetBranchInputFlag) {
            GetBranchInput(state);
            state.dataBranchInputManager->GetBranchInputFlag = false;
        }
        for (Count = 1; Count <= NumSplitters; ++Count) {
            int Found = Util::FindItemInList(state.dataBranchInputManager->Splitters(Count).InletBranchName, state.dataBranchInputManager->Branch);
            if (Found == 0) {
                ShowSevereError(state,
                                format("GetSplitterInput: Invalid Branch={}, referenced as Inlet Branch to {}={}",
                                       state.dataBranchInputManager->Splitters(Count).InletBranchName,
                                       CurrentModuleObject,
                                       state.dataBranchInputManager->Splitters(Count).Name));
                ErrorsFound = true;
            }
            for (Loop = 1; Loop <= state.dataBranchInputManager->Splitters(Count).NumOutletBranches; ++Loop) {
                Found = Util::FindItemInList(state.dataBranchInputManager->Splitters(Count).OutletBranchNames(Loop),
                                             state.dataBranchInputManager->Branch);
                if (Found == 0) {
                    ShowSevereError(state,
                                    fmt::format("GetSplitterInput: Invalid Branch={}, referenced as Outlet Branch # {} to {}={}",
                                                state.dataBranchInputManager->Splitters(Count).OutletBranchNames(Loop),
                                                Loop,
                                                CurrentModuleObject,
                                                state.dataBranchInputManager->Splitters(Count).Name));
                    ErrorsFound = true;
                }
            }
        }

        // Check for duplicate names specified in Splitters
        for (Count = 1; Count <= NumSplitters; ++Count) {
            TestName = state.dataBranchInputManager->Splitters(Count).InletBranchName;
            for (Loop = 1; Loop <= state.dataBranchInputManager->Splitters(Count).NumOutletBranches; ++Loop) {
                if (TestName != state.dataBranchInputManager->Splitters(Count).OutletBranchNames(Loop)) {
                    continue;
                }
                ShowSevereError(state,
                                format("{}={} specifies an outlet node name the same as the inlet node.",
                                       CurrentModuleObject,
                                       state.dataBranchInputManager->Splitters(Count).Name));
                ShowContinueError(state, format("..Inlet Node={}", TestName));
                ShowContinueError(state, fmt::format("..Outlet Node #{} is duplicate.", Loop));
                ErrorsFound = true;
            }
            for (Loop = 1; Loop <= state.dataBranchInputManager->Splitters(Count).NumOutletBranches; ++Loop) {
                for (Loop1 = Loop + 1; Loop1 <= state.dataBranchInputManager->Splitters(Count).NumOutletBranches; ++Loop1) {
                    if (state.dataBranchInputManager->Splitters(Count).OutletBranchNames(Loop) !=
                        state.dataBranchInputManager->Splitters(Count).OutletBranchNames(Loop1)) {
                        continue;
                    }
                    ShowSevereError(state,
                                    format("{}={} specifies duplicate outlet nodes in its outlet node list.",
                                           CurrentModuleObject,
                                           state.dataBranchInputManager->Splitters(Count).Name));
                    ShowContinueError(
                        state,
                        fmt::format("..Outlet Node #{} Name={}", Loop, state.dataBranchInputManager->Splitters(Count).OutletBranchNames(Loop)));
                    ShowContinueError(state, fmt::format("..Outlet Node #{} is duplicate.", Loop));
                    ErrorsFound = true;
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("GetSplitterInput: Fatal Errors Found in {}, program terminates.", CurrentModuleObject));
        }

        //  Everything supposed to be good.  Now make sure all branches in Splitter on same side of loop.
        SaveSupplyDemandAir = std::string();
        for (Count = 1; Count <= NumSplitters; ++Count) {
            // 2.  Find the branch name in branchlist
            TestName = state.dataBranchInputManager->Splitters(Count).InletBranchName;
            std::string BranchListName = std::string();
            for (Loop1 = 1; Loop1 <= (int)state.dataBranchInputManager->BranchList.size(); ++Loop1) {
                if (any_eq(state.dataBranchInputManager->BranchList(Loop1).BranchNames, TestName)) {
                    BranchListName = state.dataBranchInputManager->BranchList(Loop1).Name;
                    break;
                }
            }

            if (!BranchListName.empty()) {
                FoundSupplyDemandAir = std::string();
                FoundLoop = std::string();
                MatchedLoop = false;
                // 3.  Find the loop and type
                FindAirPlantCondenserLoopFromBranchList(state, BranchListName, FoundLoop, FoundSupplyDemandAir, MatchedLoop);
                if (MatchedLoop) {
                    SaveSupplyDemandAir = FoundSupplyDemandAir;
                    SaveLoop = FoundLoop;
                } else {
                    ShowSevereError(
                        state,
                        format("GetSplitterInput: Inlet Splitter Branch=\"{}\" and BranchList=\"{}\" not matched to a Air/Plant/Condenser Loop",
                               TestName,
                               BranchListName));
                    ShowContinueError(state, "...and therefore, not a valid Loop Splitter.");
                    ShowContinueError(state, format("...{}={}", CurrentModuleObject, state.dataBranchInputManager->Splitters(Count).Name));
                    ErrorsFound = true;
                }
            } else {
                ShowSevereError(state, format("GetSplitterInput: Inlet Splitter Branch=\"{}\" not on BranchList", TestName));
                ShowContinueError(state, "...and therefore, not a valid Loop Splitter.");
                ShowContinueError(state, format("...{}={}", CurrentModuleObject, state.dataBranchInputManager->Splitters(Count).Name));
                ErrorsFound = true;
            }
            for (Loop = 1; Loop <= state.dataBranchInputManager->Splitters(Count).NumOutletBranches; ++Loop) {
                TestName = state.dataBranchInputManager->Splitters(Count).OutletBranchNames(Loop);
                BranchListName = std::string();
                for (Loop1 = 1; Loop1 <= (int)state.dataBranchInputManager->BranchList.size(); ++Loop1) {
                    if (any_eq(state.dataBranchInputManager->BranchList(Loop1).BranchNames, TestName)) {
                        BranchListName = state.dataBranchInputManager->BranchList(Loop1).Name;
                        break;
                    }
                }

                if (!BranchListName.empty()) {
                    FoundSupplyDemandAir = std::string();
                    FoundLoop = std::string();
                    MatchedLoop = false;
                    // 3.  Find the loop and type
                    FindAirPlantCondenserLoopFromBranchList(state, BranchListName, FoundLoop, FoundSupplyDemandAir, MatchedLoop);
                    if (MatchedLoop) {
                        if (SaveSupplyDemandAir != FoundSupplyDemandAir || SaveLoop != FoundLoop) {
                            ShowSevereError(
                                state, format("GetSplitterInput: Outlet Splitter Branch=\"{}\" does not match types of Inlet Branch.", TestName));
                            ShowContinueError(state, format("...Inlet Branch is on \"{}\" on \"{}\" side.", SaveLoop, SaveSupplyDemandAir));
                            ShowContinueError(state, format("...Outlet Branch is on \"{}\" on \"{}\" side.", FoundLoop, FoundSupplyDemandAir));
                            ShowContinueError(state, "...All branches in Loop Splitter must be on same kind of loop and supply/demand side.");
                            ShowContinueError(state, format("...{}={}", CurrentModuleObject, state.dataBranchInputManager->Splitters(Count).Name));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(
                            state,
                            format("GetSplitterInput: Outlet Splitter Branch=\"{}\" and BranchList=\"{}\" not matched to a Air/Plant/Condenser Loop",
                                   TestName,
                                   BranchListName));
                        ShowContinueError(state, "...and therefore, not a valid Loop Splitter.");
                        ShowContinueError(state, format("...{}={}", CurrentModuleObject, state.dataBranchInputManager->Splitters(Count).Name));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("GetSplitterInput: Outlet Splitter Branch=\"{}\" not on BranchList", TestName));
                    ShowContinueError(state, "...and therefore, not a valid Loop Splitter");
                    ShowContinueError(state, format("...{}={}", CurrentModuleObject, state.dataBranchInputManager->Splitters(Count).Name));
                    ErrorsFound = true;
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("GetSplitterInput: Fatal Errors Found in {}, program terminates.", CurrentModuleObject));
        }
    }

    void GetMixerInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   Sept 2005 (moved from GetLoopMixer)

        // PURPOSE OF THIS SUBROUTINE:
        // Gets the Mixer data that is used in Loops.
        // IDD Structure:
        // Connector:Mixer,
        //   \min-fields 3
        //        \extensible:1 Just duplicate last field and \ comments (changing numbering, please)
        //        \memo Mix N inlet air/water streams into one.  Branch names cannot be duplicated within
        //        \memo a single mixer list.
        //    A1 , \field Name
        //         \required-field
        //    A2 , \field Outlet Branch Name
        //         \required-field
        //         \type object-list
        //         \object-list Branches
        //    A3 , \field Inlet Branch 1 Name
        //         \required-field
        //         \type object-list
        //         \object-list Branches
        //    A4 , \field Inlet Branch 2 Name
        //         \type object-list
        //         \object-list Branches

        // Using/Aliasing

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;           // Used to retrieve names from IDF
        Array1D_string Alphas;   // Used to retrieve names from IDF
        int NumNumbers;          // Used to retrieve numbers from IDF
        Array1D<Real64> Numbers; // Used to retrieve numbers from IDF
        Array1D_string cAlphaFields;
        Array1D_string cNumericFields;
        Array1D_bool lNumericBlanks;
        Array1D_bool lAlphaBlanks;
        int IOStat; // Could be used in the Get Routines, not currently checked
        int NumParams;
        int Loop;
        int Loop1;
        int Count;
        bool ErrorsFound(false);
        std::string TestName;
        std::string FoundSupplyDemandAir;
        std::string SaveSupplyDemandAir;
        std::string FoundLoop;
        std::string SaveLoop;
        bool MatchedLoop;

        if (!state.dataBranchInputManager->GetMixerInputFlag) {
            return;
        }

        std::string CurrentModuleObject = cMIXER;

        int NumMixers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataBranchInputManager->Mixers.allocate(NumMixers);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
        Alphas.allocate(NumAlphas);
        Numbers.dimension(NumNumbers, 0.0);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);
        for (Count = 1; Count <= NumMixers; ++Count) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
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
            state.dataBranchInputManager->Mixers(Count).Name = Alphas(1);
            state.dataBranchInputManager->Mixers(Count).OutletBranchName = Alphas(2);
            state.dataBranchInputManager->Mixers(Count).NumInletBranches = NumAlphas - 2;
            state.dataBranchInputManager->Mixers(Count).InletBranchNames.allocate(state.dataBranchInputManager->Mixers(Count).NumInletBranches);
            for (Loop = 1; Loop <= state.dataBranchInputManager->Mixers(Count).NumInletBranches; ++Loop) {
                state.dataBranchInputManager->Mixers(Count).InletBranchNames(Loop) = Alphas(2 + Loop);
            }
        }
        state.dataBranchInputManager->GetMixerInputFlag = false;
        Alphas.deallocate();
        Numbers.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        // More validity -- check mixer "names" against branches.
        if (!state.dataBranchInputManager->GetBranchInputFlag) {
            GetBranchInput(state);
            state.dataBranchInputManager->GetBranchInputFlag = false;
        }
        for (Count = 1; Count <= NumMixers; ++Count) {
            int Found = Util::FindItemInList(state.dataBranchInputManager->Mixers(Count).OutletBranchName, state.dataBranchInputManager->Branch);
            if (Found == 0) {
                ShowSevereError(state,
                                format("GetMixerInput: Invalid Branch={}, referenced as Outlet Branch in {}={}",
                                       state.dataBranchInputManager->Mixers(Count).OutletBranchName,
                                       CurrentModuleObject,
                                       state.dataBranchInputManager->Mixers(Count).Name));
                ErrorsFound = true;
            }
            for (Loop = 1; Loop <= state.dataBranchInputManager->Mixers(Count).NumInletBranches; ++Loop) {
                Found =
                    Util::FindItemInList(state.dataBranchInputManager->Mixers(Count).InletBranchNames(Loop), state.dataBranchInputManager->Branch);
                if (Found == 0) {
                    ShowSevereError(state,
                                    format("GetMixerInput: Invalid Branch={}, referenced as Inlet Branch # {} in {}={}",
                                           state.dataBranchInputManager->Mixers(Count).InletBranchNames(Loop),
                                           Loop,
                                           CurrentModuleObject,
                                           state.dataBranchInputManager->Mixers(Count).Name));
                    ErrorsFound = true;
                }
            }
        }

        // Check for duplicate names specified in Mixer
        for (Count = 1; Count <= NumMixers; ++Count) {
            TestName = state.dataBranchInputManager->Mixers(Count).OutletBranchName;
            for (Loop = 1; Loop <= state.dataBranchInputManager->Mixers(Count).NumInletBranches; ++Loop) {
                if (TestName != state.dataBranchInputManager->Mixers(Count).InletBranchNames(Loop)) {
                    continue;
                }
                ShowSevereError(state,
                                format("{}={} specifies an inlet node name the same as the outlet node.",
                                       CurrentModuleObject,
                                       state.dataBranchInputManager->Mixers(Count).Name));
                ShowContinueError(state, format("..Outlet Node={}", TestName));
                ShowContinueError(state, format("..Inlet Node #{} is duplicate.", Loop));
                ErrorsFound = true;
            }
            for (Loop = 1; Loop <= state.dataBranchInputManager->Mixers(Count).NumInletBranches; ++Loop) {
                for (Loop1 = Loop + 1; Loop1 <= state.dataBranchInputManager->Mixers(Count).NumInletBranches; ++Loop1) {
                    if (state.dataBranchInputManager->Mixers(Count).InletBranchNames(Loop) !=
                        state.dataBranchInputManager->Mixers(Count).InletBranchNames(Loop1)) {
                        continue;
                    }
                    ShowSevereError(state,
                                    format("{}={} specifies duplicate inlet nodes in its inlet node list.",
                                           CurrentModuleObject,
                                           state.dataBranchInputManager->Mixers(Count).Name));
                    ShowContinueError(
                        state, fmt::format("..Inlet Node #{} Name={}", Loop, state.dataBranchInputManager->Mixers(Count).InletBranchNames(Loop)));
                    ShowContinueError(state, fmt::format("..Inlet Node #{} is duplicate.", Loop));
                    ErrorsFound = true;
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("GetMixerInput: Fatal Errors Found in {}, program terminates.", CurrentModuleObject));
        }

        //  Everything supposed to be good.  Now make sure all branches in Splitter on same side of loop.
        SaveSupplyDemandAir = std::string();
        for (Count = 1; Count <= NumMixers; ++Count) {
            // 2.  Find the branch name in branchlist
            TestName = state.dataBranchInputManager->Mixers(Count).OutletBranchName;
            std::string BranchListName = std::string();
            for (Loop1 = 1; Loop1 <= (int)state.dataBranchInputManager->BranchList.size(); ++Loop1) {
                if (any_eq(state.dataBranchInputManager->BranchList(Loop1).BranchNames, TestName)) {
                    BranchListName = state.dataBranchInputManager->BranchList(Loop1).Name;
                    break;
                }
            }

            if (!BranchListName.empty()) {
                FoundSupplyDemandAir = std::string();
                FoundLoop = std::string();
                MatchedLoop = false;
                // 3.  Find the loop and type
                FindAirPlantCondenserLoopFromBranchList(state, BranchListName, FoundLoop, FoundSupplyDemandAir, MatchedLoop);
                if (MatchedLoop) {
                    SaveSupplyDemandAir = FoundSupplyDemandAir;
                    SaveLoop = FoundLoop;
                } else {
                    ShowSevereError(
                        state,
                        format("GetMixerInput: Outlet Mixer Branch=\"{}\" and BranchList=\"{}\" not matched to a Air/Plant/Condenser Loop",
                               TestName,
                               BranchListName));
                    ShowContinueError(state, "...and therefore, not a valid Loop Mixer.");
                    ShowContinueError(state, format("...{}={}", CurrentModuleObject, state.dataBranchInputManager->Mixers(Count).Name));
                    ErrorsFound = true;
                }
            } else {
                ShowSevereError(state, format("GetMixerInput: Outlet Mixer Branch=\"{}\" not on BranchList", TestName));
                ShowContinueError(state, "...and therefore, not a valid Loop Mixer.");
                ShowContinueError(state, format("...{}={}", CurrentModuleObject, state.dataBranchInputManager->Mixers(Count).Name));
                ErrorsFound = true;
            }
            for (Loop = 1; Loop <= state.dataBranchInputManager->Mixers(Count).NumInletBranches; ++Loop) {
                TestName = state.dataBranchInputManager->Mixers(Count).InletBranchNames(Loop);
                BranchListName = std::string();
                for (Loop1 = 1; Loop1 <= (int)state.dataBranchInputManager->BranchList.size(); ++Loop1) {
                    if (any_eq(state.dataBranchInputManager->BranchList(Loop1).BranchNames, TestName)) {
                        BranchListName = state.dataBranchInputManager->BranchList(Loop1).Name;
                        break;
                    }
                }

                if (!BranchListName.empty()) {
                    FoundSupplyDemandAir = std::string();
                    FoundLoop = std::string();
                    MatchedLoop = false;
                    // 3.  Find the plant loop and type
                    FindAirPlantCondenserLoopFromBranchList(state, BranchListName, FoundLoop, FoundSupplyDemandAir, MatchedLoop);
                    if (MatchedLoop) {
                        if (SaveSupplyDemandAir != FoundSupplyDemandAir || SaveLoop != FoundLoop) {
                            ShowSevereError(state,
                                            format("GetMixerInput: Outlet Mixer Branch=\"{}\" does not match types of Inlet Branch.", TestName));
                            ShowContinueError(state, format("...Outlet Branch is on \"{}\" on \"{}\" side.", SaveLoop, SaveSupplyDemandAir));
                            ShowContinueError(state, format("...Inlet Branch is on \"{}\" on \"{}\" side.", FoundLoop, FoundSupplyDemandAir));
                            ShowContinueError(state, "...All branches in Loop Mixer must be on same kind of loop and supply/demand side.");
                            ShowContinueError(state, format("...{}={}", CurrentModuleObject, state.dataBranchInputManager->Mixers(Count).Name));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(
                            state,
                            format("GetMixerInput: Inlet Mixer Branch=\"{}\" and BranchList=\"{}\" not matched to a Air/Plant/Condenser Loop",
                                   TestName,
                                   BranchListName));
                        ShowContinueError(state, "...and therefore, not a valid Loop Mixer.");
                        ShowContinueError(state, format("...{}={}", CurrentModuleObject, state.dataBranchInputManager->Mixers(Count).Name));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("GetMixerInput: Inlet Mixer Branch=\"{}\" not on BranchList", TestName));
                    ShowContinueError(state, "...and therefore, not a valid Loop Mixer");
                    ShowContinueError(state, format("...{}={}", CurrentModuleObject, state.dataBranchInputManager->Mixers(Count).Name));
                    ErrorsFound = true;
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("GetMixerInput: Fatal Errors Found in {}, program terminates.", CurrentModuleObject));
        }
    }

    void FindPlantLoopBranchConnection(EnergyPlusData &state,
                                       std::string const &BranchListName,
                                       std::string &FoundPlantLoopName,
                                       int &FoundPlantLoopNum,
                                       std::string &FoundSupplyDemand,
                                       Real64 &FoundVolFlowRate,
                                       bool &MatchedPlantLoop)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   October 2007

        // PURPOSE OF THIS SUBROUTINE:
        // An auxiliary routine locate a plant loop and type from a BranchListName

        // METHODOLOGY EMPLOYED:
        // Calls GetObject for PLANT LOOP

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Num;
        int NumPlantLoops;
        int NumParams;
        Array1D_string Alphas;
        int NumAlphas;
        Array1D<Real64> Numbers;
        int NumNumbers;
        int IOStat;

        // Get Inputs
        std::string CurrentModuleObject = "PlantLoop";

        NumPlantLoops = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
        Alphas.allocate(NumAlphas);
        Numbers.allocate(NumNumbers);

        for (Num = 1; Num <= NumPlantLoops; ++Num) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state, CurrentModuleObject, Num, Alphas, NumAlphas, Numbers, NumNumbers, IOStat);
            // Only looking for BranchList here.
            if (Alphas(8) == BranchListName) {
                FoundPlantLoopName = Alphas(1);
                FoundSupplyDemand = "Supply";
                FoundVolFlowRate = Numbers(3);
                FoundPlantLoopNum = Num;
                MatchedPlantLoop = true;
                break;
            } else if (Alphas(12) == BranchListName) {
                FoundPlantLoopName = Alphas(1);
                FoundSupplyDemand = "Demand";
                FoundVolFlowRate = Numbers(3);
                FoundPlantLoopNum = Num;
                MatchedPlantLoop = true;
                break;
            }
        }

        Alphas.deallocate();
        Numbers.deallocate();
    }

    void FindCondenserLoopBranchConnection(EnergyPlusData &state,
                                           std::string const &BranchListName,
                                           std::string &FoundCondLoopName,
                                           int &FoundCondLoopNum,
                                           std::string &FoundSupplyDemand,
                                           Real64 &FoundVolFlowRate,
                                           bool &MatchedCondLoop)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   February 2008

        // PURPOSE OF THIS SUBROUTINE:
        // An auxiliary routine locate a condenser loop and type from a BranchListName

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Num;
        int NumCondLoops;
        int NumParams;
        Array1D_string Alphas;
        int NumAlphas;
        Array1D<Real64> Numbers;
        int NumNumbers;
        int IOStat;

        // Get Inputs
        std::string CurrentModuleObject = "CondenserLoop";

        NumCondLoops = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
        Alphas.allocate(NumAlphas);
        Numbers.allocate(NumNumbers);

        for (Num = 1; Num <= NumCondLoops; ++Num) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state, CurrentModuleObject, Num, Alphas, NumAlphas, Numbers, NumNumbers, IOStat);
            // Only looking for BranchList here.
            if (Alphas(8) == BranchListName) {
                FoundCondLoopName = Alphas(1);
                FoundSupplyDemand = "Supply";
                FoundVolFlowRate = Numbers(3);
                FoundCondLoopNum = Num;
                MatchedCondLoop = true;
                break;
            } else if (Alphas(12) == BranchListName) {
                FoundCondLoopName = Alphas(1);
                FoundSupplyDemand = "Demand";
                FoundVolFlowRate = Numbers(3);
                FoundCondLoopNum = Num;
                MatchedCondLoop = true;
                break;
            }
        }

        Alphas.deallocate();
        Numbers.deallocate();
    }

    void FindAirLoopBranchConnection(EnergyPlusData &state,
                                     std::string const &BranchListName,
                                     std::string &FoundAirLoopName,
                                     int &FoundAirLoopNum,
                                     std::string &FoundAir,
                                     Real64 &FoundVolFlowRate,
                                     bool &MatchedAirLoop)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   February 2008

        // PURPOSE OF THIS SUBROUTINE:
        // An auxiliary routine to locate a Air condenser loop and type from a BranchListName

        // METHODOLOGY EMPLOYED:
        // calls GetObject for PRIMARY AIR LOOP

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Num;
        int NumAirLoops;
        int NumParams;
        Array1D_string Alphas;
        int NumAlphas;
        Array1D<Real64> Numbers;
        int NumNumbers;
        int IOStat;

        // Get Inputs
        std::string CurrentModuleObject = "AirLoopHVAC";
        NumAirLoops = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
        Alphas.allocate(NumAlphas);
        Numbers.allocate(NumNumbers);

        for (Num = 1; Num <= NumAirLoops; ++Num) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state, CurrentModuleObject, Num, Alphas, NumAlphas, Numbers, NumNumbers, IOStat);
            // Only looking for BranchList here.
            if (Alphas(4) == BranchListName) {
                FoundAirLoopName = Alphas(1);
                FoundAir = "Air";
                FoundVolFlowRate = Numbers(1);
                FoundAirLoopNum = Num;
                MatchedAirLoop = true;
                break;
            }
        }

        Alphas.deallocate();
        Numbers.deallocate();
    }

    void FindAirPlantCondenserLoopFromBranchList(EnergyPlusData &state,
                                                 std::string const &BranchListName, // Branch List Name
                                                 std::string &LoopType,             // LoopType (if found, Plant,Condenser or Air)
                                                 std::string &LoopSupplyDemandAir,  // Supply if "Supply" or Demand if "Demand" or Air if "Air"
                                                 bool &MatchedLoop                  // true if found
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   February 2008

        // PURPOSE OF THIS SUBROUTINE:
        // Assist in validating Loop Splitter/Mixer connections.

        // METHODOLOGY EMPLOYED:
        // Call two previously written subroutines that match a Branch List Name to
        // Plant or Condenser Loop

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string FoundLoopName;
        int FoundLoopNum;
        Real64 FoundLoopVolFlowRate;

        LoopSupplyDemandAir = std::string();
        FoundLoopName = std::string();
        FoundLoopNum = 0;
        FoundLoopVolFlowRate = 0.0;
        MatchedLoop = false;
        LoopType = std::string();

        // Try Plant first
        FindPlantLoopBranchConnection(state, BranchListName, FoundLoopName, FoundLoopNum, LoopSupplyDemandAir, FoundLoopVolFlowRate, MatchedLoop);

        if (MatchedLoop) {
            LoopType = "Plant";
        }
        if (!MatchedLoop) { // Try Condenser Loop
            LoopSupplyDemandAir = std::string();
            FoundLoopName = std::string();
            FoundLoopNum = 0;
            FoundLoopVolFlowRate = 0.0;
            MatchedLoop = false;

            // Try Condenser
            FindCondenserLoopBranchConnection(
                state, BranchListName, FoundLoopName, FoundLoopNum, LoopSupplyDemandAir, FoundLoopVolFlowRate, MatchedLoop);
            if (MatchedLoop) {
                LoopType = "Condenser";
            }
        }

        if (!MatchedLoop) { // Try Air Loop
            LoopSupplyDemandAir = std::string();
            FoundLoopName = std::string();
            FoundLoopNum = 0;
            FoundLoopVolFlowRate = 0.0;
            MatchedLoop = false;

            // Try Air
            FindAirLoopBranchConnection(state, BranchListName, FoundLoopName, FoundLoopNum, LoopSupplyDemandAir, FoundLoopVolFlowRate, MatchedLoop);
            if (MatchedLoop) {
                LoopType = "Air";
            }
        }
    }

    //==================================================================================
    //   Routines that test branch integrity
    //==================================================================================

    void AuditBranches(EnergyPlusData &state,
                       bool const mustprint,                      // true if the warning should be printed.
                       ObjexxFCL::Optional_string_const CompType, // when mustprint (ScanPlantLoop)  use CompType in error message and scan
                       ObjexxFCL::Optional_string_const CompName  // when mustprint (ScanPlantLoop)  use CompName in error message and scan
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   November 2011

        // PURPOSE OF THIS SUBROUTINE:
        // This routine will point out any "dangling branches" that are not included on a BranchList.
        // Warnings are produced as the user might clutter up the input file with unused branches.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumDanglingCount; // when mustprint not true, count and report
        int BlNum;            // Branch List Counter
        int BrN;              // Branch Counter
        int CpN;              // Components on Branch
        bool NeverFound;

        NumDanglingCount = 0;
        NeverFound = true;
        for (BrN = 1; BrN <= (int)state.dataBranchInputManager->Branch.size(); ++BrN) {
            int Found = 0;
            std::string FoundBranchName = "";
            if (present(CompType) && present(CompName)) {
                for (CpN = 1; CpN <= state.dataBranchInputManager->Branch(BrN).NumOfComponents; ++CpN) {
                    if (!Util::SameString(CompType(), state.dataBranchInputManager->Branch(BrN).Component(CpN).CType) ||
                        !Util::SameString(CompName(), state.dataBranchInputManager->Branch(BrN).Component(CpN).Name)) {
                        continue;
                    }
                    FoundBranchName = state.dataBranchInputManager->Branch(BrN).Name;
                    NeverFound = false;
                }
            }
            for (BlNum = 1; BlNum <= (int)state.dataBranchInputManager->BranchList.size(); ++BlNum) {
                Found = Util::FindItemInList(state.dataBranchInputManager->Branch(BrN).Name,
                                             state.dataBranchInputManager->BranchList(BlNum).BranchNames,
                                             state.dataBranchInputManager->BranchList(BlNum).NumOfBranchNames);
                if (Found != 0) {
                    break;
                }
            }
            if (Found != 0) {
                continue;
            }
            ++NumDanglingCount;
            if (state.dataGlobal->DisplayExtraWarnings || mustprint) {
                if (mustprint) {
                    ShowContinueError(
                        state, format("AuditBranches: Branch=\"{}\" not found on any BranchLists.", state.dataBranchInputManager->Branch(BrN).Name));
                    if (!FoundBranchName.empty()) {
                        ShowContinueError(state, format("Branch contains component, type=\"{}\", name=\"{}\"", CompType, CompName));
                    }
                } else {
                    ShowSevereMessage(
                        state, format("AuditBranches: Branch=\"{}\" not found on any BranchLists.", state.dataBranchInputManager->Branch(BrN).Name));
                    ++state.dataErrTracking->TotalSevereErrors;
                }
            }
        }
        if (mustprint && NeverFound) { // this may be caught during branch input, not sure
            ShowContinueError(state, format("Component, type=\"{}\", name=\"{}\" was not found on any Branch.", CompType, CompName));
            ShowContinueError(state, "Look for mistyped branch or component names/types.");
        }
        if (!mustprint && NumDanglingCount > 0) {
            ShowSevereMessage(state, fmt::format("AuditBranches: There are {} branch(es) that do not appear on any BranchList.", NumDanglingCount));
            state.dataErrTracking->TotalSevereErrors += NumDanglingCount;
            ShowContinueError(state, "Use Output:Diagnostics,DisplayExtraWarnings; for detail of each branch not on a branch list.");
        }
    }

    void TestBranchIntegrity(EnergyPlusData &state, bool &ErrFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   November 2001

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine tests branch integrity and displays the loop for each branch.
        // Also, input and output nodes.

        // Using/Aliasing

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Loop;
        int Count;
        int MatchNode;                    // Node Number for match
        std::string MatchNodeName;        // Name for error message if not matched
        std::string BranchInletNodeName;  // Branch Inlet Node Name
        std::string BranchOutletNodeName; // Branch Outlet Node Name
        std::string BranchLoopName;       // Loop Name which Branch is part of
        std::string BranchLoopType;       // Loop Type which Branch is part of
        int NumErr;                       // Error Counter
        Array1D_bool BranchReported;
        int BCount;
        int Found;
        //  LOGICAL UniqueNodeError
        int Loop2;
        NodeFluidType BranchFluidType;
        int InitialBranchFluidNode;
        Array1D_int BranchFluidNodes;
        Array1D_int FoundBranches;
        Array1D_int BranchPtrs;
        std::string cBranchFluidType;
        int Ptr;
        int EndPtr;

        struct BranchUniqueNodes
        {
            int NumNodes{0};
            Array1D_string UniqueNodeNames;
        };

        // Object Data
        Array1D<BranchUniqueNodes> BranchNodes;

        // Formats

        BranchReported.dimension((int)state.dataBranchInputManager->Branch.size(), false);

        // Do by Branch Lists
        ShowMessage(state, "Testing Individual Branch Integrity");
        ErrFound = false;

        BranchNodes.allocate((int)state.dataBranchInputManager->Branch.size());

        print(state.files.bnd, "{}\n", "! ===============================================================");
        static constexpr std::string_view Format_700("! <#Branch Lists>,<Number of Branch Lists>");
        print(state.files.bnd, "{}\n", Format_700);
        print(state.files.bnd, " #Branch Lists,{}\n", (int)state.dataBranchInputManager->BranchList.size());
        static constexpr std::string_view Format_702(
            "! <Branch List>,<Branch List Count>,<Branch List Name>,<Loop Name>,<Loop Type>,<Number of Branches>");
        print(state.files.bnd, "{}\n", Format_702);
        static constexpr std::string_view Format_704(
            "! <Branch>,<Branch Count>,<Branch Name>,<Loop Name>,<Loop Type>,<Branch Inlet Node Name>,<Branch Outlet Node Name>");
        print(state.files.bnd, "{}\n", Format_704);

        for (BCount = 1; BCount <= (int)state.dataBranchInputManager->BranchList.size(); ++BCount) {
            print(state.files.bnd,
                  " Branch List,{},{},{},{},{}\n",
                  BCount,
                  state.dataBranchInputManager->BranchList(BCount).Name,
                  state.dataBranchInputManager->BranchList(BCount).LoopName,
                  state.dataBranchInputManager->BranchList(BCount).LoopType,
                  state.dataBranchInputManager->BranchList(BCount).NumOfBranchNames);

            BranchFluidType = NodeFluidType::Blank;
            bool MixedFluidTypesOnBranchList = false;
            int NumNodesOnBranchList = 0;
            FoundBranches.allocate(state.dataBranchInputManager->BranchList(BCount).NumOfBranchNames);
            FoundBranches = 0;
            BranchPtrs.allocate(state.dataBranchInputManager->BranchList(BCount).NumOfBranchNames + 2);
            BranchPtrs = 0;
            for (Count = 1; Count <= state.dataBranchInputManager->BranchList(BCount).NumOfBranchNames; ++Count) {
                Found =
                    Util::FindItemInList(state.dataBranchInputManager->BranchList(BCount).BranchNames(Count), state.dataBranchInputManager->Branch);
                if (Found > 0) {
                    NumNodesOnBranchList += state.dataBranchInputManager->Branch(Found).NumOfComponents * 2;
                    FoundBranches(Count) = Found;
                    BranchPtrs(Count) = NumNodesOnBranchList;
                } else {
                    ShowSevereError(state, format("Branch not found={}", state.dataBranchInputManager->BranchList(BCount).BranchNames(Count)));
                    ErrFound = true;
                }
            }
            BranchPtrs(state.dataBranchInputManager->BranchList(BCount).NumOfBranchNames + 1) =
                BranchPtrs(state.dataBranchInputManager->BranchList(BCount).NumOfBranchNames) + 1;
            BranchFluidNodes.dimension(NumNodesOnBranchList, 0);
            std::string OriginalBranchFluidType = std::string();
            int NumFluidNodes = 0;
            for (Count = 1; Count <= state.dataBranchInputManager->BranchList(BCount).NumOfBranchNames; ++Count) {
                Found = FoundBranches(Count);
                if (Found == 0) {
                    print(state.files.bnd,
                          "   Branch,{},{},(not found),**Unknown**,**Unknown**,**Unknown**,**Unknown**\n",
                          Count,
                          state.dataBranchInputManager->BranchList(BCount).BranchNames(Count));
                    continue;
                }
                BranchReported(Found) = true;
                // Check Branch for connections

                MatchNode = 0;
                InitialBranchFluidNode = 0;
                if (state.dataBranchInputManager->Branch(Found).NumOfComponents > 0) {
                    MatchNode = state.dataBranchInputManager->Branch(Found).Component(1).InletNode;
                    MatchNodeName = state.dataBranchInputManager->Branch(Found).Component(1).InletNodeName;
                    BranchInletNodeName = state.dataBranchInputManager->Branch(Found).Component(1).InletNodeName;
                } else {
                    ShowWarningError(state, format("Branch has no components={}", state.dataBranchInputManager->Branch(Found).Name));
                }
                NumErr = 0;
                for (Loop = 1; Loop <= state.dataBranchInputManager->Branch(Found).NumOfComponents; ++Loop) {
                    if (BranchFluidType == DataLoopNode::NodeFluidType::Blank) {
                        ++NumFluidNodes;
                        BranchFluidNodes(NumFluidNodes) = state.dataBranchInputManager->Branch(Found).Component(Loop).InletNode;
                        BranchFluidType = state.dataLoopNodes->Node(state.dataBranchInputManager->Branch(Found).Component(Loop).InletNode).FluidType;
                        InitialBranchFluidNode = state.dataBranchInputManager->Branch(Found).Component(Loop).InletNode;
                        OriginalBranchFluidType = DataLoopNode::NodeFluidTypeNames[static_cast<int>(DataLoopNode::NodeFluidType::Blank)];
                    } else if (BranchFluidType !=
                                   state.dataLoopNodes->Node(state.dataBranchInputManager->Branch(Found).Component(Loop).InletNode).FluidType &&
                               state.dataLoopNodes->Node(state.dataBranchInputManager->Branch(Found).Component(Loop).InletNode).FluidType !=
                                   DataLoopNode::NodeFluidType::Blank) {
                        ++NumFluidNodes;
                        BranchFluidNodes(NumFluidNodes) = state.dataBranchInputManager->Branch(Found).Component(Loop).InletNode;
                        MixedFluidTypesOnBranchList = true;
                    } else {
                        ++NumFluidNodes;
                        BranchFluidNodes(NumFluidNodes) = state.dataBranchInputManager->Branch(Found).Component(Loop).InletNode;
                    }
                    if (BranchFluidType == DataLoopNode::NodeFluidType::Blank) {
                        ++NumFluidNodes;
                        BranchFluidNodes(NumFluidNodes) = state.dataBranchInputManager->Branch(Found).Component(Loop).InletNode;
                        BranchFluidType = state.dataLoopNodes->Node(state.dataBranchInputManager->Branch(Found).Component(Loop).OutletNode).FluidType;
                        InitialBranchFluidNode = state.dataBranchInputManager->Branch(Found).Component(Loop).OutletNode;
                        OriginalBranchFluidType = DataLoopNode::NodeFluidTypeNames[static_cast<int>(BranchFluidType)];
                    } else if (BranchFluidType !=
                                   state.dataLoopNodes->Node(state.dataBranchInputManager->Branch(Found).Component(Loop).OutletNode).FluidType &&
                               state.dataLoopNodes->Node(state.dataBranchInputManager->Branch(Found).Component(Loop).OutletNode).FluidType !=
                                   DataLoopNode::NodeFluidType::Blank) {
                        ++NumFluidNodes;
                        BranchFluidNodes(NumFluidNodes) = state.dataBranchInputManager->Branch(Found).Component(Loop).OutletNode;
                        MixedFluidTypesOnBranchList = true;
                    } else {
                        ++NumFluidNodes;
                        BranchFluidNodes(NumFluidNodes) = state.dataBranchInputManager->Branch(Found).Component(Loop).OutletNode;
                    }
                    if (state.dataBranchInputManager->Branch(Found).Component(Loop).InletNode != MatchNode) {
                        ShowSevereError(state, format("Error Detected in BranchList={}", state.dataBranchInputManager->BranchList(BCount).Name));
                        ShowContinueError(state, format("Actual Error occurs in Branch={}", state.dataBranchInputManager->Branch(Found).Name));
                        ShowContinueError(state, format("Branch Outlet does not match Inlet, Outlet={}", MatchNodeName));
                        ShowContinueError(state, format("Inlet Name={}", state.dataBranchInputManager->Branch(Found).Component(Loop).InletNodeName));
                        ErrFound = true;
                        ++NumErr;
                    } else {
                        MatchNode = state.dataBranchInputManager->Branch(Found).Component(Loop).OutletNode;
                        MatchNodeName = state.dataBranchInputManager->Branch(Found).Component(Loop).OutletNodeName;
                    }
                }
                state.dataBranchInputManager->Branch(Found).FluidType = BranchFluidType;
                BranchOutletNodeName = MatchNodeName;
                if (state.dataBranchInputManager->Branch(Found).AssignedLoopName.empty()) {
                    BranchLoopName = "**Unknown**";
                    BranchLoopType = "**Unknown**";
                } else if (state.dataBranchInputManager->Branch(Found).AssignedLoopName ==
                           state.dataBranchInputManager->BranchList(BCount).LoopName) {
                    BranchLoopName = state.dataBranchInputManager->BranchList(BCount).LoopName;
                    BranchLoopType = state.dataBranchInputManager->BranchList(BCount).LoopType;
                } else {
                    BranchLoopName = state.dataBranchInputManager->Branch(Found).AssignedLoopName;
                    BranchLoopType = "**Unknown**";
                }
                print(state.files.bnd,
                      "   Branch,{},{},{},{},{},{}\n",
                      Count,
                      state.dataBranchInputManager->Branch(Found).Name,
                      BranchLoopName,
                      BranchLoopType,
                      BranchInletNodeName,
                      BranchOutletNodeName);
            }
            if (MixedFluidTypesOnBranchList) {
                ShowSevereError(state,
                                format("BranchList={} has mixed fluid types in its nodes.", state.dataBranchInputManager->BranchList(BCount).Name));
                ErrFound = true;
                if (OriginalBranchFluidType.empty()) {
                    OriginalBranchFluidType = "**Unknown**";
                }
                ShowContinueError(
                    state, format("Initial Node={}, Fluid Type={}", state.dataLoopNodes->NodeID(InitialBranchFluidNode), OriginalBranchFluidType));
                ShowContinueError(state, "BranchList Topology - Note nodes which do not match that fluid type:");
                Ptr = 1;
                EndPtr = BranchPtrs(1);
                for (Loop = 1; Loop <= state.dataBranchInputManager->BranchList(BCount).NumOfBranchNames; ++Loop) {
                    if (FoundBranches(Loop) != 0) {
                        ShowContinueError(state, format("..Branch={}", state.dataBranchInputManager->Branch(FoundBranches(Loop)).Name));
                    } else {
                        ShowContinueError(state, format("..Illegal Branch={}", state.dataBranchInputManager->BranchList(BCount).BranchNames(Loop)));
                        continue;
                    }
                    for (Loop2 = Ptr; Loop2 <= EndPtr; ++Loop2) {
                        cBranchFluidType =
                            DataLoopNode::NodeFluidTypeNames[static_cast<int>(state.dataLoopNodes->Node(BranchFluidNodes(Loop2)).FluidType)];
                        if (cBranchFluidType.empty()) {
                            cBranchFluidType = "**Unknown**";
                        }
                        ShowContinueError(
                            state, format("....Node={}, Fluid Type={}", state.dataLoopNodes->NodeID(BranchFluidNodes(Loop2)), cBranchFluidType));
                    }
                    Ptr = EndPtr + 1;
                    EndPtr = BranchPtrs(Loop + 1);
                }
            }
            BranchFluidNodes.deallocate();
            BranchPtrs.deallocate();
            FoundBranches.deallocate();
        }

        // Build node names in branches
        for (Count = 1; Count <= (int)state.dataBranchInputManager->Branch.size(); ++Count) {
            BranchNodes(Count).UniqueNodeNames.allocate(state.dataBranchInputManager->Branch(Count).NumOfComponents * 2);
            int NodeNum = 0;
            for (Loop = 1; Loop <= state.dataBranchInputManager->Branch(Count).NumOfComponents; ++Loop) {
                Found = Util::FindItemInList(
                    state.dataBranchInputManager->Branch(Count).Component(Loop).InletNodeName, BranchNodes(Count).UniqueNodeNames, NodeNum);
                if (Found == 0) {
                    ++NodeNum;
                    BranchNodes(Count).UniqueNodeNames(NodeNum) = state.dataBranchInputManager->Branch(Count).Component(Loop).InletNodeName;
                }
                Found = Util::FindItemInList(
                    state.dataBranchInputManager->Branch(Count).Component(Loop).OutletNodeName, BranchNodes(Count).UniqueNodeNames, NodeNum);
                if (Found == 0) {
                    ++NodeNum;
                    BranchNodes(Count).UniqueNodeNames(NodeNum) = state.dataBranchInputManager->Branch(Count).Component(Loop).OutletNodeName;
                }
            }
            BranchNodes(Count).NumNodes = NodeNum;
        }
        // Check Uniqueness branch to branch
        for (Count = 1; Count <= (int)state.dataBranchInputManager->Branch.size(); ++Count) {
            for (Loop = Count + 1; Loop <= (int)state.dataBranchInputManager->Branch.size(); ++Loop) {
                for (Loop2 = 1; Loop2 <= BranchNodes(Count).NumNodes; ++Loop2) {
                    Found = Util::FindItemInList(
                        BranchNodes(Count).UniqueNodeNames(Loop2), BranchNodes(Loop).UniqueNodeNames, BranchNodes(Loop).NumNodes);
                    if (Found != 0) {
                        ShowSevereError(state, format("Non-unique node name found, name={}", BranchNodes(Count).UniqueNodeNames(Loop2)));
                        ShowContinueError(state, format("..1st occurrence in Branch={}", state.dataBranchInputManager->Branch(Count).Name));
                        ShowContinueError(state, format("..duplicate occurrence in Branch={}", state.dataBranchInputManager->Branch(Loop).Name));
                        ErrFound = true;
                    }
                }
            }
        }
        for (Count = 1; Count <= (int)state.dataBranchInputManager->Branch.size(); ++Count) {
            BranchNodes(Count).UniqueNodeNames.deallocate();
        }
        BranchNodes.deallocate();

        BCount = 0;
        for (Count = 1; Count <= (int)state.dataBranchInputManager->Branch.size(); ++Count) {
            if (BranchReported(Count)) {
                continue;
            }
            ++BCount;
        }
        if (BCount > 0) {
            static constexpr std::string_view Format_706("! <# Orphaned Branches>,<Number of Branches not on Branch Lists>");
            print(state.files.bnd, "{}\n", Format_706);
            print(state.files.bnd, " #Orphaned Branches,{}\n", BCount);
            ShowWarningError(state, "There are orphaned Branches in input. See .bnd file for details.");

            BCount = 0;

            for (Count = 1; Count <= (int)state.dataBranchInputManager->Branch.size(); ++Count) {
                if (BranchReported(Count)) {
                    continue;
                }
                ++BCount;
                ShowWarningError(state, format("Orphan Branch=\"{}\".", state.dataBranchInputManager->Branch(Count).Name));

                if (state.dataBranchInputManager->Branch(Count).NumOfComponents > 0) {
                    MatchNode = state.dataBranchInputManager->Branch(Count).Component(1).InletNode;
                    MatchNodeName = state.dataBranchInputManager->Branch(Count).Component(1).InletNodeName;
                    BranchInletNodeName = state.dataBranchInputManager->Branch(Count).Component(1).InletNodeName;
                } else {
                    ShowWarningError(state, format("Branch has no components={}", state.dataBranchInputManager->Branch(Count).Name));
                }
                NumErr = 0;
                for (Loop = 1; Loop <= state.dataBranchInputManager->Branch(Count).NumOfComponents; ++Loop) {
                    if (state.dataBranchInputManager->Branch(Count).Component(Loop).InletNode != MatchNode) {
                        ShowSevereError(state, format("Error Detected in Branch={}", state.dataBranchInputManager->Branch(Count).Name));
                        ShowContinueError(state, format("Branch Outlet does not match Inlet, Outlet={}", MatchNodeName));
                        ShowContinueError(state, format("Inlet Name={}", state.dataBranchInputManager->Branch(Count).Component(Loop).InletNodeName));
                        ErrFound = true;
                        ++NumErr;
                    } else {
                        MatchNode = state.dataBranchInputManager->Branch(Count).Component(Loop).OutletNode;
                        MatchNodeName = state.dataBranchInputManager->Branch(Count).Component(Loop).OutletNodeName;
                    }
                }
                BranchOutletNodeName = MatchNodeName;
                if (state.dataBranchInputManager->Branch(Count).AssignedLoopName.empty()) {
                    BranchLoopName = "**Unknown**";
                    BranchLoopType = "**Unknown**";
                } else {
                    BranchLoopName = state.dataBranchInputManager->Branch(Count).AssignedLoopName;
                    BranchLoopType = "**Unknown**";
                }
                print(state.files.bnd,
                      " Branch,{},{},{},{},{},{}\n",
                      BCount,
                      state.dataBranchInputManager->Branch(Count).Name,
                      BranchLoopName,
                      BranchLoopType,
                      BranchInletNodeName,
                      BranchOutletNodeName);
            }
        }

        if (ErrFound) {
            ShowSevereError(state, "Branch(es) did not pass integrity testing");
        } else {
            ShowMessage(state, "All Branches passed integrity testing");
        }
    }

} // namespace BranchInputManager

} // namespace EnergyPlus
