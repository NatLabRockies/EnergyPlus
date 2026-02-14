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

#ifndef DataBranchNodeConnections_hh_INCLUDED
#define DataBranchNodeConnections_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/NodeInputManager.hh>

namespace EnergyPlus {

namespace DataBranchNodeConnections {

    struct ComponentListData
    {
        // Members
        // std::string ParentCType; // Parent Object Type (Cannot be SPLITTER or MIXER)
        DataLoopNode::ConnectionObjectType ParentObjectType = DataLoopNode::ConnectionObjectType::Invalid;
        std::string ParentCName; // Parent Object Name
        // std::string CType;       // Component Type (Cannot be SPLITTER or MIXER)
        DataLoopNode::ConnectionObjectType ComponentObjectType = DataLoopNode::ConnectionObjectType::Invalid;
        std::string CName;          // Component Name
        std::string InletNodeName;  // Inlet Node ID
        std::string OutletNodeName; // Outlet Node ID
        std::string Description;    // Description of Component List Type
    };

    struct NodeConnectionDef
    {
        // Members
        int NodeNumber = 0;                                                                          // Node number of this node connection
        std::string NodeName;                                                                        // Node Name of this node connection
        DataLoopNode::ConnectionObjectType ObjectType = DataLoopNode::ConnectionObjectType::Invalid; // Object/Component Type of this node connection
        std::string ObjectName; // Name of the Object/Component Type of this node connection
        DataLoopNode::ConnectionType ConnectionType =
            DataLoopNode::ConnectionType::Invalid; // Connection Type (must be valid) for this node connection
        NodeInputManager::CompFluidStream FluidStream = NodeInputManager::CompFluidStream::Invalid; // Fluid Stream for this node connection
        bool ObjectIsParent = false;                                                                // Indicator whether the object is a parent or not
    };

    struct ParentListData
    {
        // Members
        DataLoopNode::ConnectionObjectType ComponentType =
            DataLoopNode::ConnectionObjectType::Invalid; // Component Type (Cannot be SPLITTER or MIXER)
        std::string ComponentName;                       // Component Name
        std::string InletNodeName;                       // Inlet Node ID
        std::string OutletNodeName;                      // Outlet Node ID
        std::string Description;                         // Description of Component List Type
    };

    struct EqNodeConnectionDef
    {
        // Members
        std::string NodeName;                                                                        // Node Name of this node connection
        DataLoopNode::ConnectionObjectType ObjectType = DataLoopNode::ConnectionObjectType::Invalid; // Object/Component Type of this node connection
        std::string ObjectName;     // Name of the Object/Component Type of this node connection
        std::string InputFieldName; // Input Field Name for this connection
        DataLoopNode::ConnectionType ConnectionType =
            DataLoopNode::ConnectionType::Invalid; // Connection Type (must be valid) for this node connection
    };

} // namespace DataBranchNodeConnections

struct BranchNodeConnectionsData : BaseGlobalStruct
{

    int NumCompSets = 0;             // Number of Component Sets found in branches
    int NumNodeConnectionErrors = 0; // Count of node connection errors
    int NumOfNodeConnections = 0;
    int MaxNumOfNodeConnections = 0;
    int NumOfActualParents = 0;
    int NumOfAirTerminalNodes = 0;
    int MaxNumOfAirTerminalNodes = 0;

    EPVector<DataBranchNodeConnections::ComponentListData> CompSets;
    EPVector<DataBranchNodeConnections::ParentListData> ParentNodeList;
    EPVector<DataBranchNodeConnections::NodeConnectionDef> NodeConnections;
    EPVector<DataBranchNodeConnections::EqNodeConnectionDef> AirTerminalNodeConnections;
    Array1D_bool NonConnectedNodes;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) BranchNodeConnectionsData();
    }
};

} // namespace EnergyPlus

#endif
