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

#ifndef DataZoneEquipment_hh_INCLUDED
#define DataZoneEquipment_hh_INCLUDED

// C++ Headers
#include <unordered_set>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACSystems.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/ExhaustAirSystemManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/SystemAvailabilityManager.hh>
#include <EnergyPlus/SystemReports.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace DataZoneEquipment {

    // Using/Aliasing
    // Data
    // -only module should be available to other modules and routines.
    // Thus, all variables in this module must be PUBLIC.

    // MODULE PARAMETER DEFINITIONS:
    enum class AirNodeType
    {
        Invalid = -1,
        PathInlet,
        CompInlet,
        Intermediate,
        Outlet,
        Num
    };

    enum class AirLoopHVACZone
    {
        Invalid = -1,
        Splitter,
        SupplyPlenum,
        Mixer,
        ReturnPlenum,
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(AirLoopHVACZone::Num)> AirLoopHVACTypeNamesCC = {
        "AirLoopHVAC:ZoneSplitter", "AirLoopHVAC:SupplyPlenum", "AirLoopHVAC:ZoneMixer", "AirLoopHVAC:ReturnPlenum"};

    constexpr std::array<std::string_view, static_cast<int>(AirLoopHVACZone::Num)> AirLoopHVACTypeNamesUC = {
        "AIRLOOPHVAC:ZONESPLITTER", "AIRLOOPHVAC:SUPPLYPLENUM", "AIRLOOPHVAC:ZONEMIXER", "AIRLOOPHVAC:RETURNPLENUM"};

    // Start zone equip objects
    // list units that are valid for zone system availability managers first
    enum ZoneEquipType
    {
        Invalid = -1,
        DUMMY,
        FourPipeFanCoil,
        PackagedTerminalHeatPump,
        PackagedTerminalAirConditioner,
        PackagedTerminalHeatPumpWaterToAir,
        WindowAirConditioner,
        UnitHeater,
        UnitVentilator,
        EnergyRecoveryVentilator,
        VentilatedSlab,
        OutdoorAirUnit,
        VariableRefrigerantFlowTerminal,
        PurchasedAir,
        EvaporativeCooler,
        HybridEvaporativeCooler, // last zone equipment type to use zone availability manager. The above list must not change or
                                 // NumValidSysAvailZoneComponents must also change.
        AirDistributionUnit,
        BaseboardConvectiveWater,
        BaseboardConvectiveElectric,
        BaseboardSteam,
        BaseboardWater,
        BaseboardElectric,
        HighTemperatureRadiant,
        LowTemperatureRadiantConstFlow,
        LowTemperatureRadiantVarFlow,
        LowTemperatureRadiantElectric,
        ExhaustFan,
        HeatExchanger,
        HeatPumpWaterHeaterPumpedCondenser,
        HeatPumpWaterHeaterWrappedCondenser,
        DehumidifierDX,
        RefrigerationChillerSet,
        UserDefinedHVACForcedAir,
        CoolingPanel,
        UnitarySystem,
        AirTerminalDualDuctConstantVolume,
        AirTerminalDualDuctVAV,
        AirTerminalSingleDuctConstantVolumeReheat,
        AirTerminalSingleDuctConstantVolumeNoReheat,
        AirTerminalSingleDuctVAVReheat,
        AirTerminalSingleDuctVAVNoReheat,
        AirTerminalSingleDuctSeriesPIUReheat,
        AirTerminalSingleDuctParallelPIUReheat,
        AirTerminalSingleDuctCAVFourPipeInduction,
        AirTerminalSingleDuctVAVReheatVariableSpeedFan,
        AirTerminalSingleDuctVAVHeatAndCoolReheat,
        AirTerminalSingleDuctVAVHeatAndCoolNoReheat,
        AirTerminalSingleDuctConstantVolumeCooledBeam,
        AirTerminalDualDuctVAVOutdoorAir,
        AirLoopHVACReturnAir,
        Num
    };

    extern const std::array<std::string_view, static_cast<int>(ZoneEquipType::Num)> zoneEquipTypeNamesUC;

    constexpr int NumValidSysAvailZoneComponents(14);
    extern Array1D_string const cValidSysAvailManagerCompTypes;

    // Per Person Ventilation Rate Mode
    enum class PerPersonVentRateMode
    {
        Invalid = -1,
        DCVByCurrentLevel,
        ByDesignLevel,
        Num
    };

    enum class LoadDist
    {
        Invalid = -1,
        Sequential,
        Uniform,
        UniformPLR,
        SequentialUniformPLR,
        Num
    };

    enum class LightReturnExhaustConfig : int
    {
        Invalid = -1,
        NoExhast = 0, // No exhaust node
        Single = 1,   // One to one configuration
        Multi = 2,    // Multiple return node referred
        Shared = 3,   // Shared exhaust node
        Num
    };

    enum class ZoneEquipTstatControl
    {
        Invalid = -1,
        SingleSpace,
        Maximum,
        Ideal,
        Num
    };

    enum class SpaceEquipSizingBasis
    {
        Invalid = -1,
        DesignCoolingLoad,
        DesignHeatingLoad,
        FloorArea,
        Volume,
        PerimeterLength,
        Num
    };

    struct SubSubEquipmentData // data for an individual component
    {
        // Members
        std::string TypeOf; // The 'keyWord' identifying  component type
        std::string Name;   // Component name
        int EquipIndex;     // Component Index for routines
        bool ON;            // When true, the designated component or operation scheme is available
        int InletNodeNum;
        int OutletNodeNum;
        int NumMeteredVars;
        Array1D<OutputProcessor::MeterData> MeteredVar; // Index of energy output report data
        int EnergyTransComp;                            // 1=EnergyTransfer, 0=No EnergyTransfer  Flag needed for reporting
        int ZoneEqToPlantPtr;                           // 0=No plant loop connection, >=0 index to ZoneEqToPlant array
        int OpMode;
        Real64 Capacity;
        Real64 Efficiency;
        Real64 TotPlantSupplyElec;
        Real64 TotPlantSupplyGas;
        Real64 TotPlantSupplyPurch;

        // Default Constructor
        SubSubEquipmentData()
            : EquipIndex(0), ON(true), InletNodeNum(0), OutletNodeNum(0), NumMeteredVars(0), EnergyTransComp(0), ZoneEqToPlantPtr(0), OpMode(0),
              Capacity(0.0), Efficiency(0.0), TotPlantSupplyElec(0.0), TotPlantSupplyGas(0.0), TotPlantSupplyPurch(0.0)
        {
        }
    };

    struct SubEquipmentData // data for an individual component
    {
        // Members
        bool Parent; // When true, the designated component is made up of sub-components
        int NumSubSubEquip;
        std::string TypeOf; // The 'keyWord' identifying  component type
        std::string Name;   // Component name
        int EquipIndex;     // Component Index for routines
        bool ON;            // When true, the designated component or operation scheme is available
        int InletNodeNum;
        int OutletNodeNum;
        int NumMeteredVars;
        Array1D<OutputProcessor::MeterData> MeteredVar; // Index of energy output report data
        Array1D<SubSubEquipmentData> SubSubEquipData;   // Component list
        int EnergyTransComp;                            // 1=EnergyTransfer, 0=No EnergyTransfer  Flag needed for reporting
        int ZoneEqToPlantPtr;                           // 0=No plant loop connection, >0 index to ZoneEqToPlant array
        int OpMode;
        Real64 Capacity;
        Real64 Efficiency;
        Real64 TotPlantSupplyElec;
        Real64 TotPlantSupplyGas;
        Real64 TotPlantSupplyPurch;

        // Default Constructor
        SubEquipmentData()
            : Parent(false), NumSubSubEquip(0), EquipIndex(0), ON(true), InletNodeNum(0), OutletNodeNum(0), NumMeteredVars(0), EnergyTransComp(0),
              ZoneEqToPlantPtr(0), OpMode(0), Capacity(0.0), Efficiency(0.0), TotPlantSupplyElec(0.0), TotPlantSupplyGas(0.0),
              TotPlantSupplyPurch(0.0)
        {
        }
    };

    struct AirIn
    {
        // Members
        int InNode;  // Air distribution unit inlet node
        int OutNode; // Air distribution unit Outlet node
        bool SupplyAirPathExists;
        int AirLoopNum = 0;            // airloop number serving this ADU
        int MainBranchIndex;           // airloop branch index (not the actual branch index)
        int SupplyBranchIndex;         // airloop branch index (not the actual branch index)
        int AirDistUnitIndex;          // equipment number in EquipList
        int TermUnitSizingIndex;       // Pointer to TermUnitSizing and TermUnitFinalZoneSizing data for this terminal unit
        int SupplyAirPathIndex;        // Pointer to SupplyAirPath serving this terminal unit
        int SupplyAirPathOutNodeIndex; // Pointer to SupplyAirPath OutletNode serving this terminal unit
        Array1D<SubSubEquipmentData> Coil;

        // Default Constructor
        AirIn()
            : InNode(0), OutNode(0), SupplyAirPathExists(false), MainBranchIndex(0), SupplyBranchIndex(0), AirDistUnitIndex(0),
              TermUnitSizingIndex(0), SupplyAirPathIndex(0)
        {
        }
    };

    struct EquipConfiguration
    {
        // Members
        std::string ZoneName;
        std::string EquipListName;
        int EquipListIndex;
        std::string ControlListName;
        int ZoneNode;
        int NumInletNodes;                              // number of inlet nodes
        int NumExhaustNodes;                            // number of exhaust nodes
        int NumReturnNodes;                             // number of return air nodes
        int NumReturnFlowBasisNodes;                    // number of return air flow basis nodes
        Sched::Schedule *returnFlowFracSched = nullptr; // return air flow fraction schedule pointer
        bool FlowError;                                 // flow error flag
        Array1D_int InletNode;                          // zone supply air inlet nodes
        Array1D_int InletNodeAirLoopNum;                // air loop number connected to this inlet node (0 if not an airloop node)
        Array1D_int InletNodeADUNum;          // AirDistUnit connected to this inlet node (0 if not an ADU node, could be zone equip or direct air)
        Array1D_int ExhaustNode;              // zone air exhaust nodes
        Array1D_int ReturnNode;               // zone return air nodes (node numbers)
        Array1D_int ReturnNodeAirLoopNum;     // air loop number connected to this return node
        Array1D_int ReturnNodeRetPathNum;     // ReturnPath number connected to this return node
        Array1D_int ReturnNodeRetPathCompNum; // ReturnPath component number connected to this return node
        Array1D_int
            ReturnNodeInletNum; // zone supply air inlet index that matched this return node (same zone, same airloop) - not the inlet node number
        Array1D_bool FixedReturnFlow;         // true if return node is fixed and cannot be adjusted in CalcZoneReturnFlows
        Array1D_int ReturnNodePlenumNum;      // number of the return plenum attached to this return node (zero if none)
        Array1D_int ReturnFlowBasisNode;      // return air flow basis nodes
        Array1D_int ReturnNodeExhaustNodeNum; // Exhaust node number flow to a corresponding return node due to light heat gain
        Array1D<LightReturnExhaustConfig>
            SharedExhaustNode; // Exhaust node number shared by return nodes 0 No exhaust; 1 No share; > 1 shared; -1 use the exhaust node value
        Array1D_int returnNodeSpaceMixerIndex; // index to SpaceHVAC:ZoneReturnMixer that feeds this return node (-1 if there is none)

        bool ZonalSystemOnly;     // TRUE if served by a zonal system (only)
        bool IsControlled;        // True when this is a controlled zone.
        Real64 ZoneExh;           // zone exhaust (unbalanced+balanced) mass flow rate [kg/s]
        Real64 ZoneExhBalanced;   // balanced zone exhaust mass flow rate [kg/s]
        Real64 PlenumMassFlow;    // zone air mass flow rate induced from plenum [kg/s]
        Real64 ExcessZoneExh;     // excess zone exhaust to be balanced by other zones (only used when !ZoneAirMassFlow.EnforceZoneMassBalance) [kg/s]
        Real64 TotAvailAirLoopOA; // total airloop OA available for systems serving this zone (used to apportion excess exhaust) [kg/s}
        Real64 TotInletAirMassFlowRate;   // total inlet node mass flow rate [kg/s]
        Real64 TotExhaustAirMassFlowRate; // total exhaust node mass flow rate [kg/s]
        // AirDistUnitCool and AirDistUnitHeat
        // do not correspond with the AIR DISTRIBUTION UNIT object in the zone equipment list.
        // AirDistUnitCool/AirDistUnitHeat, may represent a DIRECT AIR object,
        // or the cold/hot side of AIR DISTRIBUTION
        // UNIT object.  That is both AirDistUnitHeat and AirDistUnitCool are required to describe a dual
        // duct AIR DISTRIBUTION object in the ZoneEquipList.
        Array1D<AirIn> AirDistUnitHeat; // dimensioned to number of zone inlet nodes
        Array1D<AirIn> AirDistUnitCool; // dimensioned to number of zone inlet nodes.
        bool InFloorActiveElement;      // Convection adapation, true if zone has in-floor HVAC
        bool InWallActiveElement;       // Convection adapation, true if zone has in-wall HVAC
        bool InCeilingActiveElement;    // Convection adapation,
        // true when zone has in-ceiling HVAC
        bool ZoneHasAirLoopWithOASys; // true if zone is served by one or more airloops with an outdoor air system
        int ZoneAirDistributionIndex; // index to DesignSpecification:ZoneAirDistribution object
        int ZoneDesignSpecOAIndex;    // index to DesignSpecification:OutdoorAir object
        Real64 AirLoopDesSupply;      // air lood design supply air flow rate [kg/s]

        // Default Constructor
        EquipConfiguration()
            : ZoneName("Uncontrolled Zone"), EquipListIndex(0), ZoneNode(0), NumInletNodes(0), NumExhaustNodes(0), NumReturnNodes(0),
              NumReturnFlowBasisNodes(0), FlowError(false), ZonalSystemOnly(false), IsControlled(false), ZoneExh(0.0), ZoneExhBalanced(0.0),
              PlenumMassFlow(0.0), ExcessZoneExh(0.0), TotAvailAirLoopOA(0.0), TotInletAirMassFlowRate(0.0), TotExhaustAirMassFlowRate(0.0),
              InFloorActiveElement(false), InWallActiveElement(false), InCeilingActiveElement(false), ZoneHasAirLoopWithOASys(false),
              ZoneAirDistributionIndex(0), ZoneDesignSpecOAIndex(0), AirLoopDesSupply(0.0)
        {
        }

        void setTotalInletFlows(EnergyPlusData &state);

        void beginEnvirnInit(EnergyPlusData &state);

        void hvacTimeStepInit(EnergyPlusData &state, bool FirstHVACIteration);

        void calcReturnFlows(EnergyPlusData &state,
                             Real64 &ExpTotalReturnMassFlow,  // Expected total return air mass flow rate
                             Real64 &FinalTotalReturnMassFlow // Final total return air mass flow rate
        );
    };

    struct EquipmentData // data for an individual component
    {
        // Members
        bool Parent; // When true, the designated component is made up of sub-components
        int NumSubEquip;
        std::string TypeOf; // The 'keyWord' identifying  component type
        std::string Name;   // Component name
        bool ON;            // When true, the designated component or operation scheme is available
        int NumInlets;
        int NumOutlets;
        Array1D_int InletNodeNums;
        Array1D_int OutletNodeNums;
        int NumMeteredVars;
        Array1D<OutputProcessor::MeterData> MeteredVar; // Index of energy output report data
        Array1D<SubEquipmentData> SubEquipData;         // Component list
        int EnergyTransComp;                            // 1=EnergyTransfer, 0=No EnergyTransfer  Flag needed for reporting
        int ZoneEqToPlantPtr;                           // 0=No plant loop connection, >0 index to ZoneEqToPlant array
        Real64 TotPlantSupplyElec;
        Real64 TotPlantSupplyGas;
        Real64 TotPlantSupplyPurch;
        int OpMode;

        // Default Constructor
        EquipmentData()
            : Parent(false), NumSubEquip(0), ON(true), NumInlets(0), NumOutlets(0), NumMeteredVars(0), EnergyTransComp(0), ZoneEqToPlantPtr(0),
              TotPlantSupplyElec(0.0), TotPlantSupplyGas(0.0), TotPlantSupplyPurch(0.0), OpMode(0)
        {
        }
    };

    struct EquipList
    {
        // Members
        std::string Name;                                                                     // Name of the equipment list
        DataZoneEquipment::LoadDist LoadDistScheme = DataZoneEquipment::LoadDist::Sequential; // load distribution scheme
        int NumOfEquipTypes = 0;                                                              // Number of items on this list
        int NumAvailHeatEquip = 0;                                                            // Number of pieces of equipment available for heating
        int NumAvailCoolEquip = 0;                                                            // Number of pieces of equipment available for cooling
        Array1D_string EquipTypeName; // TODO: Convert this from string to enum and remove EquipTypeEnum below
        Array1D<DataZoneEquipment::ZoneEquipType> EquipType;
        Array1D_string EquipName;
        Array1D_int EquipIndex;
        EPVector<int> zoneEquipSplitterIndex; // index to SpaceHVAC:ZoneEquipmentSplitter, if any, for this equipment (0 base)
        // SystemAvailManagers need to know the index of specific equipment (e.g., PTAC as 1,2,3)
        // if UnitarySystem models PTAC, PTHP, UnitarySystems, then the index to a specific UnitarySystem is not the index to the PTAC
        std::vector<HVACSystemData *> compPointer;
        Array1D_int CoolingPriority;
        Array1D_int HeatingPriority;
        Array1D<Sched::Schedule *> sequentialCoolingFractionScheds;
        Array1D<Sched::Schedule *> sequentialHeatingFractionScheds;
        Array1D_int CoolingCapacity;      // Current cooling capacity (negative) [W]
        Array1D_int HeatingCapacity;      // Current heating capacity (positive) [W]
        Array1D<EquipmentData> EquipData; // Index of energy output report data

        void getPrioritiesForInletNode(EnergyPlusData &state,
                                       int inletNodeNum,     // Zone inlet node number to match
                                       int &coolingPriority, // Cooling priority num for matching equipment
                                       int &heatingPriority  // Heating priority num for matching equipment
        );

        Real64 SequentialHeatingFraction(EnergyPlusData &state, int equipNum);

        Real64 SequentialCoolingFraction(EnergyPlusData &state, int equipNum);
    };

    struct ZoneEquipSplitterMixerSpace
    {
        int spaceIndex = 0;    // Index to a space
        Real64 fraction = 0.0; // Fraction of equipment output or flow for this space
        int spaceNodeNum = 0;  // Space Inlet Node number (zero if not airflow equipment)
    };

    struct ZoneEquipmentSplitterMixer
    {
        std::string Name;
        Node::ConnectionObjectType spaceEquipType = Node::ConnectionObjectType::Invalid;
        DataZoneEquipment::SpaceEquipSizingBasis spaceSizingBasis = DataZoneEquipment::SpaceEquipSizingBasis::Invalid;
        std::vector<ZoneEquipSplitterMixerSpace> spaces;

        void size(EnergyPlusData &state);
    };

    struct ZoneEquipmentSplitter : ZoneEquipmentSplitterMixer
    {
        DataZoneEquipment::ZoneEquipType zoneEquipType = DataZoneEquipment::ZoneEquipType::Invalid;
        std::string zoneEquipName;
        int zoneEquipOutletNodeNum = 0;
        DataZoneEquipment::ZoneEquipTstatControl tstatControl = DataZoneEquipment::ZoneEquipTstatControl::Invalid;
        int controlSpaceIndex = 0;                                                 // Index to a space for the thermostat control space
        int controlSpaceNumber = 0;                                                // Control space number within the zone equipment splitter list
        DataZoneEnergyDemands::ZoneSystemSensibleDemand saveZoneSysSensibleDemand; // Save unadjusted zone sensible loads
        DataZoneEnergyDemands::ZoneSystemMoistureDemand saveZoneSysMoistureDemand; // Save unadjusted zone moisture loads

        void distributeOutput(EnergyPlusData &state,
                              int const zoneNum,
                              Real64 const sysOutputProvided,
                              Real64 const latOutputProvided,
                              Real64 const nonAirSysOutput,
                              int const equipTypeNum);

        void adjustLoads(EnergyPlusData &state, int zoneNum, int equipTypeNum);
    };

    struct ZoneMixer : ZoneEquipmentSplitterMixer
    {
        int outletNodeNum = 0;

        void setOutletConditions(EnergyPlusData &state);
    };

    struct ZoneEquipmentMixer : ZoneMixer
    {
        void setInletFlows(EnergyPlusData &state);
    };

    struct ZoneReturnMixer : ZoneMixer
    {
        void setInletConditions(EnergyPlusData &state);

        void setInletFlows(EnergyPlusData &state);
    };
    struct ControlList
    {
        // Members
        std::string Name;
        int NumOfControls;
        Array1D_string ControlType;
        Array1D_string ControlName;

        // Default Constructor
        ControlList() : NumOfControls(0)
        {
        }
    };

    struct SupplyAir
    {
        // Members
        std::string Name;
        int NumOfComponents;
        int InletNodeNum;
        Array1D_string ComponentType; // TODO: Convert this from string to enum and remove ComponentTypeEnum below
        Array1D<DataZoneEquipment::AirLoopHVACZone> ComponentTypeEnum;
        Array1D_string ComponentName;
        Array1D_int ComponentIndex;
        Array1D_int SplitterIndex;
        Array1D_int PlenumIndex;
        int NumOutletNodes;
        Array1D_int OutletNode;
        Array1D_int OutletNodeSupplyPathCompNum; // Index to the supply path ComponentName and ComponentType lists for this outlet node
        int NumNodes;
        Array1D_int Node;
        Array1D<DataZoneEquipment::AirNodeType> NodeType;

        // Default Constructor
        SupplyAir() : NumOfComponents(0), InletNodeNum(0), NumOutletNodes(0), NumNodes(0)
        {
        }
    };

    struct ReturnAir
    {
        // Members
        std::string Name;
        int NumOfComponents = 0;
        int OutletNodeNum = 0;        // Node num of return path outlet
        int OutletRetPathCompNum = 0; // Index to return path component number for outlet node
        Array1D_string ComponentType; // TODO: Convert this from string to enum and remove ComponentTypeEnum below
        Array1D<DataZoneEquipment::AirLoopHVACZone> ComponentTypeEnum;
        Array1D_string ComponentName;
        Array1D_int ComponentIndex;
    };

    void GetZoneEquipmentData(EnergyPlusData &state);

    void processZoneEquipmentInput(EnergyPlusData &state,
                                   std::string_view zoneEqModuleObject,
                                   int const zoneOrSpaceNum,
                                   bool const isSpace,
                                   int &locTermUnitSizingCounter,
                                   int &overallEquipCount,
                                   DataZoneEquipment::EquipConfiguration &thisEquipConfig,
                                   Array1D_string &AlphArray,
                                   Array1D_string &cAlphaFields, // Alpha field names
                                   Array1D_bool &lAlphaBlanks,   // Logical array, alpha field input BLANK = .TRUE.
                                   Array1D_int &NodeNums);

    void processZoneEquipSplitterInput(EnergyPlusData &state,
                                       std::string_view zeqSplitterModuleObject,
                                       int const zeqSplitterNum,
                                       int const zoneNum,
                                       InputProcessor::json const objectSchemaProps,
                                       InputProcessor::json const objectFields,
                                       DataZoneEquipment::ZoneEquipmentSplitter &thisZeqSplitter);

    void processZoneEquipMixerInput(EnergyPlusData &state,
                                    std::string_view zeqMixerModuleObject,
                                    int const zoneNum,
                                    InputProcessor::json const objectSchemaProps,
                                    InputProcessor::json const objectFields,
                                    DataZoneEquipment::ZoneEquipmentMixer &thisZeqMixer);

    void processZoneReturnMixerInput(EnergyPlusData &state,
                                     std::string_view zeqMixerModuleObject,
                                     int const zoneNum,
                                     InputProcessor::json const objectSchemaProps,
                                     InputProcessor::json const objectFields,
                                     int mixerIndex);

    bool CheckZoneEquipmentList(EnergyPlusData &state,
                                std::string_view ComponentType, // Type of component
                                std::string_view ComponentName, // Name of component
                                ObjexxFCL::Optional_int CtrlZoneNum = _);

    int GetControlledZoneIndex(EnergyPlusData &state, std::string const &ZoneName); // Zone name to match into Controlled Zone structure

    int FindControlledZoneIndexFromSystemNodeNumberForZone(EnergyPlusData &state,
                                                           int TrialZoneNodeNum); // Node number to match into Controlled Zone structure

    int GetSystemNodeNumberForZone(EnergyPlusData &state, int const zoneNum);

    int GetReturnAirNodeForZone(EnergyPlusData &state,
                                int const zoneNum,
                                std::string const &NodeName,             // Return air node name to match (may be blank)
                                std::string const &calledFromDescription // String identifying the calling function and object
    );

    int GetReturnNumForZone(EnergyPlusData &state,
                            int const zoneNum,
                            std::string const &NodeName // Return air node name to match (may be blank)
    );

    int GetZoneEquipControlledZoneNum(EnergyPlusData &state, DataZoneEquipment::ZoneEquipType const ZoneEquipType, std::string const &EquipmentName);

    bool VerifyLightsExhaustNodeForZone(EnergyPlusData &state, int const ZoneNum, int const ZoneExhaustNodeNum);

    void CheckSharedExhaust(EnergyPlusData &state);

    void scaleInletFlows(EnergyPlusData &state, int const zoneNodeNum, int const spaceNodeNum, Real64 const frac);

} // namespace DataZoneEquipment

struct DataZoneEquipmentData : BaseGlobalStruct
{

    bool GetZoneEquipmentDataErrorsFound = false;
    int GetZoneEquipmentDataFound = 0;
    int NumSupplyAirPaths = 0;
    int NumReturnAirPaths = 0;
    int NumExhaustAirSystems = 0;
    int NumZoneExhaustControls = 0;
    bool ZoneEquipInputsFilled = false;
    bool ZoneEquipSimulatedOnce = false;
    int NumOfZoneEquipLists = 0;
    Array1D<Avail::Status> ZoneEquipAvail;
    Array1D<DataZoneEquipment::EquipConfiguration> ZoneEquipConfig;
    Array1D<DataZoneEquipment::EquipConfiguration> spaceEquipConfig;
    std::unordered_set<std::string> UniqueZoneEquipListNames;
    Array1D<DataZoneEquipment::EquipList> ZoneEquipList;
    Array1D<DataZoneEquipment::SupplyAir> SupplyAirPath;
    Array1D<DataZoneEquipment::ReturnAir> ReturnAirPath;
    Array1D<ExhaustAirSystemManager::ExhaustAir> ExhaustAirSystem;
    Array1D<ExhaustAirSystemManager::ZoneExhaustControl> ZoneExhaustControlSystem; // 2022-01: maybe a better name?
    std::vector<DataZoneEquipment::ZoneEquipmentSplitter> zoneEquipSplitter;
    std::vector<DataZoneEquipment::ZoneEquipmentMixer> zoneEquipMixer;
    std::vector<DataZoneEquipment::ZoneReturnMixer> zoneReturnMixer;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) DataZoneEquipmentData();
    }
};

} // namespace EnergyPlus

#endif
