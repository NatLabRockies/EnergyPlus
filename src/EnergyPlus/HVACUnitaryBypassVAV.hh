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

#ifndef HVACUnitaryBypassVAV_hh_INCLUDED
#define HVACUnitaryBypassVAV_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HVACUnitaryBypassVAV {

    // Using/Aliasing

    // Data
    // MODULE PARAMETER DEFINITIONS

    // Mode of operation
    // can't change these to enum class since these are used in SetupOutputVariable()
    int constexpr CoolingMode(1); // System operating mode is cooling
    int constexpr HeatingMode(2); // System operating mode is heating

    enum class DehumidControl // Dehumidification control modes (DehumidControlMode) for Multimode units only
    {
        // TODO: enum check
        Invalid = -1,
        None,
        Multimode,
        CoolReheat,
        Num
    };

    enum class PriorityCtrlMode // Priority control mode (prioritized thermostat signal)
    {
        Invalid = -1,
        CoolingPriority, // Controls CBVAV system based on cooling priority
        HeatingPriority, // Controls CBVAV system based on heating priority
        ZonePriority,    // Controls CBVAV system based on number of zones priority
        LoadPriority,    // Controls CBVAV system based on total load priority
        Num
    };

    enum class AirFlowCtrlMode // Airflow control for contant fan mode
    {
        Invalid = -1,
        UseCompressorOnFlow,  // Set compressor OFF air flow rate equal to compressor ON air flow rate
        UseCompressorOffFlow, // Set compressor OFF air flow rate equal to user defined value
        Num
    };

    // SUBROUTINE SPECIFICATIONS FOR MODULE

    struct CBVAVData
    {
        std::string Name;                       // Name of unit
        std::string UnitType;                   // Type of unit
        std::string availSchedName;             // Availability schedule name
        Sched::Schedule *availSched = nullptr;  // availability schedule
        Real64 MaxCoolAirVolFlow = 0.0;         // System air volumetric flow rate during cooling operation [m3/s]
        Real64 MaxHeatAirVolFlow = 0.0;         // System air volumetric flow rate during heating operation [m3/s]
        Real64 MaxNoCoolHeatAirVolFlow = 0.0;   // System air volumetric flow rate when no cooling or heating [m3/s]
        Real64 MaxCoolAirMassFlow = 0.0;        // System air mass flow rate during cooling operation [kg/s]
        Real64 MaxHeatAirMassFlow = 0.0;        // System air mass flow rate during heating operation [kg/s]
        Real64 MaxNoCoolHeatAirMassFlow = 0.0;  // System air mass flow rate when no cooling or heating [kg/s]
        Real64 CoolOutAirVolFlow = 0.0;         // OA volumetric flow rate during cooling operation [m3/s]
        Real64 HeatOutAirVolFlow = 0.0;         // OA volumetric flow rate during heating operation [m3/s]
        Real64 NoCoolHeatOutAirVolFlow = 0.0;   // OA volumetric flow rate when no cooling or heating [m3/s]
        Real64 CoolOutAirMassFlow = 0.0;        // OA mass flow rate during cooling operation [kg/s]
        Real64 HeatOutAirMassFlow = 0.0;        // OA mass flow rate during heating operation [kg/s]
        Real64 NoCoolHeatOutAirMassFlow = 0.0;  // OA mass flow rate when no cooling or heating [kg/s]
        Sched::Schedule *outAirSched = nullptr; // outside air multiplier schedule
        int AirInNode = 0;                      // Inlet air node number for CBVAV unit
        int AirOutNode = 0;                     // Outlet air node number for CBVAV unit
        int CondenserNodeNum = 0;               // DX Coil condenser air inlet node number
        int MixerOutsideAirNode = 0;            // Outside air node number for OA mixer
        int MixerMixedAirNode = 0;              // Mixed air node number for OA mixer
        int MixerReliefAirNode = 0;             // Relief air node number for OA mixer
        int MixerInletAirNode = 0;              // Return air node number for OA mixer
        int SplitterOutletAirNode = 0;          // Air node number for splitter (last component outlet node)
        int PlenumMixerInletAirNode = 0;        // only used when bypass is connected to plenum or mixer
        std::string OAMixType;                  // type of outside air mixer
        std::string OAMixName;                  // Name of OA mixer
        int OAMixIndex = 0;                     // Index to OA mixer
        std::string FanName;                    // Name of fan
        HVAC::FanType fanType = HVAC::FanType::Invalid;
        HVAC::FanPlace fanPlace = HVAC::FanPlace::Invalid; // Fan placement is either blowthru (1) or drawthru (2)
        int FanIndex = 0;                                  // Index number to fan
        Sched::Schedule *fanOpModeSched = nullptr;         // Fan operating mode schedule
        Real64 FanVolFlow = 0.0;                           // Volumetric flow rate of system supply air fan [m3/s]
        Real64 HeatingSpeedRatio = 1.0;                    // Fan speed ratio in heating mode
        Real64 CoolingSpeedRatio = 1.0;                    // Fan speed ratio in cooling mode
        Real64 NoHeatCoolSpeedRatio = 1.0;                 // Fan speed ratio when no cooling or heating
        bool CheckFanFlow = true;                          // Check fan volumetric flow versus system flow in init routine.
        std::string DXCoolCoilName;                        // Name of DX cooling coil
        HVAC::CoilType CoolCoilType = HVAC::CoilType::Invalid;
        int CoolCoilCompIndex = 0;  // cooling coil component index number
        int DXCoolCoilIndexNum = 0; // actual DX cooling coil index number
        int DXHeatCoilIndexNum = 0; // actual DX heating coil index number
        std::string HeatCoilName;   // Name of heating coil
        HVAC::CoilType HeatCoilType = HVAC::CoilType::Invalid;
        int HeatCoilIndex = 0;                    // DX heating coil index number
        HVAC::FanOp fanOp = HVAC::FanOp::Invalid; // mode of operation
        int CoilControlNode = 0;                  // heating coil hot water or steam inlet node
        int CoilOutletNode = 0;                   // outlet node for hot water and steam coil
        PlantLocation plantLoc;                   // plant loop component location object for water heating coil
        int HotWaterCoilMaxIterIndex = 0;         // Index to recurring warning message
        int HotWaterCoilMaxIterIndex2 = 0;        // Index to recurring warning message
        Real64 MaxHeatCoilFluidFlow = 0.0;        // water or steam mass flow rate for heating coil [kg/s]
        Real64 DesignHeatingCapacity = 0.0;       // design heating capacity of the heating coil
        Real64 DesignSuppHeatingCapacity = 0.0;   // Operating capacity of supplemental Heating Coil [W]
        Real64 MinOATCompressor = 0.0;            // Minimum OAT for compressor operation [C]
        Real64 MinLATCooling = 0.0;               // Minimum leaving air temp for compressor cooling operation [C]
        Real64 MaxLATHeating = 0.0;               // Maximum leaving air temp for heating operation [C]
        // Report data
        Real64 TotHeatEnergyRate = 0.0;                            // Total heating output [W]
        Real64 TotHeatEnergy = 0.0;                                // Total heating output [J]
        Real64 TotCoolEnergyRate = 0.0;                            // Total cooling output [W]
        Real64 TotCoolEnergy = 0.0;                                // Total cooling output [J]
        Real64 SensHeatEnergyRate = 0.0;                           // Sensible heating output [W]
        Real64 SensHeatEnergy = 0.0;                               // Sensible heating output [J]
        Real64 SensCoolEnergyRate = 0.0;                           // Sensible cooling output [W]
        Real64 SensCoolEnergy = 0.0;                               // Sensible cooling output [J]
        Real64 LatHeatEnergyRate = 0.0;                            // Latent heating output [W]
        Real64 LatHeatEnergy = 0.0;                                // Latent heating output [J]
        Real64 LatCoolEnergyRate = 0.0;                            // Latent cooling output [W]
        Real64 LatCoolEnergy = 0.0;                                // Latent cooling output [J]
        Real64 ElecPower = 0.0;                                    // Electricity consumed [W]
        Real64 ElecConsumption = 0.0;                              // Electricity consumed [J]
        Real64 FanPartLoadRatio = 0.0;                             // Fan part-load ratio for time step
        Real64 CompPartLoadRatio = 0.0;                            // Compressor part-load ratio for time step
        int LastMode = 0;                                          // Last mode of operation, coolingmode or heatingmode
        AirFlowCtrlMode AirFlowControl = AirFlowCtrlMode::Invalid; // Fan control mode, UseCompressorOnFlow or UseCompressorOffFlow
        Real64 CompPartLoadFrac = 0.0;                             // Compressor part load ratio
        int AirLoopNumber = 0;                                     // Air loop served by the CBVAV system
        int NumControlledZones = 0;
        Array1D_int ControlledZoneNum;                                // Index to controlled zones
        Array1D_int ControlledZoneNodeNum;                            // Zone node num of controlled zone
        Array1D_int CBVAVBoxOutletNode;                               // Outlet node of CBVAV Box in controlled zone
        Array1D_int ZoneSequenceCoolingNum;                           // Index to cooling sequence/priority for this zone
        Array1D_int ZoneSequenceHeatingNum;                           // Index to heating sequence/priority for this zone
        PriorityCtrlMode PriorityControl = PriorityCtrlMode::Invalid; // Control mode - CoolingPriority, HeatingPriority, ZonePriority or LoadPriority
        int NumZonesCooled = 0;                                       // Number of zones requesting cooling
        int NumZonesHeated = 0;                                       // Number of zones requesting heating
        int PLRMaxIter = 0;                                           // Counter for recurring warning message
        int PLRMaxIterIndex = 0;                                      // Index to recurring warning message
        int DXCoilInletNode = 0;                                      // Inlet node number of DX cooling coil
        int DXCoilOutletNode = 0;                                     // Outlet node number of DX cooling coil
        int HeatingCoilInletNode = 0;                                 // Inlet node of heating coil
        int HeatingCoilOutletNode = 0;                                // Outlet node of heating coil
        int FanInletNodeNum = 0;                                      // fan inlet node number
        Real64 OutletTempSetPoint = 0.0;                              // Oulet node temperature setpoint [C]
        Real64 CoilTempSetPoint = 0.0;                                // Coil oulet node temperature setpoint (inc. fan heat) [C]
        int HeatCoolMode = 0;                                         // System operating mode (0 = floating, 1 = cooling, 2 = heating)
        Real64 BypassMassFlowRate = 0.0;                              // Bypass mass flow rate report variable [m3/s]
        HVAC::CoilMode DehumidificationMode = HVAC::CoilMode::Invalid; // Dehumidification mode (0=normal, 1=enhanced)
        DehumidControl DehumidControlType = DehumidControl::None;      // Dehumidification control type (currently only for multimode coil)
        bool HumRatMaxCheck = true;                                    // Used in Init for warning messages
        int DXIterationExceeded = 0;                                   // Counter for DX coil messages
        int DXIterationExceededIndex = 0;                              // Counter for DX coil messages
        int DXIterationFailed = 0;                                     // Counter for DX coil messages
        int DXIterationFailedIndex = 0;                                // Counter for DX coil messages
        int DXCyclingIterationExceeded = 0;                            // Counter for VS DX coil messages, when on/off cycling between off and speed 1
        int DXCyclingIterationExceededIndex = 0;                       // Counter for VS DX coil messages, when on/off cycling between off and speed 1
        int DXCyclingIterationFailed = 0;                              // Counter for VS DX coil messages, when on/off cycling between off and speed 1
        int DXCyclingIterationFailedIndex = 0;                         // Counter for VS DX coil messages, when on/off cycling between off and speed 1
        int DXHeatIterationExceeded = 0;                               // Counter for DX coil messages
        int DXHeatIterationExceededIndex = 0;                          // Counter for DX coil messages
        int DXHeatIterationFailed = 0;                                 // Counter for DX coil messages
        int DXHeatIterationFailedIndex = 0;                            // Counter for DX coil messages
        int DXHeatCyclingIterationExceeded = 0;                        // Counter for VS DX coil messages, when on/off cycling between off and speed 1
        int DXHeatCyclingIterationExceededIndex = 0;                   // Counter for VS DX coil messages, when on/off cycling between off and speed 1
        int DXHeatCyclingIterationFailed = 0;                          // Counter for VS DX coil messages, when on/off cycling between off and speed 1
        int DXHeatCyclingIterationFailedIndex = 0;                     // Counter for VS DX coil messages, when on/off cycling between off and speed 1
        int HXDXIterationExceeded = 0;                                 // Counter for HX assisted DX coil messages
        int HXDXIterationExceededIndex = 0;                            // Counter for HX assisted DX coil messages
        int HXDXIterationFailed = 0;                                   // Counter for HX assisted DX coil messages
        int HXDXIterationFailedIndex = 0;                              // Counter for HX assisted DX coil messages
        int MMDXIterationExceeded = 0;                                 // Counter for multimode DX coil messages
        int MMDXIterationExceededIndex = 0;                            // Counter for multimode DX coil messages
        int MMDXIterationFailed = 0;                                   // Counter for multimode DX coil messages
        int MMDXIterationFailedIndex = 0;                              // Counter for multimode DX coil messages
        int DMDXIterationExceeded = 0;                                 // Counter for dehumidifying multimode DX coil messages
        int DMDXIterationExceededIndex = 0;                            // Counter for dehumidifying multimode DX coil messages
        int DMDXIterationFailed = 0;                                   // Counter for dehumidifying multimode DX coil messages
        int DMDXIterationFailedIndex = 0;                              // Counter for dehumidifying multimode DX coil messages
        int CRDXIterationExceeded = 0;                                 // Counter for cool reheat multimode DX coil messages
        int CRDXIterationExceededIndex = 0;                            // Counter for cool reheat multimode DX coil messages
        int CRDXIterationFailed = 0;                                   // Counter for cool reheat multimode DX coil messages
        int CRDXIterationFailedIndex = 0;                              // Counter for cool reheat multimode DX coil messages
        bool FirstPass = true;                                         // used to determine when first call is made
        int plenumIndex = 0;                                           // index to AirloopHVAC:ReturnPlenum
        int mixerIndex = 0;                                            // index to AirloopHVAC:ZoneMixer
        Real64 changeOverTimer = -1.0;                                 // timer to avoid rapid change of operating modes (e.g., cooling to heating)
        Real64 minModeChangeTime = -1.0;                               // time limit before mode change is allowed (hr)
        int OutNodeSPMIndex = 0;                                       // set point manager index if exists on outlet node
        bool modeChanged = false;                                      // identifies operating mode changed this time step
    };

    void SimUnitaryBypassVAV(EnergyPlusData &state,
                             std::string_view CompName, // Name of the CBVAV system
                             bool FirstHVACIteration,   // TRUE if 1st HVAC simulation of system time step
                             int AirLoopNum,            // air loop index
                             int &CompIndex             // Index to changeover-bypass VAV system
    );

    void SimCBVAV(EnergyPlusData &state,
                  int CBVAVNum,              // Index of the current CBVAV system being simulated
                  bool FirstHVACIteration,   // TRUE if 1st HVAC simulation of system timestep
                  Real64 &QSensUnitOut,      // Sensible delivered capacity [W]
                  Real64 &OnOffAirFlowRatio, // Ratio of compressor ON airflow to AVERAGE airflow over timestep
                  bool HXUnitOn              // flag to enable heat exchanger
    );

    void GetCBVAV(EnergyPlusData &state);

    void InitCBVAV(EnergyPlusData &state,
                   int CBVAVNum,              // Index of the current CBVAV unit being simulated
                   bool FirstHVACIteration,   // TRUE if first HVAC iteration
                   int AirLoopNum,            // air loop index
                   Real64 &OnOffAirFlowRatio, // Ratio of compressor ON airflow to average airflow over timestep
                   bool HXUnitOn              // flag to enable heat exchanger
    );

    void SizeCBVAV(EnergyPlusData &state, int CBVAVNum); // Index to CBVAV system

    void ControlCBVAVOutput(EnergyPlusData &state,
                            int CBVAVNum,              // Index to CBVAV system
                            bool FirstHVACIteration,   // Flag for 1st HVAC iteration
                            Real64 &PartLoadFrac,      // Unit part load fraction
                            Real64 &OnOffAirFlowRatio, // Ratio of compressor ON airflow to AVERAGE airflow over timestep
                            bool HXUnitOn              // flag to enable heat exchanger
    );

    void CalcCBVAV(EnergyPlusData &state,
                   int CBVAVNum,              // Unit index in fan coil array
                   bool FirstHVACIteration,   // Flag for 1st HVAC iteration
                   Real64 &PartLoadFrac,      // Compressor part load fraction
                   Real64 &LoadMet,           // Load met by unit (W)
                   Real64 &OnOffAirFlowRatio, // Ratio of compressor ON airflow to AVERAGE airflow over timestep
                   bool HXUnitOn              // flag to enable heat exchanger
    );

    void GetZoneLoads(EnergyPlusData &state, int CBVAVNum // Index to CBVAV unit being simulated
    );

    Real64 CalcSetPointTempTarget(EnergyPlusData &state, int CBVAVNumber); // Index to changeover-bypass VAV system

    void SetAverageAirFlow(EnergyPlusData &state,
                           int CBVAVNum,             // Index to CBVAV system
                           Real64 &OnOffAirFlowRatio // Ratio of compressor ON airflow to average airflow over timestep
    );

    void ReportCBVAV(EnergyPlusData &state, int CBVAVNum); // Index of the current CBVAV unit being simulated

    void CalcNonDXHeatingCoils(EnergyPlusData &state,
                               int CBVAVNum,            // Changeover bypass VAV unit index
                               bool FirstHVACIteration, // flag for first HVAC iteration in the time step
                               Real64 &HeatCoilLoad,    // heating coil load to be met (Watts)
                               HVAC::FanOp const fanOp, // fan operation mode
                               Real64 &HeatCoilLoadmet  // coil heating load met
    );

} // namespace HVACUnitaryBypassVAV

struct HVACUnitaryBypassVAVData : BaseGlobalStruct
{

    // TODO: Try to eliminate these global-ish variables.  Just use data stored right in the component instances.
    int NumCBVAV = 0;                    // Number of CBVAV systems in input file
    Real64 CompOnMassFlow = 0.0;         // System air mass flow rate w/ compressor ON
    Real64 OACompOnMassFlow = 0.0;       // OA mass flow rate w/ compressor ON
    Real64 CompOffMassFlow = 0.0;        // System air mass flow rate w/ compressor OFF
    Real64 OACompOffMassFlow = 0.0;      // OA mass flow rate w/ compressor OFF
    Real64 CompOnFlowRatio = 0.0;        // fan flow ratio when coil on
    Real64 CompOffFlowRatio = 0.0;       // fan flow ratio when coil off
    Real64 FanSpeedRatio = 0.0;          // ratio of air flow ratio passed to fan object
    Real64 BypassDuctFlowFraction = 0.0; // Fraction of unit mass flow that returns to inlet of CBVAV unit through bypass duct
    Real64 PartLoadFrac = 0.0;           // Compressor part-load fraction
    Real64 SaveCompressorPLR = 0.0;      // Holds DX compressor PLR from active DX coil
    Real64 TempSteamIn = 100.0;          // steam coil steam inlet temperature
    Array1D_bool CheckEquipName;

    EPVector<HVACUnitaryBypassVAV::CBVAVData> CBVAV;
    bool GetInputFlag = true; // Flag set to make sure you get input once

    bool MyOneTimeFlag = true; // Initialization flag
    // TODO: Get rid of these arrays, make them bools in the data structure and probably get rid of one time flag
    Array1D_bool MyEnvrnFlag;     // Used for initializations each begin environment flag
    Array1D_bool MySizeFlag;      // Used for sizing CBVAV inputs one time
    Array1D_bool MyPlantScanFlag; // Used for initializations plant component for heating coils

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->CBVAV.deallocate();
        this->NumCBVAV = 0;
        this->CompOnMassFlow = 0.0;
        this->OACompOnMassFlow = 0.0;
        this->CompOffMassFlow = 0.0;
        this->OACompOffMassFlow = 0.0;
        this->CompOnFlowRatio = 0.0;
        this->CompOffFlowRatio = 0.0;
        this->FanSpeedRatio = 0.0;
        this->BypassDuctFlowFraction = 0.0;
        this->PartLoadFrac = 0.0;
        this->SaveCompressorPLR = 0.0;
        this->TempSteamIn = 100.0;
        this->CheckEquipName.deallocate();
        this->GetInputFlag = true;
        this->MyOneTimeFlag = true;
        this->MyEnvrnFlag.deallocate();
        this->MySizeFlag.deallocate();
        this->MyPlantScanFlag.deallocate();
    }
};

} // namespace EnergyPlus

#endif
