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

#ifndef DataAirSystems_hh_INCLUDED
#define DataAirSystems_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHVACSystems.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/SimAirServingZones.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace DataAirSystems {

    // Using/Aliasing
    using DataPlant::SubcomponentData;
    using OutputProcessor::MeterData;

    // Data
    // MODULE PARAMETER DEFINITIONS:
    // DERIVED TYPE DEFINITIONS

    // DefinePrimaryAirSystem contains the data for a primary air HVAC system

    // The ConnectionPoint derived type is used to link quickly between loops at connection points
    // and avoids the need for repetitive searches.

    // INTERFACE BLOCK SPECIFICATIONS
    // None

    // MODULE VARIABLE DECLARATIONS
    // For each type of air path, define an array of DefineAirPaths

    // Temporary arrays

    // Types
    struct AirLoopCompData // data for an individual component
    {
        // Members
        std::string TypeOf;                                                                // The 'keyWord' identifying  component type
        std::string Name;                                                                  // Component name
        SimAirServingZones::CompType CompType_Num = SimAirServingZones::CompType::Invalid; // Numeric designator for CompType (TypeOf)
        int CompIndex = 0;                                                                 // Component Index in whatever is using this component
        HVACSystemData *compPointer = nullptr;                                             // pointer to HVAC system
        int FlowCtrl = 0;                                                                  // Component flow control (ACTIVE/PASSIVE)
        bool ON = true;          // When true, the designated component or operation scheme is available
        bool Parent = false;     // When true, the designated component is made up of sub-components
        std::string NodeNameIn;  // Component inlet node name
        std::string NodeNameOut; // Component outlet node name
        int NodeNumIn = 0;       // Component inlet node number
        int NodeNumOut = 0;      // Component outlet node number
        bool MeteredVarsFound = false;
        int NumMeteredVars = 0;
        int NumSubComps = 0;
        int EnergyTransComp = 0; // 1=EnergyTransfer, 0=No EnergyTransfer  Flag needed for reporting
        Real64 Capacity = 0.0;   // ventilation load factor
        int OpMode = 0;
        Real64 TotPlantSupplyElec = 0.0;
        Real64 PlantSupplyElecEff = 0.0;
        Real64 PeakPlantSupplyElecEff = 0.0;
        Real64 TotPlantSupplyGas = 0.0;
        Real64 PlantSupplyGasEff = 0.0;
        Real64 PeakPlantSupplyGasEff = 0.0;
        Real64 TotPlantSupplyPurch = 0.0;
        Real64 PlantSupplyPurchEff = 0.0;
        Real64 PeakPlantSupplyPurchEff = 0.0;
        Real64 TotPlantSupplyOther = 0.0;
        Real64 PlantSupplyOtherEff = 0.0;
        Real64 PeakPlantSupplyOtherEff = 0.0;
        int AirSysToPlantPtr = 0;          // =0 No plant loop connection, >0 index to AirSysToPlant array
        Array1D<MeterData> MeteredVar;     // Index of energy output report data
        Array1D<SubcomponentData> SubComp; // Component list
    };

    struct AirLoopBranchData // a branch is a sequence of components
    {
        // Members
        std::string Name;                                        // Name of the branch
        std::string ControlType;                                 // Control type for the branch (not used)
        int TotalComponents = 0;                                 // Total number of high level components on the branch
        Array1D_int FirstCompIndex;                              // Gives the component index in AllComp that corresponds to Comp
        Array1D_int LastCompIndex;                               // Gives comp index in AllComp that corresponds to last subcomponent
        int NodeNumIn = 0;                                       // Branch inlet node number
        int NodeNumOut = 0;                                      // Branch outlet node number
        HVAC::AirDuctType DuctType = HVAC::AirDuctType::Invalid; // 1=main, 2=cooling, 3=heating, 4=other
        Array1D<AirLoopCompData> Comp;                           // Component list--high level components
        //  This list would include children, grandchildren, etc.
        int TotalNodes = 0;  // total number of nodes on branch
        Array1D_int NodeNum; // node list (numbers)
    };

    struct AirLoopSplitterData // a splitter joins 1 inlet branch to multiple outlet branches
    {
        // Members
        bool Exists = false;        // True if there is a splitter (only 1 allowed per loop)
        std::string Name;           // Name of the Splitter
        int NodeNumIn = 0;          // Node number for the inlet to the splitter
        int BranchNumIn = 0;        // Reference number for branch connected to splitter inlet
        std::string NodeNameIn;     // Node name for the inlet to the splitter
        int TotalOutletNodes = 0;   // Number of outlet nodes for the splitter
        Array1D_int NodeNumOut;     // Node numbers for the outlets to the splitter
        Array1D_int BranchNumOut;   // Reference numbers for branches connected to splitter outlet
        Array1D_string NodeNameOut; // Node names for the outlets to the splitter
    };

    struct AirLoopMixerData // a mixer joins multiple inlet branches to a single outlet branch
    {
        // Members
        bool Exists = false;       // True if there is a Mixer (only 1 allowed per loop)
        std::string Name;          // Name of the Mixer
        int NodeNumOut = 0;        // Node number for the outlet to the mixer
        int BranchNumOut = 0;      // Reference number for branch connected to mixer outlet
        std::string NodeNameOut;   // Node name for the outlet to the mixer
        int TotalInletNodes = 0;   // Number of inlet nodes for the mixer
        Array1D_int NodeNumIn;     // Node numbers for the inlets to the mixer
        Array1D_int BranchNumIn;   // Reference numbers for branches connected to mixer inlet
        Array1D_string NodeNameIn; // Node names for the inlets to the mixer
    };

    struct DefinePrimaryAirSystem // There is an array of these for each primary air system
    {
        // Members
        std::string Name;                      // name of the system
        Real64 DesignVolFlowRate;              // the design total supply air flow rate (m3/s)
        Real64 DesignReturnFlowFraction = 1.0; // the design return flow rate as a fraction of supply flow assuming no exhaust (0 to 1)
        int NumControllers = 0;                // number of controllers on this air path
        Array1D_string ControllerName;         // name of each controller on this system
        Array1D_string ControllerType;         // type of each controller on this system
        Array1D_int ControllerIndex;
        Array1D_bool CanBeLockedOutByEcono; // true if controller inactive
        // when the economizer is active
        int NumBranches = 0;               // number of branches making up this system
        Array1D<AirLoopBranchData> Branch; // data for each branch
        AirLoopSplitterData Splitter;      // Data for splitter (if any)
        AirLoopMixerData Mixer;            // Data for mixer (if any)
        Array1D_bool ControlConverged;     // Convergence Parameter for controllers
        int NumOutletBranches = 0;
        std::array<int, 3> OutletBranchNum = {0}; // airloop branch numbers of system outlets (not the actual branch index)
        int NumInletBranches = 0;
        std::array<int, 3> InletBranchNum = {0}; // branch number of system inlets
        bool CentralHeatCoilExists = true;       // true if there are central heating coils
        bool CentralCoolCoilExists = true;       // true if there are central cooling coils
        bool OASysExists = false;                // true if there is an Outside Air Sys
        bool isAllOA = false;                    // true if there is no return path and the main branch inlet is an outdoor air node
        int OASysInletNodeNum = 0;               // node number of return air inlet to OA sys
        int OASysOutletNodeNum = 0;              // node number of mixed air outlet of OA sys
        int OAMixOAInNodeNum = 0;                // node number of the OA stream inlet to the
        // OA mixer component.
        bool RABExists = false;                            // true if there is a RAB
        int RABMixInNode = 0;                              // node num of RAB mixer inlet
        int SupMixInNode = 0;                              // node num of supply air inlet to mixer
        int MixOutNode = 0;                                // outlet node of mixer
        int RABSplitOutNode = 0;                           // node num of RAB splitter outlet
        int OtherSplitOutNode = 0;                         // node num of nonRAB splitter outlet
        int NumOACoolCoils = 0;                            // number of cooling coils in the outside air system
        int NumOAHeatCoils = 0;                            // number of heating coils in the outside air system
        int NumOAHXs = 0;                                  // number of heat exchangers in the outside air system
        bool SizeAirloopCoil = true;                       // simulates air loop coils before calling controllers
        HVAC::FanType supFanType = HVAC::FanType::Invalid; // indicates which type of fan model to call for supply fan, legacy or new OO
        int supFanNum = 0; // index of the supply fan in the Fan data structure when model type is StructArrayLegacyFanModels
        HVAC::FanPlace supFanPlace = HVAC::FanPlace::Invalid; // location of fan relative to coil
        HVAC::FanType retFanType = HVAC::FanType::Invalid;    // indicates which type of fan model to call for return fan, legacy or new OO
        int retFanNum = 0;                       // index of the return fan in the Fan data structure when model type is StructArrayLegacyFanModels
        Real64 FanDesCoolLoad = 0.0;             // design fan heat gain for the air loop [W]
        bool EconomizerStagingCheckFlag = false; // flag to indicate that the applicability of the selected economizer staging operation mode is valid
    };

    struct ConnectionPoint
    {
        // Members
        int LoopType = 0;
        int LoopNum = 0;
        int BranchNum = 0;
        int CompNum = 0;
    };

    struct ConnectZoneComp
    {
        // Members
        int ZoneEqListNum = 0;
        int ZoneEqCompNum = 0;
        int PlantLoopType = 0;
        int PlantLoopNum = 0;
        int PlantLoopBranch = 0;
        int PlantLoopComp = 0;
        int FirstDemandSidePtr = 0;
        int LastDemandSidePtr = 0;
    };

    struct ConnectZoneSubComp
    {
        // Members
        int ZoneEqListNum = 0;
        int ZoneEqCompNum = 0;
        int ZoneEqSubCompNum = 0;
        int PlantLoopType = 0;
        int PlantLoopNum = 0;
        int PlantLoopBranch = 0;
        int PlantLoopComp = 0;
        int FirstDemandSidePtr = 0;
        int LastDemandSidePtr = 0;
    };

    struct ConnectZoneSubSubComp
    {
        // Members
        int ZoneEqListNum = 0;
        int ZoneEqCompNum = 0;
        int ZoneEqSubCompNum = 0;
        int ZoneEqSubSubCompNum = 0;
        int PlantLoopType = 0;
        int PlantLoopNum = 0;
        int PlantLoopBranch = 0;
        int PlantLoopComp = 0;
        int FirstDemandSidePtr = 0;
        int LastDemandSidePtr = 0;
    };

    struct ConnectAirSysComp
    {
        // Members
        int AirLoopNum = 0;
        int AirLoopBranch = 0;
        int AirLoopComp = 0;
        int PlantLoopType = 0;
        int PlantLoopNum = 0;
        int PlantLoopBranch = 0;
        int PlantLoopComp = 0;
        int FirstDemandSidePtr = 0;
        int LastDemandSidePtr = 0;
    };

    struct ConnectAirSysSubComp
    {
        // Members
        int AirLoopNum = 0;
        int AirLoopBranch = 0;
        int AirLoopComp = 0;
        int AirLoopSubComp = 0;
        int PlantLoopType = 0;
        int PlantLoopNum = 0;
        int PlantLoopBranch = 0;
        int PlantLoopComp = 0;
        int FirstDemandSidePtr = 0;
        int LastDemandSidePtr = 0;
    };

    struct ConnectAirSysSubSubComp
    {
        // Members
        int AirLoopNum = 0;
        int AirLoopBranch = 0;
        int AirLoopComp = 0;
        int AirLoopSubComp = 0;
        int AirLoopSubSubComp = 0;
        int PlantLoopType = 0;
        int PlantLoopNum = 0;
        int PlantLoopBranch = 0;
        int PlantLoopComp = 0;
        int FirstDemandSidePtr = 0;
        int LastDemandSidePtr = 0;
    };

    Real64 calcFanDesignHeatGain(EnergyPlusData &state, int dataFanIndex, Real64 desVolFlow);

} // namespace DataAirSystems

struct AirSystemsData : BaseGlobalStruct
{

    EPVector<DataAirSystems::DefinePrimaryAirSystem> PrimaryAirSystems;
    Array1D<DataAirSystems::ConnectionPoint> DemandSideConnect;               // Connections between loops
    Array1D<DataAirSystems::ConnectZoneComp> ZoneCompToPlant;                 // Connections between loops
    Array1D<DataAirSystems::ConnectZoneSubComp> ZoneSubCompToPlant;           // Connections between loops
    Array1D<DataAirSystems::ConnectZoneSubSubComp> ZoneSubSubCompToPlant;     // Connections between loops
    Array1D<DataAirSystems::ConnectAirSysComp> AirSysCompToPlant;             // Connections between loops
    Array1D<DataAirSystems::ConnectAirSysSubComp> AirSysSubCompToPlant;       // Connections between loops
    Array1D<DataAirSystems::ConnectAirSysSubSubComp> AirSysSubSubCompToPlant; // Connections between loops

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) AirSystemsData();
    }
};

} // namespace EnergyPlus

#endif
