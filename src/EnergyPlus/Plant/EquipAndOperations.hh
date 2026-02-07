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

#ifndef PlantOperationEquipAndOperations_hh_INCLUDED
#define PlantOperationEquipAndOperations_hh_INCLUDED

#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>
#include <EnergyPlus/PlantLoopHeatPumpEIR.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <ObjexxFCL/Array1D.hh>

namespace EnergyPlus::DataPlant {

struct EquipListPtrData
{
    // Members
    int ListPtr; // points to List on OpScheme on plant loop:
    int CompPtr; // points to this component on List on OpScheme on plant loop:

    // Default Constructor
    EquipListPtrData() : ListPtr(0), CompPtr(0)
    {
    }
};

struct OpSchemePtrData
{
    // Members
    int OpSchemePtr;                     // points to OpScheme on plant loop:
    int NumEquipLists;                   // ALLOCATABLE to the schedule (for valid schedules)
    Array1D<EquipListPtrData> EquipList; // Component  list

    // Default Constructor
    OpSchemePtrData() : OpSchemePtr(0), NumEquipLists(0)
    {
    }
};

struct EquipListCompData
{
    // Members
    std::string Name;                           // The name of each item in the list
    std::string TypeOf;                         // The name of each item in the list
    DataPlant::CtrlType CtrlType;               // CoolingOp, HeatingOp, DualOp
    int LoopNumPtr;                             // pointer to the comp location in the data structure
    DataPlant::LoopSideLocation LoopSideNumPtr; // pointer to the comp location in the data structure
    int BranchNumPtr;                           // pointer to the comp location in the data structure
    int CompNumPtr;                             // pointer to the comp location in the data structure
    Real64 SetPointFlowRate;                    // COMP SETPOINT CTRL ONLY--load calculation comp flow rate
    bool SetPointFlowRateWasAutosized;          // true if comp setpoint control flow rate was autosize on input (not used)
    std::string DemandNodeName;                 // COMP SETPOINT CTRL ONLY--The name of each item in the list
    int DemandNodeNum;                          // COMP SETPOINT CTRL ONLY--The 'keyWord' identifying each item in list
    std::string SetPointNodeName;               // COMP SETPOINT CTRL ONLY--The name of each item in the list
    int SetPointNodeNum;                        // COMP SETPOINT CTRL ONLY--The 'keyWord' identifying each item in list
    Real64 EMSIntVarRemainingLoadValue;         // EMS internal variable remaining load, neg cooling [W]
    Real64 EMSActuatorDispatchedLoadValue;      // EMS actuator for dispatched load, neg= cooling [W]

    // Default Constructor
    EquipListCompData()
        : CtrlType(DataPlant::CtrlType::Invalid), LoopNumPtr(0), LoopSideNumPtr(DataPlant::LoopSideLocation::Invalid), BranchNumPtr(0), CompNumPtr(0),
          SetPointFlowRate(0.0), SetPointFlowRateWasAutosized(false), DemandNodeNum(0), SetPointNodeNum(0), EMSIntVarRemainingLoadValue(0.0),
          EMSActuatorDispatchedLoadValue(0.0)
    {
    }
};

struct EquipOpList
{
    // Members
    std::string Name;                // The name of each item in the list
    Real64 RangeUpperLimit;          // for range based controls
    Real64 RangeLowerLimit;          // for range based controls
    int NumComps;                    // ALLOCATABLE to the schedule (for valid schedules)
    Array1D<EquipListCompData> Comp; // Component type list

    // Default Constructor
    EquipOpList() : RangeUpperLimit(0.0), RangeLowerLimit(0.0), NumComps(0)
    {
    }
};

struct TempSetpoint
{
    Real64 PrimCW = 0.0;              // Chilled water setpoint for primary plant loop
    Real64 SecCW = 0.0;               // Chilled water setpoint for secondary/distribution plant loop
    Real64 PrimHW_High = 0.0;         // Hot water primary plant setpoint at High Outdoor Air Temperature, or higher, Deg. C
    Real64 PrimHW_Low = 0.0;          // Hot water primary plant setpoint at Low Outdoor Air Temperature, or Lower, Deg. C
    Real64 SecHW = 0.0;               // hot water setpoint for secondary/distribution plant loop
    Real64 PrimHW_BackupLow = -999.0; // optional hot water setpoint at Backup Low Outdoor Air Temperature, or lower, Deg. C
};

struct TempResetData
{
    Real64 HighOutdoorTemp = 0.0;
    Real64 LowOutdoorTemp = 0.0;
    Real64 BackupLowOutdoorTemp = -999.0; // optional back up low outdoor air temperature, Deg.C
    Real64 BoilerTemperatureOffset = 0.0;
};

struct PlantOpsData
{
    int NumOfZones = 0;           // Number of zones in the list
    int NumOfAirLoops = 0;        // number of air loops
    int numPlantLoadProfiles = 0; // number of load profiles
    int numBoilers = 0;           // number of boilers
    int numPlantHXs = 0;          // number of fluid to fluid heat exchangers
    int NumHeatingOnlyEquipLists = 0;
    int NumCoolingOnlyEquipLists = 0;
    int NumSimultHeatCoolHeatingEquipLists = 0;
    int NumSimultHeatCoolCoolingEquipLists = 0;
    int EquipListNumForLastCoolingOnlyStage = 0;
    int EquipListNumForLastHeatingOnlyStage = 0;
    int EquipListNumForLastSimultHeatCoolCoolingStage = 0;
    int EquipListNumForLastSimultHeatCoolHeatingStage = 0;
    bool SimultHeatCoolOpAvailable = false;
    bool SimultHeatCoolHeatingOpInput = false;
    bool SimulHeatCoolCoolingOpInput = false;
    bool DedicatedHR_ChWRetControl_Input = false;
    bool DedicatedHR_HWRetControl_Input = false;
    bool DedicatedHR_Present = false;
    Real64 DedicatedHR_SecChW_DesignCapacity = 0.0;             // design (sizing) capacity for cooling side of dedicated heat recovery WWHP, Watts
    Real64 DedicatedHR_SecChW_CurrentCapacity = 0.0;            // current capacity for cooling side of dedicated heat recovery WWHP, Watts
    Real64 DedicatedHR_SecHW_DesignCapacity = 0.0;              // design (sizing) capacity for heating side of dedicated heat recovery WWHP, Watts
    Real64 DedicatedHR_SecHW_CurrentCapacity = 0.0;             // current capacity for heating side of dedicated heat recovery WWHP, Watts
                                                                //   Real64 DedicatedHR_CapacityControlFactor = 0.0;
    bool AirSourcePlantHeatingOnly = false;                     // operation mode, if true primary plant appears to only need heating
    bool AirSourcePlantCoolingOnly = false;                     // operation mode, if true primary plant appears to only need cooling
    bool AirSourcePlantSimultaneousHeatingAndCooling = false;   // operation mode, if true primary plant appears to need both heating and cooling
    bool SimultaneousHeatingCoolingWithCoolingDominant = false; //
    bool SimultaneousHeatingCoolingWithHeatingDominant = false;
    int PrimaryHWLoopIndex = 0;
    int PrimaryHWLoopSupInletNode = 0;
    int PrimaryChWLoopIndex = 0;
    int PrimaryChWLoopSupInletNode = 0;
    int SecondaryHWLoopIndex = 0;
    int SecondaryChWLoopIndex = 0;
};

struct ReportData
{
    int AirSourcePlant_OpMode = 0;          // heating only = 1, cooling only = 2, simult heat cool = 3
    int DedicHR_OpMode = 0;                 // not dispatched = 0, heating led = 1, cooling led = 2
    int BoilerAux_OpMode = 0;               // not Dispatched = 0, Boiler(s) On = 1
    Real64 BuildingPolledHeatingLoad = 0.0; // current  building heating loads from predicted sensible zone loads, air system ventilation loads, and
                                            // any plant load profile process loads
    Real64 BuildingPolledCoolingLoad = 0.0; //  current building Cooling loads from predicted sensible zone loads, air system ventilation loads, and
                                            //  any plant load profile process loads
    Real64 PrimaryPlantHeatingLoad = 0.0;   // current apparent plant load on primary hot water plant served by heatpumps
    Real64 PrimaryPlantCoolingLoad = 0.0;   // current apparent plant load on primary chilled water plant served by heatpumps
    Real64 SecondaryPlantHeatingLoad = 0.0; // current apparent plant load on secondary hot water plant served by heatpumps
    Real64 SecondaryPlantCoolingLoad = 0.0; // current apparent plant load on secondary chilled water plant served by heatpumps
};

struct ChillerHeaterSupervisoryOperationData
// Custom supervisory plant operation scheme, control dispatch across a set of related set of plant loops
// For two-pipe chiller heater. 1..N chiller heater, 1..M chiller only.
// poll zone list to decide mode between chiller only, heater only, or simultaneous

{
    // get rid of these strings if possible
    std::string Name;
    std::string TypeOf;
    std::string ZoneListName;
    std::string DedicatedHR_ChWRetControl_Name;
    std::string DedicatedHR_HWRetControl_Name;

    bool oneTimeSetupComplete = false;
    bool needsSimulation = false;
    DataPlant::OpScheme Type = DataPlant::OpScheme::Invalid; // Op scheme type (from keyword)

    TempSetpoint Setpoint;
    TempResetData TempReset;
    PlantOpsData PlantOps;
    Array1D_int ZonePtrs;
    Array1D_int AirLoopPtrs;
    Array1D<EquipOpList> HeatingOnlyEquipList;
    Array1D<EquipOpList> CoolingOnlyEquipList;
    Array1D<EquipOpList> SimultHeatCoolHeatingEquipList;
    Array1D<EquipOpList> SimultHeatCoolCoolingEquipList;
    EIRPlantLoopHeatPumps::EIRPlantLoopHeatPump DedicatedHR_CoolingPLHP; // real pointer to the cooling side of dedicated heat recovery WWHP
    EIRPlantLoopHeatPumps::EIRPlantLoopHeatPump DedicatedHR_HeatingPLHP; // real pointer to the heating side of dedicated heat recory WWHP
    Array1D<int> PlantLoopIndicesBeingSupervised;          // if non zero then points to index of a plant loop that has this supervisory scheme as its
                                                           // operation scheme
    Array1D<int> SecondaryPlantLoopIndicesBeingSupervised; // if not zero then points to index of a plant loop that is treated as being a
                                                           // secondary loop, as in primary secondary distribution plant configurations.
    Array1D<PlantLocation> PlantLoadProfileComps;          // LoadProfile:Plant objects that may be loading loop
    Array1D<PlantLocation> PlantBoilerComps;               // Boilers that may need to be managed.
    Array1D<PlantLocation>
        PlantHXComps; // fluid to fluid heat exchangers that may need to be managed, these are HX connection on a supply side of a loop.
    ReportData Report;

    void OneTimeInitChillerHeaterChangeoverOpScheme(EnergyPlusData &state);

    void EvaluateChillerHeaterChangeoverOpScheme(EnergyPlusData &state);

    void DetermineCurrentBuildingLoads(EnergyPlusData &state);

    void DetermineCurrentPlantLoads(EnergyPlusData &state);

    void ProcessSupervisoryControlLogicForAirSourcePlants(EnergyPlusData &state);

    void InitAirSourcePlantEquipmentOff(EnergyPlusData &state);

    void ProcessAndSetAirSourcePlantEquipLists(EnergyPlusData &state);

    void ProcessAndSetDedicatedHeatRecovWWHP(EnergyPlusData &state);

    void ProcessAndSetAuxilBoiler(EnergyPlusData &state);

    Real64 DetermineHWSetpointOARest(EnergyPlusData &state);
};

struct OperationData
{
    // Members
    std::string Name;                 // The name of each item in the list
    std::string TypeOf;               // The 'keyWord' identifying each item in the list
    DataPlant::OpScheme Type;         // Op scheme type (from keyword)
    Sched::Schedule *sched = nullptr; // schedule associated with the list
    bool Available;                   // TRUE = designated component or operation scheme available
    int NumEquipLists;                // number of equipment lists
    int CurListPtr;                   // points to the current equipment list
    Array1D<EquipOpList> EquipList;   // Component type list
    int EquipListNumForLastStage;     // points to the equipment list with the highest upper limit
    std::string ReferenceNodeName;    // DELTA CTRL ONLY--for calculation of delta Temp
    int ReferenceNodeNumber;          // DELTA CTRL ONLY--for calculation of delta Temp
    int ErlSimProgramMngr;            // EMS:ProgramManager to always run when this model is called
    int ErlInitProgramMngr;           // EMS:ProgramManager to run when this model is initialized and setup
    int initPluginLocation;           // If Python Plugins are used to init this, this defines the location in the plugin structure
    int simPluginLocation;            // If Python Plugins are used to simulate this, this defines the location in the plugin structure
    Real64 EMSIntVarLoopDemandRate;   // EMS internal variable for loop-level demand rate, neg cooling [W]
    bool MyEnvrnFlag;
    ChillerHeaterSupervisoryOperationData *ChillerHeaterSupervisoryOperation = nullptr;

    // Default Constructor
    OperationData()
        : Type(DataPlant::OpScheme::Invalid), Available(false), NumEquipLists(0), CurListPtr(0), EquipListNumForLastStage(0), ReferenceNodeNumber(0),
          ErlSimProgramMngr(0), ErlInitProgramMngr(0), initPluginLocation(-1), simPluginLocation(-1), EMSIntVarLoopDemandRate(0.0), MyEnvrnFlag(true)
    {
    }
};
} // namespace EnergyPlus::DataPlant

#endif
