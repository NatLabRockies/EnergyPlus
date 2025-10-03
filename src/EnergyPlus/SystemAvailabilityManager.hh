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

#ifndef SystemAvailabilityManager_hh_INCLUDED
#define SystemAvailabilityManager_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Avail {

    // The following parameters are used for system availability
    // status.  Putting this in SystemAvailabilityManager (where it
    // belongs) creates a circular dependecy between
    // SystemAvailabilityManager and PlantAvailablityManager which
    // needs to be resolved by unifying those two.
    enum class Status
    {
        Invalid = -1,
        NoAction,
        ForceOff,
        CycleOn,
        CycleOnZoneFansOnly,
        Num
    };

    enum class ControlAlgorithm
    {
        Invalid = -1,
        ConstantTemperatureGradient,
        AdaptiveTemperatureGradient,
        AdaptiveASHRAE,
        ConstantStartTime,
        Num
    };

    // Cycling Run Time Control Type
    enum class CyclingRunTimeControl
    {
        Invalid = -1,
        FixedRunTime,
        Thermostat,
        ThermostatWithMinimumRunTime,
        Num
    };

    enum class NightCycleControlType
    {
        Invalid = -1,
        Off,
        OnAny,
        OnControlZone,
        OnZoneFansOnly,
        OnAnyCoolingOrHeatingZone,
        OnAnyCoolingZone,
        OnAnyHeatingZone,
        OnAnyHeatingZoneFansOnly,
        Num
    };

    // Optimum start parameter definitions
    enum class OptimumStartControlType
    {
        Invalid = -1,
        Off,
        ControlZone,
        MaximumOfZoneList,
        Num
    };

    enum class ManagerType
    {
        Invalid = -1,
        Scheduled,
        ScheduledOn,
        ScheduledOff,
        NightCycle,
        DiffThermo,
        HiTempTOff,
        HiTempTOn,
        LoTempTOff,
        LoTempTOn,
        NightVent,
        HybridVent,
        OptimumStart,
        Num
    };

    struct SysAvailManager
    {
        // Members
        std::string Name;                        // Name of the manager object
        ManagerType type = ManagerType::Invalid; // Integer equivalent of availability manager type
        Sched::Schedule *availSched = nullptr;   // Schedule pointer
        Status availStatus = Status::NoAction;   // reports status of availability manager
    };

    struct SysAvailManagerScheduled : SysAvailManager // Derived type for Scheduled Sys Avail Managers
    {
    };

    struct SysAvailManagerScheduledOn : SysAvailManager // Derived type for Scheduled On Sys Avail Managers
    {
    };

    struct SysAvailManagerScheduledOff : SysAvailManager // Derived type for Scheduled Off Sys Avail Managers
    {
    };

    struct SysAvailManagerNightCycle : SysAvailManager // Derived type for Night Cycle Sys Avail Managers
    {
        // Members
        Sched::Schedule *fanSched = nullptr; // Fan schedule
        //   Cycle On Control Zone, or Cycle On Any - Zone Fans Only
        Real64 TempTolRange;                                                          // range in degrees C of thermostat tolerance
        int CyclingTimeSteps;                                                         // period (in Loads time steps) system will cycle on.
        Status priorAvailStatus = Status::NoAction;                                   // prior status of availability manager
        std::string CtrlZoneListName;                                                 // controlled zone or zonelist name
        int NumOfCtrlZones;                                                           // number of controlled zones
        Array1D_int CtrlZonePtrs;                                                     // pointers to controlled zone(s)
        std::string CoolingZoneListName;                                              // coolin zone or zonelist name
        int NumOfCoolingZones;                                                        // number of cooling zones
        Array1D_int CoolingZonePtrs;                                                  // pointers to cooling zone(s)
        std::string HeatingZoneListName;                                              // heatig zone or zonelist name
        int NumOfHeatingZones;                                                        // number of heatig zones
        Array1D_int HeatingZonePtrs;                                                  // pointers to heating zone(s)
        std::string HeatZnFanZoneListName;                                            // heating zone fans only zone or zonelist name
        int NumOfHeatZnFanZones;                                                      // number of heating zone fans only zones
        Array1D_int HeatZnFanZonePtrs;                                                // pointers to heating zone fans only zone(s)
        CyclingRunTimeControl cyclingRunTimeControl = CyclingRunTimeControl::Invalid; // Cycling Run Time Control Type
        NightCycleControlType nightCycleControlType = NightCycleControlType::Invalid; // type of control: Stay Off, Cycle On Any,

        // Default Constructor
        SysAvailManagerNightCycle()
            : TempTolRange(1.0), CyclingTimeSteps(1), NumOfCtrlZones(0), NumOfCoolingZones(0), NumOfHeatingZones(0), NumOfHeatZnFanZones(0)
        {
        }
    };

    struct SysAvailManagerOptimumStart : SysAvailManager // Derived type for Optimal Start Sys Avail Managers
    {
        // Members
        bool isSimulated; // true after availability manager is simulated

        Sched::Schedule *fanSched = nullptr; // Fan schedule
        std::string CtrlZoneName;            // Name of the control zone
        int ZoneNum;                         // zone number of control zone
        std::string ZoneListName;            // Zone List name
        int NumOfZones;                      // Number of zones in the list
        Array1D_int ZonePtrs;                // Pointers to zones in the list
        Real64 MaxOptStartTime;              // Maximum value of start time in hours
        ControlAlgorithm controlAlgorithm;   // Control algorithm: ConstantTemperatureGradient,
        // AdaptiveTemperatureGradient, AdaptiveASHRAE, ConstantStartTime
        Real64 ConstTGradCool;    // Constant temperature gradient in cooling mode, unit: degC per hour
        Real64 ConstTGradHeat;    // Constant temperature gradient in heating mode, unit: degC per hour
        Real64 InitTGradCool;     // Initial value for temperature gradient in cooling mode, unit: degC per hour
        Real64 InitTGradHeat;     // Initial value for temperature gradient in heating mode, unit: degC per hour
        Real64 AdaptiveTGradCool; // Calculated adaptive temperature gradient in cooling mode, unit: degC per hour
        Real64 AdaptiveTGradHeat; // Calculated adaptive temperature gradient in heating mode, unit: degC per hour
        Real64 ConstStartTime;    // Constant start time in hours
        int NumPreDays;           // Number of previous days for adaptive control
        Real64 NumHoursBeforeOccupancy;
        Real64 TempDiffHi;   // temperature difference for cooling mode
        Real64 TempDiffLo;   // temperature difference for heating mode
        int ATGWCZoneNumLo;  // zone index for worst case heating zone
        int ATGWCZoneNumHi;  // zone index for worst case cooling zone
        bool CycleOnFlag;    // Tracks when air loop has cycled on
        bool ATGUpdateFlag1; // updates
        bool ATGUpdateFlag2;
        bool FirstTimeATGFlag;
        bool OverNightStartFlag; // Flag to indicate the optimum start starts before mid night.
        bool OSReportVarFlag;
        Array1D<Real64> AdaTempGradTrdHeat; // Heating temp gradient for previous days
        Array1D<Real64> AdaTempGradTrdCool; // Cooling temp gradient for previous days
        Real64 AdaTempGradHeat;
        Real64 AdaTempGradCool;
        Real64 ATGUpdateTime1;
        Real64 ATGUpdateTime2;
        Real64 ATGUpdateTemp1;
        Real64 ATGUpdateTemp2;
        OptimumStartControlType optimumStartControlType =
            OptimumStartControlType::Invalid; // Type of control: Stay Off, ControlZone, MaximumofZoneList

        // Default Constructor
        SysAvailManagerOptimumStart()
            : isSimulated(false), ZoneNum(0), NumOfZones(0), MaxOptStartTime(6.0), controlAlgorithm(ControlAlgorithm::Invalid), ConstTGradCool(1.0),
              ConstTGradHeat(1.0), InitTGradCool(1.0), InitTGradHeat(1.0), AdaptiveTGradCool(1.0), AdaptiveTGradHeat(1.0), ConstStartTime(2.0),
              NumPreDays(1), NumHoursBeforeOccupancy(0.0), TempDiffHi(0.0), TempDiffLo(0.0), ATGWCZoneNumLo(0), ATGWCZoneNumHi(0), CycleOnFlag(false),
              ATGUpdateFlag1(false), ATGUpdateFlag2(false), FirstTimeATGFlag(true), OverNightStartFlag(false), OSReportVarFlag(false),
              AdaTempGradHeat(0.0), AdaTempGradCool(0.0), ATGUpdateTime1(0.0), ATGUpdateTime2(0.0), ATGUpdateTemp1(0.0), ATGUpdateTemp2(0.0)
        {
        }

        void SetOptStartFlag(EnergyPlusData &state, int const AirLoopNum);
    };

    struct DefineASHRAEAdaptiveOptimumStartCoeffs // Derived type for Differential Thermostat Sys Avail Managers
    {
        // Members
        std::string Name; // Name of the object
        Real64 Coeff1;    // 1st Coefficient of the equation
        Real64 Coeff2;    // 2nd Coefficient of the equation
        Real64 Coeff3;    // 3rd Coefficient of the equation
        Real64 Coeff4;    // 4th Coefficient of the equation

        // Default Constructor
        DefineASHRAEAdaptiveOptimumStartCoeffs() : Coeff1(0.0), Coeff2(0.0), Coeff3(0.0), Coeff4(0.0)
        {
        }
    };

    struct SysAvailManagerDiffThermo : SysAvailManager // Derived type for Differential Thermostat Sys Avail Managers
    {
        // Members
        int HotNode;        // "Hot" sensor node
        int ColdNode;       // "Cold" sensor node
        Real64 TempDiffOn;  // Temperature difference for turn on (delta C)
        Real64 TempDiffOff; // Temperature difference for turn off (delta C)

        // Default Constructor
        SysAvailManagerDiffThermo() : HotNode(0), ColdNode(0), TempDiffOn(0.0), TempDiffOff(0.0)
        {
        }
    };

    struct SysAvailManagerHiLoTemp : SysAvailManager // Derived type for High/Low Temperature On/Off Sys Avail Managers
    {
        // Members
        int Node;    // Sensor node
        Real64 Temp; // Temperature for on/off (C)

        // Default Constructor
        SysAvailManagerHiLoTemp() : Node(0), Temp(0.0)
        {
        }
    };

    struct SysAvailManagerNightVent : SysAvailManager
    {
        // Members
        Sched::Schedule *fanSched = nullptr;      // Fan schedule
        Sched::Schedule *ventTempSched = nullptr; // Ventilation temperature schedule
        Real64 VentDelT;                          // Ventilation delta T [deltaC]
        Real64 VentTempLowLim;                    // ventilation temperature low limit
        std::string CtrlZoneName;                 // Name of the control zone
        int ZoneNum;                              // zome number of control zone
        Real64 VentFlowFrac;                      // the night venting flow fraction

        // Default Constructor
        SysAvailManagerNightVent() : VentDelT(0.0), VentTempLowLim(0.0), ZoneNum(0), VentFlowFrac(0.0)
        {
        }
    };

    // Hybrid Ventilation parameters
    enum class VentCtrlType
    {
        Invalid = -1,
        No,       // No hybrid ventilation control
        Temp,     // Temperature control
        Enth,     // Enthalpy control
        DewPoint, // Dew point control
        OA,       // Outdoor air control
        OperT80,  // Operative temperature control with 80% acceptability limits
        OperT90,  // Operative temperature control with 90% acceptability limits
        CO2,      // CO2 control
        Num
    };

    enum class VentCtrlStatus
    {
        Invalid = -1,
        NoAction, // No hybrid ventilation control
        Open,     // Open windows or doors
        Close,    // Close windows or doors
        Num
    };

    struct SysAvailManagerHybridVent : SysAvailManager
    {
        // Members
        std::string AirLoopName;                              // Name of HVAC Air Loop
        int AirLoopNum;                                       // HVAC Air Loop number
        std::string ControlZoneName;                          // Controlled zone name
        int NodeNumOfControlledZone;                          // Controlled zone node number
        int ControlledZoneNum;                                // Controlled zone number
        Sched::Schedule *controlModeSched = nullptr;          // Ventilation control mode schedule
        VentCtrlType ctrlType = VentCtrlType::No;             // hybrid ventilation control mode
        VentCtrlStatus ctrlStatus = VentCtrlStatus::NoAction; // Ventilation control type: Noaction, Close, Open
        Real64 MinOutdoorTemp;                                // Minimum Outdoor Temperature [C]
        Real64 MaxOutdoorTemp;                                // Maximum Outdoor Temperature [C]
        Real64 MinOutdoorEnth;                                // Minimum Outdoor Enthalpy [J/kg]
        Real64 MaxOutdoorEnth;                                // Maximum Outdoor Enthalpy [J/kg]
        Real64 MinOutdoorDewPoint;                            // Minimum Outdoor Dew point temperature [C]
        Real64 MaxOutdoorDewPoint;                            // Maximum Outdoor Dew Point Temperature [C]
        Real64 MaxWindSpeed;                                  // Maximum Wind speed [m/s]
        bool UseRainIndicator;                                // Use WeatherFile Rain Indicators
        Sched::Schedule *minOASched = nullptr;                // Minimum Outdoor Ventilation Air Schedule
        int DewPointNoRHErrCount;                             // Dewpoint control mode error count without a humidistat
        int DewPointNoRHErrIndex;                             // Dewpoint control mode error index without a humidistat
        int DewPointErrCount;                                 // Dewpoint control mode error count without a valid humidistat
        int DewPointErrIndex;                                 // Dewpoint control mode error index without a valid humidistat
        int SingleHCErrCount;                                 // Temperature and enthalpy control mode error count
        // with a singleHeatingCooling setpoint
        int SingleHCErrIndex; // Temperature and enthalpy control mode error index
        // with a singleHeatingCooling setpoint
        int OpeningFactorFWS; // Opening factor modifier as a function of wind speed

        Sched::Schedule *afnControlTypeSched = nullptr;    // AirflowNetwork control type schedule pointer
        Sched::Schedule *simpleControlTypeSched = nullptr; // Simple airflow object control type schedule pointer
        int VentilationPtr;                                // Ventilation object name pointer
        std::string VentilationName;                       // Ventilation object name
        bool HybridVentMgrConnectedToAirLoop;              // Flag to check whether hybrid ventilation
        // manager is connected to air loop
        bool SimHybridVentSysAvailMgr; // Set to false when a zone has two hybrid ventilation
        // managers, one with air loop and one without
        Real64 OperativeTemp;    // Zone air operative temperature [C]
        Real64 CO2;              // Zone air CO2 [ppm]
        Real64 MinOperTime;      // Minimum HVAC Operation Time [minutes]
        Real64 MinVentTime;      // Minimum Ventilation Time [minutes]
        Real64 TimeOperDuration; // Time duration with continuous HVAC operation [minutes]
        Real64 TimeVentDuration; // Time duration with continuous ventilation [minutes]
        Real64 minAdaTem;        // minimum adaptive temperature for adaptive temperature control [C]
        Real64 maxAdaTem;        // maximum adaptive temperature for adaptive temperature control [C]

        int afnControlStatus = 0;
        int Master = 0;
        Real64 WindModifier = 0.0;

        // Default Constructor
        SysAvailManagerHybridVent()
            : AirLoopNum(0), NodeNumOfControlledZone(0), ControlledZoneNum(0), MinOutdoorTemp(-100.0), MaxOutdoorTemp(100.0), MinOutdoorEnth(0.1),
              MaxOutdoorEnth(300000.0), MinOutdoorDewPoint(-100.0), MaxOutdoorDewPoint(100.0), MaxWindSpeed(0.0), UseRainIndicator(true),
              DewPointNoRHErrCount(0), DewPointNoRHErrIndex(0), DewPointErrCount(0), DewPointErrIndex(0), SingleHCErrCount(0), SingleHCErrIndex(0),
              OpeningFactorFWS(0), VentilationPtr(0), HybridVentMgrConnectedToAirLoop(true), SimHybridVentSysAvailMgr(false), OperativeTemp(0.0),
              CO2(0.0), MinOperTime(0.0), MinVentTime(0.0), TimeOperDuration(0.0), TimeVentDuration(0.0), minAdaTem(0.0), maxAdaTem(0.0)
        {
        }
    };

    struct AvailManagerNTN
    {
        std::string Name;
        ManagerType type;
        int Num;
    };

    struct List // LOL
    {
        // Members
        std::string Name; // Availability Manager List Name
        int NumItems = 0;
        Array1D<AvailManagerNTN> availManagers;
    };

    struct DefineZoneCompAvailMgrs
    {
        // Members
        int NumAvailManagers = 0;               // number of availability managers for this system
        Status availStatus = Status::NoAction;  // system availability status
        int StartTime = 0;                      // cycle on time (in SimTimeSteps)
        int StopTime = 0;                       // cycle off time (in SimTimeSteps)
        std::string AvailManagerListName;       // name of each availability manager
        Array1D<AvailManagerNTN> availManagers; // type of availability manager
        int ZoneNum = 0;                        // cycle off time (in SimTimeSteps)
        bool Input = true;                      // starts off as true to initialize zone equipment availability manager data
        int Count = 0;                          // initialize twice to ensure zone equipment availability manager list name has been read in
    };

    struct ZoneCompTypeData
    {
        // Members
        Array1D<DefineZoneCompAvailMgrs> ZoneCompAvailMgrs;
        int TotalNumComp = 0; // total number of components of a zone equip type
    };

    void ManageSystemAvailability(EnergyPlusData &state);

    void GetSysAvailManagerInputs(EnergyPlusData &state);

    void GetSysAvailManagerListInputs(EnergyPlusData &state);

    void GetPlantAvailabilityManager(EnergyPlusData &state,
                                     std::string const &AvailabilityListName, // name that should be an Availability Manager List Name
                                     int const Loop,                          // which loop this is
                                     int const NumPlantLoops,                 // Total number of plant loops
                                     bool &ErrorsFound                        // true if certain errors are detected here
    );

    void GetAirLoopAvailabilityManager(EnergyPlusData &state,
                                       std::string const &AvailabilityListName, // name that should be an Availability Manager List Name
                                       int const Loop,                          // which loop this is
                                       int const NumAirLoops,                   // Total number of air loops
                                       bool &ErrorsFound                        // true if certain errors are detected here
    );

    void GetZoneEqAvailabilityManager(EnergyPlusData &state,
                                      int const ZoneEquipType, // Type of ZoneHVAC:* component
                                      int const CompNum,       // Index of a particular ZoneHVAC:* component
                                      bool &ErrorsFound        // true if certain errors are detected here
    );

    void FillPredefinedTablesForAvailManager(EnergyPlusData &state);

    void InitSysAvailManagers(EnergyPlusData &state);

    Status SimSysAvailManager(EnergyPlusData &state,
                              ManagerType const type,
                              std::string const &SysAvailName,
                              int &SysAvailNum,
                              int const PriAirSysNum, // Primary Air System index. If being called for a ZoneHVAC:* component
                              Status const previousStatus,
                              ObjexxFCL::Optional_int_const ZoneEquipType = _, // Type of ZoneHVAC:* equipment component
                              ObjexxFCL::Optional_int_const CompNum = _        // Index of ZoneHVAC:* equipment component
    );

    Status CalcSchedSysAvailMgr(EnergyPlusData &state,
                                int const SysAvailNum // number of the current scheduled system availability manager
    );

    Status CalcSchedOnSysAvailMgr(EnergyPlusData &state,
                                  int const SysAvailNum // number of the current scheduled on system availability manager
    );

    Status CalcSchedOffSysAvailMgr(EnergyPlusData &state,
                                   int const SysAvailNum // number of the current scheduled off system availability manager
    );

    Status CalcNCycSysAvailMgr(EnergyPlusData &state,
                               int const SysAvailNum,                           // number of the current scheduled system availability manager
                               int const PriAirSysNum,                          // number of the primary air system affected by this Avail. Manager
                               ObjexxFCL::Optional_int_const ZoneEquipType = _, // Type of ZoneHVAC equipment component
                               ObjexxFCL::Optional_int_const CompNum = _        // Index of ZoneHVAC equipment component
    );

    bool CoolingZoneOutOfTolerance(EnergyPlusData &state,
                                   Array1D_int const ZonePtrList, // list of controlled zone pointers
                                   int const NumZones,            // number of zones in list
                                   Real64 const TempTolerance     // temperature tolerance
    );

    bool HeatingZoneOutOfTolerance(EnergyPlusData &state,
                                   Array1D_int const ZonePtrList, // list of controlled zone pointers
                                   int const NumZones,            // number of zones in list
                                   Real64 const TempTolerance     // temperature tolerance
    );

    Status CalcOptStartSysAvailMgr(EnergyPlusData &state,
                                   int const SysAvailNum,  // number of the current scheduled system availability manager
                                   int const PriAirSysNum, // number of the primary air system affected by this Avail. Manager
                                   ObjexxFCL::Optional_int_const ZoneEquipType = _, // Type of ZoneHVAC equipment component
                                   ObjexxFCL::Optional_int_const CompNum = _        // Index of ZoneHVAC equipment component
    );

    Status CalcNVentSysAvailMgr(EnergyPlusData &state,
                                int const SysAvailNum,             // number of the current scheduled system availability manager
                                int const PriAirSysNum,            // number of the primary air system affected by this Avail. Manager
                                bool const isZoneEquipType = false // Type of zone equipment component
    );

    Status CalcDiffTSysAvailMgr(EnergyPlusData &state,
                                int const SysAvailNum,      // Number of the current scheduled system availability manager
                                Status const PreviousStatus // System status for the previous timestep
    );

    Status CalcHiTurnOffSysAvailMgr(EnergyPlusData &state,
                                    int const SysAvailNum // Number of the current scheduled system availability manager
    );

    Status CalcHiTurnOnSysAvailMgr(EnergyPlusData &state,
                                   int const SysAvailNum // Number of the current scheduled system availability manager
    );

    Status CalcLoTurnOffSysAvailMgr(EnergyPlusData &state,
                                    int const SysAvailNum // Number of the current scheduled system availability manager
    );

    Status CalcLoTurnOnSysAvailMgr(EnergyPlusData &state,
                                   int const SysAvailNum // Number of the current scheduled system availability manager
    );

    void ManageHybridVentilation(EnergyPlusData &state);

    void GetHybridVentilationInputs(EnergyPlusData &state);

    void InitHybridVentSysAvailMgr(EnergyPlusData &state);

    void CalcHybridVentSysAvailMgr(EnergyPlusData &state,
                                   int const SysAvailNum,                         // number of the current scheduled system availability manager
                                   ObjexxFCL::Optional_int_const PriAirSysNum = _ // number of the primary air system affected by this Avail. Manager
    );

    bool GetHybridVentilationControlStatus(EnergyPlusData &state, int const ZoneNum); // Index of zone

    struct PlantAvailMgrData
    {
        // Members
        int NumAvailManagers = 0;               // number of availability managers for this plant loop
        Status availStatus = Status::NoAction;  // system availability status
        int StartTime = 0;                      // cycle on time (in SimTimeSteps)
        int StopTime = 0;                       // cycle off time (in SimTimeSteps)
        Array1D<AvailManagerNTN> availManagers; // type of availability manager
    };

    struct OptStartData
    {
        int ActualZoneNum = 0;
        Real64 OccStartTime = 0.0;
        bool OptStartFlag = false;
    };

} // namespace Avail

struct SystemAvailabilityManagerData : BaseGlobalStruct
{
    int NumSchedSysAvailMgrs = 0;
    int NumSchedOnSysAvailMgrs = 0;
    int NumSchedOffSysAvailMgrs = 0;
    int NumNCycSysAvailMgrs = 0;
    int NumDiffTSysAvailMgrs = 0;
    int NumHiTurnOffSysAvailMgrs = 0;
    int NumHiTurnOnSysAvailMgrs = 0;
    int NumLoTurnOffSysAvailMgrs = 0;
    int NumLoTurnOnSysAvailMgrs = 0;
    int NumNVentSysAvailMgrs = 0;
    int NumAvailManagerLists = 0;
    int NumOptStartSysAvailMgrs = 0;
    int NumHybridVentSysAvailMgrs = 0; // Number of hybrid ventilation control

    bool GetAvailListsInput = true;
    bool GetAvailMgrInputFlag = true; // First time, input is "gotten"
    bool GetHybridInputFlag = true;   // Flag set to make sure you get input once

    bool InitSysAvailManagers_MyOneTimeFlag = true;
    bool CalcNCycSysAvailMgr_OneTimeFlag = true;
    Array1D<Real64> OptStart_AdaTempGradTrdHeat; // Heating temp gradient for previous days - used in CalcOptStartSysAvailMgr
    Array1D<Real64> OptStart_AdaTempGradTrdCool; // Cooling temp gradient for previous days - used in CalcOptStartSysAvailMgr

    EPVector<Avail::SysAvailManagerScheduled> SchedData;
    EPVector<Avail::SysAvailManagerScheduledOn> SchedOnData;
    EPVector<Avail::SysAvailManagerScheduledOff> SchedOffData;
    EPVector<Avail::SysAvailManagerNightCycle> NightCycleData;
    EPVector<Avail::SysAvailManagerDiffThermo> DiffThermoData;
    EPVector<Avail::SysAvailManagerHiLoTemp> HiTurnOffData;
    EPVector<Avail::SysAvailManagerHiLoTemp> HiTurnOnData;
    EPVector<Avail::SysAvailManagerHiLoTemp> LoTurnOffData;
    EPVector<Avail::SysAvailManagerHiLoTemp> LoTurnOnData;
    EPVector<Avail::SysAvailManagerNightVent> NightVentData;
    EPVector<Avail::SysAvailManagerHybridVent> HybridVentData;
    EPVector<Avail::List> ListData;
    EPVector<Avail::SysAvailManagerOptimumStart> OptimumStartData;
    EPVector<Avail::DefineASHRAEAdaptiveOptimumStartCoeffs> ASHRAEOptSCoeffCooling;
    EPVector<Avail::DefineASHRAEAdaptiveOptimumStartCoeffs> ASHRAEOptSCoeffHeating;

    Array1D<Avail::ZoneCompTypeData> ZoneComp;
    Array1D<Avail::PlantAvailMgrData> PlantAvailMgr;
    Array1D<Avail::OptStartData> OptStart; // For optimum start

    bool BeginOfDayResetFlag = true;

    Array1D_bool ZoneCompNCControlType;
    bool MyOneTimeFlag = true; // One time flag
    bool MyEnvrnFlag = true;

    Real64 CurrentEndTime = 0.0;     // Current end time
    Real64 CurrentEndTimeLast = 0.0; // last end time
    Real64 TimeStepSysLast = 0.0;    // last system time step

    // Hybrid ventilation control part
    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        NumSchedSysAvailMgrs = 0;
        NumSchedOnSysAvailMgrs = 0;
        NumSchedOffSysAvailMgrs = 0;
        NumNCycSysAvailMgrs = 0;
        NumDiffTSysAvailMgrs = 0;
        NumHiTurnOffSysAvailMgrs = 0;
        NumHiTurnOnSysAvailMgrs = 0;
        NumLoTurnOffSysAvailMgrs = 0;
        NumLoTurnOnSysAvailMgrs = 0;
        NumNVentSysAvailMgrs = 0;
        NumAvailManagerLists = 0;
        GetAvailListsInput = true;
        GetAvailMgrInputFlag = true;
        GetHybridInputFlag = true;
        InitSysAvailManagers_MyOneTimeFlag = true;
        CalcNCycSysAvailMgr_OneTimeFlag = true;
        NumOptStartSysAvailMgrs = 0;
        SchedData.deallocate();
        SchedOnData.deallocate();
        SchedOffData.deallocate();
        NightCycleData.deallocate();
        DiffThermoData.deallocate();
        HiTurnOffData.deallocate();
        HiTurnOnData.deallocate();
        LoTurnOffData.deallocate();
        LoTurnOnData.deallocate();
        NightVentData.deallocate();
        HybridVentData.deallocate();
        ListData.deallocate();
        OptimumStartData.deallocate();
        ASHRAEOptSCoeffCooling.deallocate();
        ASHRAEOptSCoeffHeating.deallocate();
        BeginOfDayResetFlag = true;
        OptStart_AdaTempGradTrdHeat.deallocate();
        OptStart_AdaTempGradTrdCool.deallocate();
        PlantAvailMgr.deallocate();
        MyOneTimeFlag = true;
        MyEnvrnFlag = true;
        CurrentEndTime = 0.0;
        CurrentEndTimeLast = 0.0;
        TimeStepSysLast = 0.0;
    }
};
} // namespace EnergyPlus

#endif
