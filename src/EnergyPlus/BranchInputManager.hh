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

#ifndef BranchInputManager_hh_INCLUDED
#define BranchInputManager_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace BranchInputManager {

    struct ConnectorData
    {
        // Members
        std::string Name;             // Name for this Connector
        int NumOfConnectors = 0;      // Number of Connectors in this group
        int NumOfSplitters = 0;       // Number of Splitters in this connector group
        int NumOfMixers = 0;          // Number of Mixers in this connector group
        Array1D_string ConnectorType; // Connector:Splitter or Connector:Mixer
        Array1D_string ConnectorName; // Name for that Connector:Splitter or Connector:Mixer
        Array1D_int ConnectorMatchNo; // Pointer to index where this Splitter or Mixer matches
        // Splitter => Mixer or Mixer => Splitter.  0 indicates no match
    };

    struct BranchListData
    {
        // Members
        std::string Name;           // Name of this Branch List
        int NumOfBranchNames = 0;   // Number of Branches on the Branch List
        Array1D_string BranchNames; // Names of the branches on this branch list
        std::string LoopName;       // Name of Loop this Branch list belongs to
        std::string LoopType;       // Loop type this branch is on
    };

    struct ComponentData
    {
        // Members
        std::string CType;          // Component Type (Cannot be SPLITTER or MIXER)
        std::string Name;           // Component Name
        int CtrlType = 0;           // Active, Passive, Bypass (1,2,3)
        std::string InletNodeName;  // Inlet Node ID
        int InletNode = 0;          // Inlet Node Number
        std::string OutletNodeName; // Outlet Node ID
        int OutletNode = 0;         // Outlet Node Number
    };

    struct BranchData
    {
        // Members
        std::string Name;             // Name for this Branch
        std::string AssignedLoopName; // Loop Name for this branch
        DataBranchAirLoopPlant::PressureCurveType PressureCurveType =
            DataBranchAirLoopPlant::PressureCurveType::Invalid; // Integer index of pressure curve type
        int PressureCurveIndex = 0;                             // Integer index of pressure curve
        Node::FluidType FluidType = Node::FluidType::Blank;     // Fluid type (see DataLoopNode)
        int NumOfComponents = 0;                                // Number of Components on this Branch
        Array1D<ComponentData> Component;                       // Component definitions for each component
    };

    struct SplitterData
    {
        // Members
        std::string Name;                 // Splitter Name
        std::string InletBranchName;      // Splitter Inlet Branch Name
        int NumOutletBranches = 0;        // Number of outlets on this Splitter
        Array1D_string OutletBranchNames; // Names of the Outlet Branches
    };

    struct MixerData
    {
        // Members
        std::string Name;                // Mixer Name
        std::string OutletBranchName;    // Mixer Outlet Branch Name
        int NumInletBranches = 0;        // Number of inlets for this Mixer
        Array1D_string InletBranchNames; // Names of Inlet Branches
    };

    // Functions
    void ManageBranchInput(EnergyPlusData &state);
    void ManageConnectorInput(EnergyPlusData &state);

    //==================================================================================
    //   Routines that "get" data from internal branch management structure
    //==================================================================================

    void GetBranchList(EnergyPlusData &state,
                       std::string const &LoopName,       // Name of Loop Branch List is on
                       std::string const &BranchListName, // Branch List Name from Input
                       int &NumBranchNames,               // Number of Branches for this Branch List
                       Array1D_string &BranchNames,       // Names of Branches on this Branch List
                       std::string const &LoopType        // Type of Loop Branch list is on
    );

    int NumBranchesInBranchList(EnergyPlusData &state, std::string const &BranchListName);

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
                       bool &ErrorsFound);

    int NumCompsInBranch(EnergyPlusData &state, std::string const &BranchName);

    int GetAirBranchIndex(EnergyPlusData &state, std::string const &CompType, std::string_view CompName);

    void GetBranchFanTypeName(EnergyPlusData &state,
                              int BranchNum,
                              std::string &FanType,
                              std::string &FanName,
                              bool &ErrFound // Set to true if error found, false otherwise
    );

    void GetInternalBranchData(EnergyPlusData &state,
                               std::string const &LoopName,                               // Loop Name for Branch
                               std::string const &BranchName,                             // Requested Branch Name
                               DataBranchAirLoopPlant::PressureCurveType &PressCurveType, // Index of pressure curve object
                               int &PressCurveIndex,                                      // Index of pressure curve object
                               int &NumComps,                                             // Number of Components on Branch
                               Array1D<ComponentData> const &BComponents,                 // Component data returned
                               bool &ErrorsFound // True when Loop Name is already assigned and this not same loop
    );

    void GetNumSplitterMixerInConntrList(EnergyPlusData &state,
                                         std::string const &LoopName,          // Loop Name for this Splitter (used in error message)
                                         std::string const &ConnectorListName, // Requested Connector List Name
                                         int &numSplitters,                    // Number of splitters in the loop
                                         int &numMixers,                       // Number of mixers in the loop
                                         bool &ErrorsFound                     // if no connector list
    );

    void GetConnectorList(EnergyPlusData &state,
                          std::string const &ConnectorListName,       // Requested Connector List
                          ConnectorData &Connectoid,                  // Returned Connector Data
                          ObjexxFCL::Optional_int_const NumInList = _ // Number of the current connector in the list of connectors
    );

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
                      ObjexxFCL::Optional_int_const ConnectorNumber = _, // number of the current item in connector list
                      ObjexxFCL::Optional_int MixerNumber = _            // Mixer number for this specific splitter
    );

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
                         ObjexxFCL::Optional_int_const ConnectorNumber = _, // number of the current item in connector list
                         ObjexxFCL::Optional_int SplitterNumber = _         // splitter number for this specific splitter
    );

    std::string GetFirstBranchInletNodeName(EnergyPlusData &state, std::string const &BranchListName); // Branch List name to search

    std::string GetLastBranchOutletNodeName(EnergyPlusData &state, std::string const &BranchListName); // Branch List name to search

    //==================================================================================
    //   Routines that get the input for the internal branch management structure
    //==================================================================================

    void GetBranchInput(EnergyPlusData &state);

    void GetSingleBranchInput(EnergyPlusData &state,
                              std::string_view RoutineName,
                              int BCount,
                              Array1D_string const &Alphas,
                              Array1D_string const &cAlphaFields,
                              int NumAlphas,
                              Array1D_int &NodeNums,
                              Array1D_bool const &lAlphaBlanks);

    void GetBranchListInput(EnergyPlusData &state);

    void GetConnectorListInput(EnergyPlusData &state);

    void GetSplitterInput(EnergyPlusData &state);

    void GetMixerInput(EnergyPlusData &state);

    void FindPlantLoopBranchConnection(EnergyPlusData &state,
                                       std::string const &BranchListName,
                                       std::string &FoundPlantLoopName,
                                       int &FoundPlantLoopNum,
                                       std::string &FoundSupplyDemand,
                                       Real64 &FoundVolFlowRate,
                                       bool &MatchedPlantLoop);

    void FindCondenserLoopBranchConnection(EnergyPlusData &state,
                                           std::string const &BranchListName,
                                           std::string &FoundCondLoopName,
                                           int &FoundCondLoopNum,
                                           std::string &FoundSupplyDemand,
                                           Real64 &FoundVolFlowRate,
                                           bool &MatchedCondLoop);

    void FindAirLoopBranchConnection(EnergyPlusData &state,
                                     std::string const &BranchListName,
                                     std::string &FoundAirLoopName,
                                     int &FoundAirLoopNum,
                                     std::string &FoundAir,
                                     Real64 &FoundVolFlowRate,
                                     bool &MatchedAirLoop);

    void FindAirPlantCondenserLoopFromBranchList(EnergyPlusData &state,
                                                 std::string const &BranchListName, // Branch List Name
                                                 std::string &LoopType,             // LoopType (if found, Plant,Condenser or Air)
                                                 std::string &LoopSupplyDemandAir,  // Supply if "Supply" or Demand if "Demand" or Air if "Air"
                                                 bool &MatchedLoop                  // true if found
    );

    //==================================================================================
    //   Routines that test branch integrity
    //==================================================================================

    void AuditBranches(EnergyPlusData &state,
                       bool mustprint,                                // true if the warning should be printed.
                       ObjexxFCL::Optional_string_const CompType = _, // when mustprint (ScanPlantLoop)  use CompType in error message and scan
                       ObjexxFCL::Optional_string_const CompName = _  // when mustprint (ScanPlantLoop)  use CompName in error message and scan
    );

    void TestBranchIntegrity(EnergyPlusData &state, bool &ErrFound); // ErrFound is a return value, true or false

} // namespace BranchInputManager

struct BranchInputManagerData : BaseGlobalStruct
{
    bool GetBranchInputFlag = true;        // Flag used to retrieve Input
    bool GetBranchListInputFlag = true;    // Flag used to retrieve Input
    bool GetSplitterInputFlag = true;      // Flag used to retrieve Input
    bool GetMixerInputFlag = true;         // Flag used to retrieve Input
    bool GetConnectorListInputFlag = true; // Flag used to retrieve Input
    bool InvalidBranchDefinitions = false;
    bool GetBranchInputOneTimeFlag = true;

    Array1D<BranchInputManager::BranchListData> BranchList;    // Branch List data for each Branch List
    Array1D<BranchInputManager::BranchData> Branch;            // Branch Data for each Branch
    Array1D<BranchInputManager::ConnectorData> ConnectorLists; // Connector List data for each Connector List
    Array1D<BranchInputManager::SplitterData> Splitters;       // Splitter Data for each Splitter
    Array1D<BranchInputManager::MixerData> Mixers;             // Mixer Data for each Mixer
    Array1D<BranchInputManager::ComponentData> BComponents;    // Component data to be returned

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) BranchInputManagerData();
    }
};

} // namespace EnergyPlus

#endif
