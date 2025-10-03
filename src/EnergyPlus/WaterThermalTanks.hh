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

#ifndef WaterThermalTanks_hh_INCLUDED
#define WaterThermalTanks_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.fwd.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>
#include <EnergyPlus/VariableSpeedCoils.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace WaterThermalTanks {

    enum class WTTAmbientTemp
    {
        Invalid = -1,
        Schedule,   // ambient temperature around tank (or HPWH inlet air) is scheduled
        TempZone,   // tank is located in a zone or HPWH inlet air is zone air only
        OutsideAir, // tank is located outdoors or HPWH inlet air is outdoor air only
        ZoneAndOA,  // applicable to HPWH only, inlet air is mixture of OA and zone air
        Num,
    };

    constexpr std::array<std::string_view, static_cast<int>(WTTAmbientTemp::Num)> HPWHAmbientTempNamesUC{
        "SCHEDULE", "ZONEAIRONLY", "OUTDOORAIRONLY", "ZONEANDOUTDOORAIR"};

    constexpr std::array<std::string_view, static_cast<int>(WTTAmbientTemp::Num) - 1> TankAmbientTempNamesUC{
        "SCHEDULE",
        "ZONE",
        "OUTDOORS",
    };

    enum class CrankcaseHeaterControlTemp
    {
        Invalid = -1,
        Schedule, // temperature controlling compressor crankcase heater is scheduled
        Zone,     // temperature controlling compressor crankcase heater is zone air
        Outdoors, // temperature controlling compressor crankcase heater is outdoor air
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(CrankcaseHeaterControlTemp::Num)> CrankcaseHeaterControlTempNamesUC{
        "SCHEDULE",
        "ZONE",
        "OUTDOORS",
    };

    enum class TankShape
    {
        Invalid = -1,
        VertCylinder,  // tank shape is a vertical cylinder
        HorizCylinder, // tank shape is a horizontal cylinder
        Other,         // tank shape has an arbitrary perimeter shape
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(TankShape::Num)> TankShapeNamesUC{
        "VERTICALCYLINDER",
        "HORIZONTALCYLINDER",
        "OTHER",
    };

    enum class HeaterControlMode
    {
        Invalid = -1,
        Cycle,
        Modulate,
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(HeaterControlMode::Num)> HeaterControlModeNamesUC{"CYCLE", "MODULATE"};

    enum class PriorityControlMode // For Stratified Water Heaters, this controls how the two heating elements work together
    {
        Invalid = -1,
        MasterSlave,  // water heater only, master-slave priority control of heater elements
        Simultaneous, // water heater only, simultaneous control of heater elements
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(PriorityControlMode::Num)> PriorityControlModeNamesUC{"MASTERSLAVE", "SIMULTANEOUS"};

    enum class InletPositionMode
    {
        Invalid = -1,
        Fixed,   // water heater only, inlet water always enters at the user-specified height
        Seeking, // water heater only, inlet water seeks out the node with the closest temperature
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(InletPositionMode::Num)> InletPositionModeNamesUC{"FIXED", "SEEKING"};

    // reclaim heat object types for Coil:WaterHeating:Desuperheater object
    enum class ReclaimHeatObjectType
    {
        Invalid = -1,
        CoilCoolingDX,                  // reclaim heating source is new DX Cooling coil
        CompressorRackRefrigeratedCase, // reclaim heating source is refrigerated case compressor rack
        DXCooling,                      // reclaim heating source is DX cooling coil
        DXMultiSpeed,                   // reclaim heating source is DX multispeed coil
        DXMultiMode,                    // reclaim heating source is DX multimode coil
        CondenserRefrigeration,         // reclaim heating source is detailed refrigeration system condenser
        DXVariableCooling,              // reclaim heating source is Variable Speed DX cooling coil
        AirWaterHeatPumpEQ,             // reclaim heating source is Water to air heat pump cooling coil
        AirWaterHeatPumpVSEQ,           // reclaim heating source is Water to air heat pump variable speed cooling coil
        Num
    };

    enum class WaterHeaterSide
    {
        Invalid = -1,
        Use,    // Indicates Use side of water heater
        Source, // Indicates Source side of water heater
        Num
    };

    enum class SizingMode
    {
        Invalid = -1,
        PeakDraw,
        ResidentialMin,
        PerPerson,
        PerFloorArea,
        PerUnit,
        PerSolarColArea,
        Num
    };

    enum class SourceSideControl
    {
        Invalid = -1,
        StorageTank,
        IndirectHeatPrimarySetpoint,
        IndirectHeatAltSetpoint,
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(SourceSideControl::Num)> SourceSideControlNamesUC{
        "STORAGETANK", "INDIRECTHEATPRIMARYSETPOINT", "INDIRECTHEATALTERNATESETPOINT"};

    enum class FlowMode
    {
        Invalid = -1,
        PassingFlowThru,
        MaybeRequestingFlow,
        ThrottlingFlow,
        Num
    };

    enum class TankOperatingMode
    {
        Invalid = -1,
        Heating,  // heating source is on, source will not turn off until setpoint temp is reached
        Floating, // heating source is off, source will not turn on until cut-in temp is reached
        Venting,  // tank temp is above maximum temperature and water is venting
        Cooling,  // cooling source is on, source will not turn off until setpoint temp is reached
        Num
    };

    struct StratifiedNodeData
    {
        // Members
        Real64 Mass; // All nodes have the same mass (kg)
        Real64 OnCycLossCoeff;
        Real64 OffCycLossCoeff;
        Real64 Temp;
        Real64 SavedTemp;
        Real64 NewTemp;
        Real64 TempSum;
        Real64 TempAvg; // Average node temperature over the time step (C)
        Real64 CondCoeffUp;
        Real64 CondCoeffDn;
        Real64 OffCycParaLoad; // Heat delivered to the tank from off-cycle parasitic sources
        Real64 OnCycParaLoad;
        Real64 UseMassFlowRate;
        Real64 SourceMassFlowRate;
        Real64 MassFlowFromUpper; // Mass flow rate into this node from node above
        Real64 MassFlowFromLower; // Mass flow rate into this node from node below
        Real64 MassFlowToUpper;   // Mass flow rate from this node to node above
        Real64 MassFlowToLower;   // Mass flow rate from this node to node below
        // Report Variables
        Real64 Volume;
        Real64 Height;      // Node height from top to bottom (like a thickness)
        Real64 MaxCapacity; // For reporting
        int Inlets;
        int Outlets;
        Real64 HPWHWrappedCondenserHeatingFrac; // fraction of the heat from a wrapped condenser that enters into this node, should add up to 1.

        // Default Constructor
        StratifiedNodeData()
            : Mass(0.0), OnCycLossCoeff(0.0), OffCycLossCoeff(0.0), Temp(0.0), SavedTemp(0.0), NewTemp(0.0), TempSum(0.0), TempAvg(0.0),
              CondCoeffUp(0.0), CondCoeffDn(0.0), OffCycParaLoad(0.0), OnCycParaLoad(0.0), UseMassFlowRate(0.0), SourceMassFlowRate(0.0),
              MassFlowFromUpper(0.0), MassFlowFromLower(0.0), MassFlowToUpper(0.0), MassFlowToLower(0.0), Volume(0.0), Height(0.0), MaxCapacity(0.0),
              Inlets(0), Outlets(0), HPWHWrappedCondenserHeatingFrac(0.0)
        {
        }
    };

    struct WaterHeaterSizingData
    {
        // Members
        // input data
        SizingMode DesignMode;                    // what sizing method to use
        Real64 TankDrawTime;                      // in hours, time storage can meet peak demand
        Real64 RecoveryTime;                      // time for tank to recover
        Real64 NominalVolForSizingDemandSideFlow; // nominal tank size to use in sizing demand side connections
        int NumberOfBedrooms;
        Real64 NumberOfBathrooms;
        Real64 TankCapacityPerPerson;
        Real64 RecoveryCapacityPerPerson;
        Real64 TankCapacityPerArea;
        Real64 RecoveryCapacityPerArea;
        Real64 NumberOfUnits;
        Real64 TankCapacityPerUnit;
        Real64 RecoveryCapacityPerUnit;
        Real64 TankCapacityPerCollectorArea;
        Real64 HeightAspectRatio;
        // data from elsewhere in E+
        Real64 PeakDemand;
        Real64 PeakNumberOfPeople;
        Real64 TotalFloorArea;
        Real64 TotalSolarCollectorArea;

        // Default Constructor
        WaterHeaterSizingData()
            : DesignMode(SizingMode::Invalid), TankDrawTime(0.0), RecoveryTime(0.0), NominalVolForSizingDemandSideFlow(0.0), NumberOfBedrooms(0),
              NumberOfBathrooms(0.0), TankCapacityPerPerson(0.0), RecoveryCapacityPerPerson(0.0), TankCapacityPerArea(0.0),
              RecoveryCapacityPerArea(0.0), NumberOfUnits(0.0), TankCapacityPerUnit(0.0), RecoveryCapacityPerUnit(0.0),
              TankCapacityPerCollectorArea(0.0), HeightAspectRatio(0.0), PeakDemand(0.0), PeakNumberOfPeople(0.0), TotalFloorArea(0.0),
              TotalSolarCollectorArea(0.0)
        {
        }
    };

    struct HeatPumpWaterHeaterData : PlantComponent
    {
        // Members
        std::string Name;                                           // Name of heat pump water heater
        std::string Type;                                           // Type of water heater (HEAT PUMP:WATER HEATER)
        DataPlant::PlantEquipmentType HPWHType;                     // integer parameter for heat pump water heater
        std::string TankName;                                       // Name of tank associated with heat pump water heater
        std::string TankType;                                       // Type of water heater (MIXED or STRATIFIED) used with heat pump
        DataPlant::PlantEquipmentType HPWHTankType;                 // Parameter for tank type (MIXED or STRATIFIED)
        bool StandAlone;                                            // Flag for operation with no plant connections (no use nodes)
        Sched::Schedule *availSched = nullptr;                      // Availability Schedule
        Sched::Schedule *setptTempSched = nullptr;                  // Setpoint Temperature Schedule
        Real64 DeadBandTempDiff;                                    // Dead band temperature difference (cut-in temperature)
        Real64 Capacity;                                            // Heat Pump rated capacity (W)
        Real64 BackupElementCapacity;                               // Tank backup element capacity (W)
        Real64 BackupElementEfficiency;                             // Tank backup element efficiency
        Real64 WHOnCycParaLoad;                                     // tank's on-cycle parasitic load (W), disable for rating
        Real64 WHOffCycParaLoad;                                    // tank's off-cycle parasitic load (W), disable for rating
        Real64 WHOnCycParaFracToTank;                               // tank's on-cycle parasitic frac to tank, disable for rating
        Real64 WHOffCycParaFracToTank;                              // tank's off-cycle parasitic frac to tank, disable for rating
        int WHPLFCurve;                                             // tank part-load fraction curve index, used for rating procedure
        Real64 OperatingAirFlowRate;                                // Operating volumetric air flow rate (m3/s)
        Real64 OperatingAirMassFlowRate;                            // Operating air mass flow rate (kg/s)
        Real64 OperatingWaterFlowRate;                              // Operating volumetric water flow rate (m3/s)
        Real64 COP;                                                 // Heat Pump coefficient of performance (W/W)
        Real64 SHR;                                                 // Heat Pump air-side coil sensible heat ratio
        Real64 RatedInletDBTemp;                                    // Rated evaporator inlet air dry-bulb temperature (C)
        Real64 RatedInletWBTemp;                                    // Rated evaporator inlet air wet-bulb temperature (C)
        Real64 RatedInletWaterTemp;                                 // Rated condenser inlet water temperature (C)
        bool FoundTank;                                             // Found storage tank flag associated with HP water heater
        int HeatPumpAirInletNode;                                   // HP air inlet node (for zone, zone/outdoor or scheduled)
        int HeatPumpAirOutletNode;                                  // HP air outlet node (for zone, zone/outdoor or scheduled)
        int OutsideAirNode;                                         // outdoor air node (for outdoor or zone/outdoor air unit only)
        int ExhaustAirNode;                                         // Exhaust air node (for outdoor or zone/outdoor air unit only)
        int CondWaterInletNode;                                     // Condenser water inlet node
        int CondWaterOutletNode;                                    // Condenser water outlet node
        int WHUseInletNode;                                         // Water heater tank use side inlet node
        int WHUseOutletNode;                                        // Water heater tank use side outlet node
        int WHUseSidePlantLoopNum;                                  // if not zero, then this water heater is on plant loop #
        std::string DXCoilType;                                     // Type of DX coil (Coil:DX:HeatPumpWaterHeater)
        std::string DXCoilName;                                     // Name of DX coil
        int DXCoilNum;                                              // Index of DX coil
        int DXCoilTypeNum;                                          // Type Number of DX coil
        int DXCoilAirInletNode;                                     // Inlet air node number of DX coil
        int DXCoilPLFFPLR;                                          // Index to HPWH's DX Coil PLF as a function of PLR curve
        HVAC::FanType fanType;                                      // Integer type of fan (3 = Fan:OnOff)
        std::string FanName;                                        // Name of Fan
        std::string FanInletNode_str;                               // Fan inlet node name
        std::string FanOutletNode_str;                              // Fan outlet node name
        int FanNum;                                                 // Index of Fan
        HVAC::FanPlace fanPlace;                                    // Location of Fan
        int FanOutletNode;                                          // Outlet node of heat pump water heater fan
        int WaterHeaterTankNum;                                     // Index of Water Heater Tank
        Sched::Schedule *outletAirSplitterSched = nullptr;          // air-side outlet air splitter schedule
        Sched::Schedule *inletAirMixerSched = nullptr;              // air-side inlet air mixer schedule
        TankOperatingMode Mode = TankOperatingMode::Floating;       // HP mode (0 = float, 1 = heating [-1 = venting na for HP])
        TankOperatingMode SaveMode = TankOperatingMode::Floating;   // HP mode on first iteration
        TankOperatingMode SaveWHMode = TankOperatingMode::Floating; // mode of water heater tank element (backup element)
        Real64 Power;                                               // HP power used for reporting
        Real64 Energy;                                              // HP energy used for reporting
        Real64 HeatingPLR;                                          // HP PLR used for reporting
        Real64 SetPointTemp;                                        // set point or cut-out temperature [C]
        Real64 MinAirTempForHPOperation;                            // HP does not operate below this ambient temperature
        Real64 MaxAirTempForHPOperation;                            // HP does not operate above this ambient temperature
        int InletAirMixerNode;                                      // Inlet air mixer node number of HP water heater
        int OutletAirSplitterNode;                                  // Outlet air splitter node number of HP water heater
        Real64 SourceMassFlowRate;                                  // Maximum mass flow rate on the source side (kg/s)
        WTTAmbientTemp InletAirConfiguration;                       // Identifies source of HPWH inlet air
        Sched::Schedule *ambientTempSched = nullptr;                // Schedule for ambient air temp at HPWH inlet
        Sched::Schedule *ambientRHSched = nullptr;                  // Schedule for ambient air RH at HPWH inlet
        int AmbientTempZone;                                        // Index of ambient zone for ambient air at HPWH inlet
        CrankcaseHeaterControlTemp CrankcaseTempIndicator;          // Indicator for HPWH compressor/crankcase heater location
        Sched::Schedule *crankcaseTempSched = nullptr;              // Schedule for crankcase heater
        int CrankcaseTempZone;                                      // Index of zone where compressor/crankcase heater is located
        Real64 OffCycParaLoad;                                      // Rate for off-cycle parasitic load (W)
        Real64 OnCycParaLoad;                                       // Rate for on-cycle parasitic load (W)
        WTTAmbientTemp ParasiticTempIndicator;                      // Indicator for HPWH parasitic heat rejection location
        Real64 OffCycParaFuelRate;                                  // Electric consumption rate for off-cycle parasitic load (W)
        Real64 OnCycParaFuelRate;                                   // Electric consumption rate for on-cycle parasitic load (W)
        Real64 OffCycParaFuelEnergy;                                // Electric energy consumption for off-cycle parasitic load (J)
        Real64 OnCycParaFuelEnergy;                                 // Electric energy consumption for on-cycle parasitic load (J)
        bool AirFlowRateAutoSized;                                  // Used to report air flow autosize info in Init
        bool WaterFlowRateAutoSized;                                // Used to report water flow autosize info in Init
        int HPSetPointError;                                        // Used when temperature SP's in tank and HP are reversed
        int HPSetPointErrIndex1;                                    // Index to recurring error for tank/HP set point temp
        int IterLimitErrIndex1;                                     // Index for recurring iteration limit warning messages
        int IterLimitExceededNum1;                                  // Counter for recurring iteration limit warning messages
        int RegulaFalsiFailedIndex1;                                // Index for recurring RegulaFalsi failed warning messages
        int RegulaFalsiFailedNum1;                                  // Counter for recurring RegulaFalsi failed warning messages
        int IterLimitErrIndex2;                                     // Index for recurring iteration limit warning messages
        int IterLimitExceededNum2;                                  // Counter for recurring iteration limit warning messages
        int RegulaFalsiFailedIndex2;                                // Index for recurring RegulaFalsi failed warning messages
        int RegulaFalsiFailedNum2;                                  // Counter for recurring RegulaFalsi failed warning messages
        bool FirstTimeThroughFlag;                                  // Flag for saving water heater status
        bool ShowSetPointWarning;                                   // Warn when set point is greater than max tank temp limit
        Real64 HPWaterHeaterSensibleCapacity;                       // sensible capacity delivered when HPWH is attached to a zone (W)
        Real64 HPWaterHeaterLatentCapacity;                         // latent capacity delivered when HPWH is attached to a zone (kg/s)
        Real64 WrappedCondenserBottomLocation;                      // Location of the bottom of the wrapped condenser.
        Real64 WrappedCondenserTopLocation;                         // Location of the top of the wrapped condenser.
        Real64 ControlSensor1Height;                                // location from bottom of tank of control sensor 1
        int ControlSensor1Node;                                     // Node number of control sensor 1
        Real64 ControlSensor1Weight;                                // weight of control sensor 1
        Real64 ControlSensor2Height;                                // location from bottom of tank of control sensor 2
        int ControlSensor2Node;                                     // Node number of control sensor 2
        Real64 ControlSensor2Weight;                                // weight of control sensor 2
        Real64 ControlTempAvg;                              // Measured control temperature for the heat pump, average over timestep, for reporting
        Real64 ControlTempFinal;                            // Measured control temperature at the end of the timestep, for reporting
        bool AllowHeatingElementAndHeatPumpToRunAtSameTime; // if false, if the heating element kicks on, it will recover with that before turning the
        // heat pump back on.
        // variables for variable-speed HPWH
        int NumofSpeed;                        // number of speeds for VS HPWH
        Array1D<Real64> HPWHAirVolFlowRate;    // air volume flow rate during heating operation
        Array1D<Real64> HPWHAirMassFlowRate;   // air mass flow rate during heating operation
        Array1D<Real64> HPWHWaterVolFlowRate;  // water volume flow rate during heating operation
        Array1D<Real64> HPWHWaterMassFlowRate; // water mass flow rate during heating operation
        Array1D<Real64> MSAirSpeedRatio;       // air speed ratio in heating mode
        Array1D<Real64> MSWaterSpeedRatio;     // water speed ratio in heating mode
        bool bIsIHP;                           // whether the HP is a part of Integrated Heat Pump
        bool MyOneTimeFlagHP;                  // first pass log
        bool MyTwoTimeFlagHP;                  // second pass do input check
        std::string CoilInletNode_str;         // Used to set up comp set
        std::string CoilOutletNode_str;        // Used to set up comp set
        bool CheckHPWHEquipName;

        std::string InletNodeName1;
        std::string OutletNodeName1;
        std::string InletNodeName2;
        std::string OutletNodeName2;

        bool myOneTimeInitFlag;

        // end of variables for variable-speed HPWH
        ~HeatPumpWaterHeaterData() = default;

        // Default Constructor
        HeatPumpWaterHeaterData()
            : HPWHType(DataPlant::PlantEquipmentType::Invalid), HPWHTankType(DataPlant::PlantEquipmentType::Invalid), StandAlone(false),
              DeadBandTempDiff(0.0), Capacity(0.0), BackupElementCapacity(0.0), BackupElementEfficiency(0.0), WHOnCycParaLoad(0.0),
              WHOffCycParaLoad(0.0), WHOnCycParaFracToTank(0.0), WHOffCycParaFracToTank(0.0), WHPLFCurve(0), OperatingAirFlowRate(0.0),
              OperatingAirMassFlowRate(0.0), OperatingWaterFlowRate(0.0), COP(0.0), SHR(0.0), RatedInletDBTemp(0.0), RatedInletWBTemp(0.0),
              RatedInletWaterTemp(0.0), FoundTank(false), HeatPumpAirInletNode(0), HeatPumpAirOutletNode(0), OutsideAirNode(0), ExhaustAirNode(0),
              CondWaterInletNode(0), CondWaterOutletNode(0), WHUseInletNode(0), WHUseOutletNode(0), WHUseSidePlantLoopNum(0), DXCoilNum(0),
              DXCoilTypeNum(0), DXCoilAirInletNode(0), DXCoilPLFFPLR(0), fanType(HVAC::FanType::Invalid), FanNum(0),
              fanPlace(HVAC::FanPlace::Invalid), FanOutletNode(0), WaterHeaterTankNum(0), Power(0.0), Energy(0.0), HeatingPLR(0.0), SetPointTemp(0.0),
              MinAirTempForHPOperation(5.0), MaxAirTempForHPOperation(48.8888888889), InletAirMixerNode(0), OutletAirSplitterNode(0),
              SourceMassFlowRate(0.0), InletAirConfiguration(WTTAmbientTemp::OutsideAir), AmbientTempZone(0),
              CrankcaseTempIndicator(CrankcaseHeaterControlTemp::Schedule), CrankcaseTempZone(0), OffCycParaLoad(0.0), OnCycParaLoad(0.0),
              ParasiticTempIndicator(WTTAmbientTemp::OutsideAir), OffCycParaFuelRate(0.0), OnCycParaFuelRate(0.0), OffCycParaFuelEnergy(0.0),
              OnCycParaFuelEnergy(0.0), AirFlowRateAutoSized(false), WaterFlowRateAutoSized(false), HPSetPointError(0), HPSetPointErrIndex1(0),
              IterLimitErrIndex1(0), IterLimitExceededNum1(0), RegulaFalsiFailedIndex1(0), RegulaFalsiFailedNum1(0), IterLimitErrIndex2(0),
              IterLimitExceededNum2(0), RegulaFalsiFailedIndex2(0), RegulaFalsiFailedNum2(0), FirstTimeThroughFlag(true), ShowSetPointWarning(true),
              HPWaterHeaterSensibleCapacity(0.0), HPWaterHeaterLatentCapacity(0.0), WrappedCondenserBottomLocation(0.0),
              WrappedCondenserTopLocation(0.0), ControlSensor1Height(-1.0), ControlSensor1Node(1), ControlSensor1Weight(1.0),
              ControlSensor2Height(-1.0), ControlSensor2Node(2), ControlSensor2Weight(0.0), ControlTempAvg(0.0), ControlTempFinal(0.0),
              AllowHeatingElementAndHeatPumpToRunAtSameTime(true), NumofSpeed(0), HPWHAirVolFlowRate(HVAC::MaxSpeedLevels, 0.0),
              HPWHAirMassFlowRate(HVAC::MaxSpeedLevels, 0.0), HPWHWaterVolFlowRate(HVAC::MaxSpeedLevels, 0.0),
              HPWHWaterMassFlowRate(HVAC::MaxSpeedLevels, 0.0), MSAirSpeedRatio(HVAC::MaxSpeedLevels, 0.0),
              MSWaterSpeedRatio(HVAC::MaxSpeedLevels, 0.0), bIsIHP(false), MyOneTimeFlagHP(true), MyTwoTimeFlagHP(true), CheckHPWHEquipName(true),
              myOneTimeInitFlag(true)
        {
        }

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void
        simulate(EnergyPlusData &state, const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag) override;

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation) override;

        void getDesignCapacities(EnergyPlusData &state,
                                 [[maybe_unused]] const PlantLocation &calledFromLocation,
                                 Real64 &MaxLoad,
                                 Real64 &MinLoad,
                                 Real64 &OptLoad) override;

        void oneTimeInit(EnergyPlusData &state) override;
    };

    struct WaterThermalTankData : PlantComponent
    {
        // Members
        std::string Name;                                   // Name of water heater
        std::string Type;                                   // Type of water heater (MIXED or STRATIFIED)
        DataPlant::PlantEquipmentType WaterThermalTankType; // integer parameter for water heater(if part of an HPWH,then=HPWH)
        bool IsChilledWaterTank;                            // logical flag, true if for chilled water, false if for hot water
        std::string EndUseSubcategoryName;                  // User-defined end-use subcategory name
        bool Init;                                          // Flag for initialization:  TRUE means do the init
        bool StandAlone;                                    // Flag for operation with no plant connections (no source or use)
        Real64 Volume;                                      // Tank volume (m3)
        bool VolumeWasAutoSized;                            // true if tank volume was autosize on input
        Real64 Mass;                                        // Total mass of fluid in the tank (kg)
        Real64 TimeElapsed;                                 // Fraction of the current hour that has elapsed (h)
        // Saved in order to identify the beginning of a new system time
        WTTAmbientTemp AmbientTempIndicator;                       // Indicator for ambient tank losses (SCHEDULE, ZONE, EXTERIOR)
        Sched::Schedule *ambientTempSched = nullptr;               // Schedule
        int AmbientTempZone;                                       // Number of ambient zone around tank
        int AmbientTempOutsideAirNode;                             // Number of outside air node
        Real64 AmbientTemp;                                        // Ambient temperature around tank (C)
        Real64 AmbientZoneGain;                                    // Internal gain to zone from tank losses (W)
        Real64 LossCoeff;                                          // Overall tank heat loss coefficient, UA (W/K)
        Real64 OffCycLossCoeff;                                    // Off-cycle overall tank heat loss coefficient, UA (W/K)
        Real64 OffCycLossFracToZone;                               // Fraction of off-cycle losses added to zone
        Real64 OnCycLossCoeff;                                     // On-cycle overall tank heat loss coefficient, UA (W/K)
        Real64 OnCycLossFracToZone;                                // Fraction of on-cycle losses added to zone
        TankOperatingMode Mode = TankOperatingMode::Floating;      // Indicator for current operating mode
        TankOperatingMode SavedMode = TankOperatingMode::Floating; // Mode indicator saved from previous time step
        HeaterControlMode ControlType;                             // Indicator for Heater Control type
        PriorityControlMode StratifiedControlMode;                 // Indicator for Stratified Water Heaters Priority Control Type
        Constant::eFuel FuelType;                                  // Fuel type
        Real64 MaxCapacity;                                        // Maximum capacity of auxiliary heater 1 (W)
        bool MaxCapacityWasAutoSized;                              // true if heater 1 capacity was autosized on input
        Real64 MinCapacity;                                        // Minimum capacity of auxiliary heater 1 (W)
        Real64 Efficiency;                                         // Thermal efficiency of auxiliary heater 1 ()
        int PLFCurve;                                              // Part load factor curve as a function of part load ratio
        Sched::Schedule *setptTempSched = nullptr;                 // Schedule
        Real64 SetPointTemp;                                       // Setpoint temperature of auxiliary heater 1 (C)
        Real64 DeadBandDeltaTemp;                                  // Deadband temperature difference of auxiliary heater 1 (deltaC)
        Real64 TankTempLimit;                                      // Maximum tank temperature limit before venting (C)
        Real64 IgnitionDelay;                                      // Time delay before heater is allowed to turn on (s)
        Real64 OffCycParaLoad;                                     // Rate for off-cycle parasitic load (W)
        Constant::eFuel OffCycParaFuelType;                        // Fuel type for off-cycle parasitic load
        Real64 OffCycParaFracToTank;                               // Fraction of off-cycle parasitic energy ending up in tank (W)
        Real64 OnCycParaLoad;                                      // Rate for on-cycle parasitic load (W)
        Constant::eFuel OnCycParaFuelType;                         // Fuel type for on-cycle parasitic load
        Real64 OnCycParaFracToTank;                                // Fraction of on-cycle parasitic energy ending up in tank (W)
        DataPlant::FlowLock UseCurrentFlowLock;                    // current flow lock setting on use side
        int UseInletNode;                                          // Inlet node on the use side; colder water returning to a hottank
        Real64 UseInletTemp;                                       // Use side inlet temperature (C)
        int UseOutletNode;                                         // Outlet node on the use side; hot tank water
        Real64 UseOutletTemp;                                      // Use side outlet temperature (C)
        Real64 UseMassFlowRate;                                    // Mass flow rate on the use side (kg/s)
        Real64 UseEffectiveness;                                   // Heat transfer effectiveness on use side ()
        Real64 PlantUseMassFlowRateMax;                            // Plant demand-side max flow request on use side (kg/s)
        Real64 SavedUseOutletTemp;                                 // Use side outlet temp saved for demand-side flow control (C)
        Real64 UseDesignVolFlowRate;                               // Use side plant volume flow rate (input data, autosizable) m3/s
        bool UseDesignVolFlowRateWasAutoSized;                     // true if use flow rate was autosize on input
        DataBranchAirLoopPlant::ControlType UseBranchControlType;  // Use side plant branch control type e.g active, passive, bypass
        int UseSidePlantSizNum;                                    // index in plant sizing that the use side is on
        bool UseSideSeries;
        Sched::Schedule *useSideAvailSched = nullptr; // use side availability schedule
        Real64 UseSideLoadRequested;                  // hold MyLoad request from plant management.
        PlantLocation UseSidePlantLoc;
        int SourceInletNode;                                         // Inlet node for the source side; hot water from supply
        Real64 SourceInletTemp;                                      // Source side inlet temperature (C)
        int SourceOutletNode;                                        // Outlet node for the source side; colder tank water
        Real64 SourceOutletTemp;                                     // Source side outlet temperature (C)
        Real64 SourceMassFlowRate;                                   // Mass flow rate on the source side (kg/s)
        Real64 SourceEffectiveness;                                  // Heat transfer effectiveness on source side ()
        Real64 PlantSourceMassFlowRateMax;                           // Plant demand-side max flow request on source side (kg/s)
        Real64 SavedSourceOutletTemp;                                // Source side outlet temp saved for demand-side flow control (C)
        Real64 SourceDesignVolFlowRate;                              // Source side plant volume flow rate (input, autosizable) m3/s
        bool SourceDesignVolFlowRateWasAutoSized;                    // true if source flow rate was autosize on input
        DataBranchAirLoopPlant::ControlType SourceBranchControlType; // source side plant branch control type e.g active, passive, bypass
        int SourceSidePlantSizNum;                                   // index in plant sizing that the source side is on
        bool SourceSideSeries;
        Sched::Schedule *sourceSideAvailSched = nullptr; // source side availability schedule.
        PlantLocation SrcSidePlantLoc;
        SourceSideControl SourceSideControlMode;               // flag for how source side flow is controlled
        Sched::Schedule *sourceSideAltSetpointSched = nullptr; // schedule of alternate temperature setpoint values
        Real64 SizingRecoveryTime;                             // sizing parameter for autosizing indirect water heaters (hr)
        Real64 VolFlowRateMax;                                 // Used only in reporting {m3/s)
        Real64 MassFlowRateMax;                                // Maximum flow rate for scheduled DHW (kg/s)
        Real64 VolFlowRateMin;                                 // Minimum flow rate for heater ignition (kg/s)
        Real64 MassFlowRateMin;                                // Minimum mass flow rate for heater ignition (kg/s)
        Sched::Schedule *flowRateSched = nullptr;              // Schedule
        Sched::Schedule *useInletTempSched = nullptr;          // Cold water supply temperature schedule
        Real64 TankTemp;                                       // Temperature of tank fluid (average, if stratified) (C)
        Real64 SavedTankTemp;                                  // Tank temp that is carried from time step to time step (C)
        Real64 TankTempAvg;                                    // Average tank temperature over the time step (C)
        // Stratified variables (in addition to the above)
        Real64 Height;           // Height of tank (m)
        bool HeightWasAutoSized; // true if the height of tank was autosize on input
        Real64 Perimeter;        // Perimeter of tank (m), only used for OTHER shape
        TankShape Shape;         // Tank shape:  VERTICAL CYLINDER, HORIZONTAL CYLINDER, or OTHER
        Real64 HeaterHeight1;
        int HeaterNode1;
        bool HeaterOn1;
        bool SavedHeaterOn1;
        Real64 HeaterHeight2;
        int HeaterNode2;
        bool HeaterOn2;
        bool SavedHeaterOn2;
        Real64 AdditionalCond; // Additional destratification conductivity (W/m K)
        Real64 SetPointTemp2;  // Setpoint temperature of auxiliary heater 2 (C)
        Sched::Schedule *setptTemp2Sched = nullptr;
        Real64 DeadBandDeltaTemp2;
        Real64 MaxCapacity2;
        Real64 OffCycParaHeight;
        Real64 OnCycParaHeight;
        Real64 SkinLossCoeff;
        Real64 SkinLossFracToZone;
        Real64 OffCycFlueLossCoeff;
        Real64 OffCycFlueLossFracToZone;
        Real64 UseInletHeight;              // Height of use side inlet (m)
        Real64 UseOutletHeight;             // Height of use side outlet (m)
        bool UseOutletHeightWasAutoSized;   // true if use outlet height was autosize on input
        Real64 SourceInletHeight;           // Height of source side inlet (m)
        bool SourceInletHeightWasAutoSized; // true if source inlet height was autosize on input
        Real64 SourceOutletHeight;          // Height of source side outlet (m)
        int UseInletStratNode;              // Use-side inlet node number
        int UseOutletStratNode;             // Use-side outlet node number
        int SourceInletStratNode;           // Source-side inlet node number
        int SourceOutletStratNode;          // Source-side outlet node number
        InletPositionMode InletMode;        // Inlet position mode:  1 = FIXED; 2 = SEEKING
        Real64 InversionMixingRate;
        Array1D<Real64> AdditionalLossCoeff; // Loss coefficient added to the skin loss coefficient (W/m2-K)
        int Nodes;                           // Number of nodes
        Array1D<StratifiedNodeData> Node;    // Array of node data
        // Report variables
        Real64 VolFlowRate;            // Scheduled DHW demand (m3/s)
        Real64 VolumeConsumed;         // Volume of DHW consumed (m3)
        Real64 UnmetRate;              // Energy demand to heat tank water to setpoint (W)
        Real64 LossRate;               // Energy demand to support heat losses due to ambient temp (W)
        Real64 FlueLossRate;           // Heat loss rate to flue (W)
        Real64 UseRate;                // Energy demand to heat the Use Side water to tank temp (W)
        Real64 TotalDemandRate;        // Total demand rate (sum of all above rates) (W)
        Real64 SourceRate;             // Energy supplied by the source side to help heat the tank (W)
        Real64 HeaterRate;             // The energy the water heater burner puts into the water (W)
        Real64 HeaterRate1;            // The energy heater 1 puts into the water (W)
        Real64 HeaterRate2;            // The energy heater 2 puts into the water (W)
        Real64 FuelRate;               // The fuel consumption rate for the water heater burner (W)
        Real64 FuelRate1;              // The fuel consumption rate for heater 1 (W)
        Real64 FuelRate2;              // The fuel consumption rate for heater 2 (W)
        Real64 VentRate;               // Heat recovery energy lost due to setpoint temp (W)
        Real64 OffCycParaFuelRate;     // Fuel consumption rate for off-cycle parasitic load (W)
        Real64 OffCycParaRateToTank;   // Heat rate to tank for off-cycle parasitic load (W)
        Real64 OnCycParaFuelRate;      // Fuel consumption rate for on-cycle parasitic load (W)
        Real64 OnCycParaRateToTank;    // Heat rate to tank for on-cycle parasitic load (W)
        Real64 NetHeatTransferRate;    // Net heat transfer rate to/from tank (W)
        int CycleOnCount;              // Number of times heater cycles on in the current time step
        int CycleOnCount1;             // Number of times heater 1 cycles on in the current time step
        int CycleOnCount2;             // Number of times heater 2 cycles on in the current time step
        Real64 RuntimeFraction;        // Runtime fraction, fraction of timestep that any  heater is running
        Real64 RuntimeFraction1;       // Runtime fraction, fraction of timestep that heater 1 is running
        Real64 RuntimeFraction2;       // Runtime fraction, fraction of timestep that heater 2 is running
        Real64 PartLoadRatio;          // Part load ratio, fraction of maximum heater capacity
        Real64 UnmetEnergy;            // Energy to heat tank water to setpoint (J)
        Real64 LossEnergy;             // Energy to support heat losses due to ambient temp (J)
        Real64 FlueLossEnergy;         // Energy to support heat losses to the flue (J)
        Real64 UseEnergy;              // Energy to heat the use side water to tank temp (J)
        Real64 TotalDemandEnergy;      // Total energy demand (sum of all above energies) (J)
        Real64 SourceEnergy;           // Energy supplied by the source side to help heat the tank (J)
        Real64 HeaterEnergy;           // The energy the water heater burner puts into the water (J)
        Real64 HeaterEnergy1;          // The energy heater 1 puts into the water (J)
        Real64 HeaterEnergy2;          // The energy heater 2 puts into the water (J)
        Real64 FuelEnergy;             // The fuel consumption energy for the water heater burner (J)
        Real64 FuelEnergy1;            // The fuel consumption energy for heater 1 (J)
        Real64 FuelEnergy2;            // The fuel consumption energy for heater 2 (J)
        Real64 VentEnergy;             // Heat recovery energy lost due to setpoint temp (J)
        Real64 OffCycParaFuelEnergy;   // Fuel consumption energy for off-cycle parasitic load (J)
        Real64 OffCycParaEnergyToTank; // Energy to tank for off-cycle parasitic load (J)
        Real64 OnCycParaFuelEnergy;    // Fuel consumption energy for on-cycle parasitic load (J)
        Real64 OnCycParaEnergyToTank;  // Energy to tank for on-cycle parasitic load (J)
        Real64 NetHeatTransferEnergy;  // Net heat transfer energy to/from tank (J)
        bool FirstRecoveryDone;        // Flag to indicate when first recovery to the setpoint is done
        Real64 FirstRecoveryFuel;      // Fuel energy needed for first recovery to the setpoint (J)
        int HeatPumpNum;               // Index to heat pump water heater
        int DesuperheaterNum;          // Index to desuperheating coil
        bool ShowSetPointWarning;      // Warn when set point is greater than max tank temp limit
        int MaxCycleErrorIndex;        // recurring error index
        int FreezingErrorIndex;        // recurring error index for freeze conditions
        WaterHeaterSizingData Sizing;  // ancillary data for autosizing

        Fluid::GlycolProps *water = nullptr; // Water properties

        bool MyOneTimeFlagWH; // first pass log
        bool MyTwoTimeFlagWH; // second pass do input check
        bool MyEnvrnFlag;
        bool WarmupFlag;
        bool SetLoopIndexFlag;
        bool AlreadyReported;
        bool AlreadyRated;
        bool MyHPSizeFlag;
        bool CheckWTTEquipName;

        std::string InletNodeName1;
        std::string OutletNodeName1;
        std::string InletNodeName2;
        std::string OutletNodeName2;

        bool myOneTimeInitFlag;
        bool scanPlantLoopsFlag;

        int callerLoopNum;
        int waterIndex;

        ~WaterThermalTankData() = default;

        // Default Constructor
        WaterThermalTankData()
            : WaterThermalTankType(DataPlant::PlantEquipmentType::Invalid), IsChilledWaterTank(false), Init(true), StandAlone(false), Volume(0.0),
              VolumeWasAutoSized(false), Mass(0.0), TimeElapsed(0.0), AmbientTempIndicator(WTTAmbientTemp::OutsideAir), AmbientTempZone(0),
              AmbientTempOutsideAirNode(0), AmbientTemp(0.0), AmbientZoneGain(0.0), LossCoeff(0.0), OffCycLossCoeff(0.0), OffCycLossFracToZone(0.0),
              OnCycLossCoeff(0.0), OnCycLossFracToZone(0.0), ControlType(HeaterControlMode::Cycle),
              StratifiedControlMode(PriorityControlMode::Invalid), MaxCapacity(0.0), MaxCapacityWasAutoSized(false), MinCapacity(0.0),
              Efficiency(0.0), PLFCurve(0), SetPointTemp(0.0), DeadBandDeltaTemp(0.0), TankTempLimit(0.0), IgnitionDelay(0.0), OffCycParaLoad(0.0),
              OffCycParaFracToTank(0.0), OnCycParaLoad(0.0), OnCycParaFracToTank(0.0), UseCurrentFlowLock(DataPlant::FlowLock::Unlocked),
              UseInletNode(0), UseInletTemp(0.0), UseOutletNode(0), UseOutletTemp(0.0), UseMassFlowRate(0.0), UseEffectiveness(0.0),
              PlantUseMassFlowRateMax(0.0), SavedUseOutletTemp(0.0), UseDesignVolFlowRate(0.0), UseDesignVolFlowRateWasAutoSized(false),
              UseBranchControlType(DataBranchAirLoopPlant::ControlType::Passive), UseSidePlantSizNum(0), UseSideSeries(true),
              UseSideLoadRequested(0.0), UseSidePlantLoc{}, SourceInletNode(0), SourceInletTemp(0.0), SourceOutletNode(0), SourceOutletTemp(0.0),
              SourceMassFlowRate(0.0), SourceEffectiveness(0.0), PlantSourceMassFlowRateMax(0.0), SavedSourceOutletTemp(0.0),
              SourceDesignVolFlowRate(0.0), SourceDesignVolFlowRateWasAutoSized(false),
              SourceBranchControlType(DataBranchAirLoopPlant::ControlType::Passive), SourceSidePlantSizNum(0), SourceSideSeries(true),
              SrcSidePlantLoc{}, SourceSideControlMode(SourceSideControl::IndirectHeatAltSetpoint), SizingRecoveryTime(0.0), VolFlowRateMax(0.0),
              MassFlowRateMax(0.0), VolFlowRateMin(0.0), MassFlowRateMin(0.0), TankTemp(0.0), SavedTankTemp(0.0), TankTempAvg(0.0), Height(0.0),
              HeightWasAutoSized(false), Perimeter(0.0), Shape(TankShape::VertCylinder), HeaterHeight1(0.0), HeaterNode1(0), HeaterOn1(false),
              SavedHeaterOn1(false), HeaterHeight2(0.0), HeaterNode2(0), HeaterOn2(false), SavedHeaterOn2(false), AdditionalCond(0.0),
              SetPointTemp2(0.0), DeadBandDeltaTemp2(0.0), MaxCapacity2(0.0), OffCycParaHeight(0.0), OnCycParaHeight(0.0), SkinLossCoeff(0.0),
              SkinLossFracToZone(0.0), OffCycFlueLossCoeff(0.0), OffCycFlueLossFracToZone(0.0), UseInletHeight(0.0), UseOutletHeight(0.0),
              UseOutletHeightWasAutoSized(false), SourceInletHeight(0.0), SourceInletHeightWasAutoSized(false), SourceOutletHeight(0.0),
              UseInletStratNode(0), UseOutletStratNode(0), SourceInletStratNode(0), SourceOutletStratNode(0), InletMode(InletPositionMode::Fixed),
              InversionMixingRate(0.0), Nodes(0), VolFlowRate(0.0), VolumeConsumed(0.0), UnmetRate(0.0), LossRate(0.0), FlueLossRate(0.0),
              UseRate(0.0), TotalDemandRate(0.0), SourceRate(0.0), HeaterRate(0.0), HeaterRate1(0.0), HeaterRate2(0.0), FuelRate(0.0), FuelRate1(0.0),
              FuelRate2(0.0), VentRate(0.0), OffCycParaFuelRate(0.0), OffCycParaRateToTank(0.0), OnCycParaFuelRate(0.0), OnCycParaRateToTank(0.0),
              NetHeatTransferRate(0.0), CycleOnCount(0), CycleOnCount1(0), CycleOnCount2(0), RuntimeFraction(0.0), RuntimeFraction1(0.0),
              RuntimeFraction2(0.0), PartLoadRatio(0.0), UnmetEnergy(0.0), LossEnergy(0.0), FlueLossEnergy(0.0), UseEnergy(0.0),
              TotalDemandEnergy(0.0), SourceEnergy(0.0), HeaterEnergy(0.0), HeaterEnergy1(0.0), HeaterEnergy2(0.0), FuelEnergy(0.0), FuelEnergy1(0.0),
              FuelEnergy2(0.0), VentEnergy(0.0), OffCycParaFuelEnergy(0.0), OffCycParaEnergyToTank(0.0), OnCycParaFuelEnergy(0.0),
              OnCycParaEnergyToTank(0.0), NetHeatTransferEnergy(0.0), FirstRecoveryDone(false), FirstRecoveryFuel(0.0), HeatPumpNum(0),
              DesuperheaterNum(0), ShowSetPointWarning(true), MaxCycleErrorIndex(0), FreezingErrorIndex(0), MyOneTimeFlagWH(true),
              MyTwoTimeFlagWH(true), MyEnvrnFlag(true), WarmupFlag(false), SetLoopIndexFlag(true), AlreadyReported(false), AlreadyRated(false),
              MyHPSizeFlag(true), CheckWTTEquipName(true), myOneTimeInitFlag(true), scanPlantLoopsFlag(true), callerLoopNum(0), waterIndex(1)
        {
        }

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void setupOutputVars(EnergyPlusData &state);

        void setupZoneInternalGains(EnergyPlusData &state);

        void setupChilledWaterTankOutputVars(EnergyPlusData &state);

        void setupWaterHeaterOutputVars(EnergyPlusData &state);

        void
        simulate(EnergyPlusData &state, const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag) override;

        Real64 PartLoadFactor(EnergyPlusData &state, Real64 PartLoadRatio_loc);

        void CalcNodeMassFlows(InletPositionMode inletMode);

        void SetupStratifiedNodes(EnergyPlusData &state);

        void initialize(EnergyPlusData &state, bool FirstHVACIteration);

        bool SourceHeatNeed(EnergyPlusData &state, Real64 OutletTemp, Real64 DeadBandTemp, Real64 SetPointTemp_loc);

        void SizeDemandSidePlantConnections(EnergyPlusData &state);

        void SizeTankForSupplySide(EnergyPlusData &state);

        void SizeTankForDemandSide(EnergyPlusData &state);

        void MinePlantStructForInfo(EnergyPlusData &state);

        void SizeSupplySidePlantConnections(EnergyPlusData &state, const int loopNum);

        void CalcWaterThermalTank(EnergyPlusData &state);

        void SizeStandAloneWaterHeater(EnergyPlusData &state);

        void UpdateWaterThermalTank(EnergyPlusData &state);

        void ReportWaterThermalTank(EnergyPlusData &state);

        void CalcWaterThermalTankStratified(EnergyPlusData &state); // Water Heater being simulated

        void CalcWaterThermalTankMixed(EnergyPlusData &state); // Water Heater being simulated

        void CalcStandardRatings(EnergyPlusData &state);

        void ReportCWTankInits(EnergyPlusData &state);

        Real64 GetHPWHSensedTankTemp(EnergyPlusData &state);

        Real64 FindStratifiedTankSensedTemp(EnergyPlusData &state, bool UseAverage = false);

        Real64 getDeadBandTemp();

        Real64 PlantMassFlowRatesFunc(EnergyPlusData &state,
                                      int InNodeNum,
                                      bool FirstHVACIteration,
                                      WaterHeaterSide WaterThermalTankSide,
                                      DataPlant::LoopSideLocation PlantLoopSide,
                                      bool PlumbedInSeries, // !unused1208
                                      DataBranchAirLoopPlant::ControlType BranchControlType,
                                      Real64 OutletTemp,
                                      Real64 DeadBandTemp,
                                      Real64 SetPointTemp_loc);

        static Real64 CalcTimeNeeded(Real64 Ti, // Initial tank temperature (C)
                                     Real64 Tf, // Final tank temperature (C)
                                     Real64 Ta, // Ambient environment temperature (C)
                                     Real64 T1, // Temperature of flow 1 (C)
                                     Real64 T2, // Temperature of flow 2 (C)
                                     Real64 m,  // Mass of tank fluid (kg)
                                     Real64 Cp, // Specific heat of fluid (J/kg deltaC)
                                     Real64 m1, // Mass flow rate 1 (kg/s)
                                     Real64 m2, // Mass flow rate 2 (kg/s)
                                     Real64 UA, // Heat loss coefficient to ambient environment (W/deltaC)
                                     Real64 Q   // Net heating rate for non-temp dependent sources, i.e. heater and parasitics (W)
        );

        static Real64 CalcTankTemp(Real64 Ti, // Initial tank temperature (C)
                                   Real64 Ta, // Ambient environment temperature (C)
                                   Real64 T1, // Temperature of flow 1 (C)
                                   Real64 T2, // Temperature of flow 2 (C)
                                   Real64 m,  // Mass of tank fluid (kg)
                                   Real64 Cp, // Specific heat of fluid (J/kg deltaC)
                                   Real64 m1, // Mass flow rate 1 (kg/s)
                                   Real64 m2, // Mass flow rate 2 (kg/s)
                                   Real64 UA, // Heat loss coefficient to ambient environment (W/deltaC)
                                   Real64 Q,  // Net heating rate for non-temp dependent sources, i.e. heater and parasitics (W)
                                   Real64 t   // Time elapsed from Ti to Tf (s)
        );

        static Real64 CalcTempIntegral(Real64 Ti, // Initial tank temperature (C)
                                       Real64 Tf, // Final tank temperature (C)
                                       Real64 Ta, // Ambient environment temperature (C)
                                       Real64 T1, // Temperature of flow 1 (C)
                                       Real64 T2, // Temperature of flow 2 (C)
                                       Real64 m,  // Mass of tank fluid (kg)
                                       Real64 Cp, // Specific heat of fluid (J/kg deltaC)
                                       Real64 m1, // Mass flow rate 1 (kg/s)
                                       Real64 m2, // Mass flow rate 2 (kg/s)
                                       Real64 UA, // Heat loss coefficient to ambient environment (W/deltaC)
                                       Real64 Q,  // Net heating rate for non-temp dependent sources, i.e. heater and parasitics (W)
                                       Real64 t   // Time elapsed from Ti to Tf (s)
        );

        static void CalcMixedTankSourceSideHeatTransferRate(Real64 HPWHCondenserDeltaT, // input, The temperature difference (C) across the heat pump,
                                                                                        // zero if there is no heat pump or if the heat pump is off
                                                            Real64 SourceInletTemp,     // input, Source inlet temperature (C)
                                                            Real64 Cp,                  // Specific heat of fluid (J/kg deltaC)
                                                            Real64 SetPointTemp,        // input, Mixed tank set point temperature
                                                            Real64 &SourceMassFlowRate, // source mass flow rate (kg/s)
                                                            Real64 &Qheatpump,          // heat transfer rate from heat pump
                                                            Real64 &Qsource // steady state heat transfer rate from a constant source side flow
        );

        void CalcDesuperheaterWaterHeater(EnergyPlusData &state, bool FirstHVACIteration);

        Real64 PLRResidualWaterThermalTank(EnergyPlusData &state,
                                           Real64 HPPartLoadRatio,    // compressor cycling ratio (1.0 is continuous, 0.0 is off)
                                           Array1D<Real64> const &Par // par(1) = HP set point temperature [C]
        );

        void CalcHeatPumpWaterHeater(EnergyPlusData &state, bool FirstHVACIteration);

        void ConvergeSingleSpeedHPWHCoilAndTank(EnergyPlusData &state, Real64 partLoadRatio);

        void SetVSHPWHFlowRates(EnergyPlusData &state,
                                HeatPumpWaterHeaterData &HPWH,
                                int SpeedNum,
                                Real64 SpeedRatio,
                                Real64 WaterDens,
                                Real64 &MdotWater,      // water flow rate
                                bool FirstHVACIteration // TRUE if First iteration of simulation
        );

        Real64 PLRResidualHPWH(EnergyPlusData &state, Real64 HPPartLoadRatio, Real64 desTankTemp, TankOperatingMode mode, Real64 mDotWater);

        Real64 PLRResidualIterSpeed(EnergyPlusData &state,
                                    Real64 SpeedRatio, // speed ratio between two speed levels
                                    int HPNum,
                                    int SpeedNum,
                                    int HPWaterInletNode,
                                    int HPWaterOutletNode,
                                    Real64 RhoWater,
                                    Real64 desTankTemp,
                                    TankOperatingMode mode,
                                    bool FirstHVACIteration);

        static void ValidatePLFCurve(EnergyPlusData &state, int CurveIndex, bool &IsValid);

        void onInitLoopEquip(EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation) override;

        void getDesignCapacities(EnergyPlusData &state,
                                 [[maybe_unused]] const PlantLocation &calledFromLocation,
                                 Real64 &MaxLoad,
                                 Real64 &MinLoad,
                                 Real64 &OptLoad) override;

        void oneTimeInit(EnergyPlusData &state) override;

        void setBackupElementCapacity(EnergyPlusData &state);
    };

    struct WaterHeaterDesuperheaterData
    {
        // Members
        std::string Name;                          // Name of heat pump water heater desuperheater
        std::string Type;                          // Type of water heater desuperheating coil
        int InsuffTemperatureWarn;                 // Used for recurring error count on low source temperature
        Sched::Schedule *availSched = nullptr;     // Availability Schedule
        Sched::Schedule *setptTempSched = nullptr; // Setpoint Temperature Schedule
        Real64 DeadBandTempDiff;                   // Dead band temperature difference (cut-in temperature)
        Real64 HeatReclaimRecoveryEff;             // recovery efficiency of desuperheater (0.3 max)
        int WaterInletNode;                        // Desuperheater water inlet node
        int WaterOutletNode;                       // Desuperheater water outlet node
        Real64 RatedInletWaterTemp;                // Inlet water temp at rated heat reclaim recovery eff (C)
        Real64 RatedOutdoorAirTemp;                // Outdoor air temp at rated heat reclaim recovery eff (C)
        Real64 MaxInletWaterTemp;                  // Max water temp for heat reclaim recovery (C)
        std::string TankType;                      // Type of water heater (MIXED or STRATIFIED)
        DataPlant::PlantEquipmentType TankTypeNum; // Parameter for tank type (MIXED or STRATIFIED)
        std::string TankName;                      // Name of tank associated with desuperheater
        int TankNum;
        bool StandAlone;                                            // Flag for operation with no plant connections (no use nodes)
        std::string HeatingSourceType;                              // Type of heating source (DX coil or refrigerated rack)
        std::string HeatingSourceName;                              // Name of heating source
        Real64 HeaterRate;                                          // Report variable for desuperheater heating rate [W]
        Real64 HeaterEnergy;                                        // Report variable for desuperheater heating energy [J]
        Real64 PumpPower;                                           // Report variable for water circulation pump power [W]
        Real64 PumpEnergy;                                          // Report variable for water circulation pump energy [J]
        Real64 PumpElecPower;                                       // Nominal power input to the water circulation pump [W]
        Real64 PumpFracToWater;                                     // Nominal power fraction to water for the water circulation pump
        Real64 OperatingWaterFlowRate;                              // Operating volumetric water flow rate (m3/s)
        int HEffFTemp;                                              // Heating capacity as a function of temperature curve index
        Real64 HEffFTempOutput;                                     // report variable for HEffFTemp curve
        Real64 SetPointTemp;                                        // set point or cut-out temperature [C]
        int WaterHeaterTankNum;                                     // Index of Water Heater Tank
        Real64 DesuperheaterPLR;                                    // part load ratio of desuperheater
        Real64 OnCycParaLoad;                                       // Rate for on-cycle parasitic load (W)
        Real64 OffCycParaLoad;                                      // Rate for off-cycle parasitic load (W)
        Real64 OnCycParaFuelEnergy;                                 // Electric energy consumption for on-cycle parasitic load (J)
        Real64 OnCycParaFuelRate;                                   // Electric consumption rate for on-cycle parasitic load (W)
        Real64 OffCycParaFuelEnergy;                                // Electric energy consumption for off-cycle parasitic load (J)
        Real64 OffCycParaFuelRate;                                  // Electric consumption rate for off-cycle parasitic load (W)
        TankOperatingMode Mode = TankOperatingMode::Floating;       // mode
        TankOperatingMode SaveMode = TankOperatingMode::Floating;   // desuperheater mode on first iteration
        TankOperatingMode SaveWHMode = TankOperatingMode::Floating; // mode of water heater tank element (backup element)
        Real64 BackupElementCapacity;                               // Tank backup element capacity (W)
        Real64 DXSysPLR;                                            // runtime fraction of desuperheater heating coil
        int ReclaimHeatingSourceIndexNum;                           // Index to reclaim heating source (condenser) of a specific type
        ReclaimHeatObjectType ReclaimHeatingSource;                 // The source for the Desuperheater Heating Coil
        int SetPointError;                                          // Used when temp SP in tank and desuperheater are reversed
        int SetPointErrIndex1;                                      // Index to recurring error for tank/desuperheater set point temp
        int IterLimitErrIndex1;                                     // Index for recurring iteration limit warning messages
        int IterLimitExceededNum1;                                  // Counter for recurring iteration limit warning messages
        int RegulaFalsiFailedIndex1;                                // Index for recurring RegulaFalsi failed warning messages
        int RegulaFalsiFailedNum1;                                  // Counter for recurring RegulaFalsi failed warning messages
        int IterLimitErrIndex2;                                     // Index for recurring iteration limit warning messages
        int IterLimitExceededNum2;                                  // Counter for recurring iteration limit warning messages
        int RegulaFalsiFailedIndex2;                                // Index for recurring RegulaFalsi failed warning messages
        int RegulaFalsiFailedNum2;                                  // Counter for recurring RegulaFalsi failed warning messages
        bool FirstTimeThroughFlag;                                  // Flag for saving water heater status
        bool ValidSourceType;

        std::string InletNodeName1;
        std::string OutletNodeName1;
        std::string InletNodeName2;
        std::string OutletNodeName2;

        // Default Constructor
        WaterHeaterDesuperheaterData()
            : InsuffTemperatureWarn(0), DeadBandTempDiff(0.0), HeatReclaimRecoveryEff(0.0), WaterInletNode(0), WaterOutletNode(0),
              RatedInletWaterTemp(0.0), RatedOutdoorAirTemp(0.0), MaxInletWaterTemp(0.0), TankTypeNum(DataPlant::PlantEquipmentType::Invalid),
              TankNum(0), StandAlone(false), HeaterRate(0.0), HeaterEnergy(0.0), PumpPower(0.0), PumpEnergy(0.0), PumpElecPower(0.0),
              PumpFracToWater(0.0), OperatingWaterFlowRate(0.0), HEffFTemp(0), HEffFTempOutput(0.0), SetPointTemp(0.0), WaterHeaterTankNum(0),
              DesuperheaterPLR(0.0), OnCycParaLoad(0.0), OffCycParaLoad(0.0), OnCycParaFuelEnergy(0.0), OnCycParaFuelRate(0.0),
              OffCycParaFuelEnergy(0.0), OffCycParaFuelRate(0.0), BackupElementCapacity(0.0), DXSysPLR(0.0), ReclaimHeatingSourceIndexNum(0),
              ReclaimHeatingSource(ReclaimHeatObjectType::DXCooling), SetPointError(0), SetPointErrIndex1(0), IterLimitErrIndex1(0),
              IterLimitExceededNum1(0), RegulaFalsiFailedIndex1(0), RegulaFalsiFailedNum1(0), IterLimitErrIndex2(0), IterLimitExceededNum2(0),
              RegulaFalsiFailedIndex2(0), RegulaFalsiFailedNum2(0), FirstTimeThroughFlag(true), ValidSourceType(false)
        {
        }
    };

    // Functions

    void SimulateWaterHeaterStandAlone(EnergyPlusData &state, int WaterHeaterNum, bool FirstHVACIteration);

    void SimHeatPumpWaterHeater(EnergyPlusData &state,
                                std::string_view CompName,
                                bool FirstHVACIteration,
                                Real64 &SensLoadMet, // sensible load met by this equipment and sent to zone, W
                                Real64 &LatLoadMet,  // net latent load met and sent to zone (kg/s), dehumid = negative
                                int &CompIndex);

    bool getDesuperHtrInput(EnergyPlusData &state);

    bool getHPWaterHeaterInput(EnergyPlusData &state);

    bool getWaterHeaterMixedInputs(EnergyPlusData &state);

    bool getWaterHeaterStratifiedInput(EnergyPlusData &state);

    bool getWaterTankMixedInput(EnergyPlusData &state);

    bool getWaterTankStratifiedInput(EnergyPlusData &state);

    void GetWaterThermalTankInput(EnergyPlusData &state);

    void CalcWaterThermalTankZoneGains(EnergyPlusData &state);

    int getTankIDX(EnergyPlusData &state, std::string_view CompName, int &CompIndex);

    int getHPTankIDX(EnergyPlusData &state, std::string_view CompName, int &CompIndex);

    bool GetHeatPumpWaterHeaterNodeNumber(EnergyPlusData &state, int NodeNumber);

    int getHeatPumpWaterHeaterIndex(EnergyPlusData &state, std::string_view CompName);

} // namespace WaterThermalTanks

struct WaterThermalTanksData : BaseGlobalStruct
{
    int numChilledWaterMixed = 0;        // number of mixed chilled water tanks
    int numChilledWaterStratified = 0;   // number of stratified chilled water tanks
    int numWaterHeaterMixed = 0;         // number of mixed water heaters
    int numWaterHeaterStratified = 0;    // number of stratified water heaters
    int numWaterThermalTank = 0;         // total number of water thermal tanks, hot and cold (MIXED + STRATIFIED)
    int numWaterHeaterDesuperheater = 0; // number of desuperheater heating coils
    int numHeatPumpWaterHeater = 0;      // number of heat pump water heaters
    int numWaterHeaterSizing = 0;        // Number of sizing/design objects for water heaters.

    Real64 hpPartLoadRatio = 0.0;       // part load ratio of HPWH
    Real64 mixerInletAirSchedule = 0.0; // output of inlet air mixer node schedule
    Real64 mdotAir = 0.0;               // mass flow rate of evaporator air, kg/s

    Array1D<WaterThermalTanks::WaterThermalTankData> WaterThermalTank;
    Array1D<WaterThermalTanks::HeatPumpWaterHeaterData> HPWaterHeater;
    Array1D<WaterThermalTanks::WaterHeaterDesuperheaterData> WaterHeaterDesuperheater;
    std::unordered_map<std::string, std::string> UniqueWaterThermalTankNames;

    bool getWaterThermalTankInputFlag = true; // Calls to Water Heater from multiple places in code
    bool calcWaterThermalTankZoneGainsMyEnvrnFlag = true;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) WaterThermalTanksData();
    }
};

} // namespace EnergyPlus

#endif
