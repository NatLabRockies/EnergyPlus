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

#ifndef EvaporativeCoolers_hh_INCLUDED
#define EvaporativeCoolers_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Curve {
    struct Curve;
}

namespace EvaporativeCoolers {

    // MODULE PARAMETER DEFINITIONS
    enum class WaterSupply
    {
        Invalid = -1,
        FromMains,
        FromTank,
        Num
    };

    enum class ControlType
    {
        Invalid = -1,
        ZoneTemperatureDeadBandOnOffCycling,
        ZoneCoolingLoadOnOffCycling,
        ZoneCoolingLoadVariableSpeedFan,
        Num
    };

    enum class OperatingMode
    {
        Invalid = -1,
        None,            // the indirect evaporative cooler Research Special is scheduled off or turned off
        DryModulated,    // the evaporative cooler Research Special is modulated in Dry Mode
        DryFull,         // the evaporative cooler Research Special is run in full capacity in Dry Mode
        DryWetModulated, // the evaporative cooler Research Special is modulated in Dry Mode or wet Mode
        WetModulated,    // the evaporative cooler Research Special is modulated in wet Mode
        WetFull,         // the evaporative cooler Research Special is run in full capacity in Wet Mode
        Num
    };

    enum class EvapCoolerType
    {
        Invalid = -1,
        DirectCELDEKPAD,
        IndirectCELDEKPAD,
        IndirectWETCOIL,
        IndirectRDDSpecial,
        DirectResearchSpecial,
        Num
    };

    struct EvapConditions
    {
        std::string Name; // Name of the EvapCooler
        int EquipIndex;
        EvapCoolerType evapCoolerType;         // Type of the EvapCooler
        std::string EvapControlType;           // Type of Control for the EvapCooler
        std::string Schedule;                  // HeatingCoil Operation Schedule
        Sched::Schedule *availSched = nullptr; // Pointer to the correct schedule
        Real64 VolFlowRate;                    // Volume Flow Rate in Evap Cooler needed for calculating SatEff
        Real64 DesVolFlowRate;                 // Design volume flow rate (autosize or user input) - this is only used to compute design pump power
        Real64 OutletTemp;
        Real64 OuletWetBulbTemp;
        Real64 OutletHumRat;
        Real64 OutletEnthalpy;
        Real64 OutletPressure;
        Real64 OutletMassFlowRate;         // MassFlow through the EvapCooler being Simulated [kg/Sec]
        Real64 OutletMassFlowRateMaxAvail; // [kg/Sec]
        Real64 OutletMassFlowRateMinAvail; // [kg/Sec]
        bool InitFlag;
        int InletNode;
        int OutletNode;
        int SecondaryInletNode;   // This is usually OA node feeding into the purge/secondary side
        int SecondaryOutletNode;  // This outlet node of the secondary side and ilet to the secondary fan
        int TertiaryInletNode;    // This node is used to run building exhaust into purge side.
        Real64 InletMassFlowRate; // Inlet is primary process air node at inlet to cooler
        Real64 InletMassFlowRateMaxAvail;
        Real64 InletMassFlowRateMinAvail;
        Real64 InletTemp;
        Real64 InletWetBulbTemp;
        Real64 InletHumRat;
        Real64 InletEnthalpy;
        Real64 InletPressure;
        Real64 SecInletMassFlowRate; // Secondary inlet is for indirect coolers
        Real64 SecInletMassFlowRateMaxAvail;
        Real64 SecInletMassFlowRateMinAvail;
        Real64 SecInletTemp;
        Real64 SecInletWetBulbTemp;
        Real64 SecInletHumRat;
        Real64 SecInletEnthalpy;
        Real64 SecInletPressure;
        Real64 SecOutletTemp;         // secondary air outlet node drybulb temperature
        Real64 SecOuletWetBulbTemp;   // secondarr air outlet node wetbulb temperature
        Real64 SecOutletHumRat;       // secondarr air outlet node humidity ratio
        Real64 SecOutletEnthalpy;     // secondarr air outlet node enthalpy
        Real64 SecOutletMassFlowRate; // Mass Flow through the secondary air side [kg/Sec]
        Real64 PadDepth;
        Real64 PadArea;
        Real64 RecircPumpPower;
        Real64 IndirectRecircPumpPower;
        Real64 IndirectPadDepth;
        Real64 IndirectPadArea;
        Real64 IndirectVolFlowRate;
        Real64 IndirectFanEff;
        Real64 IndirectFanDeltaPress;
        Real64 IndirectHXEffectiveness;
        Real64 DirectEffectiveness; // input saturation effectiveness for constant effectiveness model
        Real64 WetCoilMaxEfficiency;
        Real64 WetCoilFlowRatio;
        Real64 EvapCoolerEnergy;
        Real64 EvapCoolerPower;
        WaterSupply EvapWaterSupplyMode; // where does water come from
        std::string EvapWaterSupplyName; // name of water source e.g. water storage tank
        int EvapWaterSupTankID;
        int EvapWaterTankDemandARRID;
        Real64 DriftFraction;                           // excess water from drift as fraction of Evap Water Consumption rate
        Real64 BlowDownRatio;                           // excess water use for blowdown as solids ratio to be maintained
        Real64 EvapWaterConsumpRate;                    // Evap Water Consumption rate in m3/sec
        Real64 EvapWaterConsump;                        // Evap Water Consumption in m3
        Real64 EvapWaterStarvMakupRate;                 // Evap water consumed but not really available from tank m3/s
        Real64 EvapWaterStarvMakup;                     // Evap water consumed but not really available from tank m3
        Real64 SatEff;                                  // Reporting for Direct Stage and Ind Dry Saturation Efficiency
        Real64 StageEff;                                // Reporting for Indirect Total Stage Efficiency
        Real64 DPBoundFactor;                           // in RDDSpecial efficiency w.r.t. dewpoint
        int EvapControlNodeNum;                         // need to control to avoid over cooling
        Real64 DesiredOutletTemp;                       // setpoint manager should set this
        Real64 PartLoadFract;                           // reduces cooling performance and associated fan power
        int DewPointBoundFlag;                          // report when indirect research special cooler is bound by dewpoint
        Real64 MinOATDBEvapCooler;                      // Minimum outdoor air operating dry-bulb temperature for evaporative cooler
        Real64 MaxOATDBEvapCooler;                      // Maximum outdoor air operating dry-bulb temperature for evaporative cooler
        bool EvapCoolerOperationControlFlag;            // turns the evap cooler on/off depending on the outdoor air temperature min and max limits
        Real64 MaxOATWBEvapCooler;                      // Evaporative Operation Maximum Limit Outdoor Wetbulb Temperature
        Real64 DryCoilMaxEfficiency;                    // Cooler Drybulb Design Effectiveness
        Real64 IndirectFanPower;                        // Secondary Fan Design Power
        Real64 FanSizingSpecificPower;                  // secondary fan sizing specific power in W/(m3/s)
        Real64 RecircPumpSizingFactor;                  // water pump power sizing factor W/(m3/s) air
        Real64 IndirectVolFlowScalingFactor;            // secondary air flow sizing Factor
        Curve::Curve *WetbulbEffecCurve = nullptr;      // wetbulb effectiveness modifier curve name as a function of flow fraction
        Curve::Curve *DrybulbEffecCurve = nullptr;      // drybulb effectiveness modifier curve name as a function of flow fraction
        Curve::Curve *FanPowerModifierCurve = nullptr;  // secondary fan power modifier curve name as a function of flow fraction
        Curve::Curve *PumpPowerModifierCurve = nullptr; // recirculating pump power modifier curve name as a function of flow fraction
        int IECOperatingStatus; // operating mode status of indirect evaporative cooler research special (0: Off, 1: Dry, 2: Wet)
        int IterationLimit;     // used for Used for RegulaFalsi recurring error message error -1
        int IterationFailed;    // Used for RegulaFalsi recurring error message error -2
        // rather than wetbulb-depression approach
        OperatingMode EvapCoolerRDDOperatingMode; // the indirect evaporative cooler Research Special operating mode variable
        // Operational fault parameters
        bool FaultyEvapCoolerFoulingFlag;     // True if the evaporative cooler has fouling fault
        int FaultyEvapCoolerFoulingIndex;     // Index of the fault object corresponding to the evaporative cooler
        Real64 FaultyEvapCoolerFoulingFactor; // Evaporative cooler fouling factor
        bool MySizeFlag;

        // Default Constructor
        EvapConditions()
            : EquipIndex(0), evapCoolerType(EvapCoolerType::Invalid), VolFlowRate(0.0), DesVolFlowRate(0.0), OutletTemp(0.0), OuletWetBulbTemp(0.0),
              OutletHumRat(0.0), OutletEnthalpy(0.0), OutletPressure(0.0), OutletMassFlowRate(0.0), OutletMassFlowRateMaxAvail(0.0),
              OutletMassFlowRateMinAvail(0.0), InitFlag(false), InletNode(0), OutletNode(0), SecondaryInletNode(0), SecondaryOutletNode(0),
              TertiaryInletNode(0), InletMassFlowRate(0.0), InletMassFlowRateMaxAvail(0.0), InletMassFlowRateMinAvail(0.0), InletTemp(0.0),
              InletWetBulbTemp(0.0), InletHumRat(0.0), InletEnthalpy(0.0), InletPressure(0.0), SecInletMassFlowRate(0.0),
              SecInletMassFlowRateMaxAvail(0.0), SecInletMassFlowRateMinAvail(0.0), SecInletTemp(0.0), SecInletWetBulbTemp(0.0), SecInletHumRat(0.0),
              SecInletEnthalpy(0.0), SecInletPressure(0.0), SecOutletTemp(0.0), SecOuletWetBulbTemp(0.0), SecOutletHumRat(0.0),
              SecOutletEnthalpy(0.0), SecOutletMassFlowRate(0.0), PadDepth(0.0), PadArea(0.0), RecircPumpPower(0.0), IndirectRecircPumpPower(0.0),
              IndirectPadDepth(0.0), IndirectPadArea(0.0), IndirectVolFlowRate(0.0), IndirectFanEff(0.0), IndirectFanDeltaPress(0.0),
              IndirectHXEffectiveness(0.0), DirectEffectiveness(0.0), WetCoilMaxEfficiency(0.0), WetCoilFlowRatio(0.0), EvapCoolerEnergy(0.0),
              EvapCoolerPower(0.0), EvapWaterSupplyMode(WaterSupply::Invalid), EvapWaterSupTankID(0), EvapWaterTankDemandARRID(0), DriftFraction(0.0),
              BlowDownRatio(0.0), EvapWaterConsumpRate(0.0), EvapWaterConsump(0.0), EvapWaterStarvMakupRate(0.0), EvapWaterStarvMakup(0.0),
              SatEff(0.0), StageEff(0.0), DPBoundFactor(0.0), EvapControlNodeNum(0), DesiredOutletTemp(0.0), PartLoadFract(0.0), DewPointBoundFlag(0),
              MinOATDBEvapCooler(0.0), MaxOATDBEvapCooler(0.0), EvapCoolerOperationControlFlag(false), MaxOATWBEvapCooler(0.0),
              DryCoilMaxEfficiency(0.0), IndirectFanPower(0.0), FanSizingSpecificPower(0.0), RecircPumpSizingFactor(0.0),
              IndirectVolFlowScalingFactor(0.0), IECOperatingStatus(0), IterationLimit(0), IterationFailed(0),
              EvapCoolerRDDOperatingMode(OperatingMode::Invalid), FaultyEvapCoolerFoulingFlag(false), FaultyEvapCoolerFoulingIndex(0),
              FaultyEvapCoolerFoulingFactor(1.0), MySizeFlag(true)
        {
        }
    };

    struct ZoneEvapCoolerUnitStruct
    {
        std::string Name; // user identifier
        int ZoneNodeNum;
        Sched::Schedule *availSched = nullptr; // local availability schedule
        std::string AvailManagerListName;      // Name of an availability manager list object
        bool UnitIsAvailable;
        Avail::Status FanAvailStatus = Avail::Status::NoAction;
        int OAInletNodeNum;    // outdoor air inlet node index
        int UnitOutletNodeNum; // Unit air outlet (to zone) node index
        int UnitReliefNodeNum; // Unit relief air (from zone) node index (optional)
        HVAC::FanType fanType;
        std::string FanName;
        int FanIndex;
        Real64 ActualFanVolFlowRate;
        Sched::Schedule *fanAvailSched = nullptr;
        int FanInletNodeNum;
        int FanOutletNodeNum;
        HVAC::FanOp fanOp = HVAC::FanOp::Invalid;
        Real64 DesignAirVolumeFlowRate;
        Real64 DesignAirMassFlowRate;
        Real64 DesignFanSpeedRatio;
        Real64 FanSpeedRatio;
        HVAC::FanPlace fanPlace;
        ControlType ControlSchemeType;
        Real64 TimeElapsed;
        Real64 ThrottlingRange; // temperature range for hystersis type tstat control [Delta C]
        bool IsOnThisTimestep;
        bool WasOnLastTimestep;
        Real64 ThresholdCoolingLoad;
        std::string EvapCooler_1_ObjectClassName;
        std::string EvapCooler_1_Name;
        EvapCoolerType EvapCooler_1_Type_Num;
        int EvapCooler_1_Index;
        bool EvapCooler_1_AvailStatus;
        std::string EvapCooler_2_ObjectClassName;
        std::string EvapCooler_2_Name;
        EvapCoolerType EvapCooler_2_Type_Num;
        int EvapCooler_2_Index;
        bool EvapCooler_2_AvailStatus;
        Real64 OAInletRho;                    // fills internal variable, current inlet air density [kg/m3]
        Real64 OAInletCp;                     // fills internal variable, current inlet air specific heat [J/kg-c]
        Real64 OAInletTemp;                   // fills internal variable, current inlet air temperature [C]
        Real64 OAInletHumRat;                 // fills internal variable, current inlet air humidity ratio [kg/kg]
        Real64 OAInletMassFlowRate;           // fills internal variable, current inlet air mass flow rate [kg/s]
        Real64 UnitOutletTemp;                // filled by actuator, component outlet temperature [C]
        Real64 UnitOutletHumRat;              // filled by actuator, component outlet humidity ratio [kg/kg]
        Real64 UnitOutletMassFlowRate;        // filled by actuator, component outlet mass flow rate [kg/s]
        Real64 UnitReliefTemp;                // filled by actuator, component outlet temperature [C]
        Real64 UnitReliefHumRat;              // filled by actuator, component outlet humidity ratio [kg/kg]
        Real64 UnitReliefMassFlowRate;        // filled by actuator, component outlet mass flow rate [kg/s]
        Real64 UnitTotalCoolingRate;          // unit output to zone, total cooling rate [W]
        Real64 UnitTotalCoolingEnergy;        // unit output to zone, total cooling energy [J]
        Real64 UnitSensibleCoolingRate;       // unit output to zone, sensible cooling rate [W]
        Real64 UnitSensibleCoolingEnergy;     // unit output to zone, sensible cooling energy [J]
        Real64 UnitLatentHeatingRate;         // unit output to zone, latent heating rate [W]
        Real64 UnitLatentHeatingEnergy;       // unit output to zone, latent heating energy [J]
        Real64 UnitLatentCoolingRate;         // unit output to zone, latent cooling rate [W]
        Real64 UnitLatentCoolingEnergy;       // unit output to zone, latent cooling energy [J]
        Real64 UnitFanSpeedRatio;             // unit fan speed ratio, dimensionless [ ]
        Real64 UnitPartLoadRatio;             // unit part load ratio, dimensionless [ ]
        int UnitVSControlMaxIterErrorIndex;   // regula falsi errors, fan speed iteration limits
        int UnitVSControlLimitsErrorIndex;    // regula falsi errors, limits exceeded.
        int UnitLoadControlMaxIterErrorIndex; // root solver errors, part load ratio iteration limits exceeded
        int UnitLoadControlLimitsErrorIndex;  // root solver errors, art load ratio limits exceeded.
        int ZonePtr;                          // pointer to a zone served by an evaportive cooler unit
        int HVACSizingIndex;                  // index of a HVACSizing object for an evaportive cooler unit
        Real64 ShutOffRelativeHumidity;       // Zone relative humidity above which the evap cooler is shut off.
        bool MySize;                          // sizing logic flag
        bool MyEnvrn;                         // sim environmental logic flag
        bool MyFan;                           // fan sizing logic flag
        bool MyZoneEq;                        // logic flag used to set up zone equipment availability managers

        // Default Constructor
        ZoneEvapCoolerUnitStruct()
            : ZoneNodeNum(0), UnitIsAvailable(false), OAInletNodeNum(0), UnitOutletNodeNum(0), UnitReliefNodeNum(0), fanType(HVAC::FanType::Invalid),
              FanIndex(0), ActualFanVolFlowRate(0.0), FanInletNodeNum(0), FanOutletNodeNum(0), DesignAirVolumeFlowRate(0.0),
              DesignAirMassFlowRate(0.0), DesignFanSpeedRatio(0.0), FanSpeedRatio(0.0), fanPlace(HVAC::FanPlace::Invalid),
              ControlSchemeType(ControlType::Invalid), TimeElapsed(0.0), ThrottlingRange(0.0), IsOnThisTimestep(false), WasOnLastTimestep(false),
              ThresholdCoolingLoad(0.0), EvapCooler_1_Type_Num(EvapCoolerType::Invalid), EvapCooler_1_Index(0), EvapCooler_1_AvailStatus(false),
              EvapCooler_2_Type_Num(EvapCoolerType::Invalid), EvapCooler_2_Index(0), EvapCooler_2_AvailStatus(false), OAInletRho(0.0), OAInletCp(0.0),
              OAInletTemp(0.0), OAInletHumRat(0.0), OAInletMassFlowRate(0.0), UnitOutletTemp(0.0), UnitOutletHumRat(0.0), UnitOutletMassFlowRate(0.0),
              UnitReliefTemp(0.0), UnitReliefHumRat(0.0), UnitReliefMassFlowRate(0.0), UnitTotalCoolingRate(0.0), UnitTotalCoolingEnergy(0.0),
              UnitSensibleCoolingRate(0.0), UnitSensibleCoolingEnergy(0.0), UnitLatentHeatingRate(0.0), UnitLatentHeatingEnergy(0.0),
              UnitLatentCoolingRate(0.0), UnitLatentCoolingEnergy(0.0), UnitFanSpeedRatio(0.0), UnitPartLoadRatio(0.0),
              UnitVSControlMaxIterErrorIndex(0), UnitVSControlLimitsErrorIndex(0), UnitLoadControlMaxIterErrorIndex(0),
              UnitLoadControlLimitsErrorIndex(0), ZonePtr(0), HVACSizingIndex(0), ShutOffRelativeHumidity(100.0), MySize(true), MyEnvrn(true),
              MyFan(true), MyZoneEq(true)
        {
        }
    };

    void SimEvapCooler(EnergyPlusData &state, std::string_view CompName, int &CompIndex, Real64 PartLoadRatio = 1.0);

    void GetEvapInput(EnergyPlusData &state);

    void InitEvapCooler(EnergyPlusData &state, int EvapCoolNum);

    void SizeEvapCooler(EnergyPlusData &state, int EvapCoolNum);

    void CalcDirectEvapCooler(EnergyPlusData &state, int EvapCoolNum, Real64 PartLoadRatio);

    void CalcDryIndirectEvapCooler(EnergyPlusData &state, int EvapCoolNum, Real64 PartLoadRatio);

    void CalcWetIndirectEvapCooler(EnergyPlusData &state, int EvapCoolNum, Real64 PartLoadRatio);

    void CalcResearchSpecialPartLoad(EnergyPlusData &state, int EvapCoolNum);

    void CalcIndirectResearchSpecialEvapCoolerAdvanced(EnergyPlusData &state,
                                                       int EvapCoolNum,
                                                       Real64 InletDryBulbTempSec,
                                                       Real64 InletWetBulbTempSec,
                                                       Real64 InletDewPointTempSec,
                                                       Real64 InletHumRatioSec);

    OperatingMode IndirectResearchSpecialEvapCoolerOperatingMode(EnergyPlusData &state,
                                                                 int EvapCoolNum,
                                                                 Real64 InletDryBulbTempSec,
                                                                 Real64 InletWetBulbTempSec,
                                                                 Real64 TdbOutSysWetMin,
                                                                 Real64 TdbOutSysDryMin);

    void CalcSecondaryAirOutletCondition(EnergyPlusData &state,
                                         int EvapCoolNum,
                                         OperatingMode OperatingMode,
                                         Real64 AirMassFlowSec,
                                         Real64 EDBTSec,
                                         Real64 EWBTSec,
                                         Real64 EHumRatSec,
                                         Real64 QHXTotal,
                                         Real64 &QHXLatent);

    void CalcIndirectRDDEvapCoolerOutletTemp(EnergyPlusData &state,
                                             int EvapCoolNum,
                                             OperatingMode DryOrWetOperatingMode,
                                             Real64 AirMassFlowSec,
                                             Real64 EDBTSec,
                                             Real64 EWBTSec,
                                             Real64 EHumRatSec);

    Real64 IndEvapCoolerPower(EnergyPlusData &state,
                              int EvapCoolIndex,        // Unit index
                              OperatingMode DryWetMode, // dry or wet operating mode of evaporator cooler
                              Real64 FlowRatio          // secondary air flow fraction
    );

    void CalcIndirectResearchSpecialEvapCooler(EnergyPlusData &state, int EvapCoolNum, Real64 FanPLR = 1.0);

    void CalcDirectResearchSpecialEvapCooler(EnergyPlusData &state, int EvapCoolNum, Real64 FanPLR = 1.0);

    void UpdateEvapCooler(EnergyPlusData &state, int EvapCoolNum);

    void ReportEvapCooler(EnergyPlusData &state, int EvapCoolNum);

    void SimZoneEvaporativeCoolerUnit(EnergyPlusData &state,
                                      std::string_view CompName,      // name of the packaged terminal heat pump
                                      int ZoneNum,                    // number of zone being served
                                      Real64 &SensibleOutputProvided, // sensible capacity delivered to zone
                                      Real64 &LatentOutputProvided,   // Latent add/removal  (kg/s), dehumid = negative
                                      int &CompIndex                  // index to zone hvac unit
    );

    void GetInputZoneEvaporativeCoolerUnit(EnergyPlusData &state);

    void InitZoneEvaporativeCoolerUnit(EnergyPlusData &state,
                                       int UnitNum, // unit number
                                       int ZoneNum  // number of zone being served
    );

    void SizeZoneEvaporativeCoolerUnit(EnergyPlusData &state, int UnitNum); // unit number

    void CalcZoneEvaporativeCoolerUnit(EnergyPlusData &state,
                                       int UnitNum,                    // unit number
                                       int ZoneNum,                    // number of zone being served
                                       Real64 &SensibleOutputProvided, // sensible capacity delivered to zone
                                       Real64 &LatentOutputProvided    // Latent add/removal  (kg/s), dehumid = negative
    );

    void CalcZoneEvapUnitOutput(EnergyPlusData &state,
                                int UnitNum,                    // unit number
                                Real64 PartLoadRatio,           // zone evap unit part load ratiod
                                Real64 &SensibleOutputProvided, // sensible capacity delivered to zone
                                Real64 &LatentOutputProvided    // Latent add/removal  (kg/s), dehumid = negative
    );

    void ControlZoneEvapUnitOutput(EnergyPlusData &state,
                                   int UnitNum,           // unit number
                                   Real64 ZoneCoolingLoad // target cooling load
    );

    void ControlVSEvapUnitToMeetLoad(EnergyPlusData &state,
                                     int UnitNum,           // unit number
                                     Real64 ZoneCoolingLoad // target cooling load
    );

    void ReportZoneEvaporativeCoolerUnit(EnergyPlusData &state, int UnitNum); // unit number

    int GetInletNodeNum(EnergyPlusData &state, std::string const &EvapCondName, bool &ErrorsFound);

    int GetOutletNodeNum(EnergyPlusData &state, std::string const &EvapCondName, bool &ErrorsFound);

} // namespace EvaporativeCoolers

struct EvaporativeCoolersData : BaseGlobalStruct
{

    bool GetInputEvapComponentsFlag = true; // Flag set to make sure you get input once
    int NumEvapCool = 0;                    // The Number of Evap Coolers found in the Input
    Array1D_bool CheckEquipName;
    int NumZoneEvapUnits = 0;
    Array1D_bool CheckZoneEvapUnitName;
    bool GetInputZoneEvapUnit = true;
    Array1D<EvaporativeCoolers::EvapConditions> EvapCond;
    Array1D<EvaporativeCoolers::ZoneEvapCoolerUnitStruct> ZoneEvapUnit;
    std::unordered_map<std::string, std::string> UniqueEvapCondNames;
    bool MySetPointCheckFlag = true;
    bool ZoneEquipmentListChecked = false;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) EvaporativeCoolersData();
    }
};

} // namespace EnergyPlus

#endif
