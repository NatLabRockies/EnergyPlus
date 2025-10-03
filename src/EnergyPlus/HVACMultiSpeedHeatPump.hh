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

#ifndef HVACMultiSpeedHeatPump_hh_INCLUDED
#define HVACMultiSpeedHeatPump_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HVACMultiSpeedHeatPump {

    // Mode of operation
    enum class ModeOfOperation
    {
        Invalid = -1,
        CoolingMode, // System operating mode is cooling
        HeatingMode, // System operating mode is heating
        Num
    };

    // Airflow control for constant fan mode
    enum class AirflowControl
    {
        Invalid = -1,
        UseCompressorOnFlow,  // set compressor OFF air flow rate equal to compressor ON air flow rate
        UseCompressorOffFlow, // set compressor OFF air flow rate equal to user defined value
        Num
    };

    // Types

    struct MSHeatPumpData
    {
        // Members
        // Some variables in this type are arrays (dimension=MaxSpeed) to support the number of speeds
        std::string Name;                          // Name of the engine driven heat pump
        Sched::Schedule *availSched = nullptr;     // availability schedule
        int AirInletNodeNum;                       // Node number of the heat pump air inlet
        int AirOutletNodeNum;                      // Node number of the heat pump air inlet
        std::string AirInletNodeName;              // Node name of the heat pump air inlet
        std::string AirOutletNodeName;             // Node name of the heat pump air outlet
        int ControlZoneNum;                        // Controlling zone or thermostat location
        int ZoneSequenceCoolingNum;                // Index to cooling sequence/priority for this zone
        int ZoneSequenceHeatingNum;                // Index to heating sequence/priority for this zone
        std::string ControlZoneName;               // Controlled zone name
        int NodeNumOfControlledZone;               // Controlled zone node number
        Real64 FlowFraction;                       // Fraction of the total volume flow that goes through the controlling zone
        std::string FanName;                       // Name of supply air fan
        HVAC::FanType fanType;                     // Supply fan type
        int FanNum;                                // Supply fan number
        HVAC::FanPlace fanPlace;                   // Supply air fan placement: 1 Blow through; 2 Draw through
        int FanInletNode;                          // Fan Inlet node
        int FanOutletNode;                         // Fan Outlet node
        Real64 FanVolFlow;                         // Supply fan volumetric flow rate
        Sched::Schedule *fanOpModeSched = nullptr; // Supply air fan operating mode schedule
        HVAC::FanOp fanOp = HVAC::FanOp::Invalid;  // mode of operation; 1=cycling fan, cycling compressor; 2=continuous fan, cycling compresor
        std::string DXHeatCoilName;                // COIL:DX:MultiSpeed:Heating name
        int HeatCoilType;                          // Heating coil type: 1 COIL:DX:MultiSpeed:Heating only
        int HeatCoilNum;                           // Heating coil number
        int DXHeatCoilIndex;                       // DX heating coil index number
        std::string HeatCoilName;                  // Coil:Electric:MultiSpeed:Heating OR Coil:Gas:MultiSpeed:Heating name
        int HeatCoilIndex;                         // heating coil index number (Coil:Electric:MultiSpeed:Heating OR Coil:Gas:MultiSpeed:Heating)
        std::string DXCoolCoilName;                // COIL:DX:MultiSpeed:Cooling name
        int CoolCoilType;                          // Cooling coil type: 1 COIL:DX:MultiSpeed:Cooling only
        int DXCoolCoilIndex;                       // DX cooling coil index number
        std::string SuppHeatCoilName;              // Supplymental heating coil name
        int SuppHeatCoilType;                      // Supplymental heating coil type: 1 Gas; 2 Electric; 3 Recovery
        int SuppHeatCoilNum;                       // Supplymental heating coil number
        Real64 DesignSuppHeatingCapacity;          // Supplemental heating coil design capacity
        Real64 SuppMaxAirTemp;                     // Maximum supply air temperature from supplemental heater
        Real64 SuppMaxOATemp;                      // Maximum outdoor dry-bulb temperature for supplemental heater operation
        Real64 AuxOnCyclePower;                    // Auxiliary On-Cycle Electric Power
        Real64 AuxOffCyclePower;                   // Auxiliary Off-Cycle Electric Power
        Real64 DesignHeatRecFlowRate;              // Design water volume flow rate through heat recovery loop [m3/s]
        bool HeatRecActive;                        // True when entered Heat Rec Vol Flow Rate > 0
        std::string HeatRecName;                   // heat recovery water inlet name
        int HeatRecInletNodeNum;                   // Node number on heat recovery water inlet
        int HeatRecOutletNodeNum;                  // Node number on heat recovery water outlet
        Real64 MaxHeatRecOutletTemp;               // Maximum outlet water temperature for heat recovery
        Real64 DesignHeatRecMassFlowRate;          // Design water mass flow rate through heat recovery loop [kg/s]
        PlantLocation HRPlantLoc;                  // plant loop component for heat recovery
        Real64 AuxElecPower;                       // Auxiliary Electric Power
        Real64 IdleVolumeAirRate;                  // Supply air volumetric flow rate when no cooling or heating is needed
        Real64 IdleMassFlowRate;                   // Supply air mass flow rate when no cooling or heating is needed
        Real64 IdleSpeedRatio;                     // Fan speed ratio in idle mode
        int NumOfSpeedCooling;                     // The number of speeds for cooling
        int NumOfSpeedHeating;                     // The number of speeds for heating
        Array1D<Real64> HeatVolumeFlowRate;        // Supply air volume flow rate during heating operation
        Array1D<Real64> HeatMassFlowRate;          // Supply air mass flow rate during heating operation
        Array1D<Real64> CoolVolumeFlowRate;        // Supply air volume flow rate during cooling operation
        Array1D<Real64> CoolMassFlowRate;          // Supply air mass flow rate during cooling operation
        Array1D<Real64> HeatingSpeedRatio;         // Fan speed ratio in heating mode
        Array1D<Real64> CoolingSpeedRatio;         // Fan speed ratio in cooling mode
        bool CheckFanFlow;                         // Supply airflow check
        ModeOfOperation LastMode;                  // MSHP operation mode
        ModeOfOperation HeatCoolMode;              // System operating mode (0 = floating, 1 = cooling, 2 = heating)
        int AirLoopNumber;                         // Air loop served by the engine driven heat pump system
        int NumControlledZones;                    // Number of controlled zones for this system
        int ZoneInletNode;                         // Zone inlet node number in the controlled zone
        Real64 CompPartLoadRatio;                  // Compressor part load ratio
        Real64 FanPartLoadRatio;                   // Fan part load ratio
        Real64 TotCoolEnergyRate;                  // Total cooling enertgy rate
        Real64 TotHeatEnergyRate;                  // Total heating enertgy rate
        Real64 SensCoolEnergyRate;                 // Sensible cooling enertgy rate
        Real64 SensHeatEnergyRate;                 // Sensible heating enertgy rate
        Real64 LatCoolEnergyRate;                  // Latent cooling enertgy rate
        Real64 LatHeatEnergyRate;                  // Latent heating enertgy rate
        Real64 ElecPower;                          // Electric power (fan + supplemental electric coil)
        Real64 LoadMet;                            // met system load
        Real64 HeatRecoveryRate;                   // Heat recovery rate [W]
        Real64 HeatRecoveryInletTemp;              // Inlet temperature for heat recovery rate [C]
        Real64 HeatRecoveryOutletTemp;             // Outlet temperature for heat recovery rate [C]
        Real64 HeatRecoveryMassFlowRate;           // Mass flow rate for heat recovery rate [kg/s]
        AirflowControl AirFlowControl;             // fan control mode, UseCompressorOnFlow or UseCompressorOffFlow
        int ErrIndexCyc;                           // Error index at low speed
        int ErrIndexVar;                           // Error index at high speed
        Real64 LoadLoss;                           // Air distribution system loss
        int SuppCoilAirInletNode;                  // air inlet node number of supplemental heating coil
        int SuppCoilAirOutletNode;                 // air outlet node number of supplemental heating coil
        int SuppHeatCoilType_Num;                  // Numeric Equivalent for Supplemental Heat Coil Type
        int SuppHeatCoilIndex;                     // Index to supplemental heater
        int SuppCoilControlNode;                   // control node for simple water and steam heating coil
        Real64 MaxSuppCoilFluidFlow;               // water or steam mass flow rate for supplemental heating coil [kg/s]
        int SuppCoilOutletNode;                    // outlet node for hot water and steam supplemental heating coil
        int CoilAirInletNode;                      // air inlet node number of supplemental heating coil
        int CoilControlNode;                       // control node for simple water and steam heating coil
        Real64 MaxCoilFluidFlow;                   // water or steam mass flow rate for supplemental heating coil [kg/s]
        int CoilOutletNode;                        // outlet node for hot water and steam supplemental heating coil
        int HotWaterCoilControlNode;
        int HotWaterCoilOutletNode;
        std::string HotWaterCoilName;
        int HotWaterCoilNum;
        PlantLocation plantLoc;         // plant loop component location for hot water and steam heating coil
        PlantLocation SuppPlantLoc;     // plant loop component location for hot water and steam supplemental heating coil
        PlantLocation HotWaterPlantLoc; // plant loop component location for hot water and steam heating coil
        int HotWaterCoilMaxIterIndex;   // Index to recurring warning message
        int HotWaterCoilMaxIterIndex2;  // Index to recurring warning message
        int StageNum;                   // Stage number specified by staged thermostat
        bool Staged;                    // Using Staged thermostat
        int CoolCountAvail;             // Counter used to minimize the occurrence of output warnings
        int CoolIndexAvail;             // Index used to minimize the occurrence of output warnings
        int HeatCountAvail;             // Counter used to minimize the occurrence of output warnings
        int HeatIndexAvail;             // Index used to minimize the occurrence of output warnings
        bool FirstPass;                 // used to determine when first call is made
        Array1D<Real64> FullOutput;     // Full output for different speed
        Real64 MinOATCompressorCooling; // min OAT from multispeed cooling coil object
        Real64 MinOATCompressorHeating; // min OAT from multispeed heating coil object
        bool MyEnvrnFlag;
        bool MySizeFlag;
        bool MyCheckFlag;
        bool MyFlowFracFlag;
        bool MyPlantScantFlag;
        bool MyStagedFlag;
        bool EMSOverrideCoilSpeedNumOn;
        Real64 EMSOverrideCoilSpeedNumValue;
        int CoilSpeedErrIndex;
        Real64 HeatingSizingRatio = 1.0;
        bool isHeatPump = false;
        bool reportACCAManualS = true;

        // Default Constructor
        MSHeatPumpData()
            : AirInletNodeNum(0), AirOutletNodeNum(0), ControlZoneNum(0), ZoneSequenceCoolingNum(0), ZoneSequenceHeatingNum(0),
              NodeNumOfControlledZone(0), FlowFraction(0.0), fanType(HVAC::FanType::Invalid), FanNum(0), fanPlace(HVAC::FanPlace::Invalid),
              FanInletNode(0), FanOutletNode(0), FanVolFlow(0.0), HeatCoilType(0), HeatCoilNum(0), DXHeatCoilIndex(0), HeatCoilIndex(0),
              CoolCoilType(0), DXCoolCoilIndex(0), SuppHeatCoilType(0), SuppHeatCoilNum(0), DesignSuppHeatingCapacity(0.0), SuppMaxAirTemp(0.0),
              SuppMaxOATemp(0.0), AuxOnCyclePower(0.0), AuxOffCyclePower(0.0), DesignHeatRecFlowRate(0.0), HeatRecActive(false),
              HeatRecInletNodeNum(0), HeatRecOutletNodeNum(0), MaxHeatRecOutletTemp(0.0), DesignHeatRecMassFlowRate(0.0), HRPlantLoc{},
              AuxElecPower(0.0), IdleVolumeAirRate(0.0), IdleMassFlowRate(0.0), IdleSpeedRatio(0.0), NumOfSpeedCooling(0), NumOfSpeedHeating(0),
              CheckFanFlow(true), LastMode(ModeOfOperation::Invalid), HeatCoolMode(ModeOfOperation::Invalid), AirLoopNumber(0), NumControlledZones(0),
              ZoneInletNode(0), CompPartLoadRatio(0.0), FanPartLoadRatio(0.0), TotCoolEnergyRate(0.0), TotHeatEnergyRate(0.0),
              SensCoolEnergyRate(0.0), SensHeatEnergyRate(0.0), LatCoolEnergyRate(0.0), LatHeatEnergyRate(0.0), ElecPower(0.0), LoadMet(0.0),
              HeatRecoveryRate(0.0), HeatRecoveryInletTemp(0.0), HeatRecoveryOutletTemp(0.0), HeatRecoveryMassFlowRate(0.0),
              AirFlowControl(AirflowControl::Invalid), ErrIndexCyc(0), ErrIndexVar(0), LoadLoss(0.0), SuppCoilAirInletNode(0),
              SuppCoilAirOutletNode(0), SuppHeatCoilType_Num(0), SuppHeatCoilIndex(0), SuppCoilControlNode(0), MaxSuppCoilFluidFlow(0.0),
              SuppCoilOutletNode(0), CoilAirInletNode(0), CoilControlNode(0), MaxCoilFluidFlow(0.0), CoilOutletNode(0), HotWaterCoilControlNode(0),
              plantLoc{}, SuppPlantLoc{}, HotWaterPlantLoc{}, HotWaterCoilMaxIterIndex(0), HotWaterCoilMaxIterIndex2(0), StageNum(0), Staged(false),
              CoolCountAvail(0), CoolIndexAvail(0), HeatCountAvail(0), HeatIndexAvail(0), FirstPass(true), MinOATCompressorCooling(0.0),
              MinOATCompressorHeating(0.0), MyEnvrnFlag(true), MySizeFlag(true), MyCheckFlag(true), MyFlowFracFlag(true), MyPlantScantFlag(true),
              MyStagedFlag(true), EMSOverrideCoilSpeedNumOn(false), EMSOverrideCoilSpeedNumValue(0.0), CoilSpeedErrIndex(0)
        {
        }
    };

    struct MSHeatPumpReportData
    {
        // Members
        Real64 ElecPowerConsumption;   // Electricity Rate comsumption: CondenserFan+CrankcaseHeater+Defroster+aux
        Real64 HeatRecoveryEnergy;     // Heat recovery rate [J]
        Real64 CycRatio;               // Cycle ratio
        Real64 SpeedRatio;             // Speed ratio between two stages
        int SpeedNum;                  // Speed number
        Real64 AuxElecCoolConsumption; // Auxiliary Electricity Rate consumption during cooling
        Real64 AuxElecHeatConsumption; // Auxiliary Electricity Rate consumption during heating

        // Default Constructor
        MSHeatPumpReportData()
            : ElecPowerConsumption(0.0), HeatRecoveryEnergy(0.0), CycRatio(0.0), SpeedRatio(0.0), SpeedNum(0), AuxElecCoolConsumption(0.0),
              AuxElecHeatConsumption(0.0)
        {
        }
    };

    void SimMSHeatPump(EnergyPlusData &state,
                       std::string_view CompName,     // Name of the unitary engine driven heat pump system
                       bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system time step
                       int const AirLoopNum,          // air loop index
                       int &CompIndex                 // Index to changeover-bypass VAV system
    );

    //******************************************************************************

    void SimMSHP(EnergyPlusData &state,
                 int const MSHeatPumpNum,       // number of the current engine driven Heat Pump being simulated
                 bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                 int const AirLoopNum,          // air loop index
                 Real64 &QSensUnitOut,          // cooling/heating deliveded to zones [W]
                 Real64 const QZnReq,           // required zone load
                 Real64 &OnOffAirFlowRatio      // ratio of compressor ON airflow to AVERAGE airflow over timestep
    );

    //******************************************************************************

    void GetMSHeatPumpInput(EnergyPlusData &state);

    //******************************************************************************

    void InitMSHeatPump(EnergyPlusData &state,
                        int const MSHeatPumpNum,       // Engine driven heat pump number
                        bool const FirstHVACIteration, // TRUE if first HVAC iteration
                        int const AirLoopNum,          // air loop index
                        Real64 &QZnReq,                // Heating/Cooling load for all served zones
                        Real64 &OnOffAirFlowRatio      // Ratio of compressor ON airflow to average airflow over timestep
    );

    //******************************************************************************

    void SizeMSHeatPump(EnergyPlusData &state, int const MSHeatPumpNum); // Engine driven heat pump number

    //******************************************************************************

    void ControlMSHPOutput(EnergyPlusData &state,
                           int const MSHeatPumpNum,         // Unit index of engine driven heat pump
                           bool const FirstHVACIteration,   // flag for 1st HVAC iteration in the time step
                           HVAC::CompressorOp compressorOp, // compressor operation; 1=on, 0=off
                           HVAC::FanOp const fanOp,         // operating mode: FanOp::Cycling | FanOp::Continuous
                           Real64 const QZnReq,             // cooling or heating output needed by zone [W]
                           int const ZoneNum,               // Index to zone number
                           int &SpeedNum,                   // Speed number
                           Real64 &SpeedRatio,              // unit speed ratio for DX coils
                           Real64 &PartLoadFrac,            // unit part load fraction
                           Real64 &OnOffAirFlowRatio,       // ratio of compressor ON airflow to AVERAGE airflow over timestep
                           Real64 &SupHeaterLoad            // Supplemental heater load [W]
    );

    void ControlMSHPSupHeater(EnergyPlusData &state,
                              int const MSHeatPumpNum,         // Unit index of engine driven heat pump
                              bool const FirstHVACIteration,   // flag for 1st HVAC iteration in the time step
                              HVAC::CompressorOp compressorOp, // compressor operation; 1=on, 0=off
                              HVAC::FanOp const fanOp,         // operating mode: FanOp::Cycling | FanOp::Continuous
                              Real64 const QZnReq,             // cooling or heating output needed by zone [W]
                              int const FullOutput,            // unit full output when compressor is operating [W]vvvv
                              int const SpeedNum,              // Speed number
                              Real64 SpeedRatio,               // unit speed ratio for DX coils
                              Real64 PartLoadFrac,             // unit part load fraction
                              Real64 OnOffAirFlowRatio,        // ratio of compressor ON airflow to AVERAGE airflow over timestep
                              Real64 &SupHeaterLoad            // Supplemental heater load [W]

    );

    void ControlMSHPOutputEMS(EnergyPlusData &state,
                              int const MSHeatPumpNum,         // Unit index of engine driven heat pump
                              bool const FirstHVACIteration,   // flag for 1st HVAC iteration in the time step
                              HVAC::CompressorOp compressorOp, // compressor operation; 1=on, 0=off
                              HVAC::FanOp const fanOp,         // operating mode: FanOp::Cycling | FanOp::Continuous
                              Real64 const QZnReq,             // cooling or heating output needed by zone [W]
                              Real64 const SpeedVal,           // continuous speed value
                              int &SpeedNum,                   // discrete speed level
                              Real64 &SpeedRatio,              // unit speed ratio for DX coils
                              Real64 &PartLoadFrac,            // unit part load fraction
                              Real64 &OnOffAirFlowRatio,       // ratio of compressor ON airflow to AVERAGE airflow over timestep
                              Real64 &SupHeaterLoad            // Supplemental heater load [W]

    );

    //******************************************************************************

    void CalcMSHeatPump(EnergyPlusData &state,
                        int const MSHeatPumpNum,         // Engine driven heat pump number
                        bool const FirstHVACIteration,   // Flag for 1st HVAC iteration
                        HVAC::CompressorOp compressorOp, // Compressor on/off; 1=on, 0=off
                        int const SpeedNum,              // Speed number
                        Real64 const SpeedRatio,         // Compressor speed ratio
                        Real64 const PartLoadFrac,       // Compressor part load fraction
                        Real64 &LoadMet,                 // Load met by unit (W)
                        Real64 const QZnReq,             // Zone load (W)
                        Real64 &OnOffAirFlowRatio,       // Ratio of compressor ON airflow to AVERAGE airflow over timestep
                        Real64 &SupHeaterLoad            // supplemental heater load (W)
    );

    void UpdateMSHeatPump(EnergyPlusData &state, int const MSHeatPumpNum); // Engine driven heat pump number

    void ReportMSHeatPump(EnergyPlusData &state, int const MSHeatPumpNum); // Engine driven heat pump number

    void MSHPHeatRecovery(EnergyPlusData &state,
                          int const MSHeatPumpNum); // Number of the current electric MSHP being simulated

    void SetAverageAirFlow(EnergyPlusData &state,
                           int const MSHeatPumpNum,                         // Unit index
                           Real64 const PartLoadRatio,                      // unit part load ratio
                           Real64 &OnOffAirFlowRatio,                       // ratio of compressor ON airflow to average airflow over timestep
                           ObjexxFCL::Optional_int_const SpeedNum = _,      // Speed number
                           ObjexxFCL::Optional<Real64 const> SpeedRatio = _ // Speed ratio
    );

    void CalcNonDXHeatingCoils(EnergyPlusData &state,
                               int const MSHeatPumpNum,       // multispeed heatpump index
                               bool const FirstHVACIteration, // flag for first HVAC iteration in the time step
                               Real64 const HeatingLoad,      // supplemental coil load to be met by unit (watts)
                               HVAC::FanOp const fanOp,       // fan operation mode
                               Real64 &HeatCoilLoadmet,       // Heating Load Met
                               ObjexxFCL::Optional<Real64 const> PartLoadFrac = _);

} // namespace HVACMultiSpeedHeatPump

struct HVACMultiSpeedHeatPumpData : BaseGlobalStruct
{

    int NumMSHeatPumps = 0;     // Number of multi speed heat pumps
    int AirLoopPass = 0;        // Number of air loop pass
    Real64 TempSteamIn = 100.0; // steam coil steam inlet temperature

    std::string CurrentModuleObject; // Object type for getting and error messages
    Real64 CompOnMassFlow = 0.0;     // System air mass flow rate w/ compressor ON
    Real64 CompOffMassFlow = 0.0;    // System air mass flow rate w/ compressor OFF
    Real64 CompOnFlowRatio = 0.0;    // fan flow ratio when coil on
    Real64 CompOffFlowRatio = 0.0;   // fan flow ratio when coil off
    Real64 FanSpeedRatio = 0.0;      // fan speed ratio passed to on/off fan object
    Real64 SupHeaterLoad = 0.0;      // load to be met by supplemental heater [W]
    Real64 SaveLoadResidual = 0.0;   // Saved load residual used to check convergence
    Real64 SaveCompressorPLR = 0.0;  // holds compressor PLR from active DX coil
    Array1D_bool CheckEquipName;

    // SUBROUTINE SPECIFICATIONS FOR MODULE

    // Object Data
    Array1D<HVACMultiSpeedHeatPump::MSHeatPumpData> MSHeatPump;
    Array1D<HVACMultiSpeedHeatPump::MSHeatPumpReportData> MSHeatPumpReport;

    bool GetInputFlag = true;      // Get input flag
    bool FlowFracFlagReady = true; // one time flag for calculating flow fraction through controlled zone
    int ErrCountCyc = 0;           // Counter used to minimize the occurrence of output warnings
    int ErrCountVar = 0;           // Counter used to minimize the occurrence of output warnings

    std::string HeatCoilName; // TODO: What's the best plan here?

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumMSHeatPumps = 0;
        this->AirLoopPass = 0;
        this->TempSteamIn = 100.0;
        this->CurrentModuleObject = "";
        this->CompOnMassFlow = 0.0;
        this->CompOffMassFlow = 0.0;
        this->CompOnFlowRatio = 0.0;
        this->CompOffFlowRatio = 0.0;
        this->FanSpeedRatio = 0.0;
        this->SupHeaterLoad = 0.0;
        this->SaveLoadResidual = 0.0;
        this->SaveCompressorPLR = 0.0;
        this->CheckEquipName.clear();
        this->MSHeatPump.clear();
        this->MSHeatPumpReport.clear();
        this->GetInputFlag = true; // Get input flag
        this->FlowFracFlagReady = true;
        this->ErrCountCyc = 0;
        this->ErrCountVar = 0;
        this->HeatCoilName = "";
    }
};

} // namespace EnergyPlus

#endif
