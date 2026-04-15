// EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the University of Illinois,
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

#ifndef VentilatedSlab_hh_INCLUDED
#define VentilatedSlab_hh_INCLUDED

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

namespace VentilatedSlab {

    // Parameters for outside air control types:
    enum class OutsideAirControlType
    {
        Invalid = -1,
        VariablePercent,
        FixedTemperature,
        FixedOAControl,
        Num
    };

    enum class CoilsUsed
    {
        Invalid = -1,
        None,
        Heating,
        Cooling,
        Both,
        Num
    };

    //  Control Types
    enum class ControlType
    {
        Invalid = -1,
        MeanAirTemp,        // Controls system using mean air temperature
        MeanRadTemp,        // Controls system using mean radiant temperature
        OperativeTemp,      // Controls system using operative temperature
        OutdoorDryBulbTemp, // Controls system using outside air dry-bulb temperature
        OutdoorWetBulbTemp, // Controls system using outside air wet-bulb temperature
        SurfaceTemp,        // Controls system using surface temperature !Phase2-A
        DewPointTemp,       // Controls system using dew-point temperature of zone!Phase2-A
        Num
    };

    // Ventilated Slab Configurations
    enum class VentilatedSlabConfig
    {
        Invalid = -1,
        SlabOnly,    // Air circulate through cores of slab only
        SlabAndZone, // Circulated Air is introduced to zone
        SeriesSlabs,
        Num
    };

    struct VentilatedSlabData
    {
        // Members
        // Input data
        std::string Name;                      // name of system
        Sched::Schedule *availSched = nullptr; // schedule
        int ZonePtr;                           // Point to this zone in the Zone derived type
        // Variables for Delivery Config.
        Array1D_string ZName;            // Name of zone the system is serving
        Array1D_int ZPtr;                // Point to this zone in the Zone derived type
        std::string SurfListName;        // Name of surface/surface list that is the radiant system
        int NumOfSurfaces;               // Number of surfaces included in this system (coordinated control)
        Array1D_int SurfacePtr;          // Pointer to the slabs in the Surface derived type
        Array1D_string SurfaceName;      // Name of surfaces that are the radiant system (can be one or more)
        Array1D<Real64> SurfaceFlowFrac; // Fraction of flow/pipe length for a particular surface
        Array1D<Real64> CDiameter;       // Number of core diameter
        Array1D<Real64> CLength;         // Number of core length
        Array1D<Real64> CNumbers;        // Number of core numbers
        Array1D_string SlabIn;           // Name of node that is slab inlet node
        Array1D_string SlabOut;          // Name of node that is slab outlet node
        Real64 TotalSurfaceArea;         // Total surface area for all surfaces that are part of this system
        Real64 CoreDiameter;             // tube diameter for embedded tubing
        Real64 CoreLength;               // tube length embedded in radiant surface
        Real64 CoreNumbers;              // tube length embedded in radiant surface
        ControlType controlType;         // Control type for the system
        // (MAT, MRT, Op temp, ODB, OWB, DPTZ, Surf Temp.)
        int ReturnAirNode; // inlet air node number
        int RadInNode;     // outlet air node number
        int ZoneAirInNode; // outlet air node number
        int FanOutletNode; // outlet node number for fan exit
        // (assumes fan is upstream of heating coil)
        int MSlabInNode;
        int MSlabOutNode;
        std::string FanName;   // name of fan
        int Fan_Index;         // index of fan in array or vector
        HVAC::FanType fanType; // type of fan
        int ControlCompTypeNum;
        int CompErrIndex;
        Real64 MaxAirVolFlow;                        // m3/s
        Real64 MaxAirMassFlow;                       // kg/s
        OutsideAirControlType outsideAirControlType; // type of control; options are VARIABLE PERCENT and FIXED TEMPERATURE
        Sched::Schedule *minOASched = nullptr;
        Sched::Schedule *maxOASched = nullptr;
        // temperature (fixed temp.)
        Sched::Schedule *tempSched = nullptr;
        int OutsideAirNode;                                    // outside air node number
        int AirReliefNode;                                     // relief air node number
        int OAMixerOutNode;                                    // outlet node after the outside air mixer (inlet to coils if present)
        Real64 OutAirVolFlow;                                  // m3/s
        Real64 OutAirMassFlow;                                 // kg/s
        Real64 MinOutAirVolFlow;                               // m3/s
        Real64 MinOutAirMassFlow;                              // kg/s
        VentilatedSlabConfig SysConfg;                         // type of coil option; options are BOTH, HEATING, COOLING, AND NONE
        CoilsUsed coilsUsed = CoilsUsed::Invalid;              // type of coil option; options are BOTH, HEATING, COOLING, AND NONE
        bool heatingCoilPresent;                               // .TRUE. if ventilated slab has a heating coil
        HVAC::CoilType heatCoilType = HVAC::CoilType::Invalid; // type of heating coil (water, gas, electric, etc.)
        std::string heatingCoilName;                           // name of heating coil
        std::string heatingCoilTypeCh;                         // type of heating coil (character string)
        int heatingCoil_Index;
        DataPlant::PlantEquipmentType heatingCoilType;

        Fluid::RefrigProps *heatingCoil_fluid = nullptr;
        Sched::Schedule *heatingCoilSched = nullptr; // index to schedule

        Real64 heatingCoilSchedValue;
        Real64 MaxVolHotWaterFlow; // m3/s
        Real64 MaxVolHotSteamFlow; // m3/s
        Real64 MaxHotWaterFlow;    // kg/s
        Real64 MaxHotSteamFlow;
        Real64 MinHotSteamFlow;
        Real64 MinVolHotWaterFlow;                    // m3/s
        Real64 MinVolHotSteamFlow;                    // m3/s
        Real64 MinHotWaterFlow;                       // kg/s
        int HotControlNode;                           // hot water control node
        int HotCoilOutNodeNum;                        // outlet of coil
        Real64 HotControlOffset;                      // control tolerance
        PlantLocation HWPlantLoc;                     // index for plant component for hot water coil
        Sched::Schedule *hotAirHiTempSched = nullptr; // Schedule for the highest Air temperature
        Sched::Schedule *hotAirLoTempSched = nullptr; // Schedule for the lowest Air temperature
        // (where the lowest Air temperature is requested)
        Sched::Schedule *hotCtrlHiTempSched = nullptr; // Schedule for the highest control temperature
        // (where the lowest Air temperature is requested)
        // (where the highest Air temperature is requested)
        Sched::Schedule *hotCtrlLoTempSched = nullptr; // Schedule for the lowest control temperature
        // (where the highest Air temperature is requested)
        bool coolingCoilPresent;       // .TRUE. if ventilated slab has a cooling coil
        std::string coolingCoilName;   // name of cooling coil
        std::string coolingCoilTypeCh; // type of cooling coil (character string)
        int coolingCoil_Index;
        std::string coolingCoilPlantName; // name of cooling coil (child<=CoilSystem:Cooling:Water:HeatExchangerAssisted)
        std::string coolingCoilPlantType; // type of cooling coil (child<=CoilSystem:Cooling:Water:HeatExchangerAssisted)
        DataPlant::PlantEquipmentType coolingCoilType;
        HVAC::CoilType coolCoilType = HVAC::CoilType::Invalid; // type of cooling coil:
        // 'Coil:Cooling:Water:DetailedGeometry' or
        // 'CoilSystem:Cooling:Water:HeatExchangerAssisted'
        Sched::Schedule *coolingCoilSched = nullptr;
        Real64 coolingCoilSchedValue;
        Real64 MaxVolColdWaterFlow;                    // m3/s
        Real64 MaxColdWaterFlow;                       // kg/s
        Real64 MinVolColdWaterFlow;                    // m3/s
        Real64 MinColdWaterFlow;                       // kg/s
        int ColdControlNode;                           // chilled water control node
        int ColdCoilOutNodeNum;                        // chilled water coil out nod
        Real64 ColdControlOffset;                      // control tolerance
        PlantLocation CWPlantLoc;                      // index for plant component for chilled water coil
        Sched::Schedule *coldAirHiTempSched = nullptr; // Schedule for the highest Air temperature
        Sched::Schedule *coldAirLoTempSched = nullptr; // Schedule for the lowest Air temperature
        // (where the lowest Air temperature is requested)
        Sched::Schedule *coldCtrlHiTempSched = nullptr; // Schedule for the highest control temperature
        // (where the lowest Air temperature is requested)
        // (where the highest Air temperature is requested)
        Sched::Schedule *coldCtrlLoTempSched = nullptr; // Schedule for the lowest control temperature
        // (where the highest Air temperature is requested)
        int CondErrIndex;       // Error index for recurring warning messages
        int EnrgyImbalErrIndex; // Error index for recurring warning messages
        int RadSurfNum;         // Radiant Surface Number
        int MSlabIn;            // Internal Slab Inlet Node Number
        int MSlabOut;           // INternal Slab Outlet Node Number
        // Report data
        Real64 DirectHeatLossPower;  // system direct heat loss in W
        Real64 DirectHeatLossEnergy; // system direct heat loss in J
        Real64 DirectHeatGainPower;  // system direct heat gain in W
        Real64 DirectHeatGainEnergy; // system direct heat gain in J
        Real64 TotalVentSlabRadPower;
        Real64 RadHeatingPower;  // radiant heating output in watts
        Real64 RadHeatingEnergy; // radiant heating output in J
        Real64 RadCoolingPower;  // radiant cooling output in watts
        Real64 RadCoolingEnergy; // radiant cooling output in J
        Real64 HeatCoilPower;
        Real64 HeatCoilEnergy;
        Real64 TotCoolCoilPower;
        Real64 TotCoolCoilEnergy;
        Real64 SensCoolCoilPower;
        Real64 SensCoolCoilEnergy;
        Real64 LateCoolCoilPower;
        Real64 LateCoolCoilEnergy;
        Real64 ElecFanPower;
        Real64 ElecFanEnergy;
        Real64 AirMassFlowRate; // Circulated air mass flow rate in kg/s
        Real64 AirVolFlow;      // Circulated air volumetric flow rate in m3/s
        Real64 SlabInTemp;      // Slab inlet temp in degree C
        Real64 SlabOutTemp;     // Slab outlet temp in degree C
        Real64 ReturnAirTemp;
        Real64 FanOutletTemp;             // FanOutlet temp in degree C
        Real64 ZoneInletTemp;             // supply air temp
        std::string AvailManagerListName; // Name of an availability manager list object
        Avail::Status availStatus = Avail::Status::NoAction;
        int HVACSizingIndex;                 // index of a HVACSizing object for a ventilator slab
        bool FirstPass;                      // detects first time through for resetting sizing data
        Real64 ZeroVentSlabSourceSumHATsurf; // Equal to SumHATsurf for all the walls in a zone with no source
        Array1D<Real64> QRadSysSrcAvg;       // Average source over the time step for a particular radiant surfaceD
        // Record keeping variables used to calculate QRadSysSrcAvg locally
        Array1D<Real64> LastQRadSysSrc; // Need to keep the last value in case we are still iterating
        Real64 LastSysTimeElapsed;      // Need to keep the last value in case we are still iterating
        Real64 LastTimeStepSys;         // Need to keep the last value in case we are still iterating

        // Default Constructor
        VentilatedSlabData()
            : ZonePtr(0), NumOfSurfaces(0), TotalSurfaceArea(0.0), CoreDiameter(0.0), CoreLength(0.0), CoreNumbers(0.0),
              controlType(ControlType::Invalid), ReturnAirNode(0), RadInNode(0), ZoneAirInNode(0), FanOutletNode(0), MSlabInNode(0), MSlabOutNode(0),
              Fan_Index(0), fanType(HVAC::FanType::Invalid), ControlCompTypeNum(0), CompErrIndex(0), MaxAirVolFlow(0.0), MaxAirMassFlow(0.0),
              outsideAirControlType(OutsideAirControlType::Invalid), OutsideAirNode(0), AirReliefNode(0), OAMixerOutNode(0), OutAirVolFlow(0.0),
              OutAirMassFlow(0.0), MinOutAirVolFlow(0.0), MinOutAirMassFlow(0.0), SysConfg(VentilatedSlabConfig::Invalid), heatingCoilPresent(false),

              heatingCoil_Index(0), heatingCoilType(DataPlant::PlantEquipmentType::Invalid), heatingCoilSchedValue(0.0), MaxVolHotWaterFlow(0.0),
              MaxVolHotSteamFlow(0.0), MaxHotWaterFlow(0.0), MaxHotSteamFlow(0.0), MinHotSteamFlow(0.0), MinVolHotWaterFlow(0.0),
              MinVolHotSteamFlow(0.0), MinHotWaterFlow(0.0), HotControlNode(0), HotCoilOutNodeNum(0), HotControlOffset(0.0), HWPlantLoc{},
              coolingCoilPresent(false), coolingCoil_Index(0), coolingCoilType(DataPlant::PlantEquipmentType::Invalid), coolingCoilSchedValue(0.0),
              MaxVolColdWaterFlow(0.0), MaxColdWaterFlow(0.0), MinVolColdWaterFlow(0.0), MinColdWaterFlow(0.0), ColdControlNode(0),
              ColdCoilOutNodeNum(0), ColdControlOffset(0.0), CWPlantLoc{}, CondErrIndex(0), EnrgyImbalErrIndex(0), RadSurfNum(0), MSlabIn(0),
              MSlabOut(0), DirectHeatLossPower(0.0), DirectHeatLossEnergy(0.0), DirectHeatGainPower(0.0), DirectHeatGainEnergy(0.0),
              TotalVentSlabRadPower(0.0), RadHeatingPower(0.0), RadHeatingEnergy(0.0), RadCoolingPower(0.0), RadCoolingEnergy(0.0),
              HeatCoilPower(0.0), HeatCoilEnergy(0.0), TotCoolCoilPower(0.0), TotCoolCoilEnergy(0.0), SensCoolCoilPower(0.0), SensCoolCoilEnergy(0.0),
              LateCoolCoilPower(0.0), LateCoolCoilEnergy(0.0), ElecFanPower(0.0), ElecFanEnergy(0.0), AirMassFlowRate(0.0), AirVolFlow(0.0),
              SlabInTemp(0.0), SlabOutTemp(0.0), ReturnAirTemp(0.0), FanOutletTemp(0.0), ZoneInletTemp(0.0), HVACSizingIndex(0), FirstPass(true),
              ZeroVentSlabSourceSumHATsurf(0.0), QRadSysSrcAvg(0.0), LastQRadSysSrc(0.0), LastSysTimeElapsed(0.0), LastTimeStepSys(0.0)
        {
        }
    };

    struct VentSlabNumericFieldData
    {
        // Members
        Array1D_string FieldNames;

        // Default Constructor
        VentSlabNumericFieldData()
        {
        }
    };

    // Functions

    void SimVentilatedSlab(EnergyPlusData &state,
                           std::string const &CompName,   // name of the fan coil unit
                           int const ZoneNum,             // number of zone being served
                           bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                           Real64 &PowerMet,              // Sensible power supplied (W)
                           Real64 &LatOutputProvided,     // Latent add/removal supplied by window AC (kg/s), dehumid = negative
                           int &CompIndex);

    void GetVentilatedSlabInput(EnergyPlusData &state);

    void InitVentilatedSlab(EnergyPlusData &state,
                            int const Item,               // index for the current ventilated slab
                            int const VentSlabZoneNum,    // number of zone being served
                            bool const FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
    );

    void SizeVentilatedSlab(EnergyPlusData &state, int const Item);

    void CalcVentilatedSlab(EnergyPlusData &state,
                            int &Item,                     // number of the current ventilated slab being simulated
                            int const ZoneNum,             // number of zone being served
                            bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                            Real64 &PowerMet,              // power supplied (W)
                            Real64 &LatOutputProvided      // latent capacity supplied (kg/s)
    );

    void CalcVentilatedSlabComps(EnergyPlusData &state,
                                 int const Item,                // system index in ventilated slab array
                                 bool const FirstHVACIteration, // flag for 1st HVAV iteration in the time step
                                 Real64 &LoadMet                // load met by the system (watts)
    );

    void CalcVentilatedSlabCoilOutput(EnergyPlusData &state,
                                      int const Item,           // system index in ventilated slab array
                                      Real64 &PowerMet,         // power supplied (W)
                                      Real64 &LatOutputProvided // latent capacity supplied (kg/s)
    );

    void CalcVentilatedSlabRadComps(EnergyPlusData &state,
                                    int const Item,               // System index in ventilated slab array
                                    bool const FirstHVACIteration // flag for 1st HVAV iteration in the time step !unused1208
    );

    void SimVentSlabOAMixer(EnergyPlusData &state, int const Item); // System index in Ventilated Slab array

    void UpdateVentilatedSlab(EnergyPlusData &state,
                              int const Item,               // Index for the ventilated slab under consideration within the derived types
                              bool const FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep !unused1208
    );

    Real64 CalcVentSlabHXEffectTerm(EnergyPlusData &state,
                                    int const Item,            // Index number of radiant system under consideration
                                    Real64 const Temperature,  // Temperature of air entering the radiant system, in C
                                    Real64 const AirMassFlow,  // Mass flow rate of water in the radiant system, in kg/s
                                    Real64 const FlowFraction, // Mass flow rate fraction for this surface in the radiant system
                                    Real64 const CoreLength,   // Length of tubing in the radiant system, in m
                                    Real64 const CoreDiameter, // Inside diameter of the tubing in the radiant system, in m
                                    Real64 const CoreNumbers);

    void ReportVentilatedSlab(EnergyPlusData &state, int const Item); // Index for the ventilated slab under consideration within the derived types

    int getVentilatedSlabIndex(EnergyPlusData &state, std::string_view CompName);
    //*****************************************************************************************

} // namespace VentilatedSlab

struct VentilatedSlabData : BaseGlobalStruct
{

    int OperatingMode = 0; // Used to keep track of whether system is in heating or cooling mode

    // MODULE VARIABLE DECLARATIONS:
    bool HCoilOn = false;        // TRUE if the heating coil (gas or electric especially) should be running
    int NumOfVentSlabs = 0;      // Number of ventilated slab in the input file
    Real64 OAMassFlowRate = 0.0; // Outside air mass flow rate for the ventilated slab
    int MaxCloNumOfSurfaces = 0; // Used to set allocate size in CalcClo routine
    Real64 QZnReq = 0.0;         // heating or cooling needed by system [watts]

    Array1D_bool CheckEquipName;

    // Autosizing variables
    bool GetInputFlag = true;
    bool MyOneTimeFlag = true;
    Array1D_bool MySizeFlag;

    // Object Data
    EPVector<VentilatedSlab::VentilatedSlabData> VentSlab;
    EPVector<VentilatedSlab::VentSlabNumericFieldData> VentSlabNumericFields;

    bool ZoneEquipmentListChecked = false; // True after the Zone Equipment List has been checked for items
    Array1D_bool MyEnvrnFlag;
    Array1D_bool MyPlantScanFlag;
    Array1D_bool MyZoneEqFlag; // used to set up zone equipment availability managers

    Array1D<Real64> AirTempOut; // Array of outlet air temperatures for each surface in the radiant system

    int CondensationErrorCount = 0;    // Counts for # times the radiant systems are shutdown due to condensation
    int EnergyImbalanceErrorCount = 0; // Counts for # times a temperature mismatch is found in the energy balance check
    bool FirstTimeFlag = true;         // for setting size of AirTempOut array

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->MyOneTimeFlag = true;
        this->GetInputFlag = true;
        this->HCoilOn = false;
        this->NumOfVentSlabs = 0;
        this->OAMassFlowRate = 0.0;
        this->MaxCloNumOfSurfaces = 0;
        this->QZnReq = 0.0;
        this->CheckEquipName.deallocate();
        this->MySizeFlag.deallocate();
        this->VentSlab.deallocate();
        this->VentSlabNumericFields.deallocate();
        this->ZoneEquipmentListChecked = false;
        this->MyEnvrnFlag.deallocate();
        this->MyPlantScanFlag.deallocate();
        this->MyZoneEqFlag.deallocate();
        this->AirTempOut.deallocate();
        this->CondensationErrorCount = 0;
        this->EnergyImbalanceErrorCount = 0;
        this->FirstTimeFlag = true;
    }

    // Default Constructor
    VentilatedSlabData()
    {
    }
};

} // namespace EnergyPlus

#endif
