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

#ifndef PackagedThermalStorageCoil_hh_INCLUDED
#define PackagedThermalStorageCoil_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/HVACUnitaryBypassVAV.hh>

namespace EnergyPlus {

namespace PackagedThermalStorageCoil {

    // control types
    enum class PTSCCtrlType
    {
        Invalid = -1,
        ScheduledOpModes,   // control over TES modes is via local schedule
        EMSActuatedOpModes, // control over TES modes is via EMS
        Num
    };

    // Control Modes
    enum class PTSCOperatingMode
    {
        Invalid = -1,
        Off,
        CoolingOnly,
        CoolingAndCharge,
        CoolingAndDischarge,
        ChargeOnly,
        DischargeOnly,
        Num
    };

    // storage media
    enum class MediaType
    {
        Invalid = -1,
        Water,
        UserDefindFluid,
        Ice,
        Num
    };

    // Water Systems
    enum class CondensateAction
    {
        Invalid = -1,
        Discard, // default mode where water is "lost"
        ToTank,  // collect coil condensate from air and store in water storage tank
        Num
    };

    enum class EvapWaterSupply
    {
        Invalid = -1,
        WaterSupplyFromMains,
        WaterSupplyFromTank,
        Num
    };

    enum class TESCondenserType
    {
        Invalid = -1,
        Air,
        Evap,
        Num
    };

    // Dehumidification control modes (DehumidControlMode)
    constexpr int DehumidControl_CoolReheat(2);

    struct PackagedTESCoolingCoilStruct
    {
        // Members
        std::string Name;                            // Name of TES cooling package
        Sched::Schedule *availSched = nullptr;       // availability schedule
        PTSCCtrlType ModeControlType;                // how are operation modes controlled
        Sched::Schedule *controlModeSched = nullptr; // control schedule if used
        bool EMSControlModeOn;                       // if true, then EMS actuator has been used
        Real64 EMSControlModeValue;                  // value to use from EMS actuator for control mode
        PTSCOperatingMode CurControlMode = PTSCOperatingMode::Off;
        int curControlModeReport = static_cast<int>(PTSCOperatingMode::Off);
        int ControlModeErrorIndex;
        Real64 RatedEvapAirVolFlowRate;  // [m3/s]
        Real64 RatedEvapAirMassFlowRate; // [kg/s]
        int EvapAirInletNodeNum;         // evaporator inlet node pointer
        int EvapAirOutletNodeNum;        // evaporator outlet node pointer
        // Cooling Only Mode
        bool CoolingOnlyModeIsAvailable;
        Real64 CoolingOnlyRatedTotCap; // gross total cooling capacity at rating conditions [W]
        Real64 CoolingOnlyRatedSHR;    // Sensible heat ratio (sens cap/total cap) at rating conditions [W/W]
        Real64 CoolingOnlyRatedCOP;    // Coefficient of performance at rating conditions [W/W]
        int CoolingOnlyCapFTempCurve;  // curve index for total cooling capacity modifier curve
        // (function of entering wetbulb, outside drybulb)
        int CoolingOnlyCapFTempObjectNum; // type of object used for curve input
        int CoolingOnlyCapFFlowCurve;     // curve index for total cooling capacity modifier curve
        // (function of actual supply air flow vs rated air flow)
        int CoolingOnlyCapFFlowObjectNum; // type of object used for curve input
        int CoolingOnlyEIRFTempCurve;     // curve index for energy input ratio modifier curve
        // (function of entering wetbulb, outside drybulb)
        int CoolingOnlyEIRFTempObjectNum; // type of object used for curve input
        int CoolingOnlyEIRFFlowCurve;     // curve index for energy input ratio modifier curve
        // (function of actual supply air flow vs rated air flow)
        int CoolingOnlyEIRFFlowObjectNum; // type of object used for curve input
        int CoolingOnlyPLFFPLRCurve;      // curve index for part-load fact vs part load ratio,EIR modifier
        int CoolingOnlyPLFFPLRObjectNum;  // type of object used for curve input
        int CoolingOnlySHRFTempCurve;     // curve index for sensible heat ratio modifier curve
        // (function of entering wetbulb and drybulb)
        int CoolingOnlySHRFTempObjectNum; // type of object used for curve input
        int CoolingOnlySHRFFlowCurve;     // curve index for sensible heat ratio modifier curve
        // (function of actual supply air flow vs rated air flow)
        int CoolingOnlySHRFFlowObjectNum;
        // cooling and charge mode
        bool CoolingAndChargeModeAvailable;
        Real64 CoolingAndChargeRatedTotCap;                // gross total evaporator cooling capacity at rating conditions [W]
        Real64 CoolingAndChargeRatedTotCapSizingFactor;    // sizing factor for gross total evaporator [ ]
        Real64 CoolingAndChargeRatedChargeCap;             // net storage charging capacity at rating conditions [W]
        Real64 CoolingAndChargeRatedChargeCapSizingFactor; // sizing factor for charging capacity [ ]
        Real64 CoolingAndChargeRatedSHR;                   // Sensible heat ratio (sens cap/total cap) at rating conditions [W/W]
        Real64 CoolingAndChargeCoolingRatedCOP;            // Coefficient of performance at rating conditions, for cooling [W/W]
        Real64 CoolingAndChargeChargingRatedCOP;           // Coefficient of performance at rating conditions, for charging [W/W]
        int CoolingAndChargeCoolingCapFTempCurve;          // curve index for total cooling capacity modifier curve
        // (function of entering wetbulb, outside drybulb, state of TES)
        int CoolingAndChargeCoolingCapFTempObjectNum;
        int CoolingAndChargeCoolingCapFFlowCurve; // curve index for total cooling capacity modifier curve
        // (function of actual supply air flow vs rated air flow)
        int CoolingAndChargeCoolingCapFFlowObjectNum;
        int CoolingAndChargeCoolingEIRFTempCurve; // curve index for cooling energy input ratio modifier curve
        // (function of entering wetbulb, outside drybulb, state of TES)
        int CoolingAndChargeCoolingEIRFTempObjectNum;
        int CoolingAndChargeCoolingEIRFFlowCurve; // curve index for cooling energy input ratio modifier curve
        // (function of actual supply air flow vs rated air flow)
        int CoolingAndChargeCoolingEIRFFlowObjectNum;
        int CoolingAndChargeCoolingPLFFPLRCurve; // curve index for cooling part-load fact vs part load ratio, EIR modifier
        // (function of evaporator part load)
        int CoolingAndChargeCoolingPLFFPLRObjectNum;
        int CoolingAndChargeChargingCapFTempCurve; // curve index for charging capacity modifier curve
        // (function of entering wetbulb, outside drybulb, state of TES)
        int CoolingAndChargeChargingCapFTempObjectNum;
        int CoolingAndChargeChargingCapFEvapPLRCurve; // curve index for charging capacity modifier curve
        // function of evaporator part load ratio
        int CoolingAndChargeChargingCapFEvapPLRObjectNum;
        int CoolingAndChargeChargingEIRFTempCurve; // curve index for charging energy input ratio modifier curve
        // (function of entering wetbulb, outside drybulb, state of TES)
        int CoolingAndChargeChargingEIRFTempObjectNum;
        int CoolingAndChargeChargingEIRFFLowCurve; // curve index for charging energy input ratio modifier curve
        // (function of actual supply air flow vs rated air flow)
        int CoolingAndChargeChargingEIRFFLowObjectNum;
        int CoolingAndChargeChargingPLFFPLRCurve; // curve index for chargine part-load fact vs part load ratio, EIR modif
        // (function of evaporator part load)
        int CoolingAndChargeChargingPLFFPLRObjectNum;
        int CoolingAndChargeSHRFTempCurve; // curve index for sensible heat ratio modifier curve
        // (function of entering wetbulb and drybulb)
        int CoolingAndChargeSHRFFlowCurve; // curve index for sensible heat ratio modifier curve
        // (function of actual supply air flow vs rated air flow)
        int CoolingAndChargeSHRFFlowObjectNum;
        // cooling and discharge mode
        bool CoolingAndDischargeModeAvailable;
        Real64 CoolingAndDischargeRatedTotCap;                   // gross total evaporator cooling capacity at rating conditions [W]
        Real64 CoolingAndDischargeRatedTotCapSizingFactor;       // sizing factor gross total cooling capacity []
        Real64 CoolingAndDischargeRatedDischargeCap;             // net storage discharging capacity at rating conditions [W]
        Real64 CoolingAndDischargeRatedDischargeCapSizingFactor; // sizing factor discharging capacity []
        Real64 CoolingAndDischargeRatedSHR;                      // Sensible heat ratio (sens cap/total cap) at rating conditions [W/W]
        Real64 CoolingAndDischargeCoolingRatedCOP;               // Coefficient of performance at rating conditions, for cooling [W/W]
        Real64 CoolingAndDischargeDischargingRatedCOP;           // Coefficient of performance at rating conditions, for charging [W/W]
        int CoolingAndDischargeCoolingCapFTempCurve;             // curve index for total cooling capacity modifier curve
        // (function of entering wetbulb, outside drybulb, state of TES)
        int CoolingAndDischargeCoolingCapFTempObjectNum;
        int CoolingAndDischargeCoolingCapFFlowCurve; // curve index for total cooling capacity modifier curve
        // (function of actual supply air flow vs rated air flow)
        int CoolingAndDischargeCoolingCapFFlowObjectNum;
        int CoolingAndDischargeCoolingEIRFTempCurve; // curve index for cooling energy input ratio modifier curve
        // (function of entering wetbulb, outside drybulb, state of TES)
        int CoolingAndDischargeCoolingEIRFTempObjectNum;
        int CoolingAndDischargeCoolingEIRFFlowCurve; // curve index for cooling energy input ratio modifier curve
        // (function of actual supply air flow vs rated air flow)
        int CoolingAndDischargeCoolingEIRFFlowObjectNum;
        int CoolingAndDischargeCoolingPLFFPLRCurve; // curve index for cooling part-load fact vs part load ratio,
        // EIR modifier (function of evaporator part load)
        int CoolingAndDischargeCoolingPLFFPLRObjectNum;
        int CoolingAndDischargeDischargingCapFTempCurve; // curve index for discharging capacity modifier curve
        // (function of entering wetbulb, outside drybulb, state of TES)
        int CoolingAndDischargeDischargingCapFTempObjectNum;
        int CoolingAndDischargeDischargingCapFFlowCurve; // curve index for discharging capacity modifier curve
        // (function of actual supply air flow vs rated air flow)
        int CoolingAndDischargeDischargingCapFFlowObjectNum;
        int CoolingAndDischargeDischargingCapFEvapPLRCurve; // curve index for discharging capacity modifier curve
        // function of evaporator part load ratio
        int CoolingAndDischargeDischargingCapFEvapPLRObjectNum;
        int CoolingAndDischargeDischargingEIRFTempCurve; // curve index for discharging energy input ratio modifier curve
        // (function of entering wetbulb, outside drybulb, state of TES)
        int CoolingAndDischargeDischargingEIRFTempObjectNum;
        int CoolingAndDischargeDischargingEIRFFLowCurve; // curve index for discharging energy input ratio modifier curve
        // (function of actual supply air flow vs rated air flow)
        int CoolingAndDischargeDischargingEIRFFLowObjectNum;
        int CoolingAndDischargeDischargingPLFFPLRCurve; // curve index for discharging part-load fact vs part load ratio
        //  EIR modifier (function of evaporator part load)
        int CoolingAndDischargeDischargingPLFFPLRObjectNum;
        int CoolingAndDischargeSHRFTempCurve; // curve index for sensible heat ratio modifier curve
        // (function of entering wetbulb and drybulb)
        int CoolingAndDischargeSHRFTempObjectNum;
        int CoolingAndDischargeSHRFFlowCurve; // curve index for sensible heat ratio modifier curve
        // (function of actual supply air flow vs rated air flow)
        int CoolingAndDischargeSHRFFlowObjectNum;
        // Charge Only Mode
        bool ChargeOnlyModeAvailable;
        Real64 ChargeOnlyRatedCapacity;             // net storage charging capacity at rating conditions [W]
        Real64 ChargeOnlyRatedCapacitySizingFactor; // sizing factor for charging capacity []
        Real64 ChargeOnlyRatedCOP;                  // coefficient of performance at rating conditions [W/W]
        int ChargeOnlyChargingCapFTempCurve;        // curve index for charging capacity modifier curve
        // function of outside drybulb and state of TES
        int ChargeOnlyChargingCapFTempObjectNum;
        int ChargeOnlyChargingEIRFTempCurve; // curve index for charging energy input ratio modifier curve
        // function of outside drybulb and state of TES
        int ChargeOnlyChargingEIRFTempObjectNum;
        // Discharge Only mode
        bool DischargeOnlyModeAvailable;
        Real64 DischargeOnlyRatedDischargeCap;             // gross total evaporator cooling capacity at rating conditions [W]
        Real64 DischargeOnlyRatedDischargeCapSizingFactor; // sizing factor for cooling capacity []
        Real64 DischargeOnlyRatedSHR;                      // sensible heat ratio (sens cap/total cap) at rating conditions
        Real64 DischargeOnlyRatedCOP;                      // coefficient of performance at rating conditions for discharging [W/W]
        int DischargeOnlyCapFTempCurve;                    // curve index for total cooling capacity modifier curve
        // function of entering wetbulb and state of TES
        int DischargeOnlyCapFTempObjectNum;
        int DischargeOnlyCapFFlowCurve; // curve index for tot cooling capacity modifier curve
        // (function of actual supply air flow vs rated air flow)
        int DischargeOnlyCapFFlowObjectNum;
        int DischargeOnlyEIRFTempCurve; // curve index for energy input ratio modifier curve
        // function of entering wetbulb and state of TES
        int DischargeOnlyEIRFTempObjectNum;
        int DischargeOnlyEIRFFlowCurve; // curve index for energy input ratio modifier curve
        // (function of actual supply air flow vs rated air flow)
        int DischargeOnlyEIRFFlowObjectNum;
        int DischargeOnlyPLFFPLRCurve; // curve index for part-load fact vs evaporator part load ratio
        int DischargeOnlyPLFFPLRObjectNum;
        int DischargeOnlySHRFTempCurve; // curve index for sensible heat ratio modifier curve
        // (function of entering wetbulb and drybulb)
        int DischargeOnlySHRFTempObjectNum;
        int DischargeOnlySHRFFLowCurve; // curve index for
        int DischargeOnlySHRFFLowObjectNum;
        // other inputs
        Real64 AncillaryControlsPower;                          // standby and controls electric power, draws when available [W]
        Real64 ColdWeatherMinimumTempLimit;                     // temperature limit for cold weather operation mode [C]
        Real64 ColdWeatherAncillaryPower;                       // electrical power draw during cold weather [W]
        int CondAirInletNodeNum;                                // Condenser air inlet node num pointer
        int CondAirOutletNodeNum;                               // condenser air outlet node num pointer
        TESCondenserType CondenserType = TESCondenserType::Air; // Type of condenser for DX cooling coil: AIR COOLED or EVAP COOLED
        Real64 CondenserAirVolumeFlow;                          // design air flow rate thru condenser [m3/s]
        Real64 CondenserAirFlowSizingFactor;                    // scale condenser air flow relative to evap air flow when autosizing
        Real64 CondenserAirMassFlow;                            // design air flow rate thru condenser [kg/s]
        Real64 EvapCondEffect;                                  // effectiveness of the evaporatively cooled condenser
        Real64 CondInletTemp;                                   // air temperature drybulb entering condenser section after evap cooling [C]
        Real64 EvapCondPumpElecNomPower;                        // Nominal power input to the evap condenser water circulation pump [W]
        Real64 EvapCondPumpElecEnergy;                          // Electric energy used by condenser water circulation pump [J]
        Real64 BasinHeaterPowerFTempDiff;                       // Basin heater power for evaporatively cooled condensers [W/K]
        Sched::Schedule *basinHeaterAvailSched = nullptr;       // basin heater availability schedule pointer num
        Real64 BasinHeaterSetpointTemp;                         // evap water basin temperature setpoint [C]
        EvapWaterSupply EvapWaterSupplyMode;                    // where does evap water come from
        std::string EvapWaterSupplyName;                        // name of water source e.g. water storage tank
        int EvapWaterSupTankID;                                 // supply tank index, if any
        int EvapWaterTankDemandARRID;                           // evap water demand array index
        CondensateAction CondensateCollectMode;                 // where does condensate  water go to
        std::string CondensateCollectName;                      // name of water source e.g. water storage tank
        int CondensateTankID;
        int CondensateTankSupplyARRID;
        // TES tank
        MediaType StorageMedia;       // water/fluid or ice based TES
        std::string StorageFluidName; // if user defined, name of fluid type
        Fluid::GlycolProps *glycol = nullptr;

        Real64 FluidStorageVolume;          // volume of water in storage tank for water systems [m3/s]
        Real64 IceStorageCapacity;          // capacity of storage in J
        Real64 StorageCapacitySizingFactor; // storage time used to autocalculate capacity [hr]
        Real64 MinimumFluidTankTempLimit;   // optional inputs [C]
        Real64 MaximumFluidTankTempLimit;   // optional inputs [C]
        Real64 RatedFluidTankTemp;          // rating point condition for fluid storage tanks [C]
        int StorageAmbientNodeNum;          // node "pointer" for ambient conditions exposed to TES
        Real64 StorageUA;                   // overall heat transfer coefficient for TES to ambient [W/k]
        bool TESPlantConnectionAvailable;
        int TESPlantInletNodeNum;                        // plant loop inlet node index
        int TESPlantOutletNodeNum;                       // plant loop outlet node index
        int TESPlantLoopNum;                             // plant loop connection index
        DataPlant::LoopSideLocation TESPlantLoopSideNum; // plant loop side connection index
        int TESPlantBranchNum;                           // plant loop branch connection index
        int TESPlantCompNum;                             // plant loop component connection index
        Real64 TESPlantDesignVolumeFlowRate;             // plant connection design mass flow rate [m3/s]
        Real64 TESPlantDesignMassFlowRate;               // [kg/s]
        Real64 TESPlantEffectiveness;
        Real64 TimeElapsed;
        Real64 IceFracRemain;             // state of storage for current time step [0..1.0]
        Real64 IceFracRemainLastTimestep; // state of storage for previous time step [0..1.0]
        Real64 FluidTankTempFinal;
        Real64 FluidTankTempFinalLastTimestep;
        // dynamic calculated data
        Real64 QdotPlant;            // heat exchange rate for plant connection to TES tank [W]
        Real64 Q_Plant;              // heat exchange energy for plant connection to TES tank [J]
        Real64 QdotAmbient;          // heat exchange rate for skin losses/gains for TES tank to surroundings [W]
        Real64 Q_Ambient;            // heat exchange enegy for skin losses/gains for TES tank to surroundings [J]
        Real64 QdotTES;              // heat exchange rate by mechanical systems to charge or discharge TES [W]
        Real64 Q_TES;                // heat exchange energy by mechanical systems to charge or discharge TES [J]
        Real64 ElecCoolingPower;     // electric power for cooling [W]
        Real64 ElecCoolingEnergy;    // electric energy for cooling [J], metered
        Real64 EvapTotCoolingRate;   // evaporator coil total cooling rate [W]
        Real64 EvapTotCoolingEnergy; // evaporatory coil total cooling energy [J], metered
        Real64 EvapSensCoolingRate;
        Real64 EvapSensCoolingEnergy;
        Real64 EvapLatCoolingRate;
        Real64 EvapLatCoolingEnergy;
        Real64 RuntimeFraction;
        Real64 CondenserRuntimeFraction;
        Real64 ElectColdWeatherPower;  // electric power for cold weather protection [W]
        Real64 ElectColdWeatherEnergy; // electric energy for cold weather protection [J], metered
        Real64 ElectEvapCondBasinHeaterPower;
        Real64 ElectEvapCondBasinHeaterEnergy;
        Real64 EvapWaterConsumpRate;    // Evap Water Consumption rate in m3/sec
        Real64 EvapWaterConsump;        // Evap Water Consumption in m3
        Real64 EvapWaterStarvMakupRate; // Evap water consumed but not really available from tank m3/s
        Real64 EvapWaterStarvMakup;     // Evap water consumed but not really available from tank m3
        Real64 EvapCondPumpElecPower;
        Real64 EvapCondPumpElecConsumption;

        // Default Constructor
        PackagedTESCoolingCoilStruct()
            : ModeControlType(PTSCCtrlType::Invalid), EMSControlModeOn(false), EMSControlModeValue(0.0), ControlModeErrorIndex(0),
              RatedEvapAirVolFlowRate(0.0), RatedEvapAirMassFlowRate(0.0), EvapAirInletNodeNum(0), EvapAirOutletNodeNum(0),
              CoolingOnlyModeIsAvailable(false), CoolingOnlyRatedTotCap(0.0), CoolingOnlyRatedSHR(0.0), CoolingOnlyRatedCOP(0.0),
              CoolingOnlyCapFTempCurve(0), CoolingOnlyCapFTempObjectNum(0), CoolingOnlyCapFFlowCurve(0), CoolingOnlyCapFFlowObjectNum(0),
              CoolingOnlyEIRFTempCurve(0), CoolingOnlyEIRFTempObjectNum(0), CoolingOnlyEIRFFlowCurve(0), CoolingOnlyEIRFFlowObjectNum(0),
              CoolingOnlyPLFFPLRCurve(0), CoolingOnlyPLFFPLRObjectNum(0), CoolingOnlySHRFTempCurve(0), CoolingOnlySHRFTempObjectNum(0),
              CoolingOnlySHRFFlowCurve(0), CoolingOnlySHRFFlowObjectNum(0), CoolingAndChargeModeAvailable(false), CoolingAndChargeRatedTotCap(0.0),
              CoolingAndChargeRatedTotCapSizingFactor(0.0), CoolingAndChargeRatedChargeCap(0.0), CoolingAndChargeRatedChargeCapSizingFactor(0.0),
              CoolingAndChargeRatedSHR(0.0), CoolingAndChargeCoolingRatedCOP(0.0), CoolingAndChargeChargingRatedCOP(0.0),
              CoolingAndChargeCoolingCapFTempCurve(0), CoolingAndChargeCoolingCapFTempObjectNum(0), CoolingAndChargeCoolingCapFFlowCurve(0),
              CoolingAndChargeCoolingCapFFlowObjectNum(0), CoolingAndChargeCoolingEIRFTempCurve(0), CoolingAndChargeCoolingEIRFTempObjectNum(0),
              CoolingAndChargeCoolingEIRFFlowCurve(0), CoolingAndChargeCoolingEIRFFlowObjectNum(0), CoolingAndChargeCoolingPLFFPLRCurve(0),
              CoolingAndChargeCoolingPLFFPLRObjectNum(0), CoolingAndChargeChargingCapFTempCurve(0), CoolingAndChargeChargingCapFTempObjectNum(0),
              CoolingAndChargeChargingCapFEvapPLRCurve(0), CoolingAndChargeChargingCapFEvapPLRObjectNum(0), CoolingAndChargeChargingEIRFTempCurve(0),
              CoolingAndChargeChargingEIRFTempObjectNum(0), CoolingAndChargeChargingEIRFFLowCurve(0), CoolingAndChargeChargingEIRFFLowObjectNum(0),
              CoolingAndChargeChargingPLFFPLRCurve(0), CoolingAndChargeChargingPLFFPLRObjectNum(0), CoolingAndChargeSHRFTempCurve(0),
              CoolingAndChargeSHRFFlowCurve(0), CoolingAndChargeSHRFFlowObjectNum(0), CoolingAndDischargeModeAvailable(false),
              CoolingAndDischargeRatedTotCap(0.0), CoolingAndDischargeRatedTotCapSizingFactor(0.0), CoolingAndDischargeRatedDischargeCap(0.0),
              CoolingAndDischargeRatedDischargeCapSizingFactor(0.0), CoolingAndDischargeRatedSHR(0.0), CoolingAndDischargeCoolingRatedCOP(0.0),
              CoolingAndDischargeDischargingRatedCOP(0.0), CoolingAndDischargeCoolingCapFTempCurve(0), CoolingAndDischargeCoolingCapFTempObjectNum(0),
              CoolingAndDischargeCoolingCapFFlowCurve(0), CoolingAndDischargeCoolingCapFFlowObjectNum(0), CoolingAndDischargeCoolingEIRFTempCurve(0),
              CoolingAndDischargeCoolingEIRFTempObjectNum(0), CoolingAndDischargeCoolingEIRFFlowCurve(0),
              CoolingAndDischargeCoolingEIRFFlowObjectNum(0), CoolingAndDischargeCoolingPLFFPLRCurve(0),
              CoolingAndDischargeCoolingPLFFPLRObjectNum(0), CoolingAndDischargeDischargingCapFTempCurve(0),
              CoolingAndDischargeDischargingCapFTempObjectNum(0), CoolingAndDischargeDischargingCapFFlowCurve(0),
              CoolingAndDischargeDischargingCapFFlowObjectNum(0), CoolingAndDischargeDischargingCapFEvapPLRCurve(0),
              CoolingAndDischargeDischargingCapFEvapPLRObjectNum(0), CoolingAndDischargeDischargingEIRFTempCurve(0),
              CoolingAndDischargeDischargingEIRFTempObjectNum(0), CoolingAndDischargeDischargingEIRFFLowCurve(0),
              CoolingAndDischargeDischargingEIRFFLowObjectNum(0), CoolingAndDischargeDischargingPLFFPLRCurve(0),
              CoolingAndDischargeDischargingPLFFPLRObjectNum(0), CoolingAndDischargeSHRFTempCurve(0), CoolingAndDischargeSHRFTempObjectNum(0),
              CoolingAndDischargeSHRFFlowCurve(0), CoolingAndDischargeSHRFFlowObjectNum(0), ChargeOnlyModeAvailable(false),
              ChargeOnlyRatedCapacity(0.0), ChargeOnlyRatedCapacitySizingFactor(0.0), ChargeOnlyRatedCOP(0.0), ChargeOnlyChargingCapFTempCurve(0),
              ChargeOnlyChargingCapFTempObjectNum(0), ChargeOnlyChargingEIRFTempCurve(0), ChargeOnlyChargingEIRFTempObjectNum(0),
              DischargeOnlyModeAvailable(false), DischargeOnlyRatedDischargeCap(0.0), DischargeOnlyRatedDischargeCapSizingFactor(0.0),
              DischargeOnlyRatedSHR(0.0), DischargeOnlyRatedCOP(0.0), DischargeOnlyCapFTempCurve(0), DischargeOnlyCapFTempObjectNum(0),
              DischargeOnlyCapFFlowCurve(0), DischargeOnlyCapFFlowObjectNum(0), DischargeOnlyEIRFTempCurve(0), DischargeOnlyEIRFTempObjectNum(0),
              DischargeOnlyEIRFFlowCurve(0), DischargeOnlyEIRFFlowObjectNum(0), DischargeOnlyPLFFPLRCurve(0), DischargeOnlyPLFFPLRObjectNum(0),
              DischargeOnlySHRFTempCurve(0), DischargeOnlySHRFTempObjectNum(0), DischargeOnlySHRFFLowCurve(0), DischargeOnlySHRFFLowObjectNum(0),
              AncillaryControlsPower(0.0), ColdWeatherMinimumTempLimit(0.0), ColdWeatherAncillaryPower(0.0), CondAirInletNodeNum(0),
              CondAirOutletNodeNum(0), CondenserAirVolumeFlow(0.0), CondenserAirFlowSizingFactor(0.0), CondenserAirMassFlow(0.0), EvapCondEffect(0.0),
              CondInletTemp(0.0), EvapCondPumpElecNomPower(0.0), EvapCondPumpElecEnergy(0.0), BasinHeaterPowerFTempDiff(0.0),
              BasinHeaterSetpointTemp(0.0), EvapWaterSupplyMode(EvapWaterSupply::WaterSupplyFromMains), EvapWaterSupTankID(0),
              EvapWaterTankDemandARRID(0), CondensateCollectMode(CondensateAction::Discard), CondensateTankID(0), CondensateTankSupplyARRID(0),
              StorageMedia(MediaType::Invalid), FluidStorageVolume(0.0), IceStorageCapacity(0.0), StorageCapacitySizingFactor(0.0),
              MinimumFluidTankTempLimit(0.0), MaximumFluidTankTempLimit(100.0), RatedFluidTankTemp(0.0), StorageAmbientNodeNum(0), StorageUA(0.0),
              TESPlantConnectionAvailable(false), TESPlantInletNodeNum(0), TESPlantOutletNodeNum(0), TESPlantLoopNum(0),
              TESPlantLoopSideNum(DataPlant::LoopSideLocation::Invalid), TESPlantBranchNum(0), TESPlantCompNum(0), TESPlantDesignVolumeFlowRate(0.0),
              TESPlantDesignMassFlowRate(0.0), TESPlantEffectiveness(0.0), TimeElapsed(0.0), IceFracRemain(0.0), IceFracRemainLastTimestep(0.0),
              FluidTankTempFinal(0.0), FluidTankTempFinalLastTimestep(0.0), QdotPlant(0.0), Q_Plant(0.0), QdotAmbient(0.0), Q_Ambient(0.0),
              QdotTES(0.0), Q_TES(0.0), ElecCoolingPower(0.0), ElecCoolingEnergy(0.0), EvapTotCoolingRate(0.0), EvapTotCoolingEnergy(0.0),
              EvapSensCoolingRate(0.0), EvapSensCoolingEnergy(0.0), EvapLatCoolingRate(0.0), EvapLatCoolingEnergy(0.0), RuntimeFraction(0.0),
              CondenserRuntimeFraction(0.0), ElectColdWeatherPower(0.0), ElectColdWeatherEnergy(0.0), ElectEvapCondBasinHeaterPower(0.0),
              ElectEvapCondBasinHeaterEnergy(0.0), EvapWaterConsumpRate(0.0), EvapWaterConsump(0.0), EvapWaterStarvMakupRate(0.0),
              EvapWaterStarvMakup(0.0), EvapCondPumpElecPower(0.0), EvapCondPumpElecConsumption(0.0)
        {
        }
    };

    void SimTESCoil(EnergyPlusData &state,
                    std::string_view CompName, // name of the fan coil unit
                    int &CompIndex,
                    HVAC::FanOp const fanOp, // allows parent object to control fan mode
                    PTSCOperatingMode &TESOpMode,
                    ObjexxFCL::Optional<Real64 const> PartLoadRatio = _ // part load ratio (for single speed cycling unit)
    );

    void GetTESCoilInput(EnergyPlusData &state);

    void InitTESCoil(EnergyPlusData &state, int &TESCoilNum);

    void SizeTESCoil(EnergyPlusData &state, int &TESCoilNum);

    void CalcTESCoilOffMode(EnergyPlusData &state, int const TESCoilNum);

    void CalcTESCoilCoolingOnlyMode(EnergyPlusData &state, int const TESCoilNum, HVAC::FanOp const fanOp, Real64 const PartLoadRatio);

    void CalcTESCoilCoolingAndChargeMode(EnergyPlusData &state, int const TESCoilNum, HVAC::FanOp const fanOp, Real64 const PartLoadRatio);

    void CalcTESCoilCoolingAndDischargeMode(EnergyPlusData &state, int const TESCoilNum, HVAC::FanOp const fanOp, Real64 const PartLoadRatio);

    void CalcTESCoilChargeOnlyMode(EnergyPlusData &state, int const TESCoilNum);

    void CalcTESCoilDischargeOnlyMode(EnergyPlusData &state, int const TESCoilNum, Real64 const PartLoadRatio);

    void UpdateTEStorage(EnergyPlusData &state, int const TESCoilNum);

    void CalcTESWaterStorageTank(EnergyPlusData &state, int const TESCoilNum);

    void CalcTESIceStorageTank(EnergyPlusData &state, int const TESCoilNum);

    void UpdateColdWeatherProtection(EnergyPlusData &state, int const TESCoilNum);

    void UpdateEvaporativeCondenserBasinHeater(EnergyPlusData &state, int const TESCoilNum);

    void UpdateEvaporativeCondenserWaterUse(EnergyPlusData &state, int const TESCoilNum, Real64 const HumRatAfterEvap, int const InletNodeNum);

    void GetTESCoilIndex(
        EnergyPlusData &state, std::string const &CoilName, int &CoilIndex, bool &ErrorsFound, std::string_view const CurrentModuleObject = {});

    void GetTESCoilAirInletNode(
        EnergyPlusData &state, std::string const &CoilName, int &CoilAirInletNode, bool &ErrorsFound, std::string const &CurrentModuleObject);

    void GetTESCoilAirOutletNode(
        EnergyPlusData &state, std::string const &CoilName, int &CoilAirOutletNode, bool &ErrorsFound, std::string const &CurrentModuleObject);

    void GetTESCoilCoolingCapacity(
        EnergyPlusData &state, std::string const &CoilName, Real64 &CoilCoolCapacity, bool &ErrorsFound, std::string const &CurrentModuleObject);

    void GetTESCoilCoolingAirFlowRate(
        EnergyPlusData &state, std::string const &CoilName, Real64 &CoilCoolAirFlow, bool &ErrorsFound, std::string const &CurrentModuleObject);

} // namespace PackagedThermalStorageCoil

struct PackagedThermalStorageCoilData : BaseGlobalStruct
{

    int NumTESCoils = 0;
    Array1D_bool CheckEquipName;
    bool GetTESInputFlag = true;
    bool MyOneTimeFlag = true;
    Array1D<PackagedThermalStorageCoil::PackagedTESCoolingCoilStruct> TESCoil;
    Array1D_bool MyFlag;       // One time environment flag
    Array1D_bool MySizeFlag;   // One time sizing flag
    Array1D_bool MyEnvrnFlag;  // flag for init once at start of environment
    Array1D_bool MyWarmupFlag; // flag for init after warmup complete

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumTESCoils = 0;
        this->CheckEquipName.deallocate();
        this->GetTESInputFlag = true;
        this->MyOneTimeFlag = true;
        this->TESCoil.deallocate();
        this->MyFlag.clear();
        this->MySizeFlag.clear();
        this->MyEnvrnFlag.clear();
        this->MyWarmupFlag.clear();
    }
};

} // namespace EnergyPlus

#endif
