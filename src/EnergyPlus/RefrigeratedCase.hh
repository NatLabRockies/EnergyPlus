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

#ifndef RefrigeratedCase_hh_INCLUDED
#define RefrigeratedCase_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EneryPlusData;

namespace RefrigeratedCase {

    // Walk In Cooler Stock Door Protection types
    enum class WIStockDoor
    {
        Invalid = -1,
        None,
        AirCurtain,
        StripCurtain,
        Num,
    };

    // Compressor suction pressure control
    enum class CompressorSuctionPressureCtrl
    {
        Invalid = -1,
        FloatSuctionTemperature,
        ConstantSuctionTemperature,
        Num,
    };

    // Subcooler type
    enum class SubcoolerType
    {
        Invalid = -1,
        LiquidSuction,
        Mechanical,
        Num,
    };

    // Walk In Cooler Defrost Control type
    enum class DefrostCtrlType
    {
        Invalid = -1,
        Sched,
        TempTerm,
        Num
    };

    // Secondary loop parameters
    enum class SecFluidType
    {
        Invalid = -1,
        AlwaysLiquid,
        PhaseChange,
        Num
    };

    enum class SecPumpCtrl
    {
        Invalid = -1,
        Constant,
        Variable,
        Num
    };

    // Refrigerated display case energy equation form
    enum class EnergyEqnForm
    {
        Invalid = -1,
        None,
        CaseTemperatureMethod,
        RHCubic,
        DPCubic,
        Num
    };

    // Cascade condenser temperature control types
    enum class CascadeCndsrTempCtrlType
    {
        Invalid = -1,
        TempSet,
        TempFloat,
        Num
    };

    // Water-cooled condenser loop flow type
    enum class CndsrFlowType : int
    {
        Invalid = -1,
        Variable,
        Constant,
        Num
    };

    // Air- and evap-cooled condenser fan speed control types
    enum class FanSpeedCtrlType
    {
        Invalid = -1,
        VariableSpeed,
        ConstantSpeedLinear,
        TwoSpeed,
        ConstantSpeed,
        Num
    };

    // Refrigerated display case rack heat rejection location
    enum class HeatRejLocation
    {
        Invalid = -1,
        Outdoors,
        Zone,
        Num
    };

    // Refrigerated display case defrost type
    enum class RefCaseDefrostType
    {
        Invalid = -1,
        None,
        OffCycle,
        HotFluid,
        HotFluidTerm,
        Electric,
        ElectricOnDemand,
        ElectricTerm,
        Num
    };

    // Anti-sweat heater control type
    enum class ASHtrCtrlType
    {
        Invalid = -1,
        None,
        Constant,
        Linear,
        DewPoint,
        HeatBalance,
        Num
    };

    // Compressor rating types
    enum class CompRatingType
    {
        Invalid = -1,
        Superheat,
        ReturnGasTemperature,
        Subcooling,
        LiquidTemperature,
        Num
    };

    // Condenser evap cooling water supply
    enum class WaterSupply
    {
        Invalid = -1,
        FromMains,
        FromTank,
        Num
    };

    enum class RatingType
    {
        Invalid = -1,
        RatedCapacityTotal,
        EuropeanSC1Std,
        EuropeanSC1Nom,
        EuropeanSC2Std,
        EuropeanSC2Nom,
        EuropeanSC3Std,
        EuropeanSC3Nom,
        EuropeanSC4Std,
        EuropeanSC4Nom,
        EuropeanSC5Std,
        EuropeanSC5Nom,
        UnitLoadFactorSens,
        Num
    };

    enum class SHRCorrectionType
    {
        Invalid = -1,
        SHR60,
        QuadraticSHR,
        European,
        TabularRH_DT1_TRoom,
        Num
    };

    enum class VerticalLoc
    {
        Invalid = -1,
        Ceiling,
        Middle,
        Floor,
        Num
    };

    enum class SourceType
    {
        Invalid = -1,
        DetailedSystem,
        SecondarySystem,
        Num
    };

    // Warehouse coil Defrost type
    enum class DefrostType
    {
        Invalid = -1,
        Fluid,
        Elec,
        None,
        OffCycle,
        Num
    };

    enum class CriticalType
    {
        Invalid = -1,
        Subcritical,
        Transcritical,
        Num
    };

    enum class IntercoolerType
    {
        Invalid = -1,
        None,
        Flash,
        ShellAndCoil,
        Num
    };

    enum class TransSysType
    {
        Invalid = -1,
        SingleStage,
        TwoStage,
        Num
    };

    struct RefrigCaseData
    {
        std::string Name;                                             // Name of refrigerated display case
        std::string ZoneName;                                         // Zone or Location of Display Case
        int NumSysAttach = 0;                                         // Number of systems attached to case, error if /=1
        Sched::Schedule *availSched = nullptr;                        // Availability schedule
        int ZoneNodeNum = 0;                                          // Index to Zone Node
        int ActualZoneNum = 0;                                        // Index to Zone
        int ZoneRANode = 0;                                           // Node number of return node in zone
        Real64 RatedAmbientTemp = 0.0;                                // Rated ambient (zone) temperature
        Real64 RatedAmbientRH = 0.0;                                  // Rated ambient (zone) relative humidity
        Real64 RatedAmbientDewPoint = 0.0;                            // Rated ambient (zone) dew point temperature
        Real64 RateTotCapPerLength = 0.0;                             // Gross total cooling capacity at rated conditions [W/m]
        Real64 RatedLHR = 0.0;                                        // Latent heat ratio (lat cap/total cap) at rated conditions
        Real64 RatedRTF = 0.0;                                        // Run time fraction at rated conditions
        int LatCapCurvePtr = 0;                                       // Index for latent case credit modifier curve
        int DefCapCurvePtr = 0;                                       // Index for defrost load modifier curve
        EnergyEqnForm LatentEnergyCurveType = EnergyEqnForm::Invalid; // Type of latent case credit curve:
        // 1=Case Temperature Method, 2=RH cubic, 3=DP cubic
        EnergyEqnForm DefrostEnergyCurveType = EnergyEqnForm::Invalid; // Type of defrost energy curve:
        // 1=Case Temperature Method, 2=RH cubic, 3=DP cubic
        Real64 STDFanPower = 0.0;                                    // Standard power of case fan [W/m] for case credit calc
        Real64 OperatingFanPower = 0.0;                              // Operating power of refrigerated case fan [W/m]
        Real64 RatedLightingPower = 0.0;                             // Rated (consis w RateTotCapPerLength) power of refrigerated case lights [W/m]
        Real64 LightingPower = 0.0;                                  // Installed power of refrigerated case lights [W/m]
        Sched::Schedule *lightingSched = nullptr;                    // case lighting schedule
        Real64 AntiSweatPower = 0.0;                                 // Rated power of refrigerated case anti-sweat heaters [W/m]
        Real64 MinimumASPower = 0.0;                                 // Minimum power output of case anti-sweat heaters [W/m]
        ASHtrCtrlType AntiSweatControlType = ASHtrCtrlType::Invalid; // Type of anti-sweat heater control:
        // 0=None,1=Constant,2=Linear,3=DewPoint,4=HeatBalance
        Real64 HumAtZeroAS = 0.0;                                     // Relative humidity for zero AS heater output using linear control
        Real64 Height = 0.0;                                          // case height for AS heater with heat balance control
        RefCaseDefrostType defrostType = RefCaseDefrostType::Invalid; // Case defrost control type, Off-cycle,Timed,Hot-gas,Electric
        Real64 DefrostPower = 0.0;                                    // Rated power of refrigerated case defrost [W/m]
        Sched::Schedule *defrostSched = nullptr;                      // defrost schedule
        Sched::Schedule *defrostDripDownSched = nullptr;              // correct fail-safe schedule
        Real64 Length = 0.0;                                          // Length of refrigerated case [m]
        Real64 Temperature = 0.0;                                     // Rated case temperature [C]
        Real64 RAFrac = 0.0;                                          // HVAC under case return air fraction [0-1]
        Sched::Schedule *stockingSched = nullptr;                     // product stocking schedule
        Real64 LightingFractionToCase = 0.0;                          // Fraction of lighting energy that directly contributes to the
        // case cooling load. The remainder contributes to the zone load
        // (air heat balance).
        Real64 ASHeaterFractionToCase = 0.0; // Fraction of anti-sweat heater energy that results in a direct
        // heat load to the case. The remainder is a heating load
        // to the zone where the refrigerated case is located.
        Real64 DesignSensCaseCredit = 0.0;              // Design sensible case credit applied to zone load
        Real64 EvapTempDesign = 0.0;                    // Design evaporator temperature
        Real64 RefrigInventory = 0.0;                   // Design refrigerant inventory [kg/m]
        Real64 DesignRefrigInventory = 0.0;             // Design refrigerant inventory [kg total for the case]
        Real64 DesignRatedCap = 0.0;                    // Design total case capacity=RatedTotCap*Length [W]
        Real64 DesignLatentCap = 0.0;                   // Design latent case capacity=DesignRAtedCap*LatentHeatRatio*RTF [W]
        Real64 DesignDefrostCap = 0.0;                  // Design defrost case capacity=DefrostPower*Length [W]
        Real64 DesignLighting = 0.0;                    // Design case lighting=LightingPower*Length [W]
        Real64 DesignFanPower = 0.0;                    // Design power of case fan=Operatingpower*Length [W]
        Real64 StoredEnergy = 0.0;                      // Cumulative Stored Energy not met by evaporator [J]
        Real64 StoredEnergySaved = 0.0;                 // Cumulative Stored Energy not met by evaporator [J]
        Sched::Schedule *caseCreditFracSched = nullptr; // case credit reduction schedule
        // Report Variables
        Real64 TotalCoolingLoad = 0.0;         // Refrigerated case total cooling rate (W)
        Real64 TotalCoolingEnergy = 0.0;       // Refrigerated case total cooling energy (J)
        Real64 SensCoolingEnergyRate = 0.0;    // Refrigerated case sensible cooling rate (W)
        Real64 SensCoolingEnergy = 0.0;        // Refrigerated case sensible cooling energy (J)
        Real64 LatCoolingEnergyRate = 0.0;     // Refrigerated case latent cooling rate (W)
        Real64 LatCoolingEnergy = 0.0;         // Refrigerated case latent cooling energy (J)
        Real64 SensZoneCreditRate = 0.0;       // Refrigerated case sensible zone credit rate (W)
        Real64 SensZoneCreditCoolRate = 0.0;   // Refrigerated case sensible cooling zone credit rate (W)
        Real64 SensZoneCreditCool = 0.0;       // Refrigerated case sensible cooling zone credit energy (J)
        Real64 SensZoneCreditHeatRate = 0.0;   // Refrigerated case sensible heating zone credit rate (W)
        Real64 SensZoneCreditHeat = 0.0;       // Refrigerated case sensible heating zone credit energy (J)
        Real64 LatZoneCreditRate = 0.0;        // Refrigerated case latent zone credit rate (W)
        Real64 LatZoneCredit = 0.0;            // Refrigerated case latent zone credit energy (J)
        Real64 SensHVACCreditRate = 0.0;       // Refrigerated case sensible HVAC credit rate (W)
        Real64 SensHVACCreditCoolRate = 0.0;   // Refrigerated case sensible cooling HVAC credit rate (W)
        Real64 SensHVACCreditCool = 0.0;       // Refrigerated case sensible cooling HVAC credit energy (J)
        Real64 SensHVACCreditHeatRate = 0.0;   // Refrigerated case sensible heating HVAC credit rate (W)
        Real64 SensHVACCreditHeat = 0.0;       // Refrigerated case sensible heating HVAC credit energy (J)
        Real64 LatHVACCreditRate = 0.0;        // Refrigerated case latent HVAC credit rate (W)
        Real64 LatHVACCredit = 0.0;            // Refrigerated case latent HVAC credit energy (J)
        Real64 ElecAntiSweatPower = 0.0;       // Refrigerated case anti-sweat heater rate (W)
        Real64 ElecAntiSweatConsumption = 0.0; // Refrigerated case anti-sweat heater energy (J)
        Real64 ElecFanPower = 0.0;             // Refrigerated case fan electric power (W)
        Real64 ElecFanConsumption = 0.0;       // Refrigerated case fan electric energy (J)
        Real64 ElecLightingPower = 0.0;        // Refrigerated case lighting electric power (W)
        Real64 ElecLightingConsumption = 0.0;  // Refrigerated case lighting electric energy (J)
        Real64 ElecDefrostPower = 0.0;         // Refrigerated case defrost rate (W)
        Real64 ElecDefrostConsumption = 0.0;   // Refrigerated case defrost energy (J)
        Real64 DefEnergyCurveValue = 0.0;      // Refrigerated case defrost capacity modifier
        Real64 LatEnergyCurveValue = 0.0;      // Refrigerated case latent capacity modifier
        Real64 MaxKgFrost = 0.0;               // Amount of frost formation to initiate defrost for On Demand
        Real64 Rcase = 0.0;                    // Case wall resistance for AS heater calc (h-sqm-C/W)
        Real64 DefrostEnergy = 0.0;            // Refrigerated case defrost energy (J)
        Real64 StockingEnergy = 0.0;           // Refrigerated case product stocking energy (J)
        Real64 WarmEnvEnergy = 0.0;            // Refrigerated case extra sensible energy due to warm zone ambient (J)
        Real64 KgFrost = 0.0;                  // Amount of frost on case evaporator (Kg)
        Real64 DefrostEnergySaved = 0.0;       // Refrigerated case defrost energy (J)
        Real64 StockingEnergySaved = 0.0;      // Refrigerated case product stocking energy (J)
        Real64 WarmEnvEnergySaved = 0.0;       // Refrigerated case extra sensible energy due to warm zone ambient (J)
        Real64 KgFrostSaved = 0.0;             // Amount of frost on case evaporator (Kg)
        Real64 HotDefrostCondCredit = 0.0;     // Used to credit condenser when heat reclaim used for hot gas/brine defrost (W)
        Real64 DeltaDefrostEnergy = 0.0;       // Used to reverse accumulation if the zone/load time step is repeated (J)
        bool ShowStoreEnergyWarning = true;
        bool ShowFrostWarning = true;

        // Reset Initialization Values to Zeros
        void reset_init()
        {
            TotalCoolingLoad = 0.0;
            TotalCoolingEnergy = 0.0;
            SensCoolingEnergyRate = 0.0;
            SensCoolingEnergy = 0.0;
            LatCoolingEnergyRate = 0.0;
            LatCoolingEnergy = 0.0;
            SensZoneCreditRate = 0.0;
            SensZoneCreditCoolRate = 0.0;
            SensZoneCreditCool = 0.0;
            SensZoneCreditHeatRate = 0.0;
            SensZoneCreditHeat = 0.0;
            LatZoneCreditRate = 0.0;
            LatZoneCredit = 0.0;
            SensHVACCreditRate = 0.0;
            SensHVACCreditCoolRate = 0.0;
            SensHVACCreditCool = 0.0;
            SensHVACCreditHeatRate = 0.0;
            SensHVACCreditHeat = 0.0;
            LatHVACCreditRate = 0.0;
            LatHVACCredit = 0.0;
            ElecFanPower = 0.0;
            ElecFanConsumption = 0.0;
            ElecAntiSweatPower = 0.0;
            ElecAntiSweatConsumption = 0.0;
            ElecLightingPower = 0.0;
            ElecLightingConsumption = 0.0;
            ElecDefrostPower = 0.0;
            ElecDefrostConsumption = 0.0;
            DefEnergyCurveValue = 0.0;
            LatEnergyCurveValue = 0.0;
            HotDefrostCondCredit = 0.0;
        }

        // Reset Accumulation and Carry-Over Values to Zeros
        void reset_init_accum()
        {
            DefrostEnergy = 0.0;
            StockingEnergy = 0.0;
            WarmEnvEnergy = 0.0;
            KgFrost = 0.0;
            StoredEnergy = 0.0;
        }

        void CalculateCase(EnergyPlusData &state); // Absolute pointer to refrigerated case
    };

    struct RefrigRackData : PlantComponent
    {
        int MyIdx = 0;                             // Index number
        bool CoilFlag = false;                     // Flag to show if coil type load on rack
        std::string Name;                          // Name of Refrigeration Compressor rack
        std::string SupplyTankName;                // Evap water supply tank name
        std::string EndUseSubcategory = "General"; // Rack end-use subcategory
        // Index of refrigerated case (1 to NumCases) connected to rack #X
        Array1D_int CaseNum;
        Array1D_int CoilNum;
        Array1D_int WalkInNum;
        HeatRejLocation HeatRejectionLocation = HeatRejLocation::Invalid; // Refrigeration Compressor Rack heat rejection location
        DataHeatBalance::RefrigCondenserType CondenserType =
            DataHeatBalance::RefrigCondenserType::Invalid;        // Specifies cooling mode for outdoor condenser
        Real64 LaggedUsedWaterHeater = 0.0;                       // Heat reclaim used to heat water in previous zone/load time step(W)
        Real64 LaggedUsedHVACCoil = 0.0;                          // Heat reclaim used to heat HVAC coil in previous zone/load time step(W)
        Real64 EvapEffect = 0.9;                                  // Effectiveness of evaporative condenser
        Real64 CondenserAirFlowRate = 0.0;                        // Evaporative condenser air volume flow rate (m3/s)
        Real64 EvapPumpPower = 0.0;                               // Evaporative cooling water pump power (W)
        Real64 ActualEvapPumpPower = 0.0;                         // Evaporative cooling water pump power, if adjusted (W)
        Real64 EvapPumpConsumption = 0.0;                         // Evaporative cooling water pump electric consumption (J)
        Real64 EvapWaterConsumpRate = 0.0;                        // Evaporative condenser water consumption rate (m3/s)
        Real64 EvapWaterConsumption = 0.0;                        // Evaporative condenser water consumption (m3)
        Sched::Schedule *evapAvailSched = nullptr;                // evap condenser availability schedule
        Real64 BasinHeaterPowerFTempDiff = 0.0;                   // Basin heater capacity per degree K below setpoint (W/K)
        Real64 BasinHeaterSetPointTemp = 2.0;                     // Setpoint temperature for basin heater operation (C)
        Real64 BasinHeaterPower = 0.0;                            // Power demand from basin heater (W)
        Real64 BasinHeaterConsumption = 0.0;                      // Electric consumption from basin heater (J)
        Real64 RatedCOP = 0.0;                                    // Rated coefficient of performance for compressor rack (W/W)
        int COPFTempPtr = 0;                                      // Index to the correct COP curve object
        int NumCases = 0;                                         // Total number of refrigerated cases attached to each rack
        int NumCoils = 0;                                         // Total number of air chillers attached to each rack
        int NumWalkIns = 0;                                       // Total number of walk-ins attached to each rack
        WaterSupply EvapWaterSupplyMode = WaterSupply::FromMains; // Source of water for evap condenser cooling
        int EvapWaterSupTankID = 0;                               // TankID when evap condenser uses water from storage tank
        int EvapWaterTankDemandARRID = 0;                         // Demand index when evap condenser uses water from storage tank
        int OutsideAirNodeNum = 0;                                // Outside air node number
        int HeatRejectionZoneNum = 0;                             // Heat rejection zone number used when walk-ins present and ht rej to zone
        int HeatRejectionZoneNodeNum = 0;                         // Heat rejection zone node number used when walk-ins present and ht rej to zone
        Real64 TotalRackLoad = 0.0;                               // Total capacity of all refrigerated cases on rack
        Real64 RackCompressorCOP = 0.0;                           // Rack compressor COP at specific operating conditions
        Real64 RackCompressorPower = 0.0;                         // Total rack compressor power (W)
        Real64 RackElecConsumption = 0.0;                         // Total rack compressor electric consumption (J)
        Real64 RackCapacity = 0.0;                                // Total rack delivered capacity (W)
        Real64 RackCoolingEnergy = 0.0;                           // Total rack delivered energy (J)
        Real64 CondenserFanPower = 0.0;                           // Condenser fan power (W)
        int TotCondFTempPtr = 0;                                  // Index for condenser fan power modifier curve
        // (function of outdoor temperature)
        Real64 ActualCondenserFanPower = 0.0;             // Rack condenser fan power (W)
        Real64 CondenserFanConsumption = 0.0;             // Rack condenser fan electric consumption (J)
        Real64 SensZoneCreditHeatRate = 0.0;              // Rack sensible heating zone credit rate (W)
        Real64 SensZoneCreditHeat = 0.0;                  // Rack sensible heating zone credit energy (J)
        Real64 SensHVACCreditHeatRate = 0.0;              // Rack sensible heating HVAC credit rate (W)
        Real64 SensHVACCreditHeat = 0.0;                  // Rack sensible heating HVAC credit energy (J)
        int EvapFreezeWarnIndex = 0;                      // Recurring freeze warning index
        int NoFlowWarnIndex = 0;                          // No cooling water when needed warning index
        int HighTempWarnIndex = 0;                        // Water outlet high temp warning index
        int LowTempWarnIndex = 0;                         // Water outlet low temp warning index
        int HighFlowWarnIndex = 0;                        // Water outlet high flow warning index
        int HighInletWarnIndex = 0;                       // Water inlet high temp warning index
        int InletNode = 0;                                // Water-cooled condenser inlet node number
        Real64 InletTemp = 0.0;                           // Water-cooling condenser inlet temperature (C)
        int OutletNode = 0;                               // Water-cooled condenser outlet node number
        int PlantTypeOfNum = 0;                           // Water-cooled condenser plant equipment type
        PlantLocation plantLoc;                           // Water-cooled condenser plant location
        Real64 OutletTemp = 0.0;                          // Water-cooling condenser outlet temperature (C)
        Sched::Schedule *outletTempSched = nullptr;       // Schedule for condenser outlet temp setting
        Real64 VolFlowRate = 0.0;                         // Water-cooled condenser volumetric flow rate (m3/s)
        Real64 DesVolFlowRate = 0.0;                      // Water-cooled condenser design volumetric flow rate (m3/s)
        Real64 MassFlowRate = 0.0;                        // Water-cooled condenser mass flow rate (kg/s)
        Real64 CondLoad = 0.0;                            // Total condenser load (W)
        Real64 CondEnergy = 0.0;                          // Condenser energy (J)
        CndsrFlowType FlowType = CndsrFlowType::Variable; // Water-cooled condenser loop flow type
        Real64 VolFlowRateMax = 0.0;                      // Maximum condenser volumetric flow rate (m3/s)
        Real64 MassFlowRateMax = 0.0;                     // Maximum condenser mass flow rate (kg/s)
        Real64 InletTempMin = 10.0;                       // Minimum condenser water inlet temperature (C)
        Real64 OutletTempMax = 55.0;                      // Maximum condenser water outlet temperature (C)
        Real64 TotalCoolingLoad = 0.0;
        bool ShowCOPWarning = true;

        // Reset Initialization Values to Zeros
        void reset_init()
        {
            SensHVACCreditHeatRate = 0.0;
            SensHVACCreditHeat = 0.0;
            SensZoneCreditHeatRate = 0.0;
            SensZoneCreditHeat = 0.0;
            CondLoad = 0.0;
            CondEnergy = 0.0;
            MassFlowRate = 0.0;
            RackElecConsumption = 0.0;
            CondenserFanConsumption = 0.0;
            EvapPumpConsumption = 0.0;
            RackCompressorPower = 0.0;
            ActualCondenserFanPower = 0.0;
            ActualEvapPumpPower = 0.0;
        }

        void UpdateCondenserOutletNode(EnergyPlusData &state) const;

        void CalcRackSystem(EnergyPlusData &state);

        void ReportRackSystem(EnergyPlusData &state, int RackNum);

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, const PlantLocation &calledFromLocation) override;

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void oneTimeInit(EnergyPlusData &state) override;

        void oneTimeInit_new(EnergyPlusData &state) override;
    };

    struct RefrigSystemData
    {
        std::string Name;            // Name of refrigeration system
        std::string RefrigerantName; // Name of refrigerant, must match name in FluidName
        Fluid::RefrigProps *refrig = nullptr;
        std::string EndUseSubcategory;       // Used for reporting purposes
        bool SystemRejectHeatToZone = false; // Flag to show air-cooled condenser located inside zone
        bool CoilFlag = false;               // Flag to show if coil type load on system (even if below in a secondary)
        Array1D_int CascadeLoadNum;          // absolute index  of condensers placing load (allocated NumCondensers)
        Array1D_int CaseNum;                 // absolute Index of cases (allocated NumCases)
        Array1D_int CoilNum;                 // absolute Index of coils (allocated NumCoils)
        Array1D_int CompressorNum;           // absolute Index of compressors (allocated NumCompressors)
        Array1D_int CondenserNum;            // absolute Index of condensers removing load (allocated NumCondensers)
        Array1D_int GasCoolerNum;            // absolute Index of gas cooler
        Array1D_int HiStageCompressorNum;    // absolute Index of high-stage compressors (allocated NumHiStageCompressors)
        Array1D_int SecondaryNum;            // absolute Index of seocndary loops (allocated NumSecondarys)
        Array1D_int SubcoolerNum;            // Absolute Index of subcoolers (allocated NumSubcoolers)
        Array1D_int WalkInNum;               // absolute Index of walk ins (allocated NumWalkIns)
        CompressorSuctionPressureCtrl CompSuctControl = CompressorSuctionPressureCtrl::ConstantSuctionTemperature; // Index to suction control
        int HiStageWarnIndex1 = 0;                                  // Recurring warning index when hi stage compressors unable to meet coil loads
        int HiStageWarnIndex2 = 0;                                  // Recurring warning index when hi stage compressors unable to meet coil loads
        int InsuffCapWarn = 0;                                      // Recurring warning index when refrigeration system unable to meet coil loads
        IntercoolerType intercoolerType = IntercoolerType::Invalid; // Intercooler type (0=none, 1=flash intercooler, 2=shell-and-coil intercooler)
        int NumCases = 0;                                           // Number of cases on this system
        int NumCoils = 0;                                           // Number of cases on this system
        int NumCompressors = 0;                                     // Number of compressors on this system for single-stage systems
        // or number of low-stage compressors on this system for two-stage systems
        int NumCondensers = 1;         // Number of condensers on this system
        int NumGasCoolers = 0;         // Number of gas coolers on this system
        int NumHiStageCompressors = 0; // Number of high-stage compressors on this system (two-stage systems only)
        int NumSecondarys = 0;         // Number of secondary loops on this system
        int NumStages = 1;             // Number of compressor stages
        int NumSubcoolers = 0;         // Number of subcoolers on this system
        int NumWalkIns = 0;            // Number of walk in coolers on this system
        int NumMechSCServed = 0;       // Number of mech subcoolers served/powered by compressor/cond on this system
        int NumNonCascadeLoads = 0;    // Sum of NumCases, NumWalk-Ins, NumCoils, and NumSecondarys
        int NumCascadeLoads = 0;       // Number of cascade condensers cooled by this system
        int NumTransferLoads = 0;      // Sum of NumCascadeLoads and NumSecondarys
        //   and used thereafter
        int SuctionPipeActualZoneNum = 0;      // ID number for zone where suction pipes gain heat
        int SuctionPipeZoneNodeNum = 0;        // ID number for zone node where suction pipes gain heat
        Array1D<Real64> MechSCLoad;            // Mechanical subcooler load on system from other systems(W)
        Real64 AverageCompressorCOP = 0.0;     // Average COP for compressors on this system (W)
        Real64 CpSatLiqCond = 0.0;             // Spec Heat of sat liquid at condensing pressure  (J/kg-C)
        Real64 CpSatVapEvap = 0.0;             // Spec Heat of saturated vapor exiting evaporator (J/kg-C)
        Real64 FlowRatioIntercooler = 0.0;     // Refrigerant mass flow ratio through coil-side of shell-and-coil intercooler
        Real64 HCaseIn = 0.0;                  // Case inlet enthalpy (after subcoolers and pipe P drops) (J/kg)
        Real64 HCompIn = 0.0;                  // Compressor inlet enthalpy  (J/kg)
        Real64 HCompOut = 0.0;                 // Compressor outlet enthalpy (J/kg)
        Real64 HSatLiqCond = 0.0;              // Enthalpy of sat liquid at condensing pressure  (J/kg)
        Real64 HCaseOut = 0.0;                 // Enthalpy of refrigerant leaving cases, after superheat (J/kg)
        Real64 IntercoolerEffectiveness = 0.0; // Shell-and-coil intercooler effectiveness
        Real64 LSHXTrans = 0.0;                // Liquid suction subcooler load transferred within same suction group, W
        Real64 LSHXTransEnergy = 0.0;          // Liquid suction subcooler load transferred within same suction group, J
        Real64 NetHeatRejectLoad = 0.0;        // Portion of TotalCondenser load due to this system (after heat recovery) W
        Real64 NetHeatRejectEnergy = 0.0;      // Portion of TotalCondenser energy due to this system (after heat recovery) J
        Real64 PIntercooler = 0.0;             // Pressure in the intercooler (two-stage systems only) (Pa)
        Real64 PipeHeatLoad = 0.0;             // Total suction pipe heat gains, optional (W)
        Real64 PipeHeatEnergy = 0.0;           // Total suction pipe heat gains, optional (J)
        Real64 RefMassFlowtoLoads = 0.0;       // Total system refrigerant mass flow through cases(kg/s)
        Real64 RefMassFlowComps = 0.0;         // Total system refrigerant mass flow through compressors(kg/s)
        Real64 RefMassFlowHiStageComps = 0.0;  // Total system refrigerant mass flow through high-stage compressors(two-stage systems only) (kg/s)
        Real64 RefInventory = 0.0;             // Approximate refrigerant inventory entered by user (kg)
        Real64 SumMechSCLoad = 0.0;            // Total cooling load of all mech subcoolers served by suction group (W)
        Real64 SumMechSCBenefit = 0.0;         // Total cooling provided by mech subcoolers cooling liquid condensate in this system (W)
        Real64 SumCascadeCondCredit = 0.0;     // Sum of cond cred for hot brine/gas defrost on cases etc served by
        //    cascade condenser cooled by this system (W)
        Real64 SumCascadeLoad = 0.0;               // Total cooling load of all cascade condensers served by suction group (W)
        Real64 SumSecondaryLoopLoad = 0.0;         // Total cooling loads for all secondary loops served by this suction group (W)
        Real64 SumUASuctionPiping = 0.0;           // Sum of U*A for system suction piping (W/C)
        Real64 TCaseOut = 0.0;                     // Case out temperature including case superheat (C)
        Real64 TCondense = 0.0;                    // Condensing temperature (Tsat for P discharge) (C)
        Real64 TCompIn = 0.0;                      // Compressor inlet temperature (after case and LSHX superheat and pipe delta P) (C)
        Real64 TCondenseMin = 0.0;                 // Minimum allowed condensing temperature (C)
        Real64 TCondenseMinInput = 0.0;            // Minimum allowed condensing temperature, user's original input value (C)
        bool EMSOverrideOnTCondenseMin = false;    // if true, EMS is calling to override minimum allowed condensing temperature
        Real64 EMSOverrideValueTCondenseMin = 0.0; // value to use when EMS override is true [C]
        Real64 TEvapDesign = 0.0;                  // Min (on sys) design case/walkin/secondary evap temp
        //  (also basis for floating evap T calc) (C)
        Real64 TEvapNeeded = 0.0;            // Max Case evap temperature to maintain lowest case T on system (C)
        Real64 TIntercooler = 0.0;           // Temperature in the intercooler (two-stage systems only) (Pa)
        Real64 TLiqInActual = 0.0;           // Actual liquid temperature entering TXV after subcooling (C)
        Real64 TotalCondDefrostCredit = 0.0; // sum of heat reclaimed for hot gas and hot brine defrost for
        //    cases/WI/sec served directly [W]
        Real64 TotalCoolingEnergy = 0.0; // Total energy of all refrigerated cases and walkins served directly (J)
        Real64 TotalCoolingLoad = 0.0;   // Total load of all refrigerated cases and walkins served directly (W)
        Real64 TotalSystemLoad = 0.0;    // Includes cases, walk-ins, and transfer loads (cascade, second, subcooler), W
        Real64 TotCompPower = 0.0;       // Total power for compressors on this system (for single-stage systems) or
        // total power for low-stage compressors on this system (for two-stage systems) (W)
        Real64 TotCompElecConsump = 0.0; // Total Elec consump for compressors on this system (for single-stage systems) or
        // total elec consump for low-stage compressors on this system (for two-stage systems) (J)
        Real64 TotCompCapacity = 0.0; // Total design capacity for compressors on this system (for single-stage systems) or
        // total design capacity for low-stage compressors on this system (for two-stage systems) (W)
        Real64 TotCompCoolingEnergy = 0.0; // Total cooling energy from compressors on this system (for single-stage systems) or
        // total cooling energy from low-stage compressors on this system (for two-stage systems) (J)
        Real64 TotHiStageCompCapacity = 0.0;      // Total design capacity for high-stage compressors on this system (two-stage systems only) (W)
        Real64 TotHiStageCompCoolingEnergy = 0.0; // Total cooling energy from high-stage compressors on this system (two-stage systems only) (J)
        Real64 TotHiStageCompElecConsump = 0.0;   // Total Elec consump for high-stage compressors on this system (two-stage systems only) (J)
        Real64 TotHiStageCompPower = 0.0;         // Total power for high-stage compressors on this system (two-stage systems only) (W)
        Real64 TotCompElecConsumpTwoStage =
            0.0;                             // Total Elec consump for the low- and high-stage compressors on this system (two-stage systems only) (J)
        Real64 TotRejectHeatRecovered = 0.0; // Total reject heat recovered for hot gas or hot brine defrost or
        //     desuperheater coils (W)
        Real64 TotTransferLoad = 0.0; // Total load from other systems transferred to this system, incl mech subcoolers,
        // cascade, and secondary loops (W)
        Real64 TotTransferEnergy = 0.0; // Total energy from other systems transferred to this system, incl mech subcoolers,
        // cascade, and secondary loops (J)
        Real64 UnmetEnergy = 0.0; // Accumulative loads unmet by total compressors (for single-stage systems) or
        // by low-stage compressors (for two-stage systems) on this system (J)
        Real64 UnmetHiStageEnergy = 0.0; // Accumulative loads unmet by total high-stage compressors (two-stage systems only) on this system (J)
        Real64 UnmetEnergySaved = 0.0;   // Accumulative loads unmet by total compressors (for single-stage systems) on this system (J)

        // Reset Initialization Values to Zeros
        void reset_init()
        {
            TotalCoolingLoad = 0.0;
            TotalCondDefrostCredit = 0.0;
            SumSecondaryLoopLoad = 0.0;
            SumMechSCBenefit = 0.0;
            NetHeatRejectLoad = 0.0;
            NetHeatRejectEnergy = 0.0;
            AverageCompressorCOP = 0.0;
            TotCompCapacity = 0.0;
            TotHiStageCompCapacity = 0.0;
            TotCompElecConsump = 0.0;
            TotHiStageCompElecConsump = 0.0;
            TotCompElecConsumpTwoStage = 0.0;
            TotCompPower = 0.0;
            TotHiStageCompPower = 0.0;
            TotCompCoolingEnergy = 0.0;
            TotHiStageCompCoolingEnergy = 0.0;
        }

        void CalcDetailedSystem(EnergyPlusData &state, int SysNum);

        void CalculateCondensers(EnergyPlusData &state, int SysNum);

        void CalculateCompressors(EnergyPlusData &state);

        void CalculateSubcoolers(EnergyPlusData &state);
    };

    struct TransRefrigSystemData
    {
        std::string Name;            // Name of transcritical CO2 refrigeration system
        std::string RefrigerantName; // Name of refrigerant, must match name in FluidName
        //    (see fluidpropertiesrefdata.idf)
        Fluid::RefrigProps *refrig = nullptr;

        std::string EndUseSubcategory;       // Used for reporting purposes
        bool SystemRejectHeatToZone = false; // Flag to show air-cooled gas cooler located inside zone
        Array1D_int CaseNumMT;               // absolute Index of medium temperature cases (allocated NumCasesMT)
        Array1D_int CaseNumLT;               // absolute Index of low temperature cases (allocated NumCasesLT)
        Array1D_int CompressorNumHP;         // absolute Index of high pressure compressors (allocated NumCompressorsHP)
        Array1D_int CompressorNumLP;         // absolute Index of low pressure compressors (allocated NumCompressorsLP)
        Array1D_int GasCoolerNum;            // absolute Index of gas cooler
        Array1D_int WalkInNumMT;             // absolute Index of medium temperature walk ins (allocated NumWalkInsMT)
        Array1D_int WalkInNumLT;             // absolute Index of low temperature walk ins (allocated NumWalkInsLT)
        int NumCasesLT = 0;                  // Number of low temperature cases on this system
        int NumCasesMT = 0;                  // Number of medium temperature cases on this system
        int NumCompressorsHP = 0;            // Number of high pressure compressors on this system
        int NumCompressorsLP = 0;            // Number of low pressure compressors on this system
        int NumGasCoolers = 1;               // Number of gas coolers on this system
        int NumWalkInsLT = 0;                // Number of low temperature walk in coolers on this system
        int NumWalkInsMT = 0;                // Number of medium temperature walk in coolers on this system
        //   and used thereafter
        int SuctionPipeActualZoneNumMT = 0;                // ID number for zone where medium temperature suction pipes gain heat
        int SuctionPipeZoneNodeNumMT = 0;                  // ID number for zone node where medium temperature suction pipes gain heat
        int SuctionPipeActualZoneNumLT = 0;                // ID number for zone where medium temperature suction pipes gain heat
        int SuctionPipeZoneNodeNumLT = 0;                  // ID number for zone node where medium temperature suction pipes gain heat
        TransSysType transSysType = TransSysType::Invalid; // Transcritical refrigeration system type: SingleStage, TwoStage
        Real64 AverageCompressorCOP = 0.0;                 // Average COP for compressors on this system (W)
        Real64 CpSatLiqCond = 0.0;                         // Spec Heat of sat liquid at condensing pressure  (J/kg-C)
        Real64 CpSatVapEvapMT = 0.0;                       // Spec Heat of saturated vapor exiting medium temperature evaporator (J/kg-C)
        Real64 CpSatVapEvapLT = 0.0;                       // Spec Heat of saturated vapor exiting low temperature evaporator (J/kg-C)
        Real64 CpSatLiqReceiver = 0.0;                     // Spec Heat of saturated liquid in receiver (J/kg-C)
        Real64 DelHSubcoolerDis = 0.0;                     // Change in enthalpy across subcooler, hot side (J/kg)
        Real64 DelHSubcoolerSuc = 0.0;                     // Change in enthalpy across subcooler, cold side (J/kg)
        Real64 HCaseInMT = 0.0;                            // Medium temperature case inlet enthalpy (after subcoolers and pipe P drops) (J/kg)
        Real64 HCaseInLT = 0.0;                            // Low temperature case inlet enthalpy (after pipe P drops) (J/kg)
        Real64 HCompInHP = 0.0;                            // High pressure compressor inlet enthalpy  (J/kg)
        Real64 HCompInLP = 0.0;                            // Low pressure compressor inlet enthalpy  (J/kg)
        Real64 HCompOutHP = 0.0;                           // High pressure compressor outlet enthalpy (J/kg)
        Real64 HCompOutLP = 0.0;                           // Low pressure compressor outlet enthalpy (J/kg)
        Real64 HSatLiqCond = 0.0;                          // Enthalpy of sat liquid at condensing pressure  (J/kg)
        Real64 HSatLiqReceiver = 0.0;                      // Enthalpy of sat liquid in receiver (J/kg)
        Real64 HCaseOutMT = 0.0;                           // Enthalpy of refrigerant leaving medium temperature cases, after superheat (J/kg)
        Real64 HCaseOutLT = 0.0;                           // Enthalpy of refrigerant leaving low temperature cases, after superheat (J/kg)
        Real64 NetHeatRejectLoad = 0.0;                    // Portion of TotalCondenser load due to this system (after heat recovery) W
        Real64 NetHeatRejectEnergy = 0.0;                  // Portion of TotalCondenser energy due to this system (after heat recovery) J
        Real64 PipeHeatLoadMT = 0.0;                       // Total medium temperature suction pipe heat gains, optional (W)
        Real64 PipeHeatLoadLT = 0.0;                       // Total low temperature suction pipe heat gains, optional (W)
        Real64 PipeHeatEnergy = 0.0;                       // Total suction pipe heat gains, optional (J)
        Real64 PipeHeatEnergyMT = 0.0;                     // Total medium temperature suction pipe heat gains, optional (J)
        Real64 PipeHeatEnergyLT = 0.0;                     // Total low temperature suction pipe heat gains, optional (J)
        Real64 RefMassFlowtoMTLoads = 0.0;                 // Refrigerant mass flow through medium temperature cases(kg/s)
        Real64 RefMassFlowtoLTLoads = 0.0;                 // Refrigerant mass flow through low temperature cases(kg/s)
        Real64 RefMassFlowCompsHP = 0.0;                   // Total system refrigerant mass flow through high pressure compressors(kg/s)
        Real64 RefMassFlowCompsLP = 0.0;                   // Total system refrigerant mass flow through low pressure compressors(kg/s)
        Real64 RefMassFlowComps = 0.0;                     // Total system refrigerant mass flow through all compressors (kg/s)
        Real64 RefMassFlowReceiverBypass = 0.0;            // Refrigerant mass flow through receiver bypass (kg/s)
        Real64 RefInventory = 0.0;                         // Approximate refrigerant inventory entered by user (kg)
        Real64 SCEffectiveness = 0.0;                      // Heat exchanger effectiveness of the subcooler
        Real64 SumUASuctionPipingMT = 0.0;                 // Sum of U*A for medium temperature suction piping (W/C)
        Real64 SumUASuctionPipingLT = 0.0;                 // Sum of U*A for low temperature suction piping (W/C)
        Real64 TCaseOutMT = 0.0;                           // Medium temperature case out temperature including case superheat (C)
        Real64 TCaseOutLT = 0.0;                           // Low temperature case out temperature including case superheat (C)
        Real64 TCondense = 0.0;                            // Condensing temperature (Tsat for P discharge) (C)
        Real64 TReceiver = 0.0;                            // Temperature in receiver (Tsat for P receiver) (C)
        Real64 PReceiver = 0.0;                            // Pressure in receiver (Psat for T receiver) (C)
        Real64 TCompInHP = 0.0;              // High pressure compressor inlet temperature (after case and LSHX superheat and pipe delta P) (C)
        Real64 TCompInLP = 0.0;              // Low pressure compressor inlet temperature (after case and pipe delta P) (C)
        Real64 TCondenseMin = 0.0;           // Minimum allowed condensing temperature (C)
        Real64 TEvapDesignMT = 0.0;          // Min (on sys) design medium temperature case/walkin/secondary evap temp
        Real64 TEvapDesignLT = 0.0;          // Min (on sys) design low temperature case/walkin/secondary evap temp
        Real64 TEvapNeededMT = 0.0;          // Max MT Case evap temperature to maintain lowest case T on system (C)
        Real64 TEvapNeededLT = 0.0;          // Max LT Case evap temperature to maintain lowest case T on system (C)
        Real64 TLiqInActual = 0.0;           // Actual liquid temperature entering TXV after subcooling (C)
        Real64 TotalCondDefrostCredit = 0.0; // sum of heat reclaimed for hot gas and hot brine defrost for cases/WI served directly [W]
        Real64 TotalCoolingEnergy = 0.0;     // Total energy of all refrigerated cases and walkins served directly (J)
        Real64 TotalCoolingEnergyMT = 0.0;   // Total energy of all medium temperature refrigerated cases and walkins served directly (J)
        Real64 TotalCoolingEnergyLT = 0.0;   // Total energy of all low temperature refrigerated cases and walkins served directly (J)
        Real64 TotalCoolingLoadMT = 0.0;     // Total medium temperature load of all refrigerated cases and walkins served directly (W)
        Real64 TotalCoolingLoadLT = 0.0;     // Total low temperature load of all refrigerated cases and walkins served directly (W)
        Real64 TotalSystemLoad = 0.0;        // Sum of MT and LT loads, W
        Real64 TotalSystemLoadMT = 0.0;      // Includes medium temperature cases and walk-ins, W
        Real64 TotalSystemLoadLT = 0.0;      // Includes low temperature cases and walk-ins, W
        Real64 TotCompPowerHP = 0.0;         // Total power for high pressure compressors on this system (W)
        Real64 TotCompPowerLP = 0.0;         // Total power for low pressure compressors on this system (W)
        Real64 TotCompElecConsump = 0.0;     // Total Elec consump for compressors on this system (J)
        Real64 TotCompElecConsumpHP = 0.0;   // Total Elec consumption for high pressure compressors on this system (J)
        Real64 TotCompElecConsumpLP = 0.0;   // Total Elec consumption for low pressure compressors on this system (J)
        Real64 TotCompCapacity = 0.0;        // Sum of HP and LP compressor capacity (W)
        Real64 TotCompCapacityHP = 0.0;      // Total design capacity for high pressure compressors on this system (W)
        Real64 TotCompCapacityLP = 0.0;      // Total design capacity for low pressure compressors on this system (W)
        Real64 TotCompCoolingEnergy = 0.0;   // Total cooling energy from compressors on this system (J)
        Real64 TotCompCoolingEnergyHP = 0.0; // Total cooling energy from high pressure compressors on this system (J)
        Real64 TotCompCoolingEnergyLP = 0.0; // Total cooling energy from low pressure compressors on this system (J)
        Real64 TotRejectHeatRecovered = 0.0; // Total reject heat recovered for hot gas or hot brine defrost (W)
        Real64 UnmetEnergy = 0.0;            // Accumulative loads unmet by the LP and HP compressors on this system (J)
        Real64 UnmetEnergyMT = 0.0;          // Accumulative loads unmet by total HP compressors on this system (J)
        Real64 UnmetEnergyLT = 0.0;          // Accumulative loads unmet by total LP compressors on this system (J)
        Real64 UnmetEnergySaved = 0.0;       // Accumulative loads unmet by the LP and HP compressors on this system (J)
        Real64 UnmetEnergySavedMT = 0.0;     // Accumulative loads unmet by total HP compressors on this system (J)
        Real64 UnmetEnergySavedLT = 0.0;     // Accumulative loads unmet by total LP compressors on this system (J)

        // Reset Initialization Values to Zeros
        void reset_init()
        {
            TotalCoolingLoadMT = 0.0;
            TotalCoolingLoadLT = 0.0;
            TotalCondDefrostCredit = 0.0;
            NetHeatRejectLoad = 0.0;
            NetHeatRejectEnergy = 0.0;
            AverageCompressorCOP = 0.0;
            TotCompCapacityHP = 0.0;
            TotCompCapacityLP = 0.0;
            TotCompElecConsump = 0.0;
            TotCompPowerHP = 0.0;
            TotCompPowerLP = 0.0;
            TotCompCoolingEnergy = 0.0;
        }

        void CalcDetailedTransSystem(EnergyPlusData &state, int SysNum);

        void CalcGasCooler(EnergyPlusData &state, int SysNum);

        void CalculateTransCompressors(EnergyPlusData &state);
    };

    struct CaseAndWalkInListDef // Derived Type for CaseAndWalkIn Lists
    {
        std::string Name;          // Name of this CaseAndWalkIn List
        int NumCases = 0;          // Number of Cases in this CaseAndWalkIn List
        int NumCoils = 0;          // Number of Coils in this CaseAndWalkIn List
        int NumWalkIns = 0;        // Number of WalkIns in this CaseAndWalkIn List
        Array1D_int CaseItemNum;   // List of Item numbers that correspond to each Case
        Array1D_int CoilItemNum;   // List of Item numbers that correspond to each Coil
        Array1D_int WalkInItemNum; // List of Item numbers that correspond to each WalkIn
    };

    struct CompressorListDef // Derived Type for Compressor Lists
    {
        std::string Name;        // Name of this Compressor List
        int NumCompressors = 0;  // Number of Compressors in this Node List
        Array1D_int CompItemNum; // List of Item numbers that correspond to the compressors
    };

    struct RefrigCondenserData : PlantComponent
    {
        std::string Name;                          // Name of condenser
        std::string SupplyTankName;                // Evap water supply tank name
        std::string EndUseSubcategory = "General"; // Rack end-use subcategory
        bool CondenserRejectHeatToZone = false;    // Flag to show air-cooled condenser located inside zone
        bool CoilFlag = false;                     // Flag to show if coil type load on system served by condenser
        Array1D_int SysNum;                        // absolute Index of system placing load (allocated NumRefrigSystems)
        int NumSysAttach = 0;                      // Number of systems attached to condenser, error if /=1
        DataHeatBalance::RefrigCondenserType CondenserType = DataHeatBalance::RefrigCondenserType::Invalid; // Specifies cooling mode for condenser
        int EvapFreezeWarnIndex = 0;                                                                        // Recurring freeze warning index
        CndsrFlowType FlowType = CndsrFlowType::Variable;                                                   // Water-cooled condenser loop flow type
        int CondCreditWarnIndex1 = 0;                                                                       // Used to count warnings
        int CondCreditWarnIndex2 = 0;                                                                       // Used to count warnings
        int CondCreditWarnIndex3 = 0;                                                                       // Used to count warnings
        int CondCreditWarnIndex4 = 0;                                                                       // Used to count warnings
        int CondCreditWarnIndex5 = 0;                                                                       // Used to count warnings
        int CondCreditWarnIndex6 = 0;                                                                       // Used to count warnings
        int CondCreditWarnIndex7 = 0;                                                                       // Used to count warnings
        int NoFlowWarnIndex = 0;                                          // No cooling water when needed warning index
        int HighTempWarnIndex = 0;                                        // Water outlet high temp warning index
        int LowTempWarnIndex = 0;                                         // Water outlet low temp warning index
        int HighFlowWarnIndex = 0;                                        // Water outlet high flow warning index
        int HighInletWarnIndex = 0;                                       // Water inlet high temp warning index
        int InletNode = 0;                                                // Water-cooled condenser inlet node number
        Sched::Schedule *evapAvailSched = nullptr;                        // Evap condenser availability schedule
        WaterSupply EvapWaterSupplyMode = WaterSupply::FromMains;         // Source of water for evap condenser cooling
        int EvapWaterSupTankID = 0;                                       // TankID when evap condenser uses water from storage tank
        int EvapWaterTankDemandARRID = 0;                                 // Demand index when evap condenser uses water from storage tank
        int OutletNode = 0;                                               // Water-cooled condenser outlet node number
        int PlantTypeOfNum = 0;                                           // Water-cooled condenser plant equipment type
        PlantLocation plantLoc;                                           // Water-cooled condenser plant location
        Sched::Schedule *outletTempSched = nullptr;                       // Schedule for condenser outlet temp setting
        int InletAirNodeNum = 0;                                          // Inlet air node number, can be outside or in a zone
        int InletAirZoneNum = 0;                                          // Inlet air zone number, if located in a zone
        FanSpeedCtrlType FanSpeedControlType = FanSpeedCtrlType::Invalid; // fixed, two-speed, or variable
        int CapCurvePtr = 0;                                              // capacity curve pointer for air-cooled condensers
        int CascadeSysID = 0;                                             // System ID number for system rejecting heat to cascade condenser
        CascadeCndsrTempCtrlType CascadeTempControl =
            CascadeCndsrTempCtrlType::Invalid; // Determines whether cascade condenser evaporating temperature set by
        // Tevap for other loads on system (=2) or set at a constant (= 1)
        int CascadeSinkSystemID = 0; // System ID number for system absorbing condenser heat
        // INTEGER     :: ServiceType      = 1       ! Index to warehouse or supermarket (only applies to cascade condensers)
        // 1 = supermarket, 2=warehouse
        Real64 CascadeRatedEvapTemp = 0.0;      // Rated evaporating temperature in cascade condenser
        Real64 MinCondLoad = 0.0;               // minimum condenser load for air-cooled cond (W)
        Real64 TempSlope = 0.0;                 // slope for deltaT as function of heat rej for air-cooled cond (C/W)
        Real64 EvapEffect = 0.0;                // Effectiveness of evaporative condenser
        Real64 RatedAirFlowRate = 0.0;          // Evaporative condenser air volume flow rate (m3/s)
        Real64 EvapPumpPower = 0.0;             // Evaporative cooling water pump power (W)
        Real64 ActualEvapPumpPower = 0.0;       // Evaporative cooling water pump power, if adjusted (W)
        Real64 EvapPumpConsumption = 0.0;       // Evaporative cooling water pump electric consumption (J)
        Real64 EvapWaterConsumpRate = 0.0;      // Evaporative condenser water consumption rate (m3/s)
        Real64 EvapWaterConsumption = 0.0;      // Evaporative condenser water consumption (m3)
        Real64 BasinHeaterPowerFTempDiff = 0.0; // Basin heater capacity per degree K below setpoint (W/K)
        Real64 BasinHeaterSetPointTemp = 0.0;   // Setpoint temperature for basin heater operation (C)
        Real64 BasinHeaterPower = 0.0;          // Power demand from basin heater (W)
        Real64 BasinHeaterConsumption = 0.0;    // Electric consumption from basin heater (J)
        Real64 FanMinAirFlowRatio = 0.0;        // Minimum power fraction for fan (dimensionless between 0 and 1.0)
        Real64 RatedFanPower = 0.0;             // Rated Condenser fan power (W)
        Real64 ActualFanPower = 0.0;            // Condenser fan power (W)
        Real64 FanElecEnergy = 0.0;             // Condenser fan electric consumption (J)
        Real64 InletTemp = 0.0;                 // Water-cooling condenser inlet temperature (C)
        Real64 OutletTemp = 0.0;                // Water-cooling condenser outlet temperature (C)
        Real64 VolFlowRate = 0.0;               // Water-cooled condenser volumetric flow rate (m3/s)
        Real64 DesVolFlowRate = 0.0;            // Water-cooled condenser design volumetric flow rate (m3/s)
        Real64 MassFlowRate = 0.0;              // Water-cooled condenser water mass flow rate (kg/s)
        Real64 RatedTCondense = 0.0;            // Condenser rated saturated condensing Temperature (C)
        Real64 CondLoad = 0.0;                  // Total condenser load (W)
        Real64 CondEnergy = 0.0;                // Condenser energy (J)
        Real64 VolFlowRateMax = 0.0;            // Maximum condenser volumetric flow rate (m3/s)
        Real64 MassFlowRateMax = 0.0;           // Maximum condenser mass flow rate (kg/s)
        Real64 InletTempMin = 0.0;              // Minimum condenser water inlet temperature (C)
        Real64 OutletTempMax = 0.0;             // Maximum condenser water outlet temperature (C)
        Real64 RatedSubcool = 0.0;              // Subcooling included in capacity rating curves (C)
        Real64 RatedDelT = 0.0;                 // Rated difference between Tcondense and Tdrybulb for air-cooled (C)
        // Rated difference between Tcondense and Twetbulb for evap-cooled (C)
        Real64 RatedCapacity = 0.0;             // Rated heat rejection capacity (W)
        Real64 RatedWaterInletT = 0.0;          // Rated water inlet temperature (C)
        Real64 RatedApproachT = 0.0;            // Rated approach temperature difference for water-cooled or cascade condenser(C)
        Real64 MinCapFacEvap = 0.0;             // HRCF equation limit
        Real64 MaxCapFacEvap = 0.0;             // HRCF equation limit
        Real64 EvapCoeff1 = 0.0;                // First coefficient in evap condenser approach T difference equn (C)
        Real64 EvapCoeff2 = 0.0;                // Second coefficient in evap condenser approach T difference equn (C)
        Real64 EvapCoeff3 = 0.0;                // Third coefficient in evap condenser approach T difference equn (C)
        Real64 EvapCoeff4 = 0.0;                // Fourth coefficient in evap condenser approach T difference equn (dimensionless)
        Real64 EvapElevFact = 0.0;              // Elevation correction factor for evap condensers
        Real64 RefOpCharge = 0.0;               // Condenser refrigerant operating charge, kg
        Real64 RefReceiverInventory = 0.0;      // Condensate receiver refrigerant inventory, kg
        Real64 RefPipingInventory = 0.0;        // Condensate piping refrigerant inventory, kg
        Real64 TotalHeatRecoveredEnergy = 0.0;  // All recovered heat for external loads and defrost purposes, J
        Real64 TotalHeatRecoveredLoad = 0.0;    // All recovered heat for external loads and defrost purposes [W]
        Real64 ExternalEnergyRecovered = 0.0;   // ExternalHeatRecovered, J
        Real64 InternalEnergyRecovered = 0.0;   // InternalHeatRecovered, J
        Real64 ExternalHeatRecoveredLoad = 0.0; // Sum of LaggedUsedWaterHeater and LaggedUsedHVACCoil [W]
        Real64 InternalHeatRecoveredLoad = 0.0; // Sum of all heat recovered for defrost purposes [W]
        Real64 LaggedUsedWaterHeater = 0.0;     // Heat reclaim used to heat water in previous zone/load time step(W)
        Real64 LaggedUsedHVACCoil = 0.0;        // Heat reclaim used to heat HVAC coil in previous zone/load time step(W)

        // Reset Initialization Values to Zeros
        void reset_init()
        {
            CondLoad = 0.0;
            CondEnergy = 0.0;
            MassFlowRate = 0.0;
            ActualFanPower = 0.0;
            FanElecEnergy = 0.0;
            EvapWaterConsumpRate = 0.0;
            EvapWaterConsumption = 0.0;
            ActualEvapPumpPower = 0.0;
            EvapPumpConsumption = 0.0;
            ExternalHeatRecoveredLoad = 0.0;
            ExternalEnergyRecovered = 0.0;
            InternalHeatRecoveredLoad = 0.0;
            InternalEnergyRecovered = 0.0;
            TotalHeatRecoveredLoad = 0.0;
            TotalHeatRecoveredEnergy = 0.0;
        }

        void UpdateCondenserOutletNode(EnergyPlusData &state) const;

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, const PlantLocation &calledFromLocation) override;

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void oneTimeInit(EnergyPlusData &state) override;

        void oneTimeInit_new(EnergyPlusData &state) override;
    };

    struct RefrigGasCoolerData
    {
        std::string Name;                          // Name of gas cooler
        std::string EndUseSubcategory = "General"; // Gas cooler end-use subcategory
        bool GasCoolerRejectHeatToZone = false;    // Flag to show gas cooler located inside zone
        bool TransOpFlag = false;                  // Flag to show transcritical (vs subcritical) operation of the refrigeration system
        Array1D_int SysNum;                        // absolute Index of system placing load (allocated NumRefrigSystems)
        int CapCurvePtr = 0;                       // capacity curve pointer for gas cooler
        FanSpeedCtrlType FanSpeedControlType = FanSpeedCtrlType::Invalid; // fixed, two-speed, or variable
        int GasCoolerCreditWarnIndex = 0;                                 // Used to count warnings
        int InletAirNodeNum = 0;                                          // Inlet air node number, can be outside or in a zone
        int InletAirZoneNum = 0;                                          // Inlet air zone number, if located in a zone
        int NumSysAttach = 0;                                             // Number of systems attached to gas cooler
        Real64 ActualFanPower = 0.0;                                      // Actual gas cooler fan power (W)
        Real64 CpGasCoolerOut = 0.0;                                      // Saturated liquid specific heat at gas cooler outlet (J/kg-C)
        Real64 FanElecEnergy = 0.0;                                       // Gas cooler fan electric consumption (J)
        Real64 FanMinAirFlowRatio = 0.0;                                  // Minimum power fraction for fan (dimensionless between 0 and 1.0)
        Real64 GasCoolerApproachT = 3.0;                                  // Gas cooler approach temperature (C)
        Real64 GasCoolerEnergy = 0.0;                                     // Gas cooler energy (J)
        Real64 GasCoolerLoad = 0.0;                                       // Total gas cooler load (W)
        Real64 HGasCoolerOut = 0.0;                                       // Specific enthalpy at the gas cooler outlet (C)
        Real64 InternalEnergyRecovered = 0.0;                             // InternalHeatRecovered, J
        Real64 InternalHeatRecoveredLoad = 0.0;                           // Sum of all heat recovered for defrost purposes [W]
        Real64 MinCondLoad = 0.0;                                         // minimum gas cooler load for air-cooled gas cooler (W)
        Real64 MinCondTemp = 1.0e1;                                       // Minimum condensing temperature during subcritical operation (C)
        Real64 PGasCoolerOut = 0.0;                                       // Optimum pressure at the gas cooler outlet (C)
        Real64 RatedApproachT = 3.0;                                      // Rated approach temperature difference(C)
        Real64 RatedCapacity = 0.0;                                       // Rated heat rejection capacity (W)
        Real64 RatedFanPower = 0.0;                                       // Rated gas cooler fan power (W)
        Real64 RatedOutletP = 9.0e6;                                      // Rated gas cooler outlet pressure (Pa)
        Real64 RatedOutletT = 38.0;                                       // Rated gas cooler outlet temperature (C)
        Real64 RefOpCharge = 0.0;                                         // Gas cooler refrigerant operating charge, kg
        Real64 RefPipingInventory = 0.0;                                  // Gas cooler outlet piping refrigerant inventory, kg
        Real64 RefReceiverInventory = 0.0;                                // Gas cooler receiver refrigerant inventory, kg
        Real64 SubcriticalTempDiff = 1.0e1;                               // Temperature difference for subcritical operation (C)
        Real64 TempSlope = 0.0;                                           // slope for deltaT as function of heat rej for gas cooler (C/W)
        Real64 TGasCoolerOut = 0.0;                                       // Temperature at the gas cooler outlet (C)
        Real64 TotalHeatRecoveredEnergy = 0.0;                            // All recovered heat for defrost purposes, J
        Real64 TotalHeatRecoveredLoad = 0.0;                              // All recovered heat for defrost purposes [W]
        Real64 TransitionTemperature = 0.0; // Transition temperature between subcritical and transcritical operation (C)

        // Reset Initialization Values to Zeros
        void reset_init()
        {
            GasCoolerLoad = 0.0;
            GasCoolerEnergy = 0.0;
            ActualFanPower = 0.0;
            FanElecEnergy = 0.0;
            InternalHeatRecoveredLoad = 0.0;
            InternalEnergyRecovered = 0.0;
            TotalHeatRecoveredLoad = 0.0;
            TotalHeatRecoveredEnergy = 0.0;
        }
    };

    struct RefrigCompressorData
    {
        bool CoilFlag = false;                                        // Flag to show if coil type load on system served by compressor
        std::string Name;                                             // Name of compressor
        int CapacityCurvePtr = 0;                                     // Index to the capacity curve object
        int ElecPowerCurvePtr = 0;                                    // Index to the electrical power curve object
        int MassFlowCurvePtr = 0;                                     // Index to the mass flow curve object
        int TransElecPowerCurvePtr = 0;                               // Index to the transcritical electrical power curve object
        int TransCapacityCurvePtr = 0;                                // Index to the transcritical capacity curve object
        int NumSysAttach = 0;                                         // Number of systems attached to compressor, error if /=1
        CompRatingType SuperheatRatingType = CompRatingType::Invalid; // Type of manufacturer's rating info re superheat
        CompRatingType SubcoolRatingType = CompRatingType::Invalid;   // Type of manufacturer's rating info re subcooling
        Real64 Capacity = 0.0;                                        // Comprssor delivered capacity (W)
        Real64 CoolingEnergy = 0.0;                                   // Compressor delivered energy (J)
        Real64 Efficiency = 0.0;                                      // Compressor efficiency (0 to 1)
        Real64 ElecConsumption = 0.0;                                 // Compressor electric consumption (J)
        Real64 LoadFactor = 0.0;                                      // Fraction of the time the compressor runs to meet the load (0 to 1)
        Real64 MassFlow = 0.0;                                        // Compressor mass flow (kg/s)
        Real64 NomCap = 0.0;                                          // Nominal compressor capacity at ARI 540 rating conditions
        Real64 Power = 0.0;                                           // Compressor power (W)
        Real64 RatedSuperheat = 0.0;                                  // Rated Superheat at compressor suction (C)
        Real64 RatedSubcool = 0.0;                                    // Rated Subcooling, note may not match condenser rating (C)
        std::string EndUseSubcategory = "General";                    // Compressor end-use subcategory
        bool TransFlag = false;                                       // Flag to indicate if compressor can operate in transcritical region

        // Reset Initialization Values to Zeros
        void reset_init()
        {
            ElecConsumption = 0.0;
            Power = 0.0;
        }
    };

    struct CaseRAFractionData
    {
        Real64 TotalCaseRAFraction = 0.0; // Sum case return air fraction for error checking
        std::string ZoneName;             // Zone or Location of Refrigerated Case
    };

    struct SubcoolerData
    {
        bool CoilFlag = false;     // Flag to show if coil type load on system served by subcooler
        std::string Name;          // Name of Subcooler
        std::string MechSourceSys; // Name of refrigeration system providing
        // cool liquid to mechanical, needed for character comparison after systems read
        SubcoolerType subcoolerType = SubcoolerType::Invalid; // Specifies subcooler type(0=liquid suction heat exchanger,1=mechanical)
        int MechSourceSysID = 0;                              // ID number of refrigeration system providing cool liquid to mechanical
        Real64 MechSCTransLoad = 0.0;                         // Mechanical subcooler load transferred between suction groups, W
        Real64 MechSCTransEnergy = 0.0;                       // Mechanical subcooler energy transferred between suction groups, W
        Real64 LiqSuctDesignDelT = 0.0;                       // Liquid suction subcooler design subcooling, C
        Real64 LiqSuctDesignTliqIn = 0.0;                     // Liquid suction subcooler design inlet temperature liquid, C
        Real64 LiqSuctDesignTvapIn = 0.0;                     // Liquid suction subcooler design inlet temperature vapor, C
        Real64 MechControlTliqOut = 0.0;                      // Mechanical subcooler design outlet temperature subcooled liquid, C
    };

    struct SecondaryLoopData
    {
        bool CoilFlag = false; // Flag to show if coil type load on secondary system
        std::string Name;      // Name of refrigeration system
        std::string FluidName; // Name of circulating fluid
        Fluid::GlycolProps *glycol = nullptr;
        Fluid::RefrigProps *refrig = nullptr;
        std::string EndUseSubcategory;                      // Used for reporting purposes
        Array1D_int CaseNum;                                // Absolute Index of cases (dimensioned 1 to NumCases)
        Array1D_int CoilNum;                                // Absolute Index of coils (dimensioned 1 to NumCoils)
        Array1D_int WalkInNum;                              // Absolute Index of walk-ins (dimensioned 1 to NumWalkIns)
        int DistPipeZoneNum = 0;                            // ID number for zone where distribution pipe gain heat
        int DistPipeZoneNodeNum = 0;                        // ID number for zone node where distribution pipe gain heat
        Real64 DistPipeZoneHeatGain = 0.0;                  // ! sensible heat gain rate to zone with pipe
        SecFluidType FluidType = SecFluidType::Invalid;     // Indicates whether fluid always liquid or undergoes phase change
        int NumSysAttach = 0;                               // Used to check for non-unique and unused secondary loops
        int NumPumps = 0;                                   // Number of pumps (or pump stages) serving this system
        int NumCases = 0;                                   // Number of Cases served by this secondary loop
        int NumCoils = 0;                                   // Number of Cases served by this secondary loop
        int NumWalkIns = 0;                                 // Number of Walk-Ins served by this secondary loop
        SecPumpCtrl PumpControlType = SecPumpCtrl::Invalid; // Constant speed or variable speed
        int ReceiverZoneNum = 0;                            // ID number for zone where receiver gains heat
        int ReceiverZoneNodeNum = 0;                        // ID number for zone node where receiver gains heat
        Real64 ReceiverZoneHeatGain = 0.0;                  // sensible heat gain rate to zone with receiver
        int VarSpeedCurvePtr = 0;                           // Pointer for variable speed pump power curve
        Real64 AvailLoadCoils = 0.0;                        // Used to determine amount of avail heat for warehouse coils
        Real64 CpBrineRated = 0.0;                          // Specific heat of secondary loop fluid at rated average
        //    brine temperature (J/kg-C)
        Real64 ChillerRefInventory = 0.0; // Refrigerant inventory on cold side of loop heat exchanger
        Real64 CircRate = 0.0;            // For PhaseChange loop = mass flow at pump/mass gas out load (dimensionless)
        Real64 CoolingLoadRated = 0.0;    // Rated capacity of heat exchanger serving secondary loop (W)
        Real64 DensityBrineRated = 0.0;   // Density of secondary loop fluid at
        //    rated average brine temperature (J/kg-C)
        Real64 DistPipeHeatGain = 0.0;       // Secondary fluid distribution piping heat gain (W)
        Real64 DistPipeHeatGainEnergy = 0.0; // Secondary fluid distribution piping heat gain (J)
        Real64 FlowVolActual = 0.0;          // Actual Mass flow rate of circ fluid(kg/s)
        Real64 HotDefrostCondCredit = 0.0;   // Used to credit condenser when heat reclaim used for hot gas/brine defrost (W)
        Real64 HeatExchangeEta = 0.0;        // Heat exchanger effectiveness (dimensionless)
        Real64 MaxVolFlow = 0.0;             // Defined by minimum of chiller or pump ratings (m3/s)
        Real64 MaxLoad = 0.0;                // Defined by minimum of chiller rating or loat at MaxVolFlow (W)
        Real64 PumpTotRatedPower = 0.0;      // Total pump rated power on loop (W)
        Real64 PumpPowerToHeat = 0.0;        // Fraction of pump power converted to heat in circ fluid (dimensionless)
        Real64 PumpIncrementFlowVol = 0.0;   // Circ fluid flow for each pump or pump stage (m3/s)
        Real64 PumpIncrementPower = 0.0;     // Pump power for each pump or pump stage (W)
        Real64 PumpPowerTotal = 0.0;         // Total Pump Power Secondary Loop (report variable)(W)
        Real64 PumpElecEnergyTotal = 0.0;    // Total pump energy secondary loop (report variable)(W)
        Real64 ReceiverHeatGain = 0.0;       // Secondary fluid Receiver heat gain (W)
        Real64 ReceiverHeatGainEnergy = 0.0; // Secondary fluid Receiver heat gain (J)
        Real64 RefInventory = 0.0;           // Approximate refrigerant inventory entered by user (kg)
        Real64 SumUADistPiping = 0.0;        // Sum of U*A for secondary fluid dist piping (W/C)
        Real64 SumUAReceiver = 0.0;          // Sum of U*A for secondary fluid receiver (W/C)
        Real64 TBrineAverage = 0.0;          // (C)
        Real64 TBrineInRated = 0.0;          // Entering brine temperature based upon rated range,approach,
        //    and evap Temp (C)
        Real64 TCondense = 0.0; // Rated condensing temperature for heat exchanger serving
        //    secondary loop with phase change(C)
        Real64 TEvapDesign = 0.0; // Rated evaporating temperature for heat exchanger serving
        //    secondary loop (C)
        Real64 TApproachDifRated = 0.0; // Rated approach temperature diff for heat exchanger serving
        //    secondary loop (C)
        Real64 TRangeDifRated = 0.0; // Rated range temperature diff for heat exchanger serving
        //    secondary loop (C)
        Real64 TMinNeeded = 0.0;       // Lowest Tbrine to case or walk-in needed on loop (C)
        Real64 TotalCoolingLoad = 0.0; // Total load (cases + walk-ins + pump heat + distribution pipe heat gain)
        //     on this system (W)
        Real64 TotalCoolingEnergy = 0.0; // Total energy (cases + walk-ins + pump heat + distribution pipe heat gain)
        //    on this system (J)
        Real64 TotalRefrigLoad = 0.0;   // Total load (cases + walk-ins) on this system (W)
        Real64 TotalRefrigEnergy = 0.0; // Total energy (cases + walk-ins) on this system (J)
        Real64 UnmetEnergy = 0.0;       // Load that is greater than capacity of loop heat exchanger, accumulates (J)
        Real64 UnmetEnergySaved = 0.0;  // Load that is greater than capacity of loop heat exchanger, accumulates (J)

        // Reset Initialization Values to Zeros
        void reset_init()
        {
            TotalCoolingLoad = 0.0;
            PumpPowerTotal = 0.0;
            PumpElecEnergyTotal = 0.0;
            ReceiverZoneHeatGain = 0.0;
            DistPipeZoneHeatGain = 0.0;
        }

        void CalculateSecondary(EnergyPlusData &state, int SecondaryNum);
    };

    struct TransferLoadListDef // Derived Type for Transfer Load (Secondary and Cascade) Lists
    {
        std::string Name;               // Name of this TransferLoad List
        int NumSecondarys = 0;          // Number of Secondary Loops in this TransferLoad List
        int NumCascadeLoads = 0;        // Number of Cascade condenser loads in this TransferLoad List
        Array1D_int CascadeLoadItemNum; // List of Item numbers that correspond to the Cascade Condenser
        Array1D_int SecondaryItemNum;   // List of Item numbers that correspond to the Secondary
    };

    struct WalkInData
    {
        std::string Name; // Name of walk in cooler
        Array1D_string ZoneName;
        // Names of zones exchanging energy with cooler
        Sched::Schedule *circFanAvailSched = nullptr;                  // Index to the correct availability schedule
        Sched::Schedule *defrostDripDownSched = nullptr;               // fail-safe schedule
        Sched::Schedule *defrostSched = nullptr;                       // defrost schedule
        DefrostCtrlType DefrostControlType = DefrostCtrlType::Invalid; // WalkIn defrost control type, Timed,Frost level
        DefrostType defrostType = DefrostType::Invalid;                // WalkIn defrost type, Hot-gas,Electric, Hot-brine
        Sched::Schedule *heaterSched = nullptr;                        // heater availability schedule
        Sched::Schedule *lightingSched = nullptr;                      // walkIn lighting schedule
        int NumSysAttach = 0;                                          // Number of systems attached to WalkIn, error if /=1
        int NumZones = 0;                                              // Number of zones exchanging energy with WalkIn
        Sched::Schedule *availSched = nullptr;                         // availability schedule
        Sched::Schedule *stockingSched = nullptr;                      // product stocking schedule
        Array1D<Sched::Schedule *> glassDoorOpenScheds;                // door opening schedule
        Array1D<Sched::Schedule *> stockDoorOpenScheds;                // door opening schedule
        Array1D<WIStockDoor> StockDoorProtectType;                     // Index to door protection type
        Array1D_int ZoneNodeNum;                                       // Index to Zone Node
        Array1D_int ZoneNum;                                           // Index to Zone
        Real64 CircFanPower = 0.0;                                     // Operating power of  Walk In fan [W]
        Real64 CoilFanPower = 0.0;                                     // Operating power of  Walk In evap coil fan [W]
        Real64 IceTemp = 0.0;                                          // Temperature of Ice Mass [C]
        Real64 IceTempSaved = 0.0;                                     // Temperature of Ice Mass [C]
        Real64 DefrostCapacity = 0.0;                                  // Design defrost WalkIn capacity [W]
        Real64 DeltaFreezeKgFrost = 0.0;                               // Used to reverse accumulation if the zone/load time step is repeated (kg)
        Real64 DefEnergyFraction = 0.0;                                // Portion of defrost energy available to melt ice,
        //    used with fluid defrost with temp termination (dimensionless)
        Real64 DesignFanPower = 0.0;        // Design power of fans [W]
        Real64 DesignLighting = 0.0;        // Design  lighting (includes task and display lights)[W]
        Real64 DesignRatedCap = 0.0;        // Design total capacity [W]
        Real64 DesignRefrigInventory = 0.0; // Design refrigerant inventory [kg]
        Real64 FloorArea = 0.0;             // Floor area of  Walk In [m2]
        Real64 FloorUValue = 0.0;           // U-value of Walk In floor [W/m2-C]
        Real64 HeaterPower = 0.0;           // Rated power of  Walk In   heaters [W/m]
        Real64 HotDefrostCondCredit = 0.0;  // Used to credit condenser when heat reclaim used for hot gas/brine defrost (W)
        Real64 KgFrost = 0.0;               // Amount of frost on WalkIn evaporator (Kg)
        Real64 StoredEnergy = 0.0;          // Cumulative Stored Energy not met by evaporator [J]
        Real64 KgFrostSaved = 0.0;          // Amount of frost on WalkIn evaporator (Kg)
        Real64 StoredEnergySaved = 0.0;     // Cumulative Stored Energy not met by evaporator [J]
        Real64 Temperature = 0.0;           // Rated temperature [C]
        Real64 TEvapDesign = 0.0;           // Design evaporator temperature (or brine inlet T) [C]
        Real64 TotalFanPower = 0.0;         // Sum of coil and circ fan power  [W]
        Array1D<Real64> AreaGlassDr;
        Array1D<Real64> UValueGlassDr;
        Array1D<Real64> HeightGlassDr;
        Array1D<Real64> AreaStockDr;
        Array1D<Real64> UValueStockDr;
        Array1D<Real64> HeightStockDr;
        Array1D<Real64> SurfaceArea;
        Array1D<Real64> UValue;
        // Report Variables
        Real64 ElecHeaterPower = 0.0;         // Walk In heater rate (W)
        Real64 ElecHeaterConsumption = 0.0;   // Walk In heater energy (J)
        Real64 ElecFanPower = 0.0;            // Walk In fan electric power (W)
        Real64 ElecFanConsumption = 0.0;      // Walk In fan electric energy (J)
        Real64 ElecLightingPower = 0.0;       // Walk In lighting electric power (W)
        Real64 ElecLightingConsumption = 0.0; // Walk In lighting electric energy (J)
        Real64 ElecDefrostPower = 0.0;        // Walk In defrost rate (W)
        Real64 ElecDefrostConsumption = 0.0;  // Walk In defrost energy (J)
        Real64 TotalCoolingLoad = 0.0;        // Walk In total cooling rate (W)
        Real64 TotalCoolingEnergy = 0.0;      // Walk In total cooling energy (J)
        Real64 TotalElecPower = 0.0;          // Walk In total electric
        //   (fans, heaters, lighting, and elec defrost) rate (W)
        Real64 TotalElecConsumption = 0.0;      // Walk In total electric energy (J)
        Real64 TotLatCoolingEnergyRate = 0.0;   // Walk In latent cooling rate (W)
        Real64 TotLatCoolingEnergy = 0.0;       // Walk In latent cooling energy (J)
        Real64 TotSensCoolingEnergyRate = 0.0;  // Walk In sensible cooling rate (W)
        Real64 TotSensCoolingEnergy = 0.0;      // Walk In sensible cooling energy (J)
        Array1D<Real64> LatZoneCreditRate;      // Amount of latent energy provided to zone(W)
        Array1D<Real64> LatZoneCredit;          // Amount of latent energy provided to zone(J)
        Array1D<Real64> SensZoneCreditRate;     // Amount of sensible heat gain to zone, pos and neg (W)
        Array1D<Real64> SensZoneCreditCoolRate; // Amount of sensible cooling provided to the zone (W)
        Array1D<Real64> SensZoneCreditCool;     // Amount of sensible cooling provided to the zone (J)
        Array1D<Real64> SensZoneCreditHeatRate; // Amount of sensible heat provided to the zone (W)
        Array1D<Real64> SensZoneCreditHeat;     // Amount of sensible heat provided to the zone (J)
        bool ShowUnmetWIEnergyWarning = true;
        bool ShowWIFrostWarning = true;

        // Reset Initialization Values to Zeros
        void reset_init()
        {
            HotDefrostCondCredit = 0.0;
            TotalCoolingLoad = 0.0;
            TotalCoolingEnergy = 0.0;
            TotSensCoolingEnergyRate = 0.0;
            TotSensCoolingEnergy = 0.0;
            TotLatCoolingEnergyRate = 0.0;
            TotLatCoolingEnergy = 0.0;
            ElecFanPower = 0.0;
            ElecFanConsumption = 0.0;
            ElecHeaterPower = 0.0;
            ElecHeaterConsumption = 0.0;
            ElecLightingPower = 0.0;
            ElecLightingConsumption = 0.0;
            TotalElecPower = 0.0;
            TotalElecConsumption = 0.0;
            ElecDefrostPower = 0.0;
            ElecDefrostConsumption = 0.0;
        }

        void CalculateWalkIn(EnergyPlusData &state);
    };

    struct CaseWIZoneReportData
    {
        // Members
        Real64 LatCoolingToZoneRate = 0.0; // Positive for reporting Net latent credit to zone on sys time step from cases/walkins (W)
        Real64 LatCoolingToZoneEnergy = 0.0;
        Real64 SenCoolingToZoneRate = 0.0; // Positive for reporting Net sensible cooling to zone on sys time step from cases/walkins (W)
        Real64 SenCoolingToZoneEnergy = 0.0;
        Real64 HeatingToZoneRate = 0.0; // Positive for reporting Net sensible credit to zone on sys time step from cases/walkins (W)
        Real64 HeatingToZoneEnergy = 0.0;
        Real64 TotCoolingToZoneRate = 0.0; // Positive for reporting Net total cooling credit to zone from cases/walkins (W)
        Real64 TotCoolingToZoneEnergy = 0.0;
        Real64 TotHtXferToZoneRate = 0.0; // Gives negative for cooling, positive for heating net to zone from cases/walkins (W)
        Real64 TotHtXferToZoneEnergy = 0.0;
        Real64 SenCaseCreditToZoneEnergy = 0.0; // Negative (heat out zone) positive (heat into zone) (rate found in CaseCreditData) (J)

        // Reset to Zeros
        void reset()
        {
            new (this) CaseWIZoneReportData();
        }
    };

    struct WarehouseCoilData
    {
        std::string Name;                                              // Name of Warehouse Coil
        std::string ZoneName;                                          // Names of zone cooled by coil
        bool SecStatusFirst = false;                                   // Flag to show if this is the first coil on a particular secondary
        bool SecStatusLast = false;                                    // Flag to show if this is the last coil on a particular secondary
        bool SysStatusFirst = false;                                   // Flag to show if this is the first coil on a particular primary
        bool SysStatusLast = false;                                    // Flag to show if this is the last coil on a particular primary
        Sched::Schedule *coilFanAvaildSched = nullptr;                 // availability schedule
        Sched::Schedule *defrostDripDownSched = nullptr;               // fail-safe schedule
        Sched::Schedule *defrostSched = nullptr;                       // defrost schedule
        DefrostCtrlType DefrostControlType = DefrostCtrlType::Invalid; // Coil defrost control type, Timed,Frost level
        DefrostType defrostType = DefrostType::Invalid;                // Coil defrost type, Hot-gas,Electric, Hot-brine
        FanSpeedCtrlType FanType = FanSpeedCtrlType::Invalid;          // Index to coil fan type (fixed, two-speed, etc.)
        Sched::Schedule *heaterAvailSched = nullptr;                   // availability schedule
        int NumSysAttach = 0;                                          // Number of refrigerating systems cooling this coil (error check purpose)
        RatingType ratingType = RatingType::Invalid;                   // Indicates which type of manufacturer's rating is used
        Sched::Schedule *availSched = nullptr;                         // availability schedule
        int SCIndex = 0;                                               // IDs which of European standard conditions is used for rating
        int SecServeID = 0;                                            // Index to the refrigeration system serving this coil
        SHRCorrectionType SHRCorrType = SHRCorrectionType::Invalid;    // Index to type of correction for sensible heat ratio
        int SHRCorrectionCurvePtr = 0;                                 // Index to Sensible heat ratio correction curve
        int SysServeID = 0;                                            // Index to the secondary system serving this coil
        VerticalLoc VerticalLocation = VerticalLoc::Invalid;           // Index to coil location, floor, ceiling, or middle
        int ZoneNodeNum = 0;                                           // Index to the zone node for the zone served by this coil
        int ZoneNum = 0;                                               // Index to the zone served by this coil
        Real64 CorrMaterial = 0.0;                                     // Correction factor from manufacturer's rating for coil material, default 1.0
        Real64 CorrRefrigerant = 0.0;                                  // Correction factor from manufacturer's rating for refrigerant, default 1.0
        Real64 DefrostCapacity = 0.0;                                  // Design defrost Coil capacity [W]
        Real64 DefrostPower = 0.0;                                     // Defrost power for electric defrost (W)
        Real64 DeltaFreezeKgFrost = 0.0;                               // Used to reverse accumulation if the zone/load time step is repeated (kg)
        Real64 DefEnergyFraction = 0.0;                                // Portion of defrost energy available to melt ice,
        //    used with fluid defrost with temp termination (dimensionless)
        Real64 DesignRefrigInventory = 0.0; // Design refrigerant inventory [kg]
        Real64 FanMinAirFlowRatio = 0.0;    // Minimum air flow ratio set to preserve fan motor, dimensionless
        Real64 HeaterPower = 0.0;           // Rated power of  coil heaters [W/m]
        Real64 HotDefrostCondCredit = 0.0;  // Used to credit condenser when heat reclaim used for hot gas/brine defrost (W)
        Real64 IceTemp = 0.0;               // Temperature of Ice Mass [C]
        Real64 IceTempSaved = 0.0;          // Temperature of Ice Mass [C]
        Real64 KgFrost = 0.0;               // Amount of frost on coil evaporator (Kg)
        Real64 KgFrostSaved = 0.0;          // Amount of frost on coil evaporator (Kg)
        Real64 MaxTemperatureDif = 0.0;     // Maximum difference between Tevap and Tair inlet, limits capacity during initial pull-down (deltaC)
        Real64 RatedAirVolumeFlow = 0.0;    // Rated air flow through coil (m3/s)
        Real64 RatedCapTotal = 0.0;         // Rated total heat capacity, both latent and sensible [W]
        Real64 RatedFanPower = 0.0;         // Rated power of fans [W]
        Real64 RatedRH = 0.0;               // Rated RH corresponding to RatedCapacityTotal [decimal 0 to 1]
        Real64 RatedSensibleCap = 0.0;      // Rated total capacity at sensible heat ratio of 1.0 [W]
        Real64 RatedTemperatureDif = 0.0;   // Rated temperature difference DT1, T air in minus evaporating temperature [W]
        Real64 ReqLoad = 0.0;               // Load requested to meet zone load [W]
        Real64 SensHeatRatio = 0.0;         // Sensible heat ratio (sensible/total), dimensionless
        Real64 SHRCorrection60 = 0.0;       // Correction factor corresponding to sensible heat ratio of 0.6 [ dimensionless]
        Real64 Temperature = 0.0;           // Rated temperature [C]
        Real64 TEvapDesign = 0.0;           // Design evaporator temperature (or brine inlet T) [C]
        Real64 ThermalDefrostPower = 0.0;   // Thermal defrost load used to communicate with derate routine even if not electric defrost [W]
        Real64 UnitLoadFactorSens = 0.0;    // Rated sensible capacity [W/C]
        // Report Variables
        Real64 ElecHeaterPower = 0.0;             // Coil heater rate (W)
        Real64 ElecHeaterConsumption = 0.0;       // Coil heater energy (J)
        Real64 ElecFanPower = 0.0;                // Coil fan electric power (W)
        Real64 ElecFanConsumption = 0.0;          // Coil fan electric energy (J)
        Real64 ElecDefrostPower = 0.0;            // Coil defrost rate (W)
        Real64 ElecDefrostConsumption = 0.0;      // Coil defrost energy (J)
        Real64 LatCreditRate = 0.0;               // Latent heat removed from the zone [W]
        Real64 LatLoadServed = 0.0;               // Latent load met by coil (J)
        Real64 LatKgPerS_ToZone = 0.0;            // Latent load met by coil (kg/s)
        Real64 LatCreditEnergy = 0.0;             // Latent heat removed from the zone [J]
        Real64 ReportSensCoolCreditRate = 0.0;    // Coil cooling credit to zone (net) [W]
        Real64 ReportHeatingCreditRate = 0.0;     // Coil heating credit to zone (net) [J]
        Real64 ReportSensCoolCreditEnergy = 0.0;  // Coil cooling credit to zone (net) [W]
        Real64 ReportHeatingCreditEnergy = 0.0;   // Coil heating credit to zone (net) [J]
        Real64 ReportTotalCoolCreditRate = 0.0;   // Coil cooling sens + latent credit to zone[W]
        Real64 ReportTotalCoolCreditEnergy = 0.0; // Coil cooling sens + latent credit to zone[J]
        Real64 SensCreditRate = 0.0;              // Net Sensible heat removed from the zone [W]
        Real64 SensCreditEnergy = 0.0;            // Net Sensible heat removed from the zone [J]
        Real64 SensCoolingEnergyRate = 0.0;       // Gross Coil sensible cooling rate (W)
        Real64 SensCoolingEnergy = 0.0;           // Gross Coil sensible cooling energy (J)
        Real64 TotalCoolingLoad = 0.0;            // Gross total cooling rate (W)
        Real64 TotalCoolingEnergy = 0.0;          // Gross total cooling energy (J)
        Real64 TotalElecPower = 0.0;              // Coil total electric
        Real64 TotalElecConsumption = 0.0;        // Coil total electric energy (J)
        bool ShowCoilFrostWarning = true;

        // Reset Initialization Values to Zeros
        void reset_init()
        {
            HotDefrostCondCredit = 0.0;
            TotalCoolingLoad = 0.0;
            TotalCoolingEnergy = 0.0;
            SensCoolingEnergyRate = 0.0;
            SensCoolingEnergy = 0.0;
            SensCreditRate = 0.0;
            LatKgPerS_ToZone = 0.0;
            SensHeatRatio = 0.0;
            LatCreditEnergy = 0.0;
            LatCreditRate = 0.0;
            ElecFanPower = 0.0;
            ElecFanConsumption = 0.0;
            ElecHeaterPower = 0.0;
            ElecHeaterConsumption = 0.0;
            TotalElecPower = 0.0;
            TotalElecConsumption = 0.0;
            ElecDefrostPower = 0.0;
            ElecDefrostConsumption = 0.0;
            ReportTotalCoolCreditRate = 0.0;
            ReportTotalCoolCreditEnergy = 0.0;
            ReportSensCoolCreditRate = 0.0;
            ReportHeatingCreditRate = 0.0;
            ReportSensCoolCreditEnergy = 0.0;
            ReportHeatingCreditEnergy = 0.0;
        }

        void CalculateCoil(EnergyPlusData &state, Real64 QZnReq);
    };

    struct AirChillerSetData
    {
        std::string Name;     // Name of Chiller Set
        std::string ZoneName; // Name of zone where chiller set is located
        Array1D_int CoilNum;  // ID number of Individual Chiller in set
        int ChillerSetID = 0; // ID number for this set of chillers (all serving one zone,
        //                       but can be chilled by multi systems)
        Sched::Schedule *availSched = nullptr; // Schedule to take whole set off-line if needed // availability?
        int NodeNumInlet = 0;                  // Node ID Number of inlet for chiller set as a whole, not identified for specific coils
        int NodeNumOutlet = 0;                 // Node ID Number of outlet for chiller set as a whole, not identified for specific coils
        int NumCoils = 0;                      // Number of individual chillers in set
        int ZoneNum = 0;                       // ID number of zone where chiller set is located
        int ZoneNodeNum = 0;                   // ID number of zone node giving mixed conditions of zone where chiller set is located
        Real64 QZnReqSens = 0.0;               // Sensible heat needed by the zone to reach setpoint [W]

        void CalculateAirChillerSets(EnergyPlusData &state);
    };

    struct CoilCreditData // used to sum impact of all coils within a zone
    {
        Real64 LatCreditToZoneRate = 0.0; // Net latent credit to zone on sys time step from coil (W)
        Real64 LatCreditToZoneEnergy = 0.0;
        Real64 LatKgPerS_ToZoneRate = 0.0; // Latent water to zone on sys time step from coils, neg when water removed (kg/s)
        Real64 SenCreditToZoneRate = 0.0;  // Net sensible credit to zone on sys time step from coil (W)
        Real64 SenCreditToZoneEnergy = 0.0;
        Real64 ReportH2ORemovedKgPerS_FromZoneRate = 0.0; // same but positive for reporting purposes (kg/s)
        Real64 ReportLatCreditToZoneRate = 0.0;           // Positive for reporting Net latent credit to zone on sys time step from coil (W)
        Real64 ReportLatCreditToZoneEnergy = 0.0;
        Real64 ReportHeatingToZoneRate = 0.0; // Positive for reporting Net sensible credit to zone on sys time step from coil (W)
        Real64 ReportHeatingToZoneEnergy = 0.0;
        Real64 ReportSenCoolingToZoneRate = 0.0; // Positive for reporting Net sensible credit to zone on sys time step from coil (W)
        Real64 ReportSenCoolingToZoneEnergy = 0.0;
        Real64 ReportTotCoolingToZoneRate = 0.0; // Positive for reporting Net total cooling credit to zone from chillers (W)
        Real64 ReportTotCoolingToZoneEnergy = 0.0;

        // Reset to Zeros
        void reset()
        {
            new (this) CoilCreditData();
        }
    };

    // Functions

    void ManageRefrigeratedCaseRacks(EnergyPlusData &state);

    void GetRefrigerationInput(EnergyPlusData &state);

    void SetupReportInput(EnergyPlusData &state);

    void InitRefrigeration(EnergyPlusData &state);

    void InitRefrigerationPlantConnections(EnergyPlusData &state);

    void SimulateDetailedRefrigerationSystems(EnergyPlusData &state);

    void SimulateDetailedTransRefrigSystems(EnergyPlusData &state);

    void ReportRefrigerationComponents(EnergyPlusData &state);

    void SumZoneImpacts(EnergyPlusData &state);

    void CheckRefrigerationInput(EnergyPlusData &state);

    void SimAirChillerSet(EnergyPlusData &state,
                          std::string const &AirChillerSetName,
                          int ZoneNum,
                          bool FirstHVACIteration,
                          Real64 &SysOutputProvided,
                          Real64 &LatOutputProvided,
                          int &AirChillerSetPtr // from ZoneEquipList(CurZoneEqNum)%EquipIndex(EquipPtr)
    );

    void FinalRateCoils(EnergyPlusData &state,
                        bool DeRate,                 // True if compressor rack or secondary ht exchanger unable to provide capacity
                        SourceType SystemSourceType, // SecondarySystem or DetailedSystem
                        int SystemID,                // ID for Secondary loop or detailed system calling for derate
                        Real64 InitialTotalLoad,     // Load on system or secondary loop as initially calculated [W]
                        Real64 AvailableTotalLoad    // Load that system or secondary loop is able to serve [W]
    );

    void FigureRefrigerationZoneGains(EnergyPlusData &state);

    void ZeroHVACValues(EnergyPlusData &state);

} // namespace RefrigeratedCase

struct RefrigeratedCaseData : BaseGlobalStruct
{

    int NumRefrigeratedRacks = 0;            // Total number of refrigerated case compressor racks in input
    int NumRefrigSystems = 0;                // Total number of detailed refrigeration systems in input
    int NumRefrigCondensers = 0;             // Total number of detailed refrigeration condensers in input
    int NumRefrigChillerSets = 0;            // Total number of refrigerated warehouse coils in input
    int NumSimulationCondAir = 0;            // Number of air-cooled condensers in simulation
    int NumSimulationCondEvap = 0;           // Number of evaporative condensers in simulation
    int NumSimulationCondWater = 0;          // Number of water-cooled condensers in simulation
    int NumSimulationCascadeCondensers = 0;  // Total number of Cascade condensers in IDF
    int NumSimulationGasCooler = 0;          // Number of gas coolers in simulation
    int NumSimulationSharedGasCoolers = 0;   // Total number of gas coolers that serve more than one system
    int NumTransRefrigSystems = 0;           // Total number of transcritical CO2 refrigeration systems
    int NumSimulationSharedCondensers = 0;   // Total number of condensers that serve more than one system
    int NumSimulationCases = 0;              // Number of refrigerated cases in simulation
    int NumSimulationCaseAndWalkInLists = 0; // Total number of CaseAndWalkIn Lists in IDF
    int NumSimulationWalkIns = 0;            // Number of walk in coolers in simulation
    int NumSimulationCompressors = 0;        // Number of refrigeration compressors in simulation
    int NumSimulationSubcoolers = 0;         // Number of refrigeration subcoolers in simulation
    int NumSimulationMechSubcoolers = 0;     // Number of mechanical subcoolers in simulation
    int NumSimulationRefrigAirChillers = 0;  // Number of individual Air Chillers/coils in simulation
    int NumSimulationSecondarySystems = 0;   // Number of Secondary loops in simulation
    int NumSimulationTransferLoadLists = 0;  // Number of Secondary Lists in simulation
    int NumUnusedRefrigCases = 0;            // Number of refrigerated cases not connected to a rack or system
    int NumUnusedCoils = 0;                  // Number of refrigeration air coils not connected to a rack or system
    int NumUnusedCondensers = 0;             // Number of refrigeration condensers not connected to a system
    int NumUnusedGasCoolers = 0;             // Number of refrigeration gas coolers not connected to a system
    int NumUnusedCompressors = 0;            // Number of refrigeration compressors not connected to a system
    int NumUnusedSecondarys = 0;             // Number of refrigeration secondarys not connected to a system
    bool MyReferPlantScanFlag = true;

    // Refrigerated case variables
    Real64 CaseRAFactor = 0.0; // Factor determining case credit allocation (e.g. % to zone or HVAC)

    // Refrigeration compressor rack variables
    Real64 TotalRackDeliveredCapacity = 0.0; // Total capacity of all refrigerated cases attached to rack (W)
    Real64 TotalCompressorPower = 0.0;       // Total compressor electric power (W)
    Real64 CompressorCOPactual = 0.0;        // Compressor coefficient of performance at specific operating conditions (W/W)
    Real64 RackSenCreditToZone = 0.0;        // Amount of condenser heat applied to zone load (W)
    Real64 RackSenCreditToHVAC = 0.0;        // Amount of condenser heat applied to HVAC RA duct (W)

    // Refrigeration condenser variables (used for both racks and detailed systems)
    Real64 TotalCondenserFanPower = 0.0;  // Total condenser fan electric power (W)
    Real64 TotalCondenserPumpPower = 0.0; // Total condenser pump electric power (W)
    Real64 TotalCondenserHeat = 0.0;      // Total condenser heat from compressor rack (W)
    Real64 TotalBasinHeatPower = 0.0;     // Total condenser basin water heater power (W)
    Real64 TotalEvapWaterUseRate = 0.0;   // Total condenser water use rate (m3/s)

    // Refrigeration system variables
    Array1D_bool
        ShowUnmetEnergyWarning; // Used for one-time warning message for possible compressor input error regarding total system compressor capacity
    Array1D_bool ShowHiStageUnmetEnergyWarning; // Used for one-time warning message for possible high-stage compressor input error regarding
                                                // high-stage compressor capacity

    // Transcritical refrigeration system variables
    Array1D_bool ShowUnmetEnergyWarningTrans; // Used for one-time warning message for possible compressor input error regarding total system
                                              // compressor capacity

    // Refrigeration Secondary Loop variables
    Array1D_bool ShowUnmetSecondEnergyWarning; // Used for one-time warning message for possible compressor input error regarding secondary loop heat
                                               // exchanger capacity

    // Refrigeration Plant connections checks
    Array1D_bool CheckEquipNameRackWaterCondenser;
    Array1D_bool CheckEquipNameWaterCondenser;

    // Control variables
    Array1D_bool RefrigPresentInZone; // Used when translating rate to energy for reporting total refrigeration impact on a zone
    Array1D_bool CheckChillerSetName; // used when sim chiller set called form zone equip manager

    bool GetRefrigerationInputFlag = true; // Flag to show case input should be read
    bool HaveRefrigRacks = true;           // Is initialized as TRUE and remains true when refrigerated racks exist in the input deck
    bool HaveDetailedRefrig = true;        // Is initialized as TRUE and remains true when detailed refrigeration systems exist in the input deck
    bool HaveDetailedTransRefrig =
        true; // Is initialized as TRUE and remains true when detailed transcritical CO2 refrigeration systems exist in the input deck
    bool ManageRefrigeration = true; // Is initialized as TRUE and remains true when refrigerated racks or detailed systems exist in the input deck
    bool UseSysTimeStep = false;    // Flag is true IF working on a system that includes a coil cooling a controlled zone on the system time step, All
                                    // other refrigeration calculations for case and walkin systems done on the load time step
    bool HaveCasesOrWalkins = true; // Is initialized as TRUE and remains true when  refrigerated cases or walkins exist in the input deck
    bool HaveChillers = true;       // Is initialized as TRUE and remains true when chillers exist in the input deck

    // Object Data
    Array1D<RefrigeratedCase::RefrigCaseData> RefrigCase;
    Array1D<RefrigeratedCase::RefrigRackData> RefrigRack;
    Array1D<RefrigeratedCase::CaseRAFractionData> CaseRAFraction;
    Array1D<RefrigeratedCase::RefrigSystemData> System;
    Array1D<RefrigeratedCase::TransRefrigSystemData> TransSystem;
    Array1D<RefrigeratedCase::RefrigCondenserData> Condenser;
    std::unordered_map<std::string, std::string> UniqueCondenserNames;
    Array1D<RefrigeratedCase::RefrigCompressorData> Compressor;
    Array1D<RefrigeratedCase::RefrigGasCoolerData> GasCooler;
    Array1D<RefrigeratedCase::SubcoolerData> Subcooler;
    Array1D<RefrigeratedCase::CaseAndWalkInListDef> CaseAndWalkInList;
    Array1D<RefrigeratedCase::CompressorListDef> CompressorLists;
    Array1D<RefrigeratedCase::SecondaryLoopData> Secondary;
    Array1D<RefrigeratedCase::TransferLoadListDef> TransferLoadList;
    Array1D<RefrigeratedCase::WalkInData> WalkIn;
    Array1D<RefrigeratedCase::WarehouseCoilData> WarehouseCoil;
    Array1D<RefrigeratedCase::AirChillerSetData> AirChillerSet;
    Array1D<RefrigeratedCase::CoilCreditData> CoilSysCredit;
    Array1D<RefrigeratedCase::CaseWIZoneReportData> CaseWIZoneReport;

    bool MyEnvrnFlag = true; // flag to skip first pass on next begin environment flag
    bool InitRefrigerationMyBeginEnvrnFlag = true;
    bool InitRefrigerationPlantConnectionsMyBeginEnvrnFlag = true;
    bool FigureRefrigerationZoneGainsMyEnvrnFlag = true;

    Real64 MyCurrentTimeSaved = 0.0;   // Used to determine whether the zone time step is a repetition
    Real64 MyStepStartTimeSaved = 0.0; // Used to determine whether the system time step is a repetition
    Real64 TimeStepFraction = 0.0;     // Used to calculate my current time

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) RefrigeratedCaseData();
    }
};

} // namespace EnergyPlus

#endif
