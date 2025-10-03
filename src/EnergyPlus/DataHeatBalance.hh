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

#ifndef DataHeatBalance_hh_INCLUDED
#define DataHeatBalance_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Array3D.hh>
#include <ObjexxFCL/Array4D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/ConvectionConstants.hh>
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataVectorTypes.hh>
#include <EnergyPlus/DataWindowEquivalentLayer.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/ExteriorEnergyUse.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace DataHeatBalance {

    // Using/Aliasing
    using DataVectorTypes::Vector;

    // Parameters for Interior and Exterior Solar Distribution
    enum class Shadowing
    {
        Invalid = -1,
        Minimal,              // all incoming solar hits floor, no exterior shadowing except reveals
        FullExterior,         // all incoming solar hits floor, full exterior shadowing
        FullInteriorExterior, // full interior solar distribution, full exterior solar shadowing
        FullExteriorWithRefl, // all incoming solar hits floor, full exterior shadowing and reflections
        Num
    };

    // Parameters to indicate the zone type for use with the Zone derived
    // type (see below--Zone%OfType):
    constexpr int StandardZone(1);

    // Parameters for WarmupDays
    constexpr int DefaultMaxNumberOfWarmupDays(25); // Default maximum number of warmup days allowed
    constexpr int DefaultMinNumberOfWarmupDays(1);  // Default minimum number of warmup days allowed

    // Parameters for ZoneAirSolutionAlgo
    enum class SolutionAlgo
    {
        Invalid = -1,
        ThirdOrder,
        AnalyticalSolution,
        EulerMethod,
        Num
    };

    // Parameter for MRT calculation type
    enum class CalcMRT
    {
        Invalid = -1,
        EnclosureAveraged,
        SurfaceWeighted,
        AngleFactor,
        Num
    };

    // Parameters for Ventilation
    enum class VentilationType
    {
        Invalid = -1,
        Natural,
        Intake,
        Exhaust,
        Balanced,
        Num
    };

    // Parameters for hybrid ventilation using Ventilation and Mixing objects
    enum class HybridCtrlType
    {
        Invalid = -1,
        Indiv,
        Close,
        Global,
        Num
    };

    // System type, detailed refrigeration or refrigerated case rack
    enum class RefrigSystemType
    {
        Invalid = -1,
        Detailed,
        Rack,
        Num
    };

    // Refrigeration condenser type
    enum class RefrigCondenserType
    {
        Invalid = -1,
        Air,
        Evap,
        Water,
        Cascade,
        WaterHeater,
        Num
    };

    constexpr std::array<std::string_view, (int)RefrigCondenserType::Num> refrigCondenserTypeNames = {
        "AirCooled", "EvaporativelyCooled", "WaterCooled", "Cascade", "WaterHeater"}; // Are the last two used?
    constexpr std::array<std::string_view, (int)RefrigCondenserType::Num> refrigCondenserTypeNamesUC = {
        "AIRCOOLED", "EVAPORATIVELYCOOLED", "WATERCOOLED", "CASCADE", "WATERHEATER"}; // Are the last two used?

    // Parameters for type of infiltration model
    enum class InfiltrationModelType
    {
        Invalid = -1,
        DesignFlowRate,
        ShermanGrimsrud,
        AIM2,
        Num
    };

    // Parameters for type of ventilation model
    enum class VentilationModelType
    {
        Invalid = -1,
        DesignFlowRate,
        WindAndStack,
        Num
    };

    enum class InfVentDensityBasis
    {
        Invalid = -1,
        Outdoor,
        Standard,
        Indoor,
        Num
    };

    // Parameters for type of zone air balance model
    enum class AirBalance
    {
        Invalid = -1,
        None,
        Quadrature,
        Num
    };

    // Parameter for source zone air flow mass balance infiltration treatment
    enum class InfiltrationFlow
    {
        Invalid = -1,
        No,
        Add,
        Adjust,
        Num
    };

    enum class InfiltrationZoneType
    {
        Invalid = -1,
        MixingSourceZonesOnly,
        AllZones,
        Num
    };

    // zone air flow balancing method
    enum class AdjustmentType
    {
        Invalid = -1,
        AdjustMixingOnly,
        AdjustReturnOnly,
        AdjustMixingThenReturn,
        AdjustReturnThenMixing,
        NoAdjustReturnAndMixing,
        Num
    };

    enum class IntGainType
    {
        Invalid = -1,
        People,
        Lights,
        ElectricEquipment,
        GasEquipment,
        HotWaterEquipment,
        SteamEquipment,
        OtherEquipment,
        ZoneBaseboardOutdoorTemperatureControlled,
        ZoneContaminantSourceAndSinkCarbonDioxide,
        WaterUseEquipment,
        DaylightingDeviceTubular,
        WaterHeaterMixed,
        WaterHeaterStratified,
        ThermalStorageChilledWaterMixed,
        ThermalStorageChilledWaterStratified,
        GeneratorFuelCell,
        GeneratorMicroCHP,
        ElectricLoadCenterTransformer,
        ElectricLoadCenterInverterSimple,
        ElectricLoadCenterInverterFunctionOfPower,
        ElectricLoadCenterInverterLookUpTable,
        ElectricLoadCenterStorageLiIonNmcBattery,
        ElectricLoadCenterStorageBattery,
        ElectricLoadCenterStorageSimple,
        PipeIndoor,
        RefrigerationCase,
        RefrigerationCompressorRack,
        RefrigerationSystemAirCooledCondenser,
        RefrigerationTransSysAirCooledGasCooler,
        RefrigerationSystemSuctionPipe,
        RefrigerationTransSysSuctionPipeMT,
        RefrigerationTransSysSuctionPipeLT,
        RefrigerationSecondaryReceiver,
        RefrigerationSecondaryPipe,
        RefrigerationWalkIn,
        Pump_VarSpeed,
        Pump_ConSpeed,
        Pump_Cond,
        PumpBank_VarSpeed,
        PumpBank_ConSpeed,
        ZoneContaminantSourceAndSinkGenericContam,
        PlantComponentUserDefined,
        CoilUserDefined,
        ZoneHVACForcedAirUserDefined,
        AirTerminalUserDefined,
        PackagedTESCoilTank,
        ElectricEquipmentITEAirCooled,
        SecCoolingDXCoilSingleSpeed,
        SecHeatingDXCoilSingleSpeed,
        SecCoolingDXCoilTwoSpeed,
        SecCoolingDXCoilMultiSpeed,
        SecHeatingDXCoilMultiSpeed,
        ElectricLoadCenterConverter,
        FanSystemModel,
        IndoorGreen,
        Num
    };

    static constexpr std::array<std::string_view, static_cast<int>(DataHeatBalance::CalcMRT::Num)> CalcMRTTypeNamesUC = {
        "ENCLOSUREAVERAGED", "SURFACEWEIGHTED", "ANGLEFACTOR"};

    static constexpr std::array<std::string_view, static_cast<int>(DataHeatBalance::AirBalance::Num)> AirBalanceTypeNamesUC = {"NONE", "QUADRATURE"};

    static constexpr std::array<std::string_view, static_cast<int>(DataHeatBalance::InfiltrationFlow::Num)> InfiltrationFlowTypeNamesUC = {
        "NONE", "ADDINFILTRATIONFLOW", "ADJUSTINFILTRATIONFLOW"};

    static constexpr std::array<std::string_view, static_cast<int>(DataHeatBalance::InfiltrationFlow::Num)> InfiltrationFlowTypeNamesCC = {
        "None", "AddInfiltrationFlow", "AdjustInfiltrationFlow"};

    static constexpr std::array<std::string_view, static_cast<int>(DataHeatBalance::InfiltrationZoneType::Num)> InfiltrationZoneTypeNamesUC = {
        "MIXINGSOURCEZONESONLY", "ALLZONES"};

    static constexpr std::array<std::string_view, static_cast<int>(DataHeatBalance::InfiltrationZoneType::Num)> InfiltrationZoneTypeNamesCC = {
        "MixingSourceZonesOnly", "AllZones"};

    static constexpr std::array<std::string_view, static_cast<int>(DataHeatBalance::AdjustmentType::Num)> AdjustmentTypeNamesUC = {
        "ADJUSTMIXINGONLY", "ADJUSTRETURNONLY", "ADJUSTMIXINGTHENRETURN", "ADJUSTRETURNTHENMIXING", "NONE"};

    static constexpr std::array<std::string_view, static_cast<int>(DataHeatBalance::AdjustmentType::Num)> AdjustmentTypeNamesCC = {
        "AdjustMixingOnly", "AdjustReturnOnly", "AdjustMixingThenReturn", "AdjustReturnThenMixing", "None"};

    static constexpr std::array<std::string_view, static_cast<int>(DataHeatBalance::IntGainType::Num)> IntGainTypeNamesUC = {
        "PEOPLE",
        "LIGHTS",
        "ELECTRICEQUIPMENT",
        "GASEQUIPMENT",
        "HOTWATEREQUIPMENT",
        "STEAMEQUIPMENT",
        "OTHEREQUIPMENT",
        "ZONEBASEBOARD:OUTDOORTEMPERATURECONTROLLED",
        "ZONECONTAMINANTSOURCEANDSINK:CARBONDIOXIDE",
        "WATERUSE:EQUIPMENT",
        "DAYLIGHTINGDEVICE:TUBULAR",
        "WATERHEATER:MIXED",
        "WATERHEATER:STRATIFIED",
        "THERMALSTORAGE:CHILLEDWATER:MIXED",
        "THERMALSTORAGE:CHILLEDWATER:STRATIFIED",
        "GENERATOR:FUELCELL",
        "GENERATOR:MICROCHP",
        "ELECTRICLOADCENTER:TRANSFORMER",
        "ELECTRICLOADCENTER:INVERTER:SIMPLE",
        "ELECTRICLOADCENTER:INVERTER:FUNCTIONOFPOWER",
        "ELECTRICLOADCENTER:INVERTER:LOOKUPTABLE",
        "ELECTRICLOADCENTER:STORAGE:LIIONNMCBATTERY",
        "ELECTRICLOADCENTER:STORAGE:BATTERY",
        "ELECTRICLOADCENTER:STORAGE:SIMPLE",
        "PIPE:INDOOR",
        "REFRIGERATION:CASE",
        "REFRIGERATION:COMPRESSORRACK",
        "REFRIGERATION:SYSTEM:CONDENSER:AIRCOOLED",
        "REFRIGERATION:TRANSCRITICALSYSTEM:GASCOOLER:AIRCOOLED",
        "REFRIGERATION:SYSTEM:SUCTIONPIPE",
        "REFRIGERATION:TRANSCRITICALSYSTEM:SUCTIONPIPEMT",
        "REFRIGERATION:TRANSCRITICALSYSTEM:SUCTIONPIPELT",
        "REFRIGERATION:SECONDARYSYSTEM:RECEIVER",
        "REFRIGERATION:SECONDARYSYSTEM:PIPE",
        "REFRIGERATION:WALKIN",
        "PUMP:VARIABLESPEED",
        "PUMP:CONSTANTSPEED",
        "PUMP:VARIABLESPEED:CONDENSATE",
        "HEADEREDPUMPS:VARIABLESPEED",
        "HEADEREDPUMPS:CONSTANTSPEED",
        "ZONECONTAMINANTSOURCEANDSINK:GENERICCONTAMINANT",
        "PLANTCOMPONENT:USERDEFINED",
        "COIL:USERDEFINED",
        "ZONEHVAC:FORCEDAIR:USERDEFINED",
        "AIRTERMINAL:SINGLEDUCT:USERDEFINED",
        "COIL:COOLING:DX:SINGLESPEED:THERMALSTORAGE",
        "ELECTRICEQUIPMENT:ITE:AIRCOOLED",
        "COIL:COOLING:DX:SINGLESPEED",
        "COIL:HEATING:DX:SINGLESPEED",
        "COIL:COOLING:DX:TWOSPEED",
        "COIL:COOLING:DX:MULTISPEED",
        "COIL:HEATING:DX:MULTISPEED",
        "ELECTRICLOADCENTER:STORAGE:CONVERTER",
        "FAN:SYSTEMMODEL",
        "INDOORGREEN"};

    static constexpr std::array<std::string_view, static_cast<int>(DataHeatBalance::IntGainType::Num)> IntGainTypeNamesCC = {
        "People",
        "Lights",
        "ElectricEquipment",
        "GasEquipment",
        "HotWaterEquipment",
        "SteamEquipment",
        "OtherEquipment",
        "ZoneBaseboard:OutdoorTemperatureControlled",
        "ZoneContaminantSourceAndSink:CarbonDioxide",
        "WaterUse:Equipment",
        "DaylightingDevice:Tubular",
        "WaterHeater:Mixed",
        "WaterHeater:Stratified",
        "ThermalStorage:ChilledWater:Mixed",
        "ThermalStorage:ChilledWater:Stratified",
        "Generator:FuelCell",
        "Generator:MicroCHP",
        "ElectricLoadCenter:Transformer",
        "ElectricLoadCenter:Inverter:Simple",
        "ElectricLoadCenter:Inverter:FunctionOfPower",
        "ElectricLoadCenter:Inverter:LookUpTable",
        "ElectricLoadCenter:Storage:LiIonNMCBattery",
        "ElectricLoadCenter:Storage:Battery",
        "ElectricLoadCenter:Storage:Simple",
        "Pipe:Indoor",
        "Refrigeration:Case",
        "Refrigeration:CompressorRack",
        "Refrigeration:System:Condenser:AirCooled",
        "Refrigeration:TranscriticalSystem:GasCooler:AirCooled",
        "Refrigeration:System:SuctionPipe",
        "Refrigeration:TranscriticalSystem:SuctionPipeMT",
        "Refrigeration:TranscriticalSystem:SuctionPipeLT",
        "Refrigeration:SecondarySystem:Receiver",
        "Refrigeration:SecondarySystem:Pipe",
        "Refrigeration:WalkIn",
        "Pump:VariableSpeed",
        "Pump:ConstantSpeed",
        "Pump:VariableSpeed:Condensate",
        "HeaderedPumps:VariableSpeed",
        "HeaderedPumps:ConstantSpeed",
        "ZoneContaminantSourceAndSink:GenericContaminant",
        "PlantComponent:UserDefined",
        "Coil:UserDefined",
        "ZoneHVAC:ForcedAir:UserDefined",
        "AirTerminal:SingleDuct:UserDefined",
        "Coil:Cooling:DX:SingleSpeed:ThermalStorage",
        "ElectricEquipment:ITE:AirCooled",
        "Coil:Cooling:DX:SingleSpeed",
        "Coil:Heating:DX:SingleSpeed",
        "Coil:Cooling:DX:TwoSpeed",
        "Coil:Cooling:DX:MultiSpeed",
        "Coil:Heating:DX:MultiSpeed",
        "ElectricLoadCenter:Storage:Converter",
        "Fan:SystemModel",
        "IndoorGreen"};

    // Parameters for checking surface heat transfer models
    constexpr Real64 HighDiffusivityThreshold(1.e-5);   // used to check if Material properties are out of line.
    constexpr Real64 ThinMaterialLayerThreshold(0.003); // 3 mm lower limit to expected material layers

    // Const for initialization
    constexpr Real64 ZoneInitialTemp(23.0);       // Zone temperature for initialization
    constexpr Real64 SurfInitialTemp(23.0);       // Surface temperature for initialization
    constexpr Real64 SurfInitialConvCoeff(3.076); // Surface convective coefficient for initialization

    struct ZoneSpaceData
    {
        // Base class for zones and spaces.
        // For now, only including fields that are new to Space to avoid excess changes
        // due to case differences between existing space and zone field names.
        std::string Name;
        Real64 CeilingHeight = Constant::AutoCalculate; // Ceiling Height entered by user [m] or calculated
        Real64 Volume = Constant::AutoCalculate;        // Volume entered by user [m3] or calculated
        Real64 ExtGrossWallArea = 0.0;                  // Exterior Wall Area for Zone (Gross)
        Real64 ExteriorTotalSurfArea = 0.0;             // Total surface area of all exterior surfaces for Zone
        Real64 extPerimeter = 0.0;                      // Total exposed perimeter (sum of width of exterior walls)
        int SystemZoneNodeNumber = 0;                   // This is the zone or space node number for the system for a controlled zone
        Real64 FloorArea = 0.0;                         // Floor area used for this space
        Real64 TotOccupants = 0.0;                      // total design occupancy (sum of NumberOfPeople for the space People objects, not multiplied)
        bool IsControlled = false;                      // True when this is a controlled zone or space.
    };

    struct SpaceData : ZoneSpaceData
    {
        int zoneNum = 0;                                       // Pointer to Zone wich contains this space
        Real64 userEnteredFloorArea = Constant::AutoCalculate; // User input floor area for this space
        std::string spaceType = "General";                     // Space type tag
        int spaceTypeNum = 0;                                  // Points to spaceType for this space
        EPVector<std::string> tags;                            // Optional tags for reporting
        EPVector<int> surfaces;                                // Pointers to surfaces in this space
        bool hasFloor = false;                                 // Has "Floor" surface
        Real64 fracZoneFloorArea = 0.0;                        // fraction of total floor area for all spaces in zone
        Real64 fracZoneVolume = 0.0;                           // fraction of total volume for all spaces in zone
        Real64 extWindowArea = 0.0;                            // Exterior Window Area for space
        Real64 totalSurfArea = 0.0;                            // Total surface area for space
        int radiantEnclosureNum = 0;                           // Radiant exchange enclosure this space belongs to
        int solarEnclosureNum = 0;                             // Solar distribution enclosure this space belongs to
        Real64 minOccupants = 0.0;     // minimum occupancy (sum of NomMinNumberPeople for the space People objects, not multiplied)
        Real64 maxOccupants = 0.0;     // maximum occupancy (sum of NomMaxNumberPeople for the space People objects, not multiplied)
        bool isRemainderSpace = false; // True if this space is auto-generated "-Remainder" space

        std::vector<Constant::eFuel> otherEquipFuelTypeNums; // List of fuel types used by other equipment in this space

        // Pointers to Surface Data Structure
        // |AllSurfF                                                                      |AllSurfL
        // |            |HTSurfF                                                          |HTSurfL
        // |            |OpaqOrWinMassSurfF                              |OpaqOrWinSurfL  |
        // |            |OpaqOrIntMassSurfF      |OpaqOrIntMassSurfL                      |
        // |            |                        ||WindowSurfF           |WindowSurfL     |
        // |            |                        ||                      ||DomeF          |DomeL
        // {[ SurfAir ] [(   SurfOpaqOrIntMass   )( SurfWinOrTDDDiffuser )( TDDDome       )]}
        // HTSurfaceFirst == OpaqOrWinMassSurfaceFirst == OpaqOrIntMassSurfaceFirst
        // WindowSurfaceFirst == OpaqOrIntMassSurfaceLast + 1
        // TDDDomeFirst == OpaqOrWinSurfaceLast + 1 == WindowSurfaceLast + 1
        // AllSurfaceLast == HTSurfaceLast = TDDDomeLast
        int AllSurfaceFirst = 0;           // First surface in space including air boundaries
        int AllSurfaceLast = -1;           // Last  surface in space including air boundaries
        int HTSurfaceFirst = 0;            // First Heat Transfer Surface in space
        int HTSurfaceLast = -1;            // Last  Heat Transfer Surface in space
        int OpaqOrIntMassSurfaceFirst = 0; // First Opaque or Interior Mass Heat Transfer Surface (including opaque doors) in space
        int OpaqOrIntMassSurfaceLast = -1; // Last  Opaque or Interior Mass Heat Transfer Surface (including opaque doors) in space
        int WindowSurfaceFirst = 0;        // First Window Heat Transfer Surface in space
        int WindowSurfaceLast = -1;        // Last  Window Heat Transfer Surface in space
        int OpaqOrWinSurfaceFirst = 0;     // First opaque (including IntMass) or window (non TDD Dome) Surface in space
        int OpaqOrWinSurfaceLast = -1;     // Last  opaque (including IntMass) or window (non TDD Dome) Surface in space
        int TDDDomeFirst = 0;              // First TDD Dome Surface in space
        int TDDDomeLast = -1;              // Last  TDD Dome Surface in space

        Real64 sumHATsurf(EnergyPlusData &state);
    };

    struct SpaceListData
    {
        std::string Name;                               // Space List name
        int numListSpaces = 0;                          // Number of spaces in the list
        std::string::size_type maxSpaceNameLength = 0u; // Max Name length of Spaces in the list
        EPVector<int> spaces;                           // Pointers to Spaces in the list
    };

    //    number of columns in resilience report tables
    constexpr int numColumnThermalTbl(5);
    constexpr int numColumnUnmetDegreeHourTbl(6);
    constexpr int numColumnDiscomfortWtExceedHourTbl(4);
    constexpr int numColumnCO2Tbl(3);
    constexpr int numColumnVisualTbl(4);

    struct ZoneResilience
    {
        Real64 ZoneNumOcc;
        Real64 ColdStressTempThresh;
        Real64 HeatStressTempThresh;
        Real64 PierceSET;
        Real64 PMV;
        Real64 ZonePierceSET;
        Real64 ZonePierceSETLastStep;
        Real64 ZoneHeatIndex;
        Real64 ZoneHumidex;
        bool CrossedColdThresh;
        bool CrossedHeatThresh;

        std::array<Real64, numColumnThermalTbl> ZoneHeatIndexHourBins = {0.0};
        std::array<Real64, numColumnThermalTbl> ZoneHeatIndexOccuHourBins = {0.0};
        std::array<Real64, numColumnThermalTbl> ZoneHeatIndexOccupiedHourBins = {0.0};
        std::array<Real64, numColumnThermalTbl> ZoneHumidexHourBins = {0.0};
        std::array<Real64, numColumnThermalTbl> ZoneHumidexOccuHourBins = {0.0};
        std::array<Real64, numColumnThermalTbl> ZoneHumidexOccupiedHourBins = {0.0};
        std::array<Real64, numColumnThermalTbl> ZoneLowSETHours = {0.0};
        std::array<Real64, numColumnThermalTbl> ZoneHighSETHours = {0.0};
        std::array<Real64, numColumnThermalTbl> ZoneColdHourOfSafetyBins = {0.0};
        std::array<Real64, numColumnThermalTbl> ZoneHeatHourOfSafetyBins = {0.0};
        std::array<Real64, numColumnUnmetDegreeHourTbl> ZoneUnmetDegreeHourBins = {0.0};
        std::array<Real64, numColumnDiscomfortWtExceedHourTbl> ZoneDiscomfortWtExceedOccuHourBins = {0.0};
        std::array<Real64, numColumnDiscomfortWtExceedHourTbl> ZoneDiscomfortWtExceedOccupiedHourBins = {0.0};

        std::array<Real64, numColumnCO2Tbl> ZoneCO2LevelHourBins = {0.0};
        std::array<Real64, numColumnCO2Tbl> ZoneCO2LevelOccuHourBins = {0.0};
        std::array<Real64, numColumnCO2Tbl> ZoneCO2LevelOccupiedHourBins = {0.0};

        std::array<Real64, numColumnVisualTbl> ZoneLightingLevelHourBins = {0.0};
        std::array<Real64, numColumnVisualTbl> ZoneLightingLevelOccuHourBins = {0.0};
        std::array<Real64, numColumnVisualTbl> ZoneLightingLevelOccupiedHourBins = {0.0};

        // Default Constructor
        ZoneResilience()
            : ZoneNumOcc(0.0), ColdStressTempThresh(15.56), HeatStressTempThresh(30.0), PierceSET(-999.0), PMV(0.0), ZonePierceSET(-1.0),
              ZonePierceSETLastStep(-1.0), ZoneHeatIndex(0.0), ZoneHumidex(0.0), CrossedColdThresh(false), CrossedHeatThresh(false)
        {
        }
    };

    enum class HeatIndexMethod : int
    {
        Invalid = -1,
        Simplified,
        Extended,
        Num
    };
    static constexpr std::array<std::string_view, static_cast<int>(HeatIndexMethod::Num)> HeatIndexMethodUC = {
        "SIMPLIFIED",
        "EXTENDED",
    };

    struct ZoneData : ZoneSpaceData
    {
        // Members
        int Multiplier = 1;     // Used in reporting and for systems calculations
        int ListMultiplier = 1; // For Zone Group object:  used in reporting and systems calculations
        int ListGroup = 0;      // used only in Zone Group verification.  and for error message.
        Real64 RelNorth = 0.0;  // Relative North (to building north) [Degrees]
        Real64 OriginX = 0.0;   // X origin  [m]
        Real64 OriginY = 0.0;   // Y origin  [m]
        Real64 OriginZ = 0.0;   // Z origin  [m]
        int OfType = 1;         // 1=Standard Zone, Not yet used:
        // 2=Plenum Zone, 11=Solar Wall, 12=Roof Pond
        Real64 UserEnteredFloorArea = Constant::AutoCalculate; // User input floor area for this zone
        // Calculated after input
        Real64 geometricFloorArea = 0.0;   // Calculated floor area including air boundary surfaces
        Real64 CeilingArea = 0.0;          // Ceiling area excluding air boundary surfaces
        Real64 geometricCeilingArea = 0.0; // Ceiling area area including air boundary surfaces
        bool ceilingHeightEntered = false; // True is user input ceiling height
        bool HasFloor = false;             // Has "Floor" surface
        bool HasRoof = false;              // Has "Roof" or "Ceiling" Surface
        bool HasWindow = false;            // Window(s) present in this zone
        Real64 AirCapacity = 0.0;
        Real64 ExtWindowArea = 0.0;               // Exterior Window Area for Zone
        Real64 ExtWindowArea_Multiplied = 0.0;    // Exterior Window Area for Zone with multipliers
        Real64 ExtGrossWallArea_Multiplied = 0.0; // Exterior Wall Area for Zone (Gross) with multipliers
        Real64 ExtNetWallArea = 0.0;              // Exterior Wall Area for Zone (Net)
        Real64 TotalSurfArea = 0.0;               // Total surface area for Zone
        // (ignoring windows as they will be included in their base surfaces)
        Real64 ExteriorTotalGroundSurfArea = 0.0;                  // Total surface area of all surfaces for Zone with ground contact
        Real64 ExtGrossGroundWallArea = 0.0;                       // Ground contact Wall Area for Zone (Gross)
        Real64 ExtGrossGroundWallArea_Multiplied = 0.0;            // Ground contact Wall Area for Zone (Gross) with multipliers
        bool IsSupplyPlenum = false;                               // True when this zone is a supply plenum
        bool IsReturnPlenum = false;                               // True when this zone is a return plenum
        int PlenumCondNum = 0;                                     // Supply or return plenum conditions number, 0 if this is not a plenum zone
        int TempControlledZoneIndex = 0;                           // this is the index number for TempControlledZone structure for lookup
        int humidityControlZoneIndex = 0;                          // this is the index number for HumidityControlZone structure for lookup
        int AllSurfaceFirst = 0;                                   // First surface in zone including air boundaries
        int AllSurfaceLast = -1;                                   // Last  surface in zone including air boundaries
        Convect::HcInt IntConvAlgo = Convect::HcInt::ASHRAESimple; // Ref: appropriate values for Inside Convection solution
        int NumSurfaces = 0;                                       // Number of surfaces for this zone
        int NumSubSurfaces = 0;                                    // Number of subsurfaces for this zone (windows, doors, tdd dome and diffusers)
        int NumShadingSurfaces = 0;                                // Number of shading surfaces for this zone
        Convect::HcExt ExtConvAlgo = Convect::HcExt::ASHRAESimple; // Ref: appropriate values for Outside Convection solution
        Vector Centroid;                                           // Center of the zone found by averaging wall, floor, and roof centroids
        Real64 MinimumX = 0.0;                                     // Minimum X value for entire zone
        Real64 MaximumX = 0.0;                                     // Maximum X value for entire zone
        Real64 MinimumY = 0.0;                                     // Minimum Y value for entire zone
        Real64 MaximumY = 0.0;                                     // Maximum Y value for entire zone
        Real64 MinimumZ = 0.0;                                     // Minimum Z value for entire zone
        Real64 MaximumZ = 0.0;                                     // Maximum Z value for entire zone
        std::vector<int> ZoneHTSurfaceList;                        // List of HT surfaces related to this zone (includes adjacent interzone surfaces)
        std::vector<int> ZoneIZSurfaceList;                        // List of interzone surfaces in this zone
        std::vector<int> ZoneHTNonWindowSurfaceList; // List of non-window HT surfaces related to this zone (includes adjacent interzone surfaces)
        std::vector<int> ZoneHTWindowSurfaceList;    // List of window surfaces related to this zone (includes adjacent interzone surfaces)
        int zoneRadEnclosureFirst = -1;              // For Zone resimulation, need a range of enclosures for CalcInteriorRadExchange
        int zoneRadEnclosureLast = -1;               // For Zone resimulation, need a range of enclosures for CalcInteriorRadExchange

        Real64 OutDryBulbTemp = 0.0;                 // Zone outside dry bulb air temperature (C)
        bool OutDryBulbTempEMSOverrideOn = false;    // if true, EMS is calling to override the surface's outdoor air temp
        Real64 OutDryBulbTempEMSOverrideValue = 0.0; // value to use for EMS override of outdoor air drybulb temp (C)
        Real64 OutWetBulbTemp = 0.0;                 // Zone outside wet bulb air temperature (C)
        bool OutWetBulbTempEMSOverrideOn = false;    // if true, EMS is calling to override the surface's outdoor wetbulb
        Real64 OutWetBulbTempEMSOverrideValue = 0.0; // value to use for EMS override of outdoor air wetbulb temp (C)
        Real64 WindSpeed = 0.0;                      // Zone outside wind speed (m/s)
        bool WindSpeedEMSOverrideOn = false;         // if true, EMS is calling to override the surface's outside wind speed
        Real64 WindSpeedEMSOverrideValue = 0.0;      // value to use for EMS override of the surface's outside wind speed
        Real64 WindDir = 0.0;                        // Zone outside wind direction (degree)
        bool WindDirEMSOverrideOn = false;           // if true, EMS is calling to override the surface's outside wind direction
        Real64 WindDirEMSOverrideValue = 0.0;        // value to use for EMS override of the surface's outside wind speed

        int LinkedOutAirNode = 0; // Index of the an OutdoorAir:Node,, zero if none

        bool isPartOfTotalArea = true;           // Count the zone area when determining the building total floor area
        bool isNominalOccupied = false;          // has occupancy nominally specified
        bool isNominalControlled = false;        // has Controlled Zone Equip Configuration reference
        Real64 minOccupants = 0.0;               // minimum occupancy (sum of NomMinNumberPeople for the zone People objects, not multiplied)
        Real64 maxOccupants = 0.0;               // maximum occupancy (sum of NomMaxNumberPeople for the zone People objects, not multiplied)
        int AirHBimBalanceErrIndex = 0;          // error management counter
        bool NoHeatToReturnAir = false;          // TRUE means that heat to return air should be added to the zone load
        bool RefrigCaseRA = false;               // TRUE means there is potentially heat removal from return air
        bool HasAdjustedReturnTempByITE = false; // TRUE means that return temp to return air is adjusted by return temperature of ITE object
        Real64 AdjustedReturnTempByITE = 0.0;    // Diff of the return temp from the zone mixed air temp adjusted by ITE object

        bool HasLtsRetAirGain = false;       // TRUE means that zone lights return air heat > 0.0 calculated from plenum temperature
        bool HasAirFlowWindowReturn = false; // TRUE means that zone has return air flow from windows
        // from refrigeration cases for this zone
        Real64 InternalHeatGains = 0.0;                       // internal loads (W)
        Real64 NominalInfilVent = 0.0;                        // internal infiltration/ventilation
        Real64 NominalMixing = 0.0;                           // internal mixing/cross mixing
        bool TempOutOfBoundsReported = false;                 // if any temp out of bounds errors, first will show zone details.
        bool EnforcedReciprocity = false;                     // if zone/space required forced reciprocity -- less out of bounds temp errors allowed
        Sched::Schedule *zoneMinCO2Sched = nullptr;           // Index for the schedule the schedule which determines minimum CO2 concentration
        Sched::Schedule *zoneMaxCO2Sched = nullptr;           // Index for the schedule the schedule which determines maximum CO2 concentration
        Sched::Schedule *zoneContamControllerSched = nullptr; // Index for this schedule
        bool FlagCustomizedZoneCap = false;                   // True if customized Zone Capacitance Multiplier is used
        std::vector<Constant::eFuel> otherEquipFuelTypeNums;  // List of fuel types used by other equipment in this zone

        // Hybrid Modeling
        Real64 ZoneMeasuredTemperature = 0.0;               // Measured zone air temperature input by user
        Real64 ZoneMeasuredHumidityRatio = 0.0;             // Measured zone air humidity ratio by user
        Real64 ZoneMeasuredCO2Concentration = 0.0;          // Measured zone air CO2 concentration input by user
        Real64 ZoneMeasuredSupplyAirTemperature = 0.0;      // Measured zone supply air temperature input by user
        Real64 ZoneMeasuredSupplyAirFlowRate = 0.0;         // Measured zone supply air flow rate input by user
        Real64 ZoneMeasuredSupplyAirHumidityRatio = 0.0;    // Measured zone supply air flow rate input by user
        Real64 ZoneMeasuredSupplyAirCO2Concentration = 0.0; // Measured zone supply air flow rate input by user
        Real64 ZonePeopleActivityLevel = 0.0;               // People activity level input by user
        Real64 ZonePeopleSensibleHeatFraction = 0.0;        // People activity level input by user
        Real64 ZonePeopleRadiantHeatFraction = 0.0;         // People activity level input by user
        Real64 ZoneVolCapMultpSens = 1.0;                   // Zone temperature capacity multiplier, i.e. internal thermal mass multiplier
        Real64 ZoneVolCapMultpMoist = 1.0;                  // Zone humidity capacity multiplier
        Real64 ZoneVolCapMultpCO2 = 1.0;                    // Zone carbon dioxide capacity multiplier
        Real64 ZoneVolCapMultpGenContam = 1.0;              // Zone generic contaminant capacity multiplier
        Real64 ZoneVolCapMultpSensHM = 1.0;                 // Calculated temperature capacity multiplier by hybrid model
        Real64 ZoneVolCapMultpSensHMSum = 0.0;              // for temperature capacity multiplier average calculation
        Real64 ZoneVolCapMultpSensHMCountSum = 0.0;         // for temperature capacity multiplier average calculation
        Real64 ZoneVolCapMultpSensHMAverage = 1.0;          // Temperature capacity multiplier average
        Real64 MCPIHM = 0.0;                                // Calculated mass flow rate by hybrid model
        Real64 InfilOAAirChangeRateHM = 0.0;                // Calculated infiltration air change per hour by hybrid model
        Real64 NumOccHM = 0.0;                              // Inversely solved people count
        Real64 delta_T = 0.0;                               // Indoor and outdoor temperature
        Real64 delta_HumRat = 0.0;                          // Indoor and outdoor humidity ratio delta

        bool zoneOAQuadratureSum = false; // True when zone OA balance method is Quadrature
        int zoneOABalanceIndex = 0;       // Index to ZoneAirBalance for this zone, if any

        // Spaces
        EPVector<int> spaceIndexes; // Indexes to spaces in this zone
        int numSpaces = 0;          // Number of spaces in this zone

        // Default Constructor
        ZoneData() : Centroid(0.0, 0.0, 0.0)
        {
        }

        void SetOutBulbTempAt(EnergyPlusData &state);

        void SetWindSpeedAt(EnergyPlusData const &state, Real64 fac);

        void SetWindDirAt(Real64 fac);

        Real64 sumHATsurf(EnergyPlusData &state);
    };

    struct ZoneListData
    {
        // Members
        std::string Name;                         // Zone List name
        int NumOfZones = 0;                       // Number of zones in the list
        std::string::size_type MaxZoneNameLength; // Max Name length of zones in the list
        Array1D_int Zone;                         // Pointers to zones in the list

        // Default Constructor
        ZoneListData() : MaxZoneNameLength(0u)
        {
        }
    };

    struct ZoneGroupData
    {
        // Members
        std::string Name;   // Zone Group name
        int ZoneList = 0;   // Pointer to the zone list
        int Multiplier = 1; // Zone List multiplier
    };

    enum class ClothingType
    {
        Invalid = -1,
        InsulationSchedule,
        DynamicAshrae55,
        CalculationSchedule,
        Num
    };
    constexpr std::array<std::string_view, static_cast<int>(ClothingType::Num)> clothingTypeNamesUC = {
        "CLOTHINGINSULATIONSCHEDULE", "DYNAMICCLOTHINGMODELASHRAE55", "CALCULATIONMETHODSCHEDULE"};
    constexpr std::array<std::string_view, static_cast<int>(ClothingType::Num)> clothingTypeEIOStrings = {
        "Clothing Insulation Schedule,", "Dynamic Clothing Model ASHRAE55,", "Calculation Method Schedule,"};

    struct PeopleData
    {
        // Members
        std::string Name;                 // PEOPLE object name
        int ZonePtr = 0;                  // Zone index for this people statement
        int spaceIndex = 0;               // Space index for this people statement
        Real64 NumberOfPeople = 0.0;      // Maximum number of people for this statement
        Sched::Schedule *sched = nullptr; // schedule for number of people
        bool EMSPeopleOn = false;         // EMS actuating number of people if .TRUE.
        Real64 EMSNumberOfPeople = 0.0;   // Value EMS is directing to use for override
        // Note that the schedule and maximum number was kept for people since it seemed likely that
        // users would want to assign the same schedule to multiple people statements.
        Sched::Schedule *activityLevelSched = nullptr; // schedule for activity level
        Real64 FractionRadiant = 0.0;                  // Percentage (fraction 0.0-1.0) of sensible heat gain from people
        // that is radiant
        Real64 FractionConvected = 0.0; // Percentage (fraction 0.0-1.0) of sensible heat gain from people
        // that is convective
        Real64 NomMinNumberPeople = 0.0;          // Nominal Minimum Number of People (min sch X number of people)
        Real64 NomMaxNumberPeople = 0.0;          // Nominal Maximum Number of People (min sch X number of people)
        Sched::Schedule *workEffSched = nullptr;  // schedule for work efficiency
        Sched::Schedule *clothingSched = nullptr; // schedule for clothing insulation
        Sched::Schedule *clothingMethodSched = nullptr;
        ClothingType clothingType = ClothingType::Invalid; // Clothing type
        Sched::Schedule *airVelocitySched = nullptr;       // schedule for air velocity in zone
        Sched::Schedule *ankleAirVelocitySched = nullptr;  // schedule for ankle air velocity in zone
        bool Fanger = false;                               // True when Fanger calculation to be performed
        bool Pierce = false;                               // True when Pierce 2-node calculation to be performed
        bool KSU = false;                                  // True when KSU 2-node calculation to be performed
        bool AdaptiveASH55 = false;                        // True when ASHRAE Standard 55 adaptive comfort calculation
        //   to be performed
        bool AdaptiveCEN15251 = false; // True when CEN Standard 15251 adaptive comfort calculation
        //   to be performed
        bool CoolingEffectASH55 = false;                         // True when ASHRAE Standard 55 cooling effect calculation to be performed
        bool AnkleDraftASH55 = false;                            // True when ASHRAE Standard 55 ankle draft calculation to be performed
        CalcMRT MRTCalcType = DataHeatBalance::CalcMRT::Invalid; // MRT calculation type (See MRT Calculation type parameters)
        int SurfacePtr = -1;                                     // Pointer to the name of surface
        std::string AngleFactorListName;                         // Name of angle factor list
        int AngleFactorListPtr = -1;                             // Pointer to the name of angle factor list
        Real64 UserSpecSensFrac = 0.0;                           // User specified sensible fraction
        bool Show55Warning = false;                              // show the warning messages about ASHRAE 55-2004
        Real64 CO2RateFactor = 0.0;                              // Carbon Dioxide Generation Rate [m3/s-W]
        // Report variables
        Real64 NumOcc = 0.0;            // Number of occupants at current timestep []
        Real64 TemperatureInZone = 0.0; // Temperature in zone (C)
        Real64 ColdStressTempThresh = 15.56;
        Real64 HeatStressTempThresh = 30.0;
        Real64 RelativeHumidityInZone = 0.0; // Relative humidity in zone
        Real64 RadGainRate = 0.0;            // Radiant heat gain [W]
        Real64 ConGainRate = 0.0;            // Convective heat gain [W]
        Real64 SenGainRate = 0.0;            // Sensible heat gain [W]
        Real64 LatGainRate = 0.0;            // Latent heat gain [W]
        Real64 TotGainRate = 0.0;            // Total heat gain [W]
        Real64 CO2GainRate = 0.0;            // Carbon Dioxide Gain Rate [m3/s]
        Real64 RadGainEnergy = 0.0;          // Radiant heat gain [J]
        Real64 ConGainEnergy = 0.0;          // Convective heat gain [J]
        Real64 SenGainEnergy = 0.0;          // Sensible heat gain [J]
        Real64 LatGainEnergy = 0.0;          // Latent heat gain [J]
        Real64 TotGainEnergy = 0.0;          // Total heat gain [J]
        // Air velocity check during run time for thermal comfort control
        int AirVelErrIndex = 0; // Air velocity error index
        // For AdaptiveComfort tabular report
        Real64 TimeNotMetASH5580 = 0.0;
        Real64 TimeNotMetASH5590 = 0.0;
        Real64 TimeNotMetCEN15251CatI = 0.0;
        Real64 TimeNotMetCEN15251CatII = 0.0;
        Real64 TimeNotMetCEN15251CatIII = 0.0;
    };

    struct LightsData
    {
        // Members
        std::string Name;                 // LIGHTS object name
        int ZonePtr = 0;                  // Which zone lights are in
        int spaceIndex = 0;               // Space index for this lights instance
        Sched::Schedule *sched = nullptr; // Schedule for lights
        Real64 DesignLevel = 0.0;         // design level for lights [W]
        bool EMSLightsOn = false;         // EMS actuating Lighting power if .TRUE.
        Real64 EMSLightingPower = 0.0;    // Value EMS is directing to use for override
        Real64 FractionReturnAir = 0.0;   // Percentage (fraction 0.0-1.0) of sensible heat gain that is return air
        Real64 FractionRadiant = 0.0;     // Percentage (fraction 0.0-1.0) of sensible heat gain that is radiant
        Real64 FractionShortWave = 0.0;   // Percentage (fraction 0.0-1.0) of sensible heat gain that is short wave
        Real64 FractionReplaceable = 0.0; // Percentage (fraction 0.0-1.0) of sensible heat gain that is replaceable
        Real64 FractionConvected = 0.0;   // Percentage (fraction 0.0-1.0) of sensible heat gain that is convective
        bool FractionReturnAirIsCalculated = false;
        Real64 FractionReturnAirPlenTempCoeff1 = 0.0;
        Real64 FractionReturnAirPlenTempCoeff2 = 0.0;
        int ZoneReturnNum = 1;          // zone return index (not the node number) for return heat gain
        std::string RetNodeName;        // Zone return node name
        int ZoneExhaustNodeNum = 0;     // Exhaust node number
        Real64 NomMinDesignLevel = 0.0; // Nominal Minimum Design Level (min sch X design level)
        Real64 NomMaxDesignLevel = 0.0; // Nominal Maximum Design Level (max sch X design level)
        bool ManageDemand = false;      // Flag to indicate whether to use demand limiting
        Real64 DemandLimit = 0.0;       // Demand limit set by demand manager [W]
        // Report variables
        Real64 Power = 0.0;              // Electric power [W]
        Real64 RadGainRate = 0.0;        // Radiant heat gain [W]
        Real64 VisGainRate = 0.0;        // Visible heat gain [W]
        Real64 ConGainRate = 0.0;        // Convective heat gain [W]
        Real64 RetAirGainRate = 0.0;     // Return air heat gain [W]
        Real64 TotGainRate = 0.0;        // Total heat gain [W]
        Real64 Consumption = 0.0;        // Electric consumption [J]
        Real64 RadGainEnergy = 0.0;      // Radiant heat gain [J]
        Real64 VisGainEnergy = 0.0;      // Visible heat gain [J]
        Real64 ConGainEnergy = 0.0;      // Convective heat gain [J]
        Real64 RetAirGainEnergy = 0.0;   // Return air heat gain [J]
        Real64 TotGainEnergy = 0.0;      // Total heat gain [J]
        std::string EndUseSubcategory;   // user defined name for the end use category
        Real64 SumConsumption = 0.0;     // sum of electric consumption [J] for reporting
        Real64 SumTimeNotZeroCons = 0.0; // sum of time of positive electric consumption [hr]
    };

    struct ZoneEquipData // Electric, Gas, Other Equipment, CO2
    {
        // Members
        std::string Name;                    // EQUIPMENT object name
        int ZonePtr = 0;                     // Which zone internal gain is in
        int spaceIndex = 0;                  // Space index for this equipment instance
        Sched::Schedule *sched = nullptr;    // Schedule for internal gain
        Real64 DesignLevel = 0.0;            // design level for internal gain [W]
        bool EMSZoneEquipOverrideOn = false; // EMS actuating equipment power if .TRUE.
        Real64 EMSEquipPower = 0.0;          // Value EMS is directing to use for override
        Real64 FractionLatent = 0.0;         // Percentage (fraction 0.0-1.0) of sensible heat gain that is latent
        Real64 FractionRadiant = 0.0;        // Percentage (fraction 0.0-1.0) of sensible heat gain that is radiant
        Real64 FractionLost = 0.0;           // Percentage (fraction 0.0-1.0) of sensible heat gain that is lost
        Real64 FractionConvected = 0.0;      // Percentage (fraction 0.0-1.0) of sensible heat gain that is convective
        Real64 CO2DesignRate = 0.0;          // CO2 design Rate [m3/s]
        Real64 CO2RateFactor = 0.0;          // CO2 rate factor [m3/s/W]
        Real64 NomMinDesignLevel = 0.0;      // Nominal Minimum Design Level (min sch X design level)
        Real64 NomMaxDesignLevel = 0.0;      // Nominal Maximum Design Level (max sch X design level)
        bool ManageDemand = false;           // Flag to indicate whether to use demand limiting
        Real64 DemandLimit = 0.0;            // Demand limit set by demand manager [W]
        // Report variables
        Real64 Power = 0.0;                                            // Electric/Gas/Fuel power [W]
        Real64 RadGainRate = 0.0;                                      // Radiant heat gain [W]
        Real64 ConGainRate = 0.0;                                      // Convective heat gain [W]
        Real64 LatGainRate = 0.0;                                      // Latent heat gain [W]
        Real64 LostRate = 0.0;                                         // Lost energy (converted to work) [W]
        Real64 TotGainRate = 0.0;                                      // Total heat gain [W]
        Real64 CO2GainRate = 0.0;                                      // CO2 gain rate [m3/s]
        Real64 Consumption = 0.0;                                      // Electric/Gas/Fuel consumption [J]
        Real64 RadGainEnergy = 0.0;                                    // Radiant heat gain [J]
        Real64 ConGainEnergy = 0.0;                                    // Convective heat gain [J]
        Real64 LatGainEnergy = 0.0;                                    // Latent heat gain [J]
        Real64 LostEnergy = 0.0;                                       // Lost energy (converted to work) [J]
        Real64 TotGainEnergy = 0.0;                                    // Total heat gain [J]
        std::string EndUseSubcategory;                                 // user defined name for the end use category
        std::string otherEquipFuelTypeString;                          // Fuel Type string for Other Equipment
        Constant::eFuel OtherEquipFuelType = Constant::eFuel::Invalid; // Fuel Type Number of the Other Equipment
    };

    struct ExtVentedCavityStruct
    {
        // Members
        // from input data
        std::string Name;
        std::string OSCMName;                       // OtherSideConditionsModel
        int OSCMPtr;                                // OtherSideConditionsModel index
        Real64 Porosity;                            // fraction of absorber plate [--]
        Real64 LWEmitt;                             // Thermal Emissivity of Baffle Surface [dimensionless]
        Real64 SolAbsorp;                           // Solar Absorbtivity of Baffle Surface [dimensionless]
        Material::SurfaceRoughness BaffleRoughness; // surface roughness for exterior convection calcs.
        Real64 PlenGapThick;                        // Depth of Plenum Behind Baffle [m]
        int NumSurfs;                               // a single baffle can have multiple surfaces underneath it
        Array1D_int SurfPtrs;                       // = 0  ! array of pointers for participating underlying surfaces
        Real64 HdeltaNPL;                           // Height scale for Cavity buoyancy  [m]
        Real64 AreaRatio;                           // Ratio of actual surface are to projected surface area [dimensionless]
        Real64 Cv;                                  // volume-based effectiveness of openings for wind-driven vent when Passive
        Real64 Cd;                                  // discharge coefficient of openings for buoyancy-driven vent when Passive
        // data from elsewhere and calculated
        Real64 ActualArea;  // Overall Area of Collect with surface corrugations.
        Real64 ProjArea;    // Overall Area of Collector projected, as if flat [m2]
        Vector Centroid;    // computed centroid
        Real64 TAirCav;     // modeled drybulb temperature for air between baffle and wall [C]
        Real64 Tbaffle;     // modeled surface temperature for baffle[C]
        Real64 TairLast;    // Old Value for modeled drybulb temp of air between baffle and wall [C]
        Real64 TbaffleLast; // Old value for modeled surface temperature for baffle [C]
        Real64 HrPlen;      // Modeled radiation coef for OSCM [W/m2-C]
        Real64 HcPlen;      // Modeled Convection coef for OSCM [W/m2-C]
        Real64 MdotVent;    // air mass flow exchanging with ambient when passive.
        Real64 Tilt;        // Tilt from area weighted average of underlying surfaces
        Real64 Azimuth;     // Azimuth from area weighted average of underlying surfaces
        Real64 QdotSource;  // Source/sink term
        // reporting data
        Real64 Isc;              // total incident solar on baffle [W]
        Real64 PassiveACH;       // air changes per hour when passive [1/hr]
        Real64 PassiveMdotVent;  // Total Nat Vent air change rate  [kg/s]
        Real64 PassiveMdotWind;  // Nat Vent air change rate from Wind-driven [kg/s]
        Real64 PassiveMdotTherm; // Nat. Vent air change rate from buoyancy-driven flow [kg/s]

        // Default Constructor
        ExtVentedCavityStruct()
            : OSCMPtr(0), Porosity(0.0), LWEmitt(0.0), SolAbsorp(0.0), BaffleRoughness(Material::SurfaceRoughness::VeryRough), PlenGapThick(0.0),
              NumSurfs(0), HdeltaNPL(0.0), AreaRatio(0.0), Cv(0.0), Cd(0.0), ActualArea(0.0), ProjArea(0.0), Centroid(0.0, 0.0, 0.0), TAirCav(0.0),
              Tbaffle(0.0), TairLast(20.0), TbaffleLast(20.0), HrPlen(0.0), HcPlen(0.0), MdotVent(0.0), Tilt(0.0), Azimuth(0.0), QdotSource(0.0),
              Isc(0.0), PassiveACH(0.0), PassiveMdotVent(0.0), PassiveMdotWind(0.0), PassiveMdotTherm(0.0)
        {
        }
    };

    // ITE Equipment Environmental Class Data
    // MODULE PARAMETER DEFINITIONS:
    enum class ITEClass
    {
        Invalid = -1,
        None, // (0)
        A1,   // (1)
        A2,   // (2)
        A3,   // (3)
        A4,   // (4)
        B,    // (5)
        C,    // (6)
        H1,   // (7)
        Num
    };
    static constexpr std::array<std::string_view, static_cast<int>(ITEClass::Num)> ITEClassNamesUC = {"NONE", "A1", "A2", "A3", "A4", "B", "C", "H1"};

    enum class ITEInletConnection
    {
        Invalid = -1,
        AdjustedSupply,
        ZoneAirNode,
        RoomAirModel,
        Num
    };
    static constexpr std::array<std::string_view, static_cast<int>(ITEInletConnection::Num)> ITEInletConnectionNamesUC = {
        "ADJUSTEDSUPPLY", "ZONEAIRNODE", "ROOMAIRMODEL"};

    enum class PERptVars
    {
        CPU = 0,       // ITE CPU Electric Power/Energy
        Fan,           // ITE Fan Electric Power/Energy
        UPS,           // ITE UPS Electric Power/Energy
        CPUAtDesign,   // ITE CPU Electric Power/Energy at Design Inlet Conditions
        FanAtDesign,   // ITE Fan Electric Power/Energy at Design Inlet Conditions
        UPSGainToZone, // ITE UPS Heat Gain to Zone Power(Rate)/Energy - convective gain
        ConGainToZone, // ITE Total Heat Gain to Zone Power(Rate)/Energy - convective gain - includes heat gain from UPS, plus CPU and Fans if
                       // room air model not used
        Num
    };

    struct ITEquipData // IT Equipment
    {
        // Members
        std::string Name;                          // EQUIPMENT object name
        int ZonePtr = 0;                           // Which zone internal gain is in
        int spaceIndex = 0;                        // Space index for this equipment instance
        bool FlowControlWithApproachTemps = false; // True if using supply and return approach temperature for ITE object.
        Real64 DesignTotalPower = 0.0;             // Design level for internal gain [W]
        Real64 NomMinDesignLevel = 0.0;            // Nominal Minimum Design Level (min sch X design level)
        Real64 NomMaxDesignLevel = 0.0;            // Nominal Maximum Design Level (max sch X design level)
        Real64 DesignFanPowerFrac = 0.0;           // Fraction (0.0-1.0) of design power level that is fans
        Sched::Schedule *operSched = nullptr;      // Schedule for design power input or operating schedule
        Sched::Schedule *cpuLoadSched = nullptr;   // Schedule for CPU loading schedule
        Real64 SizingTAirIn = 0.0;                 // Entering air dry-bulb temperature at maximum value during sizing[C]
        Real64 DesignTAirIn = 0.0;                 // Design entering air dry-bulb temperature [C]
        Real64 DesignFanPower = 0.0;               // Design fan power input [W]
        Real64 DesignCPUPower = 0.0;               // Design CPU power input [W]
        Real64 DesignAirVolFlowRate = 0.0;         // Design air volume flow rate [m3/s]
        ITEClass Class = ITEClass::None;           // Environmental class index (A1=1, A2=2, A3=3, A4=4, B=5, C=6, H1=7)
        int AirFlowFLTCurve = 0;                   // Index for airflow function of CPULoadFrac (x) and TAirIn (y) curve
        int CPUPowerFLTCurve = 0;                  // Index for CPU power function of CPULoadFrac (x) and TAirIn (y) curve
        int FanPowerFFCurve = 0;                   // Index for fan power function of flow fraction curve
        ITEInletConnection AirConnectionType = ITEInletConnection::AdjustedSupply; // Air connection type (AdjustedSupply, ZoneAirNode, RoomAirModel)
        int InletRoomAirNodeNum = 0;                                               // Room air model node number for air inlet
        int OutletRoomAirNodeNum = 0;                                              // Room air model node number for air outlet
        int SupplyAirNodeNum = 0;                                                  // Node number for supply air inlet
        Real64 DesignRecircFrac = 0.0;                                             // Design recirculation fraction (0.0-0.5)
        int RecircFLTCurve = 0;                             // Index for recirculation function of CPULoadFrac (x) and TSupply (y) curve
        Real64 DesignUPSEfficiency = 0.0;                   // Design power supply efficiency (>0.0 - 1.0)
        int UPSEfficFPLRCurve = 0;                          // Index for recirculation function of part load ratio
        Real64 UPSLossToZoneFrac = 0.0;                     // Fraction of UPS power loss to zone (0.0 - 1.0); remainder is lost
        std::string EndUseSubcategoryCPU;                   // User defined name for the end use category for the CPU
        std::string EndUseSubcategoryFan;                   // User defined name for the end use category for the Fans
        std::string EndUseSubcategoryUPS;                   // User defined name for the end use category for the power supply
        bool EMSCPUPowerOverrideOn = false;                 // EMS actuating CPU power if .TRUE.
        Real64 EMSCPUPower = 0.0;                           // Value EMS is directing to use for override of CPU power [W]
        bool EMSFanPowerOverrideOn = false;                 // EMS actuating Fan power if .TRUE.
        Real64 EMSFanPower = 0.0;                           // Value EMS is directing to use for override of Fan power [W]
        bool EMSUPSPowerOverrideOn = false;                 // EMS actuating UPS power if .TRUE.
        Real64 EMSUPSPower = 0.0;                           // Value EMS is directing to use for override of UPS power [W]
        Real64 SupplyApproachTemp = 0.0;                    // The difference of the IT inlet temperature from the AHU supply air temperature
        Sched::Schedule *supplyApproachTempSched = nullptr; // The difference schedule of the IT inlet temperature from the AHU supply air temperature
        Real64 ReturnApproachTemp = 0.0;                    // The difference of the unit outlet temperature from the well mixed zone temperature
        Sched::Schedule *returnApproachTempSched =
            nullptr;                   // The difference schedule of the unit outlet temperature from the well mixed zone temperature
        bool inControlledZone = false; // True if in a controlled zone

        // Report variables
        std::array<Real64, (int)PERptVars::Num> PowerRpt;
        std::array<Real64, (int)PERptVars::Num> EnergyRpt;

        Real64 AirVolFlowStdDensity = 0.0; // Air volume flow rate at standard density [m3/s]
        Real64 AirVolFlowCurDensity = 0.0; // Air volume flow rate at current density [m3/s]
        Real64 AirMassFlow = 0.0;          // Air mass flow rate [kg/s]
        Real64 AirInletDryBulbT = 0.0;     // Air inlet dry-bulb temperature [C]
        Real64 AirInletDewpointT = 0.0;    // Air inlet dewpoint temperature [C]
        Real64 AirInletRelHum = 0.0;       // Air inlet relative humidity [%]
        Real64 AirOutletDryBulbT = 0.0;    // Air outlet dry-bulb temperature [C]
        Real64 SHI = 0.0;                  // Supply Heat Index []
        Real64 TimeOutOfOperRange = 0.0;   // ITE Air Inlet Operating Range Exceeded Time [hr]
        Real64 TimeAboveDryBulbT = 0.0;    // ITE Air Inlet Dry-Bulb Temperature Above Operating Range Time [hr]
        Real64 TimeBelowDryBulbT = 0.0;    // ITE Air Inlet Dry-Bulb Temperature Below Operating Range Time [hr]
        Real64 TimeAboveDewpointT = 0.0;   // ITE Air Inlet Dewpoint Temperature Above Operating Range Time [hr]
        Real64 TimeBelowDewpointT = 0.0;   // ITE Air Inlet Dewpoint Temperature Below Operating Range Time [hr]
        Real64 TimeAboveRH = 0.0;          // ITE Air Inlet Relative Humidity Above Operating Range Time [hr]
        Real64 TimeBelowRH = 0.0;          // ITE Air Inlet Relative Humidity Below Operating Range Time [hr]
        Real64 DryBulbTAboveDeltaT = 0.0;  // ITE Air Inlet Dry-Bulb Temperature Difference Above Operating Range [deltaC]
        Real64 DryBulbTBelowDeltaT = 0.0;  // ITE Air Inlet Dry-Bulb Temperature Difference Below Operating Range [deltaC]
        Real64 DewpointTAboveDeltaT = 0.0; // ITE Air Inlet Dewpoint Temperature Difference Above Operating Range [deltaC]
        Real64 DewpointTBelowDeltaT = 0.0; // ITE Air Inlet Dewpoint Temperature Difference Below Operating Range [deltaC]
        Real64 RHAboveDeltaRH = 0.0;       // ITE Air Inlet Relative Humidity Difference Above Operating Range [%]
        Real64 RHBelowDeltaRH = 0.0;       // ITE Air Inlet Relative Humidity Difference Below Operating Range [%]
    };

    struct BBHeatData
    {
        // Members
        std::string Name; // BASEBOARD HEAT object name
        int ZonePtr = 0;
        int spaceIndex = 0; // Space index for this equipment instance
        Sched::Schedule *sched = nullptr;
        Real64 CapatLowTemperature = 0.0;
        Real64 LowTemperature = 0.0;
        Real64 CapatHighTemperature = 0.0;
        Real64 HighTemperature = 0.0;
        bool EMSZoneBaseboardOverrideOn = false; // EMS actuating equipment power if .TRUE.
        Real64 EMSZoneBaseboardPower = 0.0;      // Value EMS is directing to use for override
        Real64 FractionRadiant = 0.0;
        Real64 FractionConvected = 0.0;
        bool ManageDemand = false; // Flag to indicate whether to use demand limiting
        Real64 DemandLimit = 0.0;  // Demand limit set by demand manager [W]
        // Report variables
        Real64 Power = 0.0;            // Electric power [W]
        Real64 RadGainRate = 0.0;      // Radiant heat gain [W]
        Real64 ConGainRate = 0.0;      // Convective heat gain [W]
        Real64 TotGainRate = 0.0;      // Total heat gain [W]
        Real64 Consumption = 0.0;      // Electric consumption [J]
        Real64 RadGainEnergy = 0.0;    // Radiant heat gain [J]
        Real64 ConGainEnergy = 0.0;    // Convective heat gain [J]
        Real64 TotGainEnergy = 0.0;    // Total heat gain [J]
        std::string EndUseSubcategory; // user defined name for the end use category
    };

    struct InfiltrationData
    {
        // Members
        std::string Name;
        int ZonePtr = 0;                                                  // Which zone infiltration is in
        int spaceIndex = 0;                                               // Space index for this infiltration instance
        Sched::Schedule *sched = nullptr;                                 // Schedule for infiltration
        InfiltrationModelType ModelType = InfiltrationModelType::Invalid; // which model is used for infiltration
        InfVentDensityBasis densityBasis = InfVentDensityBasis::Outdoor;  // which density is used to convert to mass flow
        // Design Flow Rate model terms
        Real64 DesignLevel = 0.0;
        Real64 ConstantTermCoef = 0.0;
        Real64 TemperatureTermCoef = 0.0;
        Real64 VelocityTermCoef = 0.0;
        Real64 VelocitySQTermCoef = 0.0;
        // Effective Leakage Area, Sherman Grimsrud terms
        Real64 LeakageArea = 0.0;           // "AL" effective air leakage area
        Real64 BasicStackCoefficient = 0.0; // "Cs" Stack coefficient
        Real64 BasicWindCoefficient = 0.0;  // "Cw" wind coefficient
        // Flow Coefficient, AIM-2, Walker and Wilson terms
        Real64 FlowCoefficient = 0.0;              // "c" Flow coefficient
        Real64 AIM2StackCoefficient = 0.0;         // "Cs" stack coefficient
        Real64 AIM2WindCoefficient = 0.0;          // "Cw" wind coefficient
        Real64 PressureExponent = 0.0;             // "n" pressure power law exponent
        Real64 ShelterFactor = 0.0;                // "s" shelter factor
        bool EMSOverrideOn = false;                // if true then EMS is requesting to override
        Real64 EMSAirFlowRateValue = 0.0;          // value EMS is setting for air flow rate
        Real64 VolumeFlowRate = 0.0;               // infiltration air volume flow rate
        Real64 MassFlowRate = 0.0;                 // infiltration air mass flow rate
        Real64 MCpI_temp = 0.0;                    // INFILTRATION MASS FLOW * AIR SPECIFIC HEAT
        Real64 InfilHeatGain = 0.0;                // Heat Gain {J} due to infiltration
        Real64 InfilHeatLoss = 0.0;                // Heat Loss {J} due to infiltration
        Real64 InfilLatentGain = 0.0;              // Latent Gain {J} due to infiltration
        Real64 InfilLatentLoss = 0.0;              // Latent Loss {J} due to infiltration
        Real64 InfilTotalGain = 0.0;               // Total Gain {J} due to infiltration (sensible+latent)
        Real64 InfilTotalLoss = 0.0;               // Total Loss {J} due to infiltration (sensible+latent)
        Real64 InfilVolumeCurDensity = 0.0;        // Volume of Air {m3} due to infiltration at current zone air density
        Real64 InfilVolumeStdDensity = 0.0;        // Volume of Air {m3} due to infiltration at standard density (adjusted for elevation)
        Real64 InfilVdotCurDensity = 0.0;          // Volume flow rate of Air {m3/s} due to infiltration at current zone air density
        Real64 InfilVdotStdDensity = 0.0;          // Volume flow rate of Air {m3/s} due to infiltration standard density (adjusted elevation)
        Real64 InfilVdotOutDensity = 0.0;          // Volume flow rate of Air {m3/s} due to infiltration at current outdoor air density
        Real64 InfilMdot = 0.0;                    // Mass flow rate {kg/s} due to infiltration for reporting
        Real64 InfilMass = 0.0;                    // Mass of Air {kg} due to infiltration
        Real64 InfilAirChangeRateCurDensity = 0.0; // Infiltration air change rate {ach} at current zone air density
        Real64 InfilAirChangeRateStdDensity = 0.0; // Infiltration air change rate {ach} at standard density (adjusted elevation)
        Real64 InfilAirChangeRateOutDensity = 0.0; // Infiltration air change rate {ach} at current outdoor air density
    };

    struct VentilationData
    {
        // Members
        std::string Name;
        int ZonePtr = 0;
        int spaceIndex = 0; // Space index for this ventilation instance
        Sched::Schedule *availSched = nullptr;
        VentilationModelType ModelType = VentilationModelType::Invalid;  // DesignFlowRate or WindandStackOpenArea
        InfVentDensityBasis densityBasis = InfVentDensityBasis::Outdoor; // which density is used to convert to mass flow
        Real64 DesignLevel = 0.0;
        bool EMSSimpleVentOn = false;      // EMS actuating ventilation flow rate if .TRUE.
        Real64 EMSimpleVentFlowRate = 0.0; // Value EMS is directing to use for override
        Real64 MinIndoorTemperature = -100.0;
        Real64 DelTemperature = 0.0;
        VentilationType FanType = VentilationType::Natural;
        Real64 FanPressure = 0.0;
        Real64 FanEfficiency = 0.0;
        Real64 FanPower = 0.0;
        Real64 AirTemp = 0.0;
        Real64 ConstantTermCoef = 0.0;
        Real64 TemperatureTermCoef = 0.0;
        Real64 VelocityTermCoef = 0.0;
        Real64 VelocitySQTermCoef = 0.0;
        Real64 MaxIndoorTemperature = 100.0;
        Real64 MinOutdoorTemperature = -100.0;
        Real64 MaxOutdoorTemperature = 100.0;
        Real64 MaxWindSpeed = 40.0;
        Sched::Schedule *minIndoorTempSched = nullptr;            // Minimum indoor temperature schedule
        Sched::Schedule *maxIndoorTempSched = nullptr;            // Maximum indoor temperature schedule
        Sched::Schedule *deltaTempSched = nullptr;                // Delta temperature schedule
        Sched::Schedule *minOutdoorTempSched = nullptr;           // Minimum outdoor temperature schedule
        Sched::Schedule *maxOutdoorTempSched = nullptr;           // Maximum outdoor temperature schedule
        int IndoorTempErrCount = 0;                               // Indoor temperature error count
        int OutdoorTempErrCount = 0;                              // Outdoor temperature error count
        int IndoorTempErrIndex = 0;                               // Indoor temperature error Index
        int OutdoorTempErrIndex = 0;                              // Outdoor temperature error Index
        HybridCtrlType HybridControlType = HybridCtrlType::Indiv; // Hybrid ventilation control type: 0 Individual, 1 Close, 2 Global
        int HybridControlMasterNum = 0;                           // Hybrid ventilation control master object number
        bool HybridControlMasterStatus = false;                   // Hybrid ventilation control master object opening status
        // WindandStackOpenArea
        Real64 OpenArea = 0.0;                        // Opening area [m2]
        Sched::Schedule *openAreaFracSched = nullptr; // Opening area fraction schedule
        Real64 OpenEff = 0.0;                         // Opening effectiveness [dimensionless]
        Real64 EffAngle = 0.0;                        // Effective angle [degree]
        Real64 DH = 0.0;                              // Height difference [m]
        Real64 DiscCoef = 0.0;                        // Discharge coefficient
        Real64 MCP = 0.0;                             // Product of mass flow rate and Cp
    };

    struct ZoneAirBalanceData
    {
        // Members
        std::string Name;                            // Object name
        std::string ZoneName;                        // Zone name
        int ZonePtr = 0;                             // Zone number
        AirBalance BalanceMethod = AirBalance::None; // Air Balance Method
        Real64 InducedAirRate = 0.0;                 // Induced Outdoor Air Due to Duct Leakage Unbalance [m3/s]
        Sched::Schedule *inducedAirSched = nullptr;  // Induced Outdoor Air Fraction Schedule
        Real64 BalMassFlowRate = 0.0;                // balanced mass flow rate
        Real64 InfMassFlowRate = 0.0;                // unbalanced mass flow rate from infiltration
        Real64 NatMassFlowRate = 0.0;                // unbalanced mass flow rate from natural ventilation
        Real64 ExhMassFlowRate = 0.0;                // unbalanced mass flow rate from exhaust ventilation
        Real64 IntMassFlowRate = 0.0;                // unbalanced mass flow rate from intake ventilation
        Real64 ERVMassFlowRate = 0.0;                // unbalanced mass flow rate from stand-alone ERV
        bool OneTimeFlag = false;                    // One time flag to get nodes of stand alone ERV
        int NumOfERVs = 0;                           // Number of zone stand alone ERVs
        Array1D_int ERVInletNode;                    // Stand alone ERV supply air inlet nodes
        Array1D_int ERVExhaustNode;                  // Stand alone ERV air exhaust nodes
    };

    struct MixingData
    {
        // Members
        std::string Name;
        int ZonePtr = 0;
        int spaceIndex = 0; // Space index for this mixing instance
        Sched::Schedule *sched = nullptr;
        Real64 DesignLevel = 0.0;
        int FromZone = 0;
        int fromSpaceIndex = 0; // Source space index for this mixing instance
        Real64 DeltaTemperature = 0.0;
        Real64 DesiredAirFlowRate = 0.0;
        Real64 DesiredAirFlowRateSaved = 0.0;
        Real64 MixingMassFlowRate = 0.0;
        Sched::Schedule *deltaTempSched = nullptr;                // Delta temperature schedule index
        Sched::Schedule *minIndoorTempSched = nullptr;            // Minimum indoor temperature schedule index
        Sched::Schedule *maxIndoorTempSched = nullptr;            // Maximum indoor temperature schedule index
        Sched::Schedule *minSourceTempSched = nullptr;            // Minimum source zone temperature schedule index
        Sched::Schedule *maxSourceTempSched = nullptr;            // Maximum source zone temperature schedule index
        Sched::Schedule *minOutdoorTempSched = nullptr;           // Minimum outdoor temperature schedule index
        Sched::Schedule *maxOutdoorTempSched = nullptr;           // Maximum outdoor temperature schedule index
        int IndoorTempErrCount = 0;                               // Indoor temperature error count
        int SourceTempErrCount = 0;                               // Source zone temperature error count
        int OutdoorTempErrCount = 0;                              // Outdoor temperature error count
        int IndoorTempErrIndex = 0;                               // Indoor temperature error Index
        int SourceTempErrIndex = 0;                               // Source zone temperature error Index
        int OutdoorTempErrIndex = 0;                              // Outdoor temperature error Index
        HybridCtrlType HybridControlType = HybridCtrlType::Indiv; // Hybrid ventilation control type: 0 Individual, 1 Close, 2 Global
        int HybridControlMasterNum = 0;                           // Hybrid ventilation control master ventilation object number
        int NumRefDoorConnections = 0;
        bool EMSSimpleMixingOn = false;      // EMS actuating ventilation flow rate if .TRUE.
        bool RefDoorMixFlag = false;         // Refrigeration door mixing within zone
        bool ReportFlag = false;             // TRUE when Mixing or Cross Mixing is active based on controls
        Real64 EMSimpleMixingFlowRate = 0.0; // Value EMS is directing to use for override
        Array1D_bool EMSRefDoorMixingOn;
        Array1D<Real64> EMSRefDoorFlowRate;
        Array1D<Real64> VolRefDoorFlowRate;
        Array1D<Sched::Schedule *> openScheds; // Schedule for Refrigeration door open fraction
        Array1D<Real64> DoorHeight;            // Door height for refrigeration door, m
        Array1D<Real64> DoorArea;              // Door area for refrigeration door, m2
        Array1D<Real64> Protection;            // Refrigeration door protection factor, dimensionless
        Array1D_int MateZonePtr;               // Zone connected by refrigeration door (MateZone > ZonePtr)
        Array1D_string DoorMixingObjectName;   // Used in one error statement and eio
        Array1D_string DoorProtTypeName;       // Used in eio
                                               // Note, for mixing and crossmixing, this type dimensioned by number of mixing objects.
        // For ref door mixing, dimensioned by number of zones.
    };

    struct AirBoundaryMixingSpecs
    {
        int space1;                       // Air boundary simple mixing space 1
        int space2;                       // Air boundary simple mixing space 2
        Sched::Schedule *sched = nullptr; // Air boundary simple mixing schedule index
        Real64 mixingVolumeFlowRate;      // Air boundary simple mixing volume flow rate [m3/s]
    };

    struct ZoneAirMassFlowConservation
    {
        // Members
        bool EnforceZoneMassBalance = false; // flag to enforce zone air mass conservation
        AdjustmentType ZoneFlowAdjustment =
            AdjustmentType::NoAdjustReturnAndMixing; // determines how zone air flow is adjusted (AdjustMixingOnly, AdjustReturnOnly,
                                                     // AdjustMixingThenReturn, AdjustReturnThenMixing, None)        int InfiltrationTreatment;   //
                                                     // determines how infiltration is treated for zone mass balance
        InfiltrationFlow InfiltrationTreatment = InfiltrationFlow::No;             // determines how infiltration is treated for zone mass balance
        InfiltrationZoneType InfiltrationForZones = InfiltrationZoneType::Invalid; // specifies which types of zones allow infiltration to be changed
        bool AdjustZoneMixingFlow = false;                                         // used to adjust zone mixing air flows to enforce air flow balance
        bool AdjustZoneInfiltrationFlow = false; // used to adjust zone infiltration air flows to enforce air flow balance
                                                 // Note, unique global object
    };

    struct ZoneMassConservationData
    {
        // Members
        std::string Name;
        int ZonePtr = 0;                       // pointer to the mixing zone
        Real64 InMassFlowRate = 0.0;           // zone total supply air mass flow rate, kg/s
        Real64 ExhMassFlowRate = 0.0;          // zone exhaust total air mass flow rate, kg/s
        Real64 RetMassFlowRate = 0.0;          // zone return air mass flow rate, kg/s
        Real64 MixingMassFlowRate = 0.0;       // zone mixing air mass flow rate, kg/s
        Real64 MixingSourceMassFlowRate = 0.0; // Zone source mass flow rate for mixing zone, kg/s
        int NumSourceZonesMixingObject = 0;    // number of zone mixing object references as a source zone
        int NumReceivingZonesMixingObject = 0; // number of zone mixing object references as a receiving zone
        bool IsOnlySourceZone = false;         // true only if used only as a source zone in zone mixing object
        bool IsSourceAndReceivingZone = false; // true only if a zone is used as a source and receiving zone in zone mixing objects
        int InfiltrationPtr = 0;               // pointer to infiltration object
        Real64 InfiltrationMassFlowRate = 0.0; // infiltration added to enforced source zone mass balance, kg/s
        int IncludeInfilToZoneMassBal = 0;     // not self-balanced, include infiltration in zone air mass balance
        Array1D_int ZoneMixingSourcesPtr;      // source zones pointer
        Array1D_int ZoneMixingReceivingPtr;    // receiving zones pointer
        Array1D<Real64> ZoneMixingReceivingFr; // receiving zones fraction
                                               // Note, this type dimensioned by number of zones
    };

    struct GenericComponentZoneIntGainStruct
    {
        // Members
        std::string CompObjectName;                   // device user unique name
        IntGainType CompType = IntGainType::Invalid;  // type of internal gain device identifier
        Real64 spaceGainFrac = 1.0;                   // Fraction of gain value assigned to this Space (because gain rate might be for an entire zone)
        Real64 *PtrConvectGainRate = nullptr;         // POINTER to value of convection heat gain rate for device, watts
        Real64 ConvectGainRate = 0.0;                 // current timestep value of convection heat gain rate for device, watts
        Real64 *PtrReturnAirConvGainRate = nullptr;   // POINTER to value of return air convection heat gain rate for device, W
        Real64 ReturnAirConvGainRate = 0.0;           // current timestep value of return air convection heat gain rate for device, W
        Real64 *PtrRadiantGainRate = nullptr;         // POINTER to value of thermal radiation heat gain rate for device, watts
        Real64 RadiantGainRate = 0.0;                 // current timestep value of thermal radiation heat gain rate for device, watts
        Real64 *PtrLatentGainRate = nullptr;          // POINTER to value of moisture gain rate for device, Watts
        Real64 LatentGainRate = 0.0;                  // current timestep value of moisture gain rate for device, Watts
        Real64 *PtrReturnAirLatentGainRate = nullptr; // POINTER to value of return air moisture gain rate for device, Watts
        Real64 ReturnAirLatentGainRate = 0.0;         // current timestep value of return air moisture gain rate for device, Watts
        Real64 *PtrCarbonDioxideGainRate = nullptr;   // POINTER to value of carbon dioxide gain rate for device
        Real64 CarbonDioxideGainRate = 0.0;           // current timestep value of carbon dioxide gain rate for device
        Real64 *PtrGenericContamGainRate = nullptr;   // POINTER to value of generic contaminant gain rate for device
        Real64 GenericContamGainRate = 0.0;           // current timestep value of generic contaminant gain rate for device
        int ReturnAirNodeNum = 0;                     // return air node number for return air convection heat gain
    };

    struct SpaceZoneSimData // Calculated data by Space or Zone during each time step/hour
    {
        Real64 NOFOCC = 0.0; // Number of Occupants
        Real64 QLTSW = 0.0;  // VISIBLE ENERGY FROM LIGHTS
    };

    struct SpaceIntGainDeviceData
    {
        int numberOfDevices = 0;
        int maxNumberOfDevices = 0;
        Array1D<GenericComponentZoneIntGainStruct> device;
    };

    struct ZoneCatEUseData
    {
        // Members
        Array1D<Real64> EEConvected; // Category (0 to 25) Energy Convected from Electric Equipment
        Array1D<Real64> EERadiated;  // Category (0 to 25) Energy Radiated from Electric Equipment
        Array1D<Real64> EELost;      // Category (0 to 25) Energy from Electric Equipment (lost)
        Array1D<Real64> EELatent;    // Category (0 to 25) Latent Energy from Electric Equipment

        // Default Constructor
        ZoneCatEUseData() : EEConvected({0, 25}, 0.0), EERadiated({0, 25}, 0.0), EELost({0, 25}, 0.0), EELatent({0, 25}, 0.0)
        {
        }
    };

    struct RefrigCaseCreditData
    {
        // Members
        Real64 SenCaseCreditToZone = 0.0; // Refrigerated display case sensible energy delivered to zone
        // includes refrigeration pipe and receiver heat exchange with zone
        Real64 LatCaseCreditToZone = 0.0; // Refrigerated display case latent energy delivered to zone
        Real64 SenCaseCreditToHVAC = 0.0; // Refrigerated display case sensible energy delivered to HVAC RA duct
        Real64 LatCaseCreditToHVAC = 0.0; // Refrigerated display case latent energy delivered to HVAC RA duct

        // Reset to Zeros
        void reset()
        {
            SenCaseCreditToZone = 0.0;
            LatCaseCreditToZone = 0.0;
            SenCaseCreditToHVAC = 0.0;
            LatCaseCreditToHVAC = 0.0;
        }
    };

    struct HeatReclaimDataBase
    {
        // Members
        std::string Name;                                         // Name of Coil
        std::string SourceType;                                   // SourceType for Coil
        Real64 AvailCapacity = 0.0;                               // Total available heat reclaim capacity
        Real64 ReclaimEfficiencyTotal = 0.0;                      // Total reclaimed portion
        Real64 WaterHeatingDesuperheaterReclaimedHeatTotal = 0.0; // total reclaimed heat by water heating desuperheater coils
        Real64 HVACDesuperheaterReclaimedHeatTotal = 0.0;         // total reclaimed heat by water heating desuperheater coils
        Array1D<Real64> WaterHeatingDesuperheaterReclaimedHeat;   // heat reclaimed by water heating desuperheater coils
        Array1D<Real64> HVACDesuperheaterReclaimedHeat;           // heat reclaimed by water heating desuperheater coils
    };

    struct HeatReclaimRefrigCondenserData : HeatReclaimDataBase // inherited from base struct
    {
        // Customized Members
        Real64 AvailTemperature = 0.0; // Temperature of heat reclaim source
    };

    struct AirReportVars
    {
        // Members
        Real64 MeanAirTemp = 0.0;                   // Mean Air Temperature {C}
        Real64 OperativeTemp = 0.0;                 // Average of Mean Air Temperature {C} and Mean Radiant Temperature {C}
        Real64 WetbulbGlobeTemp = 0.0;              // Wet-bulb Globe Temperature
        Real64 MeanAirHumRat = 0.0;                 // Mean Air Humidity Ratio {kg/kg} (averaged over zone time step)
        Real64 MeanAirDewPointTemp = 0.0;           // Mean Air Dewpoint Temperature {C}
        Real64 ThermOperativeTemp = 0.0;            // Mix of MRT and MAT for Zone Control:Thermostatic:Operative Temperature {C}
        Real64 InfilHeatGain = 0.0;                 // Heat Gain {J} due to infiltration
        Real64 InfilHeatLoss = 0.0;                 // Heat Loss {J} due to infiltration
        Real64 InfilLatentGain = 0.0;               // Latent Gain {J} due to infiltration
        Real64 InfilLatentLoss = 0.0;               // Latent Loss {J} due to infiltration
        Real64 InfilTotalGain = 0.0;                // Total Gain {J} due to infiltration (sensible+latent)
        Real64 InfilTotalLoss = 0.0;                // Total Loss {J} due to infiltration (sensible+latent)
        Real64 InfilVolumeCurDensity = 0.0;         // Volume of Air {m3} due to infiltration at current zone air density
        Real64 InfilVolumeStdDensity = 0.0;         // Volume of Air {m3} due to infiltration at standard density (adjusted for elevation)
        Real64 InfilVdotCurDensity = 0.0;           // Volume flow rate of Air {m3/s} due to infiltration at current zone air density
        Real64 InfilVdotStdDensity = 0.0;           // Volume flow rate of Air {m3/s} due to infiltration standard density (adjusted elevation)
        Real64 InfilVdotOutDensity = 0.0;           // Volume flow rate of Air {m3/s} due to infiltration  at current outdoor density
        Real64 InfilMass = 0.0;                     // Mass of Air {kg} due to infiltration
        Real64 InfilMdot = 0.0;                     // Mass flow rate of Air (kg/s) due to infiltration
        Real64 InfilAirChangeRateCurDensity = 0.0;  // Infiltration air change rate {ach} at current zone air density
        Real64 InfilAirChangeRateStdDensity = 0.0;  // Infiltration air change rate {ach} at standard density (adjusted elevation)
        Real64 InfilAirChangeRateOutDensity = 0.0;  // Infiltration air change rate {ach} at current outdoor density
        Real64 VentilHeatLoss = 0.0;                // Heat Gain {J} due to ventilation
        Real64 VentilHeatGain = 0.0;                // Heat Loss {J} due to ventilation
        Real64 VentilLatentLoss = 0.0;              // Latent Gain {J} due to ventilation
        Real64 VentilLatentGain = 0.0;              // Latent Loss {J} due to ventilation
        Real64 VentilTotalLoss = 0.0;               // Total Gain {J} due to ventilation
        Real64 VentilTotalGain = 0.0;               // Total Loss {J} due to ventilation
        Real64 VentilVolumeCurDensity = 0.0;        // Volume of Air {m3} due to ventilation at current zone air density
        Real64 VentilVolumeStdDensity = 0.0;        // Volume of Air {m3} due to ventilation at standard density (adjusted for elevation)
        Real64 VentilVdotCurDensity = 0.0;          // Volume flow rate of Air {m3/s} due to ventilation at current zone air density
        Real64 VentilVdotStdDensity = 0.0;          // Volume flow rate of Air {m3/s} due to ventilation at standard density (adjusted elevation)
        Real64 VentilVdotOutDensity = 0.0;          // Volume flow rate of Air {m3/s} due to ventilation at outdoor density
        Real64 VentilMass = 0.0;                    // Mass of Air {kg} due to ventilation
        Real64 VentilMdot = 0.0;                    // Mass flow rate of Air {kg/s} due to ventilation
        Real64 VentilAirChangeRateCurDensity = 0.0; // Ventilation air change rate (ach) at current zone air density
        Real64 VentilAirChangeRateStdDensity = 0.0; // Ventilation air change rate (ach) at standard density (adjusted elevation)
        Real64 VentilAirChangeRateOutDensity = 0.0; // Ventilation air change rate (ach) at current outdoor density
        Real64 VentilFanElec = 0.0;                 // Fan Electricity {W} due to ventilation
        Real64 VentilAirTemp = 0.0;                 // Air Temp {C} of ventilation
        Real64 MixVolume = 0.0;                     // Mixing volume of Air {m3}
        Real64 MixVdotCurDensity = 0.0;             // Mixing volume flow rate of Air {m3/s} at current zone air density
        Real64 MixVdotStdDensity = 0.0;             // Mixing volume flow rate of Air {m3/s} at standard density (adjusted for elevation)
        Real64 MixMass = 0.0;                       // Mixing mass of air {kg}
        Real64 MixMdot = 0.0;                       // Mixing mass flow rate of air {kg/s}
        Real64 MixSenLoad = 0.0;                    // Heat Gain(+)/Loss(-) {J} due to mixing and cross mixing and refrigeration door mixing
        Real64 MixLatLoad = 0.0;                    // Latent Gain(+)/Loss(-) {J} due to mixing and cross mixing and refrigeration door mixing
        Real64 MixHeatLoss = 0.0;                   // Heat Gain {J} due to mixing and cross mixing and refrigeration door mixing
        Real64 MixHeatGain = 0.0;                   // Heat Loss {J} due to mixing and cross mixing and refrigeration door mixing
        Real64 MixLatentLoss = 0.0;                 // Latent Gain {J} due to mixing and cross mixing and refrigeration door mixing
        Real64 MixLatentGain = 0.0;                 // Latent Loss {J} due to mixing and cross mixing and refrigeration door mixing
        Real64 MixTotalLoss = 0.0;                  // Total Gain {J} due to mixing and cross mixing and refrigeration door mixing
        Real64 MixTotalGain = 0.0;                  // Total Loss {J} due to mixing and cross mixing and refrigeration door mixing
        Real64 SysInletMass = 0.0;                  // Total mass of Air {kg} from all system inlets
        Real64 SysOutletMass = 0.0;                 // Total mass of Air {kg} from all system outlets
        Real64 ExfilMass = 0.0;                     // Mass of Air {kg} due to exfiltration
        Real64 ExfilTotalLoss = 0.0;                // Total Loss rate {W} due to exfiltration (sensible+latent)
        Real64 ExfilSensiLoss = 0.0;                // Sensible Loss rate {W} due to exfiltration
        Real64 ExfilLatentLoss = 0.0;               // Latent Loss rate {W} due to exfiltration
        Real64 ExhTotalLoss = 0.0;                  // Total Loss rate {W} due to zone exhaust air (sensible+latent)
        Real64 ExhSensiLoss = 0.0;                  // Sensible Loss rate {W} due to zone exhaust air
        Real64 ExhLatentLoss = 0.0;                 // Latent Loss rate {W} due to zone exhaust air
        // air heat balance component load summary results
        Real64 SumIntGains = 0.0;     // Zone sum of convective internal gains
        Real64 SumHADTsurfs = 0.0;    // Zone sum of Hc*Area*(Tsurf - Tz)
        Real64 SumMCpDTzones = 0.0;   // zone sum of MassFlowRate*cp*(TremotZone - Tz) transfer air from other zone, Mixing
        Real64 SumMCpDtInfil = 0.0;   // Zone sum of MassFlowRate*Cp*(Tout - Tz) transfer from outside, ventil, earth tube
        Real64 SumMCpDTsystem = 0.0;  // Zone sum of air system MassFlowRate*Cp*(Tsup - Tz)
        Real64 SumNonAirSystem = 0.0; // Zone sum of system convective gains, collected via NonAirSystemResponse
        Real64 CzdTdt = 0.0;          // Zone air energy storage term.
        Real64 imBalance = 0.0;       // put all terms in eq. 5 on RHS , should be zero
        // for ZoneAirBalance:OutdoorAir object Outputs only
        Real64 OABalanceHeatLoss = 0.0;         // Heat Gain {J} due to OA air balance
        Real64 OABalanceHeatGain = 0.0;         // Heat Loss {J} due to OA air balance
        Real64 OABalanceLatentLoss = 0.0;       // Latent Gain {J} due to OA air balance
        Real64 OABalanceLatentGain = 0.0;       // Latent Loss {J} due to OA air balance
        Real64 OABalanceTotalLoss = 0.0;        // Total Gain {J} due to OA air balance
        Real64 OABalanceTotalGain = 0.0;        // Total Loss {J} due to OA air balance
        Real64 OABalanceVolumeCurDensity = 0.0; // Volume of Air {m3} due to OA air balance
        // at current zone air density
        Real64 OABalanceVolumeStdDensity = 0.0; // Volume of Air {m3} due to OA air balance
        // at standard density (adjusted for elevation)
        Real64 OABalanceVdotCurDensity = 0.0; // Volume flow rate of Air {m3/s} due to OA air balance
        // at current zone air density
        Real64 OABalanceVdotStdDensity = 0.0; // Volume flow rate of Air {m3/s} due to OA air balance
        // at standard density (adjusted elevation)
        Real64 OABalanceMass = 0.0;          // Mass of Air {kg} due to OA air balance
        Real64 OABalanceMdot = 0.0;          // Mass flow rate of Air {kg/s} due to OA air balance
        Real64 OABalanceAirChangeRate = 0.0; // OA air balance air change rate (ach)
        Real64 OABalanceFanElec = 0.0;       // Fan Electricity {W} due to OA air balance
        Real64 SumEnthalpyM = 0.0;           // Zone sum of EnthalpyM
        Real64 SumEnthalpyH = 0.0;           // Zone sum of EnthalpyH
        // reporting flags
        bool ReportWBGT = false; // whether the wetbulb globe temperature is reqeusted as an output variable or used as an EMS sensor

        void setUpOutputVars(EnergyPlusData &state, std::string_view prefix, std::string const &name);
    };

    struct ZonePreDefRepType
    {
        // Members
        bool isOccupied = false;      // occupied during the current time step
        Real64 NumOcc = 0.0;          // number of occupants - used in calculating Vbz
        Real64 NumOccAccum = 0.0;     // number of occupants accumulating for entire simulation
        Real64 NumOccAccumTime = 0.0; // time that the number of occupants is accumulating to compute average
        //  - zone time step [hrs]
        Real64 TotTimeOcc = 0.0; // time occupied (and the mechanical ventilation volume is accumulating)
        //  - system time step [hrs]

        // OA Reports - accumulated values
        // All Vol variables are in m3
        Real64 MechVentVolTotalOcc = 0.0;    // volume for mechanical ventilation of outside air for entire simulation during occupied at current
        Real64 MechVentVolMin = 9.9e9;       // a large number since finding minimum volume at current zone air density
        Real64 InfilVolTotalOcc = 0.0;       // volume for infiltration of outside air for entire simulation during occupied at current density
        Real64 InfilVolMin = 9.9e9;          // a large number since finding minimum volume at current zone air density
        Real64 AFNInfilVolTotalOcc = 0.0;    // volume for AFN infiltration of outside air for entire simulation during occupied at zone air density
        Real64 AFNInfilVolMin = 9.9e9;       // a large number since finding minimum volume at current zone air density
        Real64 SimpVentVolTotalOcc = 0.0;    // volume for simple 'ZoneVentilation' of outside air for entire simulation during occupied current
        Real64 SimpVentVolMin = 9.9e9;       // a large number since finding minimum volumeat current zone air density
        Real64 AFNVentVolTotalOcc = 0.0;     // volume for AFN ventilation of outside air for entire simulation during occupied at zone air density
        Real64 AFNVentVolMin = 9.9e9;        // a large number since finding minimum volume at current zone air density
        Real64 MechVentVolTotalStdDen = 0.0; // volume for mechanical ventilation of outside air for entire simulation at standard density
        Real64 MechVentVolTotalOccStdDen = 0.0; // volume for mechanical ventilation of outside air for entire simulation during occupied at std
        Real64 InfilVolTotalStdDen = 0.0;       // volume for infiltration of outside air for entire simulation at standard density
        Real64 InfilVolTotalOccStdDen = 0.0;    // volume for infiltration of outside air for entire simulation during occupied standard density
        Real64 AFNInfilVolTotalStdDen = 0.0;    // volume for AFN infiltration of outside air for entire simulation at standard density
        Real64 AFNInfilVolTotalOccStdDen = 0.0; // volume for AFN infiltration of outside air for entire simulation during occupied at std density
        Real64 AFNVentVolStdDen = 0.0;          // volume flow rate for natural ventilation at standard density
        Real64 AFNVentVolTotalStdDen = 0.0;     // volume for natural ventilation for entire simulation at standard density
        Real64 AFNVentVolTotalOccStdDen = 0.0;  // volume for natural ventilatiofor entire simulation n during occupied at standard density
        Real64 SimpVentVolTotalStdDen = 0.0;    // volume for simple 'ZoneVentilation' for entire simulation at standard density
        Real64 SimpVentVolTotalOccStdDen = 0.0; // volume for simple 'ZoneVentilation' for entire simulation during occupied at standard density
        Real64 VozMin = 0.0;                    // minimum outdoor zone ventilation
        Real64 VozTargetTotal = 0.0;            // volume for target Voz-dyn for entire simulation at std density
        Real64 VozTargetTotalOcc = 0.0;         // volume for target Voz-dyn for entire simulation during occupied
        Real64 VozTargetTimeBelow = 0.0;        // time [hrs] that mechanical+natural ventilation is < VozTarget - 1%
        Real64 VozTargetTimeAt = 0.0;           // time [hrs] that mechanical+natural ventilation is = VozTarget within 1% and > zero
        Real64 VozTargetTimeAbove = 0.0;        // time [hrs] that mechanical+natural ventilation is > VozTarget + 1%
        Real64 VozTargetTimeBelowOcc = 0.0;     // time [hrs] that mechanical+natural ventilation is < VozTarget - 1% during occupied
        Real64 VozTargetTimeAtOcc = 0.0;        // time [hrs] that mechanical+natural ventilation is = VozTarget within 1% and > zero during occupied
        Real64 VozTargetTimeAboveOcc = 0.0;     // time [hrs] that mechanical+natural ventilation is > VozTarget + 1% during occupied
        Real64 TotVentTimeNonZeroUnocc = 0.0;   // time [hrs] that mechanical+natural ventilation is > zero during UNoccupied

        // for Sensible Heat Gas Component Report
        // annual
        Real64 SHGSAnZoneEqHt = 0.0;  // Zone Eq heating
        Real64 SHGSAnZoneEqCl = 0.0;  // Zone Eq cooling
        Real64 SHGSAnHvacATUHt = 0.0; // heating by Air Terminal Unit [J]
        Real64 SHGSAnHvacATUCl = 0.0; // cooling by Air Terminal Unit [J]
        Real64 SHGSAnSurfHt = 0.0;    // heated surface heating
        Real64 SHGSAnSurfCl = 0.0;    // cooled surface cooling
        Real64 SHGSAnPeoplAdd = 0.0;  // people additions
        Real64 SHGSAnLiteAdd = 0.0;   // lighting addition
        Real64 SHGSAnEquipAdd = 0.0;  // equipment addition
        Real64 SHGSAnWindAdd = 0.0;   // window addition
        Real64 SHGSAnIzaAdd = 0.0;    // inter zone air addition
        Real64 SHGSAnInfilAdd = 0.0;  // infiltration addition
        Real64 SHGSAnOtherAdd = 0.0;  // opaque surface and other addition
        Real64 SHGSAnEquipRem = 0.0;  // equipment removal
        Real64 SHGSAnWindRem = 0.0;   // window removal
        Real64 SHGSAnIzaRem = 0.0;    // inter-zone air removal
        Real64 SHGSAnInfilRem = 0.0;  // infiltration removal
        Real64 SHGSAnOtherRem = 0.0;  // opaque surface and other removal
        // peak cooling
        int clPtTimeStamp = 0;        // timestamp for the cooling peak
        Real64 clPeak = 0.0;          // cooling peak value (hvac air cooling + cooled surface)
        Real64 SHGSClHvacHt = 0.0;    // hvac air heating
        Real64 SHGSClHvacCl = 0.0;    // hvac air cooling
        Real64 SHGSClHvacATUHt = 0.0; // heating by air terminal unit at cool peak [W]
        Real64 SHGSClHvacATUCl = 0.0; // cooling by air terminal unit at cool peak [W]
        Real64 SHGSClSurfHt = 0.0;    // heated surface heating
        Real64 SHGSClSurfCl = 0.0;    // cooled surface cooling
        Real64 SHGSClPeoplAdd = 0.0;  // people additions
        Real64 SHGSClLiteAdd = 0.0;   // lighting addition
        Real64 SHGSClEquipAdd = 0.0;  // equipment addition
        Real64 SHGSClWindAdd = 0.0;   // window addition
        Real64 SHGSClIzaAdd = 0.0;    // inter zone air addition
        Real64 SHGSClInfilAdd = 0.0;  // infiltration addition
        Real64 SHGSClOtherAdd = 0.0;  // opaque surface and other addition
        Real64 SHGSClEquipRem = 0.0;  // equipment removal
        Real64 SHGSClWindRem = 0.0;   // window removal
        Real64 SHGSClIzaRem = 0.0;    // inter-zone air removal
        Real64 SHGSClInfilRem = 0.0;  // infiltration removal
        Real64 SHGSClOtherRem = 0.0;  // opaque surface and other removal
        // peak heating
        int htPtTimeStamp = 0;        // timestamp for the heating peak
        Real64 htPeak = 0.0;          // heating peak value (hvac air heating + heated surface)
        Real64 SHGSHtHvacHt = 0.0;    // hvac air heating
        Real64 SHGSHtHvacCl = 0.0;    // hvac air cooling
        Real64 SHGSHtHvacATUHt = 0.0; // heating by air terminal unit at heat peak [W]
        Real64 SHGSHtHvacATUCl = 0.0; // cooling by air terminal unit at heat peak [W]
        Real64 SHGSHtSurfHt = 0.0;    // heated surface heating
        Real64 SHGSHtSurfCl = 0.0;    // cooled surface cooling
        Real64 SHGSHtPeoplAdd = 0.0;  // people additions
        Real64 SHGSHtLiteAdd = 0.0;   // lighting addition
        Real64 SHGSHtEquipAdd = 0.0;  // equipment addition
        Real64 SHGSHtWindAdd = 0.0;   // window addition
        Real64 SHGSHtIzaAdd = 0.0;    // inter zone air addition
        Real64 SHGSHtInfilAdd = 0.0;  // infiltration addition
        Real64 SHGSHtOtherAdd = 0.0;  // opaque surface and other addition
        Real64 SHGSHtEquipRem = 0.0;  // equipment removal
        Real64 SHGSHtWindRem = 0.0;   // window removal
        Real64 SHGSHtIzaRem = 0.0;    // inter-zone air removal
        Real64 SHGSHtInfilRem = 0.0;  // infiltration removal
        Real64 SHGSHtOtherRem = 0.0;  // opaque surface and other removal

        // heat emission
        Real64 emiEnvelopConv = 0.0;      // heat emission from envelope convection
        Real64 emiZoneExfiltration = 0.0; // heat emission from zone exfiltration
        Real64 emiZoneExhaust = 0.0;      // heat emission from zone exhaust air
        Real64 emiHVACRelief = 0.0;       // heat emission from HVAC relief air
        Real64 emiHVACReject = 0.0;       // heat emission from HVAC reject air
        Real64 emiTotHeat = 0.0;          // total building heat emission
    };

    struct ZoneLocalEnvironmentData
    {
        // Members
        std::string Name;
        int ZonePtr = 0;           // surface pointer
        int OutdoorAirNodePtr = 0; // schedule pointer
    };

    struct ZoneReportVars // Zone and Space report variables
    {
        // Members
        // People
        Real64 PeopleRadGain = 0.0;
        Real64 PeopleConGain = 0.0;
        Real64 PeopleSenGain = 0.0;
        Real64 PeopleNumOcc = 0.0;
        Real64 PeopleLatGain = 0.0;
        Real64 PeopleTotGain = 0.0;
        Real64 PeopleRadGainRate = 0.0;
        Real64 PeopleConGainRate = 0.0;
        Real64 PeopleSenGainRate = 0.0;
        Real64 PeopleLatGainRate = 0.0;
        Real64 PeopleTotGainRate = 0.0;
        // Lights
        Real64 LtsPower = 0.0;
        Real64 LtsElecConsump = 0.0;
        Real64 LtsRadGain = 0.0;
        Real64 LtsVisGain = 0.0;
        Real64 LtsConGain = 0.0;
        Real64 LtsRetAirGain = 0.0;
        Real64 LtsTotGain = 0.0;
        Real64 LtsRadGainRate = 0.0;
        Real64 LtsVisGainRate = 0.0;
        Real64 LtsConGainRate = 0.0;
        Real64 LtsRetAirGainRate = 0.0;
        Real64 LtsTotGainRate = 0.0;
        // Baseboard Heat
        Real64 BaseHeatPower = 0.0;
        Real64 BaseHeatElecCons = 0.0;
        Real64 BaseHeatRadGain = 0.0;
        Real64 BaseHeatConGain = 0.0;
        Real64 BaseHeatTotGain = 0.0;
        Real64 BaseHeatRadGainRate = 0.0;
        Real64 BaseHeatConGainRate = 0.0;
        Real64 BaseHeatTotGainRate = 0.0;
        // Electric Equipment
        Real64 ElecPower = 0.0;
        Real64 ElecConsump = 0.0;
        Real64 ElecRadGain = 0.0;
        Real64 ElecConGain = 0.0;
        Real64 ElecLatGain = 0.0;
        Real64 ElecLost = 0.0;
        Real64 ElecTotGain = 0.0;
        Real64 ElecRadGainRate = 0.0;
        Real64 ElecConGainRate = 0.0;
        Real64 ElecLatGainRate = 0.0;
        Real64 ElecLostRate = 0.0;
        Real64 ElecTotGainRate = 0.0;
        // Gas Equipment
        Real64 GasPower = 0.0;
        Real64 GasConsump = 0.0;
        Real64 GasRadGain = 0.0;
        Real64 GasConGain = 0.0;
        Real64 GasLatGain = 0.0;
        Real64 GasLost = 0.0;
        Real64 GasTotGain = 0.0;
        Real64 GasRadGainRate = 0.0;
        Real64 GasConGainRate = 0.0;
        Real64 GasLatGainRate = 0.0;
        Real64 GasLostRate = 0.0;
        Real64 GasTotGainRate = 0.0;
        // Hot Water Equipment
        Real64 HWPower = 0.0;
        Real64 HWConsump = 0.0;
        Real64 HWRadGain = 0.0;
        Real64 HWConGain = 0.0;
        Real64 HWLatGain = 0.0;
        Real64 HWLost = 0.0;
        Real64 HWTotGain = 0.0;
        Real64 HWRadGainRate = 0.0;
        Real64 HWConGainRate = 0.0;
        Real64 HWLatGainRate = 0.0;
        Real64 HWLostRate = 0.0;
        Real64 HWTotGainRate = 0.0;
        // Steam Equipment
        Real64 SteamPower = 0.0;
        Real64 SteamConsump = 0.0;
        Real64 SteamRadGain = 0.0;
        Real64 SteamConGain = 0.0;
        Real64 SteamLatGain = 0.0;
        Real64 SteamLost = 0.0;
        Real64 SteamTotGain = 0.0;
        Real64 SteamRadGainRate = 0.0;
        Real64 SteamConGainRate = 0.0;
        Real64 SteamLatGainRate = 0.0;
        Real64 SteamLostRate = 0.0;
        Real64 SteamTotGainRate = 0.0;
        // Other Equipment
        std::array<Real64, (int)Constant::eFuel::Num> OtherPower;
        std::array<Real64, (int)Constant::eFuel::Num> OtherConsump;
        Real64 OtherRadGain = 0.0;
        Real64 OtherConGain = 0.0;
        Real64 OtherLatGain = 0.0;
        Real64 OtherLost = 0.0;
        Real64 OtherTotGain = 0.0;
        Real64 OtherRadGainRate = 0.0;
        Real64 OtherConGainRate = 0.0;
        Real64 OtherLatGainRate = 0.0;
        Real64 OtherLostRate = 0.0;
        Real64 OtherTotGainRate = 0.0;
        // IT Equipment
        std::array<Real64, (int)PERptVars::Num> PowerRpt;
        std::array<Real64, (int)PERptVars::Num> EnergyRpt;
        Real64 ITEqAirVolFlowStdDensity = 0.0; // Zone Air volume flow rate at standard density [m3/s]
        Real64 ITEqAirMassFlow = 0.0;          // Zone Air mass flow rate [kg/s]
        Real64 ITEqSHI = 0.0;                  // Zone Supply Heat Index []
        Real64 ITEqTimeOutOfOperRange = 0.0;   // Zone ITE Air Inlet Operating Range Exceeded Time [hr]
        Real64 ITEqTimeAboveDryBulbT = 0.0;    // Zone ITE Air Inlet Dry-Bulb Temperature Above Operating Range Time [hr]
        Real64 ITEqTimeBelowDryBulbT = 0.0;    // Zone ITE Air Inlet Dry-Bulb Temperature Below Operating Range Time [hr]
        Real64 ITEqTimeAboveDewpointT = 0.0;   // Zone ITE Air Inlet Dewpoint Temperature Above Operating Range Time [hr]
        Real64 ITEqTimeBelowDewpointT = 0.0;   // Zone ITE Air Inlet Dewpoint Temperature Below Operating Range Time [hr]
        Real64 ITEqTimeAboveRH = 0.0;          // Zone ITE Air Inlet Relative Humidity Above Operating Range Time [hr]
        Real64 ITEqTimeBelowRH = 0.0;          // Zone ITE Air Inlet Relative Humidity Below Operating Range Time [hr]
        Real64 ITEAdjReturnTemp = 0.0;         // Zone ITE Adjusted Return Air Temperature
        // Overall Zone Variables
        Real64 TotRadiantGain = 0.0;
        Real64 TotVisHeatGain = 0.0;
        Real64 TotConvectiveGain = 0.0;
        Real64 TotLatentGain = 0.0;
        Real64 TotTotalHeatGain = 0.0;
        Real64 TotRadiantGainRate = 0.0;
        Real64 TotVisHeatGainRate = 0.0;
        Real64 TotConvectiveGainRate = 0.0;
        Real64 TotLatentGainRate = 0.0;
        Real64 TotTotalHeatGainRate = 0.0;
        // Contaminant
        Real64 CO2Rate = 0.0;
        Real64 GCRate = 0.0;

        Real64 SumTinMinusTSup = 0.0;  // Numerator for zone-level sensible heat index (SHI)
        Real64 SumToutMinusTSup = 0.0; // Denominator for zone-level sensible heat index (SHI)
    };

    // Functions

    void SetZoneOutBulbTempAt(EnergyPlusData &state);

    void CheckZoneOutBulbTempAt(EnergyPlusData &state);

    void SetZoneWindSpeedAt(EnergyPlusData &state);

    void SetZoneWindDirAt(EnergyPlusData &state);

    void CheckAndSetConstructionProperties(EnergyPlusData &state,
                                           int ConstrNum,    // Construction number to be set/checked
                                           bool &ErrorsFound // error flag that is set when certain errors have occurred
    );

    int AssignReverseConstructionNumber(EnergyPlusData &state,
                                        int ConstrNum, // Existing Construction number of first surface
                                        bool &ErrorsFound);

    Real64 ComputeNominalUwithConvCoeffs(EnergyPlusData &state,
                                         int numSurf,  // index for Surface array.
                                         bool &isValid // returns true if result is valid
    );

    void SetFlagForWindowConstructionWithShadeOrBlindLayer(EnergyPlusData &state);

    void AllocateIntGains(EnergyPlusData &state);

} // namespace DataHeatBalance

struct HeatBalanceData : BaseGlobalStruct
{

    int MaxSolidWinLayers = 0; // Maximum number of solid layers in a window construction

    // SiteData aka building data
    Real64 LowHConvLimit = 0.1; // Lowest allowed convection coefficient for detailed model
    // before reverting to the simple model.  This avoids a
    // divide by zero elsewhere.  Not based on any physical
    // reasoning, just the number that was picked.  It corresponds
    // to a delta T for a vertical surface of 0.000444C.
    //                           ! A lower limit is needed to avoid numerical problems
    //                           ! Natural convection correlations are a function of temperature difference,
    //                           !   there are many times when those temp differences pass through zero leading to non-physical results
    //                           ! Value of 1.0 chosen here is somewhat arbitrary, but based on the following reasons:
    //                           !  1) Low values of HconvIn indicate a layer of high thermal resistance, however
    //                           !       the R-value of a convection film layer should be relatively low (compared to building surfaces)
    //                           !  2) The value of 1.0 corresponds to the thermal resistance of 0.05 m of batt insulation
    //                           !  3) Limit on the order of 1.0 is suggested by the abrupt changes in an inverse relationship
    //                           !  4) A conduction-only analysis can model a limit by considering the thermal performance of
    //                           !       boundary layer to be pure conduction (with no movement to enhance heat transfer);
    //                           !       Taking the still gas thermal conductivity for air at 0.0267 W/m-K (at 300K), then
    //                           !       this limit of 1.0 corresponds to a completely still layer of air that is around 0.025 m thick
    //                           !  5) The previous limit of 0.1 (before ver. 3.1) caused loads initialization problems in test files
    Real64 HighHConvLimit = 1000.0;         // upper limit for HConv, mostly used for user input limits in practice. !W/m2-K
    Real64 MaxAllowedDelTemp = 0.002;       // Convergence criteria for inside surface temperatures
    Real64 MaxAllowedDelTempCondFD = 0.002; // Convergence criteria for inside surface temperatures for CondFD
    std::string BuildingName;               // Name of building
    Real64 BuildingAzimuth = 0.0;           // North Axis of Building
    Real64 LoadsConvergTol = 0.0;           // Tolerance value for Loads Convergence
    Real64 TempConvergTol = 0.0;            // Tolerance value for Temperature Convergence
    Convect::HcInt DefaultIntConvAlgo = Convect::HcInt::ASHRAESimple;
    Convect::HcExt DefaultExtConvAlgo = Convect::HcExt::ASHRAESimple;
    DataHeatBalance::Shadowing SolarDistribution = DataHeatBalance::Shadowing::FullExterior;                // Solar Distribution Algorithm
    int InsideSurfIterations = 0;                                                                           // Counts inside surface iterations
    DataSurfaces::HeatTransferModel OverallHeatTransferSolutionAlgo = DataSurfaces::HeatTransferModel::CTF; // Global HeatBalanceAlgorithm setting
    // Flags for HeatTransfer Algorithms Used
    bool AllCTF = true;                  // CTF used for everything - no EMPD, no CondFD, No HAMT, No Kiva - true until flipped otherwise
    bool AnyCTF = false;                 // CTF used
    bool AnyEMPD = false;                // EMPD used
    bool AnyCondFD = false;              // CondFD used
    bool AnyHAMT = false;                // HAMT used
    bool AnyKiva = false;                // Kiva used
    bool AnyAirBoundary = false;         // Construction:AirBoundary used (implies grouped solar and radiant is present)
    bool AnyBSDF = false;                // True if any WindowModelType == WindowModel:: BSDF
    int MaxNumberOfWarmupDays = 25;      // Maximum number of warmup days allowed
    int MinNumberOfWarmupDays = 1;       // Minimum number of warmup days allowed
    Real64 CondFDRelaxFactor = 1.0;      // Relaxation factor, for looping across all the surfaces.
    Real64 CondFDRelaxFactorInput = 1.0; // Relaxation factor, for looping across all the surfaces, user input value
    DataHeatBalance::SolutionAlgo ZoneAirSolutionAlgo =
        DataHeatBalance::SolutionAlgo::ThirdOrder; // ThirdOrderBackwardDifference, AnalyticalSolution, and EulerMethod
    bool doSpaceHeatBalanceSizing = false;         // Do space heat balance during sizing
    bool doSpaceHeatBalanceSimulation = false;     // Do space heat balance during simulation
    bool doSpaceHeatBalance = false;               // Do space heat balance currently
    bool OverrideZoneAirSolutionAlgo = false;      // Override the zone air solution algorithm in PerformancePrecisionTradeoffs
    Real64 BuildingRotationAppendixG = 0.0;        // Building Rotation for Appendix G
    Real64 ZoneTotalExfiltrationHeatLoss = 0.0;    // Building total heat emission through zone exfiltration;
    Real64 ZoneTotalExhaustHeatLoss = 0.0;         // Building total heat emission through zone air exhaust;
    Real64 SysTotalHVACReliefHeatLoss = 0.0;       // Building total heat emission through HVAC system relief air;
    Real64 SysTotalHVACRejectHeatLoss = 0.0;       // Building total heat emission through HVAC system heat rejection;
    // END SiteData
    int NumOfZoneLists = 0;    // Total number of zone lists
    int NumOfZoneGroups = 0;   // Total number of zone groups
    int TotPeople = 0;         // Total People instances after expansion to spaces
    int TotLights = 0;         // Total Lights instances after expansion to spaces
    int TotElecEquip = 0;      // Total Electric Equipment instances after expansion to spaces
    int TotGasEquip = 0;       // Total Gas Equipment instances after expansion to spaces
    int TotOthEquip = 0;       // Total Other Equipment instances after expansion to spaces
    int TotHWEquip = 0;        // Total Hot Water Equipment instances after expansion to spaces
    int TotStmEquip = 0;       // Total Steam Equipment instances after expansion to spaces
    int TotITEquip = 0;        // Total IT Equipment instances after expansion to spaces
    int TotInfiltration = 0;   // Total Infiltration (all types) instances after expansion to spaces
    int TotVentilation = 0;    // Total Ventilation (all types) instances after expansion to spaces
    int TotMixing = 0;         // Total Mixing Statementsn instances after expansion to spaces
    int TotCrossMixing = 0;    // Total Cross Mixing Statementsn instances after expansion to spaces
    int TotRefDoorMixing = 0;  // Total RefrigerationDoor Mixing Statements in input
    int TotBBHeat = 0;         // Total BBHeat Statements instances after expansion to spaces
    int TotConstructs = 0;     // Total number of unique constructions in this simulation
    int TotSpectralData = 0;   // Total window glass spectral data sets
    int TotZoneAirBalance = 0; // Total Zone Air Balance Statements in input
    int TotFrameDivider = 0;   // Total number of window frame/divider objects
    bool AirFlowFlag = false;
    int TotCO2Gen = 0;                       // Total CO2 source and sink statements in input
    bool CalcWindowRevealReflection = false; // True if window reveal reflection is to be calculated for at least one exterior window
    bool StormWinChangeThisDay = false; // True if a storm window has been added or removed from any window during the current day; can only be true
                                        // for first time step of the day.
    bool SimpleCTFOnly = true;          // true if all constructions are simple CTF construction (CTFTimestep = TimeStepZone) with no internal sources
    int MaxCTFTerms = 0;                // Maximum CTF terms to shift for all surfaces in the simulation
    bool AnyInternalHeatSourceInInput = false;        // true if the user has entered any constructions with internal sources
    bool AdaptiveComfortRequested_CEN15251 = false;   // true if people objects have adaptive comfort requests. CEN15251
    bool AdaptiveComfortRequested_ASH55 = false;      // true if people objects have adaptive comfort requests. ASH55
    bool AnyThermalComfortPierceModel = false;        // true if people objects use pierce thermal comfort model
    bool AnyThermalComfortKSUModel = false;           // true if people objects use KSU thermal comfort model
    bool AnyThermalComfortCoolingEffectModel = false; // true if people objects use ASH55 cooling effect adjusted thermal comfort model
    bool AnyThermalComfortAnkleDraftModel = false;    // true if people objects use ASH55 ankle draft thermal comfort model

    bool NoFfactorConstructionsUsed = true;
    bool NoCfactorConstructionsUsed = true;
    bool NoRegularMaterialsUsed = true;
    bool DoLatentSizing = false; // true when latent sizing is performed during zone sizing
    bool isAnyLatentLoad = false;
    DataHeatBalance::HeatIndexMethod heatIndexMethod = DataHeatBalance::HeatIndexMethod::Simplified;

    Array1D<Real64> ZoneListSNLoadHeatEnergy;
    Array1D<Real64> ZoneListSNLoadCoolEnergy;
    Array1D<Real64> ZoneListSNLoadHeatRate;
    Array1D<Real64> ZoneListSNLoadCoolRate;
    Array1D<Real64> ZoneGroupSNLoadHeatEnergy;
    Array1D<Real64> ZoneGroupSNLoadCoolEnergy;
    Array1D<Real64> ZoneGroupSNLoadHeatRate;
    Array1D<Real64> ZoneGroupSNLoadCoolRate;

    Array1D<Real64> ZoneTransSolar; // Exterior beam plus diffuse solar entering zone sum of WinTransSolar for exterior windows in zone (W)
    Array1D<Real64>
        ZoneWinHeatGain; // Heat gain to zone from all exterior windows (includes oneTransSolar); sum of WinHeatGain for exterior windows in zone (W)
    Array1D<Real64> ZoneWinHeatGainRep;             // = ZoneWinHeatGain when ZoneWinHeatGain >= 0
    Array1D<Real64> ZoneWinHeatLossRep;             // = -ZoneWinHeatGain when ZoneWinHeatGain < 0
    Array1D<Real64> ZoneBmSolFrExtWinsRep;          // Beam solar into zone from exterior windows [W]
    Array1D<Real64> ZoneBmSolFrIntWinsRep;          // Beam solar into zone from interior windows [W]
    Array1D<Real64> EnclSolInitialDifSolReflW;      // Initial diffuse solar in zone from ext and int windows reflected from interior surfaces [W]
    Array1D<Real64> ZoneDifSolFrExtWinsRep;         // Diffuse solar into zone from exterior windows [W]
    Array1D<Real64> ZoneDifSolFrIntWinsRep;         // Diffuse solar into zone from interior windows [W]
    Array1D<Real64> ZoneOpaqSurfInsFaceCond;        // Zone inside face opaque surface conduction (W)
    Array1D<Real64> ZoneOpaqSurfInsFaceCondGainRep; // = Zone inside face opaque surface conduction when >= 0
    Array1D<Real64> ZoneOpaqSurfInsFaceCondLossRep; // = -Zone inside face opaque surface conduction when < 0
    Array1D<Real64> ZoneOpaqSurfExtFaceCond;        // Zone outside face opaque surface conduction (W)
    Array1D<Real64> ZoneOpaqSurfExtFaceCondGainRep; // = Zone outside face opaque surface conduction when >= 0
    Array1D<Real64> ZoneOpaqSurfExtFaceCondLossRep; // = -Zone outside face opaque surface conduction when < 0
    Array1D<Real64> ZoneTransSolarEnergy;           // Energy of ZoneTransSolar [J]
    Array1D<Real64> ZoneWinHeatGainRepEnergy;       // Energy of ZoneWinHeatGainRep [J]
    Array1D<Real64> ZoneWinHeatLossRepEnergy;       // Energy of ZoneWinHeatLossRep [J]
    Array1D<Real64> ZoneBmSolFrExtWinsRepEnergy;    // Energy of ZoneBmSolFrExtWinsRep [J]
    Array1D<Real64> ZoneBmSolFrIntWinsRepEnergy;    // Energy of ZoneBmSolFrIntWinsRep [J]
    Array1D<Real64> ZoneDifSolFrExtWinsRepEnergy;   // Energy of ZoneDifSolFrExtWinsRep [J]
    Array1D<Real64> ZoneDifSolFrIntWinsRepEnergy;   // Energy of ZoneDifSolFrIntWinsRep [J]
    Array1D<Real64> ZnOpqSurfInsFaceCondGnRepEnrg;  // Energy of ZoneOpaqSurfInsFaceCondGainRep [J]
    Array1D<Real64> ZnOpqSurfInsFaceCondLsRepEnrg;  // Energy of ZoneOpaqSurfInsFaceCondLossRep [J]
    Array1D<Real64> ZnOpqSurfExtFaceCondGnRepEnrg;  // Energy of ZoneOpaqSurfExtFaceCondGainRep [J]
    Array1D<Real64> ZnOpqSurfExtFaceCondLsRepEnrg;  // Energy of ZoneOpaqSurfExtFaceCondLossRep [J]

    Array1D<Real64> SurfQdotRadIntGainsInPerArea;       // Thermal radiation absorbed on inside surfaces
    Array1D<Real64> SurfQRadSWOutIncident;              // Exterior beam plus diffuse solar incident on surface (W/m2)
    Array1D<Real64> SurfQRadSWOutIncidentBeam;          // Exterior beam solar incident on surface (W/m2)
    Array1D<Real64> SurfBmIncInsSurfIntensRep;          // Beam sol irrad from ext wins on inside of surface (W/m2)
    Array1D<Real64> SurfBmIncInsSurfAmountRep;          // Beam sol amount from ext wins incident on inside of surface (W)
    Array1D<Real64> SurfIntBmIncInsSurfIntensRep;       // Beam sol irrad from int wins on inside of surface (W/m2)
    Array1D<Real64> SurfIntBmIncInsSurfAmountRep;       // Beam sol amount from int wins incident on inside of surface (W)
    Array1D<Real64> SurfQRadSWOutIncidentSkyDiffuse;    // Exterior sky diffuse solar incident on surface (W/m2)
    Array1D<Real64> SurfQRadSWOutIncidentGndDiffuse;    // Exterior ground diffuse solar incident on surface (W/m2)
    Array1D<Real64> SurfQRadSWOutIncBmToDiffReflGnd;    // Exterior diffuse solar incident from beam to diffuse reflection from ground (W/m2)
    Array1D<Real64> SurfQRadSWOutIncSkyDiffReflGnd;     // Exterior diffuse solar incident from sky diffuse reflection from ground (W/m2)
    Array1D<Real64> SurfQRadSWOutIncBmToBmReflObs;      // Exterior beam solar incident from beam-to-beam reflection from obstructions (W/m2)
    Array1D<Real64> SurfQRadSWOutIncBmToDiffReflObs;    // Exterior diffuse solar incident from beam-to-diffuse reflection from obstructions (W/m2)
    Array1D<Real64> SurfQRadSWOutIncSkyDiffReflObs;     // Exterior diffuse solar incident from sky diffuse reflection from obstructions (W/m2)
    Array1D<Real64> SurfSWInAbsTotalReport;             // Report - Total interior/exterior shortwave absorbed on inside of surface (W)
    Array1D<Real64> SurfBmIncInsSurfAmountRepEnergy;    // energy of BmIncInsSurfAmountRep [J]
    Array1D<Real64> SurfIntBmIncInsSurfAmountRepEnergy; // energy of IntBmIncInsSurfAmountRep [J]
    Array1D<Real64> SurfInitialDifSolInAbsReport;       // Report - Initial transmitted diffuse solar absorbed on inside of surface (W)
    Array1D_int SurfWinBSDFBeamDirectionRep;            // BSDF beam direction number for given complex fenestration state (for reporting) []
    Array1D<Real64> SurfWinBSDFBeamThetaRep;            // BSDF beam Theta angle (for reporting) [rad]
    Array1D<Real64> SurfWinBSDFBeamPhiRep;              // BSDF beam Phi angle (for reporting) [rad]
    Array1D<Real64> SurfWinQRadSWwinAbsTot;             // Exterior beam plus diffuse solar absorbed in glass layers of window (W)
    Array2D<Real64> SurfWinQRadSWwinAbsLayer;           // Exterior beam plus diffuse solar absorbed in glass layers of window (W)
    Array2D<Real64> SurfWinFenLaySurfTempFront;         // Front surface temperatures of fenestration layers
    Array2D<Real64> SurfWinFenLaySurfTempBack;          // Back surface temperatures of fenestration layers
    Array1D<Real64> SurfWinQRadSWwinAbsTotEnergy;       // Energy of QRadSWwinAbsTot [J]
    Array1D<Real64> SurfWinSWwinAbsTotalReport;         // Report - Total interior/exterior shortwave absorbed in all glass layers of window (W)
    Array1D<Real64> SurfWinInitialDifSolInTransReport;  // Report - Initial transmitted diffuse solar transmitted out
                                                        // through inside of window surface (W)
    Array2D<Real64> SurfWinQRadSWwinAbs;                // Short wave radiation absorbed in window glass layers
    Array2D<Real64> SurfWinInitialDifSolwinAbs;         // Initial diffuse solar absorbed in window glass layers from inside(W/m2)
    Array1D<Real64> SurfOpaqSWOutAbsTotalReport;        // Report - Total exterior shortwave/solar absorbed on outside of surface (W)
    Array1D<Real64> SurfOpaqSWOutAbsEnergyReport;       // Report - Total exterior shortwave/solar absorbed on outside of surface (j)
    Array1D<Real64> SurfTempEffBulkAir;                 // air temperature adjacent to the surface used for inside surface heat balances

    // Material
    Array1D<Real64> NominalRforNominalUCalculation; // Nominal R values are summed to calculate NominalU values for constructions
    Array1D<Real64> NominalU;                       // Nominal U value for each construction -- used in matching interzone surfaces
    Array1D<Real64> NominalUBeforeAdjusted;         // Nominal U value for glazing system only
    Array1D<Real64> CoeffAdjRatio;                  // Conductive coefficient adjustment ratio

    Array1D<Real64>
        EnclSolQSWRad; // Zone short-wave flux density; used to calculate short-wave  radiation absorbed on inside surfaces of zone or enclosure
    Array1D<Real64> EnclSolQSWRadLights; // Like QS, but Lights short-wave only.
    Array1D<Real64> EnclSolDB;           // Factor for diffuse radiation in a zone from beam reflecting from inside surfaces
    Array1D<Real64> EnclSolDBSSG;        // Factor for diffuse radiation in a zone from beam reflecting from inside surfaces.
    // Used only for scheduled surface gains
    Array1D<Real64> EnclSolDBIntWin; // Value of factor for beam solar entering a zone through interior windows
    // (considered to contribute to diffuse in zone)
    Array1D<Real64> EnclSolQSDifSol; // Like QS, but diffuse solar short-wave only.
    Array1D<Real64> EnclSolQD;       // Diffuse solar radiation in a zone from sky and ground diffuse entering
    // through exterior windows and reflecting from interior surfaces,
    // beam from exterior windows reflecting from interior surfaces,
    // and beam entering through interior windows (considered diffuse)
    Array1D<Real64> EnclSolQDforDaylight; // Diffuse solar radiation in a zone from sky and ground diffuse entering
    // through exterior windows, beam from exterior windows reflecting
    // from interior surfaces, and beam entering through interior windows
    // (considered diffuse)
    // Originally QD, now used only for EnclSolQSDifSol calc for daylighting

    bool EnclRadAlwaysReCalc = false; // Enclosure solar or thermal radiation properties always needs to be recalc at any time step

    Array1D<Real64> SurfCosIncidenceAngle;       // Cosine of beam solar incidence angle (for reporting)
    Array2D<Real64> SurfSunlitFracHR;            // Hourly fraction of heat transfer surface that is sunlit
    Array2D<Real64> SurfCosIncAngHR;             // Hourly cosine of beam radiation incidence angle on surface
    Array3D<Real64> SurfSunlitFrac;              // TimeStep fraction of heat transfer surface that is sunlit
    Array3D<Real64> SurfSunlitFracWithoutReveal; // For a window with reveal, the sunlit fraction  without shadowing by the reveal
    Array3D<Real64> SurfCosIncAng;               // TimeStep cosine of beam radiation incidence angle on surface
    Array4D_int SurfWinBackSurfaces;     // For a given hour and timestep, a list of up to 20 surfaces receiving  beam solar radiation from a given
                                         // exterior window
    Array4D<Real64> SurfWinOverlapAreas; // For a given hour and timestep, the areas of the exterior window sending beam solar radiation to the
                                         // surfaces listed in BackSurfaces
    Real64 zeroPointerVal = 0.0;
    EPVector<DataHeatBalance::ZonePreDefRepType> ZonePreDefRep;
    DataHeatBalance::ZonePreDefRepType BuildingPreDefRep;
    EPVector<DataHeatBalance::SpaceZoneSimData> ZoneIntGain;
    EPVector<DataHeatBalance::SpaceZoneSimData> spaceIntGain;
    EPVector<DataHeatBalance::SpaceIntGainDeviceData> spaceIntGainDevices;
    EPVector<DataHeatBalance::SpaceData> space;
    EPVector<DataHeatBalance::SpaceListData> spaceList;
    EPVector<DataHeatBalance::ZoneData> Zone;
    EPVector<DataHeatBalance::ZoneResilience> Resilience;
    EPVector<DataHeatBalance::ZoneListData> ZoneList;
    EPVector<DataHeatBalance::ZoneGroupData> ZoneGroup;
    EPVector<DataHeatBalance::PeopleData> People;
    EPVector<DataHeatBalance::LightsData> Lights;
    EPVector<DataHeatBalance::ZoneEquipData> ZoneElectric;
    EPVector<DataHeatBalance::ZoneEquipData> ZoneGas;
    EPVector<DataHeatBalance::ZoneEquipData> ZoneOtherEq;
    EPVector<DataHeatBalance::ZoneEquipData> ZoneHWEq;
    EPVector<DataHeatBalance::ZoneEquipData> ZoneSteamEq;
    EPVector<DataHeatBalance::ITEquipData> ZoneITEq;
    EPVector<DataHeatBalance::BBHeatData> ZoneBBHeat;
    EPVector<DataHeatBalance::InfiltrationData> Infiltration;
    EPVector<DataHeatBalance::VentilationData> Ventilation;
    EPVector<DataHeatBalance::ZoneAirBalanceData> ZoneAirBalance;
    EPVector<DataHeatBalance::MixingData> Mixing;
    EPVector<DataHeatBalance::MixingData> CrossMixing;
    EPVector<DataHeatBalance::AirBoundaryMixingSpecs> airBoundaryMixing;
    EPVector<DataHeatBalance::MixingData> RefDoorMixing;
    EPVector<DataHeatBalance::ZoneCatEUseData> ZoneIntEEuse;
    EPVector<DataHeatBalance::RefrigCaseCreditData> RefrigCaseCredit;
    EPVector<DataHeatBalance::HeatReclaimDataBase> HeatReclaimRefrigeratedRack;
    EPVector<DataHeatBalance::HeatReclaimRefrigCondenserData> HeatReclaimRefrigCondenser;
    EPVector<DataHeatBalance::HeatReclaimDataBase> HeatReclaimDXCoil;
    EPVector<DataHeatBalance::HeatReclaimDataBase> HeatReclaimVS_Coil;
    EPVector<DataHeatBalance::HeatReclaimDataBase> HeatReclaimSimple_WAHPCoil;
    EPVector<DataHeatBalance::AirReportVars> ZnAirRpt;
    EPVector<DataHeatBalance::AirReportVars> spaceAirRpt;
    EPVector<DataHeatBalance::ZoneEquipData> ZoneCO2Gen;
    EPVector<DataHeatBalance::ZoneReportVars> ZoneRpt;
    EPVector<DataHeatBalance::ZoneReportVars> spaceRpt;
    EPVector<DataHeatBalance::ZoneMassConservationData> MassConservation;
    DataHeatBalance::ZoneAirMassFlowConservation ZoneAirMassFlow;
    EPVector<DataHeatBalance::ZoneLocalEnvironmentData> ZoneLocalEnvironment;
    bool MundtFirstTimeFlag = true;
    EPVector<std::string> spaceTypes;
    EPVector<DataHeatBalance::ExtVentedCavityStruct> ExtVentedCavity;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) HeatBalanceData();
    }
};

} // namespace EnergyPlus

#endif
