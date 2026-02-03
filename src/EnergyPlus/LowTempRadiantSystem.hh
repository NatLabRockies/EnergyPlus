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

#ifndef LowTempRadiantSystem_hh_INCLUDED
#define LowTempRadiantSystem_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace LowTempRadiantSystem {

    // Using/Aliasing

    // Data
    // MODULE PARAMETER DEFINITIONS:
    // System types:

    enum class SystemType
    {
        Invalid = -1,
        Hydronic,     // Variable flow hydronic radiant system
        ConstantFlow, // Constant flow, variable (controlled) temperature radiant system
        Electric,     // Electric resistance radiant heating system
        Num
    };

    enum class OpMode
    {
        Cool = -1, // Ok for this to be -1 because this isn't coming from the IDF so there is no Invalid
        None,
        Heat,
    };

    // Control types:
    enum class CtrlType
    {
        Invalid = -1,
        MAT,            // Controls system using mean air temperature
        MRT,            // Controls system using mean radiant temperature
        Operative,      // Controls system using operative temperature
        ODB,            // Controls system using outside air dry-bulb temperature
        OWB,            // Controls system using outside air wet-bulb temperature
        SurfFaceTemp,   // Controls system using the surface inside face temperature
        SurfIntTemp,    // Controls system using a temperature inside the radiant system construction as defined by the Construction +
                        // ConstructionProperty:InternalHeatSource inputs
        RunningMeanODB, // Controls system using the running mean outdoor dry-bulb temperature
        Num
    };

    // Setpoint Types:
    enum class SetpointType
    {
        Invalid = -1,
        HalfFlowPower, // Controls system where the setpoint is at the 50% flow/power point
        ZeroFlowPower, // Controls system where the setpoint is at the 0% flow/power point
        Num
    };

    // Fluid to Slab Heat Transfer Types:
    enum class FluidToSlabHeatTransferType
    {
        Invalid = -1,
        ConvectionOnly, // Convection only model (legacy code, original model)
        ISOStandard,    // Using ISO Standard 1185-2 (convection, conduction through pipe, contact resistance)
        Num
    };

    enum class CondCtrlType
    {
        Invalid = -1,
        None,      // Condensation control--none, so system never shuts down
        SimpleOff, // Condensation control--simple off, system shuts off when condensation predicted
        VariedOff, // Condensation control--variable off, system modulates to keep running if possible
        Num
    };

    // Number of Circuits per Surface Calculation Method
    enum class CircuitCalc
    {
        Invalid = -1,
        OneCircuit,          // there is 1 circuit per surface
        CalculateFromLength, // The number of circuits is TubeLength*SurfaceFlowFrac / CircuitLength
        Num
    };

    struct RadiantSystemBaseData
    {
        // Members
        std::string Name;                      // name of hydronic radiant system
        Sched::Schedule *availSched = nullptr; // index to schedule
        std::string ZoneName;                  // Name of zone the system is serving
        int ZonePtr = 0;                       // Point to this zone in the Zone derived type
        std::string SurfListName;              // Name of surface/surface list that is the radiant system
        int NumOfSurfaces = 0;                 // Number of surfaces included in this radiant system (coordinated control)
        Array1D_int SurfacePtr;                // Pointer to the surface(s) in the Surface derived type
        Array1D_string SurfaceName;            // Name of surfaces that are the radiant system (can be one or more)
        Array1D<Real64> SurfaceFrac;           // Fraction of flow/pipe length or electric power for a particular surface
        Real64 TotalSurfaceArea = 0.0;         // Total surface area for all surfaces that are part of this radiant system
        Real64 ZeroLTRSourceSumHATsurf = 0.0;  // Equal to SumHATsurf for all the walls in a zone with no source
        Array1D<Real64> QRadSysSrcAvg;         // Average source over the time step for a particular radiant surface
        // Record keeping variables used to calculate QRadSysSrcAvg locally
        Array1D<Real64> LastQRadSysSrc;                          // Need to keep the last value in case we are still iterating
        Real64 LastSysTimeElapsed;                               // Need to keep the last value in case we are still iterating
        Real64 LastTimeStepSys;                                  // Need to keep the last value in case we are still iterating
        CtrlType controlType = CtrlType::MAT;                    // Control type for the system (MAT, MRT, Op temp, ODB, OWB,
                                                                 // Surface Face Temp, Surface Interior Temp, Running Mean
                                                                 // Temp for Constant Flow systems only)
        SetpointType setpointType = SetpointType::HalfFlowPower; // Setpoint type for the system, (HalfFlowPower or ZeroFlowPower)
        OpMode opMode = OpMode::None;                            // Operating mode currently being used (NotOperating, Heating, Cooling)
        Real64 HeatPower;                                        // heating sent to panel in Watts
        Real64 HeatEnergy;                                       // heating sent to panel in Joules
        Real64 runningMeanOutdoorAirTemperatureWeightingFactor =
            0.0;                                                    // Weighting factor for running mean outdoor air temperature equation (user input)
        Real64 todayRunningMeanOutdoorDryBulbTemperature = 0.0;     // Current running mean outdoor air dry-bulb temperature
        Real64 yesterdayRunningMeanOutdoorDryBulbTemperature = 0.0; // Running mean outdoor air dry-bulb temperature from yesterday
        Real64 todayAverageOutdoorDryBulbTemperature = 0.0;         // Average outdoor dry-bulb temperature for today
        Real64 yesterdayAverageOutdoorDryBulbTemperature = 0.0;     // Average outdoor dry-bulb temperature for yesterday

        void errorCheckZonesAndConstructions(EnergyPlusData &state, bool &errorsFound);

        Real64 setRadiantSystemControlTemperature(EnergyPlusData &state, CtrlType TempControlType);

        Real64 calculateOperationalFraction(Real64 const offTemperature, Real64 const controlTemperature, Real64 const throttlingRange);

        virtual void calculateLowTemperatureRadiantSystem(EnergyPlusData &state, Real64 &LoadMet) = 0;

        Real64 setOffTemperatureLowTemperatureRadiantSystem(EnergyPlusData &state,
                                                            Sched::Schedule const *sched,
                                                            Real64 const throttlingRange,
                                                            SetpointType SetpointControlType);

        void updateLowTemperatureRadiantSystemSurfaces(EnergyPlusData &state);

        virtual void updateLowTemperatureRadiantSystem(EnergyPlusData &state) = 0;

        virtual void reportLowTemperatureRadiantSystem(EnergyPlusData &state) = 0;

        // Default Constructor
        RadiantSystemBaseData() = default;
        ~RadiantSystemBaseData() = default;
    };

    struct HydronicSystemBaseData : RadiantSystemBaseData
    {
        // Members
        Array1D<Real64> NumCircuits; // Number of fluid circuits in the surface
        Real64 TubeLength = 0.0;     // tube length embedded in radiant surface (meters)
        bool HeatingSystem = false;  // .TRUE. when the system is able to heat (parameters are valid)
        int HotWaterInNode = 0;      // hot water inlet node
        int HotWaterOutNode = 0;     // hot water outlet node
        PlantLocation HWPlantLoc{};
        bool CoolingSystem = false; // .TRUE. when the system is able to cool (parameters are valid)
        int ColdWaterInNode = 0;    // cold water inlet node
        int ColdWaterOutNode = 0;   // cold water outlet node
        PlantLocation CWPlantLoc{};
        Fluid::GlycolProps *water = nullptr; // Water properties

        int CondErrIndex = 0;            // Error index for recurring warning messages
        Real64 CondCausedTimeOff = 0.0;  // Amount of time condensation did or could have turned system off
        bool CondCausedShutDown = false; // .TRUE. when condensation predicted at surface
        CircuitCalc NumCircCalcMethod =
            CircuitCalc::Invalid; // Calculation method for number of circuits per surface; 1=1 per surface, 2=use cicuit length
        Real64 CircLength = 0.0;  // Circuit length {m}
        Sched::Schedule *changeoverDelaySched = nullptr; // schedule for the changeover delay in hours
        OpMode lastOpMode = OpMode::None;                // Last mode of operation (heating or cooling)
        int lastDayOfSim = 1;                            // Last day of simulation radiant system operated in lastOperatingMode
        int lastHourOfDay = 1;                           // Last hour of the day radiant system operated in lastOperatingMode
        int lastTimeStep = 1;                            // Last time step radiant system operated in lastOperatingMode
        // Other parameters
        bool EMSOverrideOnWaterMdot = false;
        Real64 EMSWaterMdotOverrideValue = 0.0;
        // Report data
        Real64 WaterInletTemp = 0.0;  // water inlet temperature
        Real64 WaterOutletTemp = 0.0; // water outlet temperature
        Real64 CoolPower = 0.0;       // cooling sent to panel in Watts
        Real64 CoolEnergy = 0.0;      // cooling sent to panel in Joules
        int OutRangeHiErrorCount = 0; // recurring errors for crazy results too high fluid temperature
        int OutRangeLoErrorCount = 0; // recurring errors for crazy results too low fluid temperature

        void updateOperatingModeHistory(EnergyPlusData &state);

        void setOperatingModeBasedOnChangeoverDelay(EnergyPlusData &state);

        Real64 calculateHXEffectivenessTerm(EnergyPlusData &state,
                                            int const SurfNum,          // Surface Number
                                            Real64 const Temperature,   // Temperature of water entering the radiant system, in C
                                            Real64 const WaterMassFlow, // Mass flow rate of water in the radiant system, in kg/s
                                            Real64 const FlowFraction,  // Mass flow rate fraction for this surface in the radiant system
                                            Real64 const NumCircs,      // Number of fluid circuits in this surface
                                            int const DesignObjPtr,     // Design Object Pointer,
                                            SystemType typeOfRadiantSystem

        );

        Real64 calculateUFromISOStandard(EnergyPlusData &state,
                                         int const SurfNum,
                                         Real64 const WaterMassFlow,
                                         SystemType typeOfRadiantSystem,
                                         int const DesignObjPtr // Design Object Pointer
        );

        Real64 sizeRadiantSystemTubeLength(EnergyPlusData &state);

        void checkForOutOfRangeTemperatureResult(EnergyPlusData &state, Real64 const outletTemp, Real64 const inletTemp);

        // Default Constructor
        HydronicSystemBaseData() = default;
        ~HydronicSystemBaseData() = default;
    };

    struct VariableFlowRadiantSystemData : HydronicSystemBaseData
    {
        // Members
        std::string designObjectName; // Design Object
        int DesignObjectPtr = 0;
        int HeatingCapMethod = 0; // - Method for Low Temp Radiant system heating capacity scaled sizing calculation (HeatingDesignCapacity,
        // CapacityPerFloorArea, FracOfAutosizedHeatingCapacity)
        Real64 ScaledHeatingCapacity =
            0.0; // -  Low Temp Radiant system scaled maximum heating capacity {W} or scalable variable of zone HVAC equipment,
        Real64 WaterVolFlowMaxHeat = 0.0; // maximum water flow rate for heating, m3/s
        Real64 WaterFlowMaxHeat = 0.0;    // maximum water flow rate for heating, kg/s
        Real64 WaterVolFlowMaxCool = 0.0; // maximum water flow rate for cooling, m3/s
        Real64 WaterFlowMaxCool = 0.0;    // maximum water flow rate for cooling, kg/s
        Real64 WaterMassFlowRate = 0.0;   // water mass flow rate
        int CoolingCapMethod = 0;         // - Method for Low Temp Radiant system cooling capacity scaled sizing calculation (CoolingDesignCapacity,
        // CapacityPerFloorArea, FracOfAutosizedCoolingCapacity)
        Real64 ScaledCoolingCapacity =
            0.0; // -  Low Temp Radiant system scaled maximum cooling capacity {W} or scalable variable of zone HVAC equipment,
        // {-}, or {W/m2}

        void calculateLowTemperatureRadiantSystem(EnergyPlusData &state, Real64 &LoadMet);

        void calculateLowTemperatureRadiantSystemComponents(EnergyPlusData &state, Real64 &LoadMet, SystemType typeOfRadiantSystem);

        void updateLowTemperatureRadiantSystem(EnergyPlusData &state);

        void reportLowTemperatureRadiantSystem(EnergyPlusData &state);

        // Default Constructor
        VariableFlowRadiantSystemData() = default;
        ~VariableFlowRadiantSystemData() = default;
    };

    struct VarFlowRadDesignData : VariableFlowRadiantSystemData
    {
        // Members
        // This data could be shared between multiple Var flow LowTempRad Systems
        std::string designName;         // name of the design object+
        Real64 TubeDiameterInner = 0.0; // inside tube diameter for embedded tubing (meters)
        Real64 TubeDiameterOuter = 0.0; // outside tube diameter for embedded tubing (meters)
        FluidToSlabHeatTransferType FluidToSlabHeatTransfer =
            FluidToSlabHeatTransferType::ConvectionOnly; // Model used for calculating heat transfer between fluid and slab
        Real64 VarFlowTubeConductivity = 0.0;            // tube conductivity in W/m-K
        CtrlType VarFlowControlType = CtrlType::MAT;     // Control type for the system (MAT, MRT, Op temp, ODB, OWB,
        // Surface Face Temp, Surface Interior Temp, Running Mean Temp
        // for Constant Flow systems only)
        SetpointType VarFlowSetpointType = SetpointType::HalfFlowPower; // Setpoint type for the system, (HalfFlowPower or ZeroFlowPower)
        std::string DesignHeatingCapMethodInput;
        int DesignHeatingCapMethod = 0; // - Method for Low Temp Radiant system heating capacity scaledsizing calculation (HeatingDesignCapacity,
        // CapacityPerFloorArea, FracOfAutosizedHeatingCapacity)
        Real64 DesignScaledHeatingCapacity =
            0.0; // -  Low Temp Radiant system scaled maximum heating capacity {W} or scalable variable of zone HVAC equipment,
        // {-}, or {W/m2}
        Real64 HotThrottlRange = 0.0;              // Throttling range for heating [C]
        Sched::Schedule *heatSetptSched = nullptr; // Schedule index for the zone setpoint temperature
        Real64 ColdThrottlRange = 0.0;             // Throttling range for cooling [C]
        Array1D_string FieldNames;
        CondCtrlType condCtrlType = CondCtrlType::SimpleOff; // Condensation control type (initialize to simple off)
        Real64 CondDewPtDeltaT = 1.0;                        // Diff between surface temperature and dew point for cond. shut-off
        Sched::Schedule *coolSetptSched = nullptr;           // Schedule index for the zone setpoint temperature
        std::string DesignCoolingCapMethodInput;
        int DesignCoolingCapMethod = 0; // - Method for Low Temp Radiant system cooling capacity scaledsizing calculation (CoolingDesignCapacity,
                                        // CapacityPerFloorArea, FracOfAutosizedCoolingCapacity)
        Real64 DesignScaledCoolingCapacity =
            0.0; // -  Low Temp Radiant system scaled maximum cooling capacity {W} or scalable variable of zone HVAC equipment,
                 // {-}, or {W/m2}

        // Default Constructor
        VarFlowRadDesignData() = default;
        ~VarFlowRadDesignData() = default;
    };

    struct ConstantFlowRadiantSystemData : HydronicSystemBaseData
    {
        // Members
        Real64 WaterVolFlowMax; // design nominal capacity of constant flow pump (volumetric flow rate)
        Real64 ColdDesignWaterMassFlowRate;
        Real64 HotDesignWaterMassFlowRate;
        Real64 WaterMassFlowRate = 0.0;    // current flow rate through system (calculated)
        Real64 HotWaterMassFlowRate = 0.0; // current hot water flow rate through heating side of system (calculated)
        Real64 ChWaterMassFlowRate = 0.0;  // current chilled water flow rate through cooling side of system (calculated)
        std::string designObjectName;      // Design Object
        int DesignObjectPtr = 0;
        Sched::Schedule *volFlowSched = nullptr;        // index to the volumetric flow schedule
        Real64 NomPumpHead = 0.0;                       // nominal head of the constant flow pump
        Real64 NomPowerUse = 0.0;                       // nominal power use of the constant flow pump
        Real64 PumpEffic = 0.0;                         // overall efficiency of the pump (calculated)
        Sched::Schedule *hotWaterHiTempSched = nullptr; // Schedule for the highest water temperature
        Sched::Schedule *hotWaterLoTempSched = nullptr; // Schedule for the lowest water temperature
        Sched::Schedule *hotCtrlHiTempSched = nullptr;  // Schedule for the highest control temperature
        // (where the lowest water temperature is requested)
        Sched::Schedule *hotCtrlLoTempSched = nullptr; // Schedule for the lowest control temperature
        // (where the highest water temperature is requested)
        Sched::Schedule *coldWaterHiTempSched = nullptr; // Schedule for the highest water temperature
        Sched::Schedule *coldWaterLoTempSched = nullptr; // Schedule for the lowest water temperature
        Sched::Schedule *coldCtrlHiTempSched = nullptr;  // Schedule for the highest control temperature
        // (where the lowest water temperature is requested)
        Sched::Schedule *coldCtrlLoTempSched = nullptr; // Schedule for the lowest control temperature
        // (where the highest water temperature is requested)
        Real64 WaterInjectionRate = 0.0;    // water injection mass flow rate from main loop
        Real64 WaterRecircRate = 0.0;       // water recirculation rate (outlet from radiant system recirculated)
        Real64 PumpPower = 0.0;             // pump power in Watts
        Real64 PumpEnergy = 0.0;            // pump energy consumption in Joules
        Real64 PumpMassFlowRate = 0.0;      // mass flow rate through the radiant system in kg/sec
        Real64 PumpHeattoFluid = 0.0;       // heat transfer rate from pump motor to fluid in Watts
        Real64 PumpHeattoFluidEnergy = 0.0; // Pump Energy dissipated into fluid stream in Joules
        Real64 PumpInletTemp = 0.0;         // inlet temperature of pump (inlet temperature from loop)
        bool setRunningMeanValuesAtBeginningOfDay =
            true; // flag to help certain variables only being set once per day (running mean temperature variables)

        void calculateLowTemperatureRadiantSystem(EnergyPlusData &state, Real64 &LoadMet);

        void
        calculateLowTemperatureRadiantSystemComponents(EnergyPlusData &state,
                                                       int const MainLoopNodeIn, // Node number on main loop of the inlet node to the radiant system
                                                       bool const Iteration,     // FALSE for the regular solution, TRUE when we had to loop back
                                                       Real64 &LoadMet,          // Load met by the low temperature radiant system, in Watts
                                                       SystemType typeOfRadiantSystem);

        void calculateRunningMeanAverageTemperature(EnergyPlusData &state, int const RadSysNum);

        Real64 calculateCurrentDailyAverageODB(EnergyPlusData &state);

        void updateLowTemperatureRadiantSystem(EnergyPlusData &state);

        void reportLowTemperatureRadiantSystem(EnergyPlusData &state);

        // Default Constructor
        ConstantFlowRadiantSystemData() = default;
        ~ConstantFlowRadiantSystemData() = default;
    };

    struct ConstantFlowRadDesignData : ConstantFlowRadiantSystemData
    {
        // Members
        // This data could be shared between multiple constant flow LowTempRad Systems
        std::string designName; // name of the design object
        Real64 runningMeanOutdoorAirTemperatureWeightingFactor =
            0.8;                                       // Weighting factor for running mean outdoor air temperature equation (user input)
        CtrlType ConstFlowControlType = CtrlType::MAT; // Control type for the system (MAT, MRT, Op temp, ODB, OWB,
                                                       // Surface Face Temp, Surface Interior Temp, Running Mean Temp
                                                       // for Constant Flow systems only)
        Real64 TubeDiameterInner = 0.0;                // inside tube diameter for embedded tubing (meters)
        Real64 TubeDiameterOuter = 0.0;                // outside tube diameter for embedded tubing (meters)
        FluidToSlabHeatTransferType FluidToSlabHeatTransfer =
            FluidToSlabHeatTransferType::ConvectionOnly; // Model used for calculating heat transfer between fluid and slab
        Real64 ConstFlowTubeConductivity = 0.0;          // tube conductivity in W/m-K
        Real64 MotorEffic = 0.0;                         // efficiency of the pump motor
        Real64 FracMotorLossToFluid = 0.0;               // amount of heat generated by pump motor that is added to the fluid

        Array1D_string FieldNames;
        CondCtrlType condCtrlType = CondCtrlType::SimpleOff; // Condensation control type (initialize to simple off)
        Real64 CondDewPtDeltaT = 1.0;                        // Diff between surface temperature and dew point for cond. shut-off

        ConstantFlowRadDesignData() = default;
        ~ConstantFlowRadDesignData() = default;
    };

    struct ElectricRadiantSystemData : RadiantSystemBaseData
    {
        // Members
        // Input data
        Real64 MaxElecPower = 0.0;             // Maximum electric power that can be supplied to surface, Watts
        Real64 ThrottlRange = 0.0;             // Throttling range for heating [C]
        Sched::Schedule *setptSched = nullptr; // Schedule for the zone setpoint temperature
        // Other parameters
        // Report data
        Real64 ElecPower = 0.0;   // heating sent to panel in Watts
        Real64 ElecEnergy;        // heating sent to panel in Joules
        int HeatingCapMethod = 0; // - Method for Low Temp Radiant system heating capacity scaledsizing calculation
        //- (HeatingDesignCapacity, CapacityPerFloorArea, FracOfAutosizedHeatingCapacity)
        Real64 ScaledHeatingCapacity =
            0.0; // -  Low Temp Radiant system scaled maximum heating capacity {W} or scalable variable of zone HVAC equipment,
                 // {-}, or {W/m2}

        void calculateLowTemperatureRadiantSystem(EnergyPlusData &state, Real64 &LoadMet);

        void updateLowTemperatureRadiantSystem(EnergyPlusData &state);

        void reportLowTemperatureRadiantSystem(EnergyPlusData &state);

        // Default Constructor
        ElectricRadiantSystemData() = default;
        ~ElectricRadiantSystemData() = default;
    };

    struct RadSysTypeData
    {
        // Members
        // This type used to track different components/types for efficiency
        std::string Name;                            // name of radiant system
        SystemType systemType = SystemType::Invalid; // Type of System (see System Types in Parameters)
        int CompIndex = 0;                           // Index in specific system types

        // Default Constructor
        RadSysTypeData() = default;
        ~RadSysTypeData() = default;
    };

    struct ElecRadSysNumericFieldData
    {
        // Members
        Array1D_string FieldNames;

        // Default Constructor
        ElecRadSysNumericFieldData() = default;
        ~ElecRadSysNumericFieldData() = default;
    };

    struct HydronicRadiantSysNumericFieldData
    {
        // Members
        Array1D_string FieldNames;

        // Default Constructor
        HydronicRadiantSysNumericFieldData() = default;
        ~HydronicRadiantSysNumericFieldData() = default;
    };

    // Functions

    void SimLowTempRadiantSystem(EnergyPlusData &state,
                                 std::string_view CompName,     // name of the low temperature radiant system
                                 bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                                 Real64 &LoadMet,               // load met by the radiant system, in Watts
                                 int &CompIndex);

    void GetLowTempRadiantSystem(EnergyPlusData &state);

    void InitLowTempRadiantSystem(EnergyPlusData &state,
                                  bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                                  int const RadSysNum, // Index for the low temperature radiant system under consideration within the derived types
                                  SystemType const systemType, // Type of radiant system: hydronic, constant flow, or electric
                                  bool &InitErrorFound         // Set to true when a severe or worse error is discovered during initialization
    );

    void SizeLowTempRadiantSystem(EnergyPlusData &state,
                                  int const RadSysNum, // Index for the low temperature radiant system under consideration within the derived types
                                  SystemType const systemType // Type of radiant system: hydronic, constant flow, or electric
    );

    void UpdateRadSysSourceValAvg(EnergyPlusData &state,
                                  bool &LowTempRadSysOn); // .TRUE. if the radiant system has run this zone time step

} // namespace LowTempRadiantSystem

struct LowTempRadiantSystemData : BaseGlobalStruct
{

    // DERIVED TYPE DEFINITIONS:

    // MODULE VARIABLE DECLARATIONS:
    // Standard, run-of-the-mill variables...
    int NumOfHydrLowTempRadSys = 0;    // Number of hydronic low tempererature radiant systems
    int NumOfHydrLowTempRadSysDes = 0; // Number of hydronic low tempererature radiant design systems
    int NumOfCFloLowTempRadSys = 0;    // Number of constant flow (hydronic) low tempererature radiant systems
    int NumOfCFloLowTempRadSysDes = 0; // Number of constant flow (hydronic) low tempererature radiant design systems
    int NumOfElecLowTempRadSys = 0;    // Number of electric low tempererature radiant systems
    int TotalNumOfRadSystems = 0;      // Total number of low temperature radiant systems

    bool GetInputFlag = true;
    int CFloCondIterNum = 0;     // Number of iterations for a constant flow radiant system--controls variable cond sys ctrl
    int MaxCloNumOfSurfaces = 0; // Used to set allocate size in CalcClo routine
    bool VarOffCond = false;     // Set to true when in cooling for constant flow system + variable off condensation predicted
    bool FirstTimeInit = true;   // Set to true for first pass through init routine then set to false
    bool anyRadiantSystemUsingRunningMeanAverage =
        false;                // Set to true when there is at least one constant flow radiant system that uses the running mean average
    Real64 LoopReqTemp = 0.0; // Temperature required at the inlet of the pump (from the loop) to meet control logic
    std::unordered_map<std::string, std::string> LowTempRadUniqueNames;
    bool FirstTimeFlag = true; // for setting size of Ckj, Cmj, WaterTempOut arrays // state
    bool MyEnvrnFlagGeneral = true;
    bool ZoneEquipmentListChecked = false; // True after the Zone Equipment List has been checked for items
    bool MyOneTimeFlag = true;             // Initialization flag
    bool warnTooLow = false;
    bool warnTooHigh = false;

    // Limit temperatures to indicate that a system cannot heat or cannot cool
    Real64 LowTempHeating = -200.0; // Used to indicate that a user does not have a heating control temperature
    Real64 HighTempCooling = 200.0; // Used to indicate that a user does not have a cooling control temperature

    Array1D<Real64> Ckj; // Coefficients for individual surfaces within a radiant system
    Array1D<Real64> Cmj;
    Array1D<Real64> WaterTempOut; // Array of outlet water temperatures for
    // each surface in the radiant system

    // For Init:
    Array1D_bool MyEnvrnFlagHydr;
    Array1D_bool MyEnvrnFlagCFlo;
    Array1D_bool MyEnvrnFlagElec;
    Array1D_bool MyPlantScanFlagHydr;
    Array1D_bool MyPlantScanFlagCFlo;
    // Autosizing variables
    Array1D_bool MySizeFlagHydr;
    Array1D_bool MySizeFlagCFlo;
    Array1D_bool MySizeFlagElec;
    Array1D_bool CheckEquipName;

    // Object Data
    Array1D<LowTempRadiantSystem::VariableFlowRadiantSystemData> HydrRadSys;
    Array1D<LowTempRadiantSystem::ConstantFlowRadiantSystemData> CFloRadSys;
    Array1D<LowTempRadiantSystem::ElectricRadiantSystemData> ElecRadSys;
    Array1D<LowTempRadiantSystem::RadSysTypeData> RadSysTypes;
    Array1D<LowTempRadiantSystem::ElecRadSysNumericFieldData> ElecRadSysNumericFields;
    Array1D<LowTempRadiantSystem::HydronicRadiantSysNumericFieldData> HydronicRadiantSysNumericFields;
    Array1D<LowTempRadiantSystem::ConstantFlowRadDesignData> CflowRadiantSysDesign;
    Array1D<LowTempRadiantSystem::VarFlowRadDesignData> HydronicRadiantSysDesign;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        LowTempHeating = -200.0;
        HighTempCooling = 200.0;
        NumOfHydrLowTempRadSys = 0;
        NumOfHydrLowTempRadSysDes = 0;
        NumOfCFloLowTempRadSys = 0;
        NumOfCFloLowTempRadSysDes = 0;
        NumOfElecLowTempRadSys = 0;
        TotalNumOfRadSystems = 0;

        // These are in the state space for Unit tests to work properly
        CFloCondIterNum = 0;
        MaxCloNumOfSurfaces = 0;
        VarOffCond = false;
        FirstTimeInit = true;
        anyRadiantSystemUsingRunningMeanAverage = false;
        LoopReqTemp = 0.0;
        LowTempRadUniqueNames.clear();
        GetInputFlag = true;
        FirstTimeFlag = true;
        MyEnvrnFlagGeneral = true;
        ZoneEquipmentListChecked = false;
        MyOneTimeFlag = true;
        warnTooLow = false;
        warnTooHigh = false;
        //

        Ckj.clear();
        Cmj.clear();
        WaterTempOut.clear();
        MyEnvrnFlagHydr.clear();
        MyEnvrnFlagCFlo.clear();
        MyEnvrnFlagElec.clear();
        MyPlantScanFlagHydr.clear();
        MyPlantScanFlagCFlo.clear();
        MySizeFlagHydr.clear();
        MySizeFlagCFlo.clear();
        MySizeFlagElec.clear();
        CheckEquipName.clear();
        HydrRadSys.clear(); //
        CFloRadSys.clear();
        ElecRadSys.clear();
        RadSysTypes.clear();
        ElecRadSysNumericFields.clear();
        HydronicRadiantSysNumericFields.clear();
        HydronicRadiantSysDesign.clear();
        CflowRadiantSysDesign.clear();
    }
};

} // namespace EnergyPlus

#endif
