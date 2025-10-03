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

#ifndef MixedAir_hh_INCLUDED
#define MixedAir_hh_INCLUDED

// C++ Headers
#include <unordered_set>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataAirLoop.hh>
// #include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/SimAirServingZones.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace MixedAir {

    // Data
    // MODULE PARAMETER DEFINITIONS

    enum class LockoutType
    {
        Invalid = -1,
        NoLockoutPossible,
        LockoutWithHeatingPossible,
        LockoutWithCompressorPossible,
        Num
    };

    enum class EconoOp
    {
        Invalid = -1,
        NoEconomizer,
        FixedDryBulb,
        FixedEnthalpy,
        DifferentialDryBulb,
        DifferentialEnthalpy,
        FixedDewPointAndDryBulb,
        ElectronicEnthalpy,
        DifferentialDryBulbAndEnthalpy,
        Num
    };

    enum class MixedAirControllerType
    {
        Invalid = -1,
        ControllerOutsideAir,
        ControllerStandAloneERV,
        Num,
    };

    // Parameters below (CMO - Current Module Object.  used primarily in Get Inputs)
    // Multiple Get Input routines in this module or these would be in individual routines.
    enum class CMO
    {
        Invalid = -1,
        None,
        OASystem,
        AirLoopEqList,
        ControllerList,
        SysAvailMgrList,
        OAController,
        ERVController,
        MechVentilation,
        OAMixer,
        Num,
    };

    // OA Controller Limiting Factor
    // Must keep these values to use for integer output variable OAControllerProps::OALimitingFactorReport
    enum class OALimitFactor
    {
        Invalid = -1,
        None = 0,        // No limit other than fixed OA amount
        Limits = 1,      // Limits and scheduled limits
        Economizer = 2,  // Economizer operation
        Exhaust = 3,     // Exhaust flow
        MixedAir = 4,    // Mixed air flow rate
        HighHum = 5,     // High humidity economizer control
        DCV = 6,         // Demand-controlled ventilation
        NightVent = 7,   // Night ventilation
        DemandLimit = 8, // Demand-limiting
        EMS = 9,         // EMS override
        Num
    };

    extern const std::array<std::string_view, static_cast<int>(CMO::Num)> CurrentModuleObjects;

    struct ControllerListProps
    {
        // Members
        std::string Name;
        int NumControllers = 0; // number of controllers on list
        EPVector<DataAirLoop::ControllerKind> ControllerType;
        Array1D_string ControllerName;
    };

    struct OAControllerProps // Derived type for Outside Air Controller data
    {
        // Members
        std::string Name;
        MixedAirControllerType ControllerType = MixedAirControllerType::Invalid; // Mixed air controller type
        LockoutType Lockout = LockoutType::NoLockoutPossible;                    // 0=NoLockoutPossible; 1=LockoutWithHeatingPossible;
        // 2=LockoutWithCompressorPossible;
        bool FixedMin = true;                  // Fixed Minimum or Proportional Minimum
        Real64 TempLim = 0.0;                  // Temperature Limit
        Real64 TempLowLim = 0.0;               // Temperature Lower Limit
        Real64 EnthLim = 0.0;                  // Enthalpy Limit
        Real64 DPTempLim = 0.0;                // Dew Point Temperature Limit
        int EnthalpyCurvePtr = 0;              // Electronic Enthalpy Curve Index (max HumRat = f[OAT])
        Real64 MinOA = 0.0;                    // Minimum outside air flow (m3/sec)
        Real64 MaxOA = 0.0;                    // Maximum outside air flow (m3/sec)
        EconoOp Econo = EconoOp::NoEconomizer; // 0 = NoEconomizer, 1 = FixedDryBulb, 2 = FixedEnthalpy, 3=DifferentialDryBulb,
        // 4=DifferentialEnthalpy, 5=FixedDewPointAndDryBulb, 6 = ElectronicEnthalpy,
        // 7 =DifferentialDryBulbAndEnthalpy
        bool EconBypass = false;               // ModulateFlow =FALSE , MinimumFlowWithBypass =TRUE
        int MixNode = 0;                       // Controlled node (mixed air node)
        int OANode = 0;                        // Actuated node (outside air node)
        int InletNode = 0;                     // Inlet Air Node for into Mixer  (BTG Nov 2004)
        int RelNode = 0;                       // Relief Air Node Number
        int RetNode = 0;                       // Return Air Node Number
        Sched::Schedule *minOASched = nullptr; // minimum outside air schedule
        Real64 RelMassFlow = 0.0;
        Real64 OAMassFlow = 0.0;
        Real64 ExhMassFlow = 0.0;
        Real64 MixMassFlow = 0.0;
        Real64 InletTemp = 0.0;
        Real64 InletEnth = 0.0;
        Real64 InletPress = 0.0;
        Real64 InletHumRat = 0.0;
        Real64 OATemp = 0.0;
        Real64 OAEnth = 0.0;
        Real64 OAPress = 0.0;
        Real64 OAHumRat = 0.0;
        Real64 RetTemp = 0.0;
        Real64 RetEnth = 0.0;
        Real64 MixSetTemp = 0.0;
        Real64 MinOAMassFlowRate = 0.0; // Minimum outside air flow (kg/s)
        Real64 MaxOAMassFlowRate = 0.0; // Maximum outside air flow (kg/s)
        Real64 RelTemp = 0.0;
        Real64 RelEnth = 0.0;
        Real64 RelSensiLossRate = 0.0; // Heat lost to ambient from relief air (W)
        Real64 RelLatentLossRate = 0.0;
        Real64 RelTotalLossRate = 0.0;

        int ZoneEquipZoneNum = 0;
        std::string VentilationMechanicalName;   // Name of ventilation:mechanical object used for DCV
        int VentMechObjectNum = 0;               // Index to VENTILATION:MECHANICAL object for this controller
        int HumidistatZoneNum = 0;               // zone number where humidistat is located
        int NodeNumofHumidistatZone = 0;         // node number of zone where humidistat is located
        Real64 HighRHOAFlowRatio = 1.0;          // Modify ratio with respect to maximum outdoor air flow rate (high RH)
        bool ModifyDuringHighOAMoisture = false; // flag to Modify outdoor air flow, TRUE when modify any time, FALSE when modify only when indoor air
                                                 // humrat is less than outdoor HR
        Sched::Schedule *economizerOASched = nullptr; // schedule to modify outdoor air flow
        Sched::Schedule *minOAflowSched = nullptr;    // Index to the Minimum Fraction of Outdoor Air Schedule
        Sched::Schedule *maxOAflowSched = nullptr;    // Index to the Maximum Fraction of Outdoor Air Schedule
        //   Economizer Status, which is currently following the EconomizerOperationFlag, might be something like "Economizer status
        //   indicates when the conditions are favorable for the economizer to operate (i.e., none of the control limits have been exceeded).
        //   While this status signal indicates favorable conditions for economizer operation, it does not guarantee that the air-side
        //   economizer has increased outdoor air flow above the minimum level since the actual outdoor air flow rate is also governed
        //   by other controls (e.g., mixed air setpoint temperature, time of day economizer control, etc.).
        int EconomizerStatus = 0;             // Air Economizer status (1 = on, 0 = off or economizer not exists)
        int HeatRecoveryBypassStatus = 0;     // OA Sys Heat Recovery Bypass status (1 = on, 0 = off or economizer not exists)
        int HRHeatingCoilActive = 0;          // OA Sys Heat Recovery Heating Coil Was Active status (1 = on, 0 = off)
        Real64 MixedAirTempAtMinOAFlow = 0.0; // calculated mixed air temp at min flow rate when using special HX bypass control
        int HighHumCtrlStatus = 0;            // High Humidity Control status (1 = on, 0 = off or high hum ctrl not used)
        Real64 OAFractionRpt = 0.0;           // Actual outdoor air fraction for reporting (based on mixed air flow rate),
        // 0 to 1 (normally)
        Real64 MinOAFracLimit = 0.0;            // Minimum OA fraction limit
        Real64 MechVentOAMassFlowRequest = 0.0; // outside air mass flow rate calculated by mechanical ventilation object [kg/s]
        bool EMSOverrideOARate = false;         // if true, EMS is calling to override OA rate
        Real64 EMSOARateValue = 0.0;            // Value EMS is directing to use. [kg/s]
        int HeatRecoveryBypassControlType = HVAC::BypassWhenWithinEconomizerLimits; // User input selects type of heat recovery optimization
        HVAC::EconomizerStagingType EconomizerStagingType =
            HVAC::EconomizerStagingType::InterlockedWithMechanicalCooling; // User input select type of economizer staging operation
        bool ManageDemand = false;                                         // Used by demand manager to manage ventilation
        Real64 DemandLimitFlowRate = 0.0;                                  // Current demand limit if demand manager is ON
        Real64 MaxOAFracBySetPoint = 0.0;                                  // The maximum OA fraction due to freezing cooling coil check
        int MixedAirSPMNum = 0;                                            // index of mixed air setpoint manager
        bool CoolCoilFreezeCheck = false;                        // if true, cooling coil freezing is prevented by recalculating the amount of OA
        bool EconoActive = false;                                // if true economizer is active
        bool HighHumCtrlActive = false;                          // if true high humidity control is active
        Array1D_int EconmizerFaultNum;                           // index to economizer fault
        int NumFaultyEconomizer = 0;                             // total number of economizer faults
        int CountMechVentFrac = 0;                               // Count when OA min fraction > mech vent fraction
        int IndexMechVentFrac = 0;                               // Index when OA min fraction > mech vent fraction
        OALimitFactor OALimitingFactor = OALimitFactor::Invalid; // OA controller limiting factor
        int OALimitingFactorReport = 0;                          // OA controller limiting factor - integer for reporting

        void CalcOAController(EnergyPlusData &state, int AirLoopNum, bool FirstHVACIteration);

        void CalcOAEconomizer(
            EnergyPlusData &state, int AirLoopNum, Real64 OutAirMinFrac, Real64 &OASignal, bool &HighHumidityOperationFlag, bool FirstHVACIteration);

        void SizeOAController(EnergyPlusData &state);

        void UpdateOAController(EnergyPlusData &state);

        void Checksetpoints(EnergyPlusData &state,
                            Real64 OutAirMinFrac,         // Local variable used to calculate min OA fraction
                            Real64 &OutAirSignal,         // Used to set OA mass flow rate
                            bool &EconomizerOperationFlag // logical used to show economizer status
        );
    };

    struct VentilationMechanicalZoneProps
    {
        std::string name;                                   // name of mech vent zone
        int zoneNum = 0;                                    // Actual zones number
        int ZoneDesignSpecOAObjIndex = 0;                   // index of the design specification outdoor air object for each zone
        Real64 ZoneADEffCooling = 1.0;                      // Zone air distribution effectiveness in cooling mode for each zone
        Real64 ZoneADEffHeating = 1.0;                      // Zone air distribution effectiveness in heating mode for each zone
        Sched::Schedule *zoneADEffSched = nullptr;          // air distribution effectiveness schedule for each zone
        int ZoneDesignSpecADObjIndex = 0;                   // index of the design specification zone air distribution object for each zone
        Real64 ZoneSecondaryRecirculation = 0.0;            // zone air secondary recirculation ratio for each zone
        Sched::Schedule *zoneOASched = nullptr;             // Outdoor air schedule for each zone (from DesignSpecification:OutdoorAir or default)
        Sched::Schedule *zonePropCtlMinRateSched = nullptr; // Outdoor design OA flow rate schedule from DesignSpecification:OutdoorAir
        Real64 zoneOABZ = 0.0;                              // Zone breathing-zone OA flow rate [m3/s]
        EPVector<int> peopleIndexes; // List of People objects in this zone (for SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOcc)
    };

    struct VentilationMechanicalProps // Derived type for Ventilation:Mechanical data
    {
        // Members
        std::string Name;                                                          // Name of Ventilation:Mechanical object
        Sched::Schedule *availSched = nullptr;                                     // Mechanical ventilation schedule
        bool DCVFlag = false;                                                      // if true, implement OA based on demand controlled ventilation
        int NumofVentMechZones = 0;                                                // Number of zones with mechanical ventilation
        DataSizing::SysOAMethod SystemOAMethod = DataSizing::SysOAMethod::Invalid; // System Outdoor Air Method - SOAM_ZoneSum, SOAM_VRP, SOAM_VRPL
        Real64 ZoneMaxOAFraction = 1.0;                                            // Zone maximum outdoor air fraction
        int CO2MaxMinLimitErrorCount = 0; // Counter when max CO2 concentration < min CO2 concentration for SOAM_ProportionalControlSchOcc
        int CO2MaxMinLimitErrorIndex = 0; // Index for max CO2 concentration < min CO2 concentration recurring error message for
                                          // SOAM_ProportionalControlSchOcc
        int CO2GainErrorCount = 0;        // Counter when CO2 generation from people is zero for SOAM_ProportionalControlSchOcc
        int CO2GainErrorIndex = 0; // Index for recurring error message when CO2 generation from people is zero for SOAM_ProportionalControlSchOcc
        int OAMaxMinLimitErrorCount = 0; // Counter when max OA < min OA for SOAM_ProportionalControlDesOARate
        int OAMaxMinLimitErrorIndex = 0; // Index for max OA < min OA recurring error message for SOAM_ProportionalControlDesOARate
        Real64 Ep = 1.0;                 // zone primary air fraction
        Real64 Er = 0.0;                 // zone secondary recirculation fraction
        Real64 Fa = 1.0;                 // temporary variable used in multi-path VRP calc
        Real64 Fb = 1.0;
        Real64 Fc = 1.0;
        Real64 Xs = 1.0;       // uncorrected system outdoor air fraction
        Real64 Evz = 1.0;      // zone ventilation efficiency
        Real64 SysDesOA = 0.0; // System design OA
        EPVector<VentilationMechanicalZoneProps> VentMechZone;

        Real64 CalcMechVentController(EnergyPlusData &state,
                                      Real64 SysSA // System supply air mass flow rate [kg/s]
        );
    };

    struct OAMixerProps // Derived type for Outside Air Mixing Component
    {
        // Members
        std::string Name;
        int MixerIndex = 0; // Set on first call...
        int MixNode = 0;    // Outlet node - mixed air
        int InletNode = 0;  // Inlet node for outside air stream (Nov. 2004 BTG was OANode )
        int RelNode = 0;    // Outlet node - relief air
        int RetNode = 0;    // Inlet node - return air
        Real64 MixTemp = 0.0;
        Real64 MixHumRat = 0.0;
        Real64 MixEnthalpy = 0.0;
        Real64 MixPressure = 0.0;
        Real64 MixMassFlowRate = 0.0;
        Real64 OATemp = 0.0;
        Real64 OAHumRat = 0.0;
        Real64 OAEnthalpy = 0.0;
        Real64 OAPressure = 0.0;
        Real64 OAMassFlowRate = 0.0;
        Real64 RelTemp = 0.0;
        Real64 RelHumRat = 0.0;
        Real64 RelEnthalpy = 0.0;
        Real64 RelPressure = 0.0;
        Real64 RelMassFlowRate = 0.0;
        Real64 RetTemp = 0.0;
        Real64 RetHumRat = 0.0;
        Real64 RetEnthalpy = 0.0;
        Real64 RetPressure = 0.0;
        Real64 RetMassFlowRate = 0.0;

        void InitOAMixer(EnergyPlusData &state);

        void CalcOAMixer(EnergyPlusData &state);

        void UpdateOAMixer(EnergyPlusData &state) const;
    };

    // Functions

    Real64 OAGetFlowRate(EnergyPlusData &state, int OAPtr);

    Real64 OAGetMinFlowRate(EnergyPlusData &state, int OAPtr);

    void OASetDemandManagerVentilationState(EnergyPlusData &state, int OAPtr, bool aState);

    void OASetDemandManagerVentilationFlow(EnergyPlusData &state, int OAPtr, Real64 aFlow);

    int GetOAController(EnergyPlusData &state, std::string const &OAName);

    void ManageOutsideAirSystem(EnergyPlusData &state, std::string const &OASysName, bool FirstHVACIteration, int AirLoopNum, int &OASysNum);

    void SimOutsideAirSys(EnergyPlusData &state, int OASysNum, bool FirstHVACIteration, int AirLoopNum);

    void SimOASysComponents(EnergyPlusData &state, int OASysNum, bool FirstHVACIteration, int AirLoopNum);

    void SimOAComponent(EnergyPlusData &state,
                        std::string const &CompType,              // the component type
                        std::string const &CompName,              // the component Name
                        SimAirServingZones::CompType CompTypeNum, // Component Type -- Integerized for this module
                        bool FirstHVACIteration,
                        int &CompIndex,
                        int AirLoopNum,      // air loop index for economizer lockout coordination
                        bool Sim,            // if TRUE, simulate component; if FALSE, just set the coil exisitence flags
                        int OASysNum,        // index to outside air system
                        bool &OAHeatingCoil, // TRUE indicates a heating coil has been found
                        bool &OACoolingCoil, // TRUE indicates a cooling coil has been found
                        bool &OAHX);         // TRUE indicates a heat exchanger has been found

    void SimOAMixer(EnergyPlusData &state, std::string const &CompName, int &CompIndex);

    void SimOAController(EnergyPlusData &state, std::string const &CtrlName, int &CtrlIndex, bool FirstHVACIteration, int AirLoopNum);

    void GetOutsideAirSysInputs(EnergyPlusData &state);

    void GetOAControllerInputs(EnergyPlusData &state);

    void AllocateOAControllers(EnergyPlusData &state);

    void GetOAMixerInputs(EnergyPlusData &state);

    void ProcessOAControllerInputs(EnergyPlusData &state,
                                   std::string_view CurrentModuleObject,
                                   int OutAirNum,
                                   Array1D_string const &AlphArray,
                                   int const NumAlphas,
                                   Array1D<Real64> const &NumArray,
                                   int const NumNums,
                                   Array1D_bool const &lNumericBlanks, // Unused
                                   Array1D_bool const &lAlphaBlanks,
                                   Array1D_string const &cAlphaFields,
                                   Array1D_string const &cNumericFields, // Unused
                                   bool &ErrorsFound                     // If errors found in input
    );

    void InitOutsideAirSys(EnergyPlusData &state, int OASysNum, int AirLoopNum);

    void InitOAController(EnergyPlusData &state, int OAControllerNum, bool FirstHVACIteration, int AirLoopNum);

    Array1D_int GetOAMixerNodeNumbers(EnergyPlusData &state,
                                      std::string const &OAMixerName, // must match OA mixer names for the OA mixer type
                                      bool &ErrorsFound               // set to true if problem
    );

    int GetNumOAMixers(EnergyPlusData &state);

    int GetNumOAControllers(EnergyPlusData &state);

    int GetOAMixerReliefNodeNumber(EnergyPlusData &state, int OAMixerNum); // Which Mixer

    int GetOASysControllerListIndex(EnergyPlusData &state, int OASysNumber); // OA Sys Number

    int GetOASysNumSimpControllers(EnergyPlusData &state, int OASysNumber); // OA Sys Number

    int GetOASysNumHeatingCoils(EnergyPlusData &state, int OASysNumber); // OA Sys Number

    int GetOASysNumHXs(EnergyPlusData &state, int OASysNumber); // OA Sys Number

    int GetOASysNumCoolingCoils(EnergyPlusData &state, int OASysNumber); // OA Sys Number

    int GetOASystemNumber(EnergyPlusData &state, std::string const &OASysName); // OA Sys Name

    int FindOAMixerMatchForOASystem(EnergyPlusData &state, int OASysNumber); // Which OA System

    int GetOAMixerIndex(EnergyPlusData &state, std::string const &OAMixerName); // Which Mixer

    int GetOAMixerInletNodeNumber(EnergyPlusData &state, int OAMixerNumber); // Which Mixer

    int GetOAMixerReturnNodeNumber(EnergyPlusData &state, int OAMixerNumber); // Which Mixer

    int GetOAMixerMixedNodeNumber(EnergyPlusData &state, int OAMixerNumber); // Which Mixer

    bool CheckForControllerWaterCoil(EnergyPlusData &state,
                                     DataAirLoop::ControllerKind ControllerType, // should be passed in as UPPERCASE
                                     std::string const &ControllerName           // should be passed in as UPPERCASE
    );

    void CheckControllerLists(EnergyPlusData &state, bool &ErrFound);

    void CheckOAControllerName(
        EnergyPlusData &state, std::string &OAControllerName, std::string const &ObjectType, std::string const &FieldName, bool &ErrorsFound);

    int GetNumOASystems(EnergyPlusData &state);

    int GetOACompListNumber(EnergyPlusData &state, int OASysNum); // OA Sys Number

    std::string GetOACompName(EnergyPlusData &state,
                              int OASysNum, // OA Sys Number
                              int InListNum // In-list Number
    );

    std::string GetOACompType(EnergyPlusData &state,
                              int OASysNum, // OA Sys Number
                              int InListNum // In-list Number
    );

    SimAirServingZones::CompType GetOACompTypeNum(EnergyPlusData &state,
                                                  int OASysNum, // OA Sys Number
                                                  int InListNum // In-list Number
    );

    int GetOAMixerNumber(EnergyPlusData &state, std::string const &OAMixerName); // must match OA mixer names for the OA mixer type

} // namespace MixedAir

struct MixedAirData : BaseGlobalStruct
{

    int NumControllerLists = 0;     // Number of Controller Lists
    int NumOAControllers = 0;       // Number of OA Controllers (includes ERV controllers)
    int NumERVControllers = 0;      // Number of ERV Controllers
    int NumOAMixers = 0;            // Number of Outdoor Air Mixers
    int NumVentMechControllers = 0; // Number of Controller:MechanicalVentilation objects in input deck
    Array1D_bool MyOneTimeErrorFlag;
    Array1D_bool MyOneTimeCheckUnitarySysFlag;
    Array1D_bool initOASysFlag;
    bool GetOASysInputFlag = true;
    bool GetOAMixerInputFlag = true;
    bool GetOAControllerInputFlag = true;
    bool InitOAControllerOneTimeFlag = true;
    Array1D_bool InitOAControllerSetPointCheckFlag;
    bool InitOAControllerSetUpAirLoopHVACVariables = true;
    bool AllocateOAControllersFlag = true;
    EPVector<MixedAir::ControllerListProps> ControllerLists;
    EPVector<MixedAir::OAControllerProps> OAController;
    EPVector<MixedAir::OAMixerProps> OAMixer;
    EPVector<MixedAir::VentilationMechanicalProps> VentilationMechanical;
    std::unordered_set<std::string> ControllerListUniqueNames;
    std::unordered_map<std::string, std::string> OAControllerUniqueNames;
    std::string CompType;
    std::string CompName;
    std::string CtrlName;
    Array1D_bool OAControllerMyOneTimeFlag;
    Array1D_bool OAControllerMyEnvrnFlag;
    Array1D_bool OAControllerMySizeFlag;
    Array1D_bool MechVentCheckFlag;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumControllerLists = 0;
        this->NumOAControllers = 0;
        this->NumERVControllers = 0;
        this->NumOAMixers = 0;
        this->NumVentMechControllers = 0;
        this->MyOneTimeErrorFlag.deallocate();
        this->MyOneTimeCheckUnitarySysFlag.deallocate();
        this->initOASysFlag.deallocate();
        this->GetOASysInputFlag = true;
        this->GetOAMixerInputFlag = true;
        this->GetOAControllerInputFlag = true;
        this->InitOAControllerOneTimeFlag = true;
        this->InitOAControllerSetPointCheckFlag.deallocate();
        this->InitOAControllerSetUpAirLoopHVACVariables = true;
        this->AllocateOAControllersFlag = true;
        this->ControllerLists.deallocate();
        this->OAController.deallocate();
        this->OAMixer.deallocate();
        this->VentilationMechanical.deallocate();
        this->ControllerListUniqueNames.clear();
        this->OAControllerUniqueNames.clear();
        this->OAControllerMyOneTimeFlag.clear();
        this->OAControllerMyEnvrnFlag.clear();
        this->OAControllerMySizeFlag.clear();
        this->MechVentCheckFlag.clear();
    }
};

} // namespace EnergyPlus

#endif
