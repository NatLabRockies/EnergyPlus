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

#ifndef FanCoilUnits_hh_INCLUDED
#define FanCoilUnits_hh_INCLUDED

// C++ Headers
#include <memory>
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/SystemAvailabilityManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace FanCoilUnits {

    Real64 constexpr Small5WLoad = 5.0; // load threshold 5.0 W

    // coil type units supported in this module
    constexpr int FanCoilUnit_4Pipe(1);

    enum class CCoil
    {
        Invalid = -1,
        Water,
        Detailed,
        HXAssist,
        Num
    };

    enum class HCoil
    {
        Invalid = -1,
        Water,
        Electric,
        Num
    };

    enum class CCM // capacity control method supported in this module
    {
        Invalid = -1,
        ConsFanVarFlow,
        CycFan,
        VarFanVarFlow,
        VarFanConsFlow,
        MultiSpeedFan,
        ASHRAE,
        Num
    };

    struct FanCoilData
    {
        // Members
        // Input data
        int UnitType_Num = 0;
        std::string availSchedName;                     // availability schedule
        Sched::Schedule *availSched = nullptr;          // availability schedule
        std::string SchedOutAir;                        // outside air schedule, multipliy maximum outdoor air flow rate
        Sched::Schedule *oaSched = nullptr;             // outside air schedule
        HVAC::FanType fanType = HVAC::FanType::Invalid; // fan type
        int SpeedFanSel = 0;                            // Speed fan selected
        CCM CapCtrlMeth_Num = CCM::Invalid;
        Real64 PLR = 0.0;             // Part Load Ratio, fraction of time step fancoil is on
        int MaxIterIndexH = 0;        // Maximum iterations exceeded for heating
        int BadMassFlowLimIndexH = 0; // Bad mass flow limit error index for heating
        int MaxIterIndexC = 0;        // Maximum iterations exceeded for cooling
        int BadMassFlowLimIndexC = 0; // Bad mass flow limit error index for cooling
        Real64 FanAirVolFlow = 0.0;   // m3/s
        Real64 MaxAirVolFlow = 0.0;   // m3/s
        Real64 MaxAirMassFlow = 0.0;  // kg/s
        Real64 LowSpeedRatio = 0.0;   // Low speed fan supply air flow ratio
        Real64 MedSpeedRatio = 0.0;   // Medium speed fan supply air flow ratio
        Real64 SpeedFanRatSel = 0.0;  // Speed fan ratio determined by fan speed selection at each timestep
        Real64 OutAirVolFlow = 0.0;   // m3/s
        Real64 OutAirMassFlow = 0.0;  // kg/s
        int AirInNode = 0;            // inlet air node number
        int AirOutNode = 0;           // outlet air node number
        int OutsideAirNode = 0;       // outside air node number
        int AirReliefNode = 0;        // relief air node number
        int MixedAirNode = 0;         // Mixed Air Node number
        std::string OAMixName;        // name of outside air mixer
        std::string OAMixType;        // type of outside air mixer
        int OAMixIndex = 0;
        std::string FanName;     // name of fan
        int FanIndex = 0;        // index for fan
        std::string CCoilName;   // name of cooling coil
        int CCoilName_Index = 0; // Index for this Cooling Coil in SimWaterComp
        std::string CCoilType;   // type of cooling coil:
        // 'Coil:Cooling:Water' or
        // 'Coil:Cooling:Water:DetailedGeometry' or
        // 'CoilSystem:Cooling:Water:HeatExchangerAssisted'
        CCoil CCoilType_Num = CCoil::Invalid; // Numeric equivalent for type of cooling coil
        std::string CCoilPlantName;           // name of cooling coil (child<=CoilSystem:Cooling:Water:HeatExchangerAssisted)
        DataPlant::PlantEquipmentType CCoilPlantType = DataPlant::PlantEquipmentType::Invalid;
        int ControlCompTypeNum = 0;
        int CompErrIndex = 0;
        Real64 MaxColdWaterVolFlow = 0.0; // m3/s
        Real64 MinColdWaterVolFlow = 0.0; // m3/s
        Real64 MinColdWaterFlow = 0.0;    // kg/s
        Real64 ColdControlOffset = 0.0;   // control tolerance
        std::string HCoilName;            // name of heating coil
        int HCoilName_Index = 0;
        std::string HCoilType; // type of heating coil:
        // 'Coil:Heating:Water' or
        HCoil HCoilType_Num = HCoil::Invalid; // Numeric equivalent for type of cooling coil
        DataPlant::PlantEquipmentType HCoilPlantTypeOf = DataPlant::PlantEquipmentType::Invalid;
        Real64 MaxHotWaterVolFlow = 0.0;    // m3/s
        Real64 MinHotWaterVolFlow = 0.0;    // m3/s
        Real64 MinHotWaterFlow = 0.0;       // kg/s
        Real64 HotControlOffset = 0.0;      // control tolerance
        Real64 DesignHeatingCapacity = 0.0; // size of electric heating coil [W]
        Avail::Status availStatus = Avail::Status::NoAction;
        std::string AvailManagerListName; // Name of an availability manager list object
        // addition for OA to Zone Units
        std::string ATMixerName;                                // name of air terminal mixer
        int ATMixerIndex = 0;                                   // index to the air terminal mixer
        HVAC::MixerType ATMixerType = HVAC::MixerType::Invalid; // 1 = inlet side mixer, 2 = supply side mixer
        int ATMixerPriNode = 0;                                 // primary inlet air node number for the air terminal mixer
        int ATMixerSecNode = 0;                                 // secondary air inlet node number for the air terminal mixer
        int HVACSizingIndex = 0;                                // index of a HVACSizing object for a fancoil unit
        Real64 SpeedRatio = 0.0;                                // speed ratio when the fan is cycling between stages
        Sched::Schedule *fanOpModeSched = nullptr;              // supply air fan operating mode schedule
        HVAC::FanOp fanOp = HVAC::FanOp::Cycling;               // 1=cycling fan cycling coil; 2=constant fan cycling coil
        bool ASHRAETempControl = false;                         // ASHRAE90.1 control to temperature set point when true
        Real64 QUnitOutNoHC = 0.0;                              // unit output with coils off [W]
        Real64 QUnitOutMaxH = 0.0;                              // unit output at maximum heating [W]
        Real64 QUnitOutMaxC = 0.0;                              // unit output at maximum cooling [W]
        int LimitErrCountH = 0;                                 // count of SolveRoot limit errors
        int LimitErrCountC = 0;                                 // count of SolveRoot limit errors
        int ConvgErrCountH = 0;                                 // count of SolveRoot iteration limit errors
        int ConvgErrCountC = 0;                                 // count of SolveRoot iteration limit errors
        // Report data
        Real64 HeatPower = 0.0;                   // unit heating output in watts
        Real64 HeatEnergy = 0.0;                  // unit heating output in J
        Real64 TotCoolPower = 0.0;                // unit total cooling power output in watts
        Real64 TotCoolEnergy = 0.0;               // unit total cooling energy output in joules
        Real64 SensCoolPower = 0.0;               // unit sensible cooling power output in watts
        Real64 SensCoolEnergy = 0.0;              // unit sensible cooling energy output in joules
        Real64 ElecPower = 0.0;                   // unit electric power consumption in watts
        Real64 ElecEnergy = 0.0;                  // unit electiric energy consumption in joules
        Real64 DesCoolingLoad = 0.0;              // used for reporting in watts
        Real64 DesHeatingLoad = 0.0;              // used for reporting in watts
        Real64 DesZoneCoolingLoad = 0.0;          // used for reporting in watts
        Real64 DesZoneHeatingLoad = 0.0;          // used for reporting in watts
        int DSOAPtr = 0;                          // design specification outdoor air object index
        bool FirstPass = true;                    // detects first time through for resetting sizing data
        Sched::Schedule *fanAvailSched = nullptr; // fan availability schedule index

        // SZVAV Model inputs
        std::string Name;                      // name of unit
        std::string UnitType;                  // type of unit
        Real64 MaxCoolCoilFluidFlow = 0.0;     // kg/s
        Real64 MaxHeatCoilFluidFlow = 0.0;     // kg/s
        Real64 DesignMinOutletTemp = 0.0;      // ASHRAE90.1 maximum supply air temperature in Cooling mode
        Real64 DesignMaxOutletTemp = 0.0;      // ASHRAE90.1 maximum supply air temperature in Heating mode
        Real64 MaxNoCoolHeatAirMassFlow = 0.0; // minimum air flow rate using constant fan and ASHRAE90.1 control method
        Real64 MaxCoolAirMassFlow = 0.0;       // used in ASHRAE90.1 model, same as MaxAirMassFlow
        Real64 MaxHeatAirMassFlow = 0.0;       // used in ASHRAE90.1 model, same as MaxAirMassFlow
        Real64 LowSpeedCoolFanRatio = 0.0;     // ratio of min air flow to max air flow
        Real64 LowSpeedHeatFanRatio = 0.0;     // ratio of min air flow to max air flow
        int CoolCoilFluidInletNode = 0;        // chilled water control node
        int CoolCoilFluidOutletNodeNum = 0;    // chilled water coil outlet plant node
        int HeatCoilFluidInletNode = 0;        // hot water control node
        int HeatCoilFluidOutletNodeNum = 0;    // hot water coil outlet plant node
        PlantLocation CoolCoilPlantLoc{};      // index for plant location for chilled water coil
        PlantLocation HeatCoilPlantLoc{};      // index for plant location for hot water coil
        int CoolCoilInletNodeNum = 0;          // index of cooling coil inlet node number
        int CoolCoilOutletNodeNum = 0;         // index of cooling coil outlet node number
        int HeatCoilInletNodeNum = 0;          // index of heating coil inlet node number
        int HeatCoilOutletNodeNum = 0;         // index of heating coil outlet node number
        int ControlZoneNum = 0;                // pointer to a zone served by a fancoil unit
        int NodeNumOfControlledZone = 0;       // node number of controlled zone
        bool ATMixerExists = false;            // True if there is an ATMixer
        int ATMixerOutNode = 0;                // outlet air node number for the air terminal mixer
        Real64 FanPartLoadRatio = 0.0;         // ratio of air flow to max air flow to simulation modulating fan
        Real64 HeatCoilWaterFlowRatio = 0.0;   // ratio of water flow rate to max water flow rate
        Real64 ControlZoneMassFlowFrac = 1.0;  // flow fraction of control zone (always 1 for zone equipment)
        int MaxIterIndex = 0;                  // recurring message index
        int RegulaFalsiFailedIndex = 0;        // iteration loop warning
    };

    struct FanCoilNumericFieldData
    {
        // Members
        Array1D_string FieldNames;
    };

    void SimFanCoilUnit(EnergyPlusData &state,
                        std::string_view CompName, // name of the fan coil unit
                        int ControlledZoneNum,     // number of zone being served
                        bool FirstHVACIteration,   // TRUE if 1st HVAC simulation of system timestep
                        Real64 &PowerMet,          // Sensible power supplied (W)
                        Real64 &LatOutputProvided, // Latent add/removal supplied by window AC (kg/s), dehumid = negative
                        int &CompIndex);

    void GetFanCoilUnits(EnergyPlusData &state);

    void InitFanCoilUnits(EnergyPlusData &state,
                          int FanCoilNum,         // number of the current fan coil unit being simulated
                          int ControlledZoneNum); // number of zone being served

    void SizeFanCoilUnit(EnergyPlusData &state, int FanCoilNum,
                         int ControlledZoneNum); // index into ZoneEquipConfig array

    void Sim4PipeFanCoil(EnergyPlusData &state,
                         int &FanCoilNum,          // number of the current fan coil unit being simulated
                         int ControlledZoneNum,    // index into ZoneEqupConfig
                         bool FirstHVACIteration,  // TRUE if 1st HVAC simulation of system timestep
                         Real64 &PowerMet,         // Sensible power supplied (W)
                         Real64 &LatOutputProvided // Latent power supplied (kg/s), negative = dehumidification
    );

    void TightenWaterFlowLimits(EnergyPlusData &state,
                                int FanCoilNum,          // Unit index in fan coil array
                                bool CoolingLoad,        // true if zone requires cooling
                                bool HeatingLoad,        // true if zone requires heating
                                int WaterControlNode,    // water control node, either cold or hot water
                                int ControlledZoneNum,   // controlling zone index
                                bool FirstHVACIteration, //  TRUE if 1st HVAC simulation of system timestep
                                Real64 QZnReq,           // zone load [W]
                                Real64 &MinWaterFlow,    // minimum water flow rate
                                Real64 &MaxWaterFlow     // maximum water flow rate
    );

    void TightenAirAndWaterFlowLimits(EnergyPlusData &state,
                                      int FanCoilNum,          // Unit index in fan coil array
                                      bool CoolingLoad,        // true if zone requires cooling
                                      bool HeatingLoad,        // true if zone requires heating
                                      int WaterControlNode,    // water control node, either cold or hot water
                                      int ControlledZoneNum,   // controlling zone index
                                      bool FirstHVACIteration, //  TRUE if 1st HVAC simulation of system timestep
                                      Real64 QZnReq,           // zone load [W]
                                      Real64 &PLRMin,          // minimum part-load ratio
                                      Real64 &PLRMax           // maximum part-load ratio
    );

    void Calc4PipeFanCoil(EnergyPlusData &state,
                          int FanCoilNum,                      // Unit index in fan coil array
                          int ControlledZoneNum,               // ZoneEquipConfig index
                          bool FirstHVACIteration,             // flag for 1st HVAV iteration in the time step
                          Real64 &LoadMet,                     // load met by unit (watts)
                          ObjexxFCL::Optional<Real64> PLR = _, // Part Load Ratio, fraction of time step fancoil is on
                          Real64 ElecHeatCoilPLR = 1.0         // electric heating coil PLR used with MultiSpeedFan capacity control
    );

    void SimMultiStage4PipeFanCoil(EnergyPlusData &state,
                                   int &FanCoilNum,         // number of the current fan coil unit being simulated
                                   int ControlledZoneNum,   // index into ZoneEqupConfig
                                   bool FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                                   Real64 &PowerMet         // Sensible power supplied (W)
    );

    void CalcMultiStage4PipeFanCoil(EnergyPlusData &state,
                                    int &FanCoilNum,         // number of the current fan coil unit being simulated
                                    int ZoneNum,             // number of zone being served
                                    bool FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                                    Real64 QZnReq,           // current zone cooling or heating load
                                    Real64 &SpeedRatio,      // fan coil speed ratio
                                    Real64 &PartLoadRatio,   // fan coil part load ratio
                                    Real64 &PowerMet         // Sensible power supplied (W)
    );

    void ReportFanCoilUnit(EnergyPlusData &state, int FanCoilNum); // number of the current fan coil unit being simulated

    int GetFanCoilZoneInletAirNode(EnergyPlusData &state, int const FanCoilNum);

    int GetFanCoilOutAirNode(EnergyPlusData &state, int FanCoilNum);

    int GetFanCoilReturnAirNode(EnergyPlusData &state, int FanCoilNum);

    int GetFanCoilMixedAirNode(EnergyPlusData &state, int FanCoilNum);

    Real64 CalcFanCoilLoadResidual(EnergyPlusData &state,
                                   int FanCoilNum,          // Index to this fan coil unit
                                   bool FirstHVACIteration, // FirstHVACIteration flag
                                   int ControlledZoneNum,   // zone index
                                   Real64 QZnReq,           // Sensible load to be met [W]
                                   Real64 PartLoadRatio     // coil part load ratio
    );

    Real64 CalcFanCoilPLRResidual(EnergyPlusData &state,
                                  Real64 PLR,              // part-load ratio of air and water mass flow rate
                                  int FanCoilNum,          // Index to this fan coil unit
                                  bool FirstHVACIteration, // FirstHVACIteration flag
                                  int ControlledZoneNum,   // zone index
                                  int WaterControlNode,    // water node to control
                                  Real64 QZnReq            // Sensible load to be met [W] // Function parameters
    );

    Real64 CalcFanCoilHeatCoilPLRResidual(EnergyPlusData &state,
                                          Real64 CyclingR, // electric heating coil cycling ratio
                                          int FanCoilNum,
                                          bool FirstHVACIteration,
                                          int ZoneNum,
                                          Real64 QZnReq // Function parameters
    );

    Real64 CalcFanCoilCWLoadResidual(EnergyPlusData &state,
                                     Real64 CWFlow, // water mass flow rate [kg/s]
                                     int FanCoilNum,
                                     bool FirstHVACIteration,
                                     int ControlledZoneNum,
                                     Real64 QZnReq);

    Real64 CalcFanCoilWaterFlowResidual(EnergyPlusData &state,
                                        Real64 PLR,
                                        int FanCoilNum,
                                        bool FirstHVACIteration,
                                        int ControlledZoneNum,
                                        Real64 QZnReq,
                                        int AirInNode,
                                        int WaterControlNode,
                                        Real64 maxCoilFluidFlow,
                                        Real64 AirMassFlowRate);

    Real64 CalcFanCoilAirAndWaterFlowResidual(EnergyPlusData &state,
                                              Real64 WaterFlow, // water mass flow rate [kg/s]
                                              int FanCoilNum,
                                              bool FirstHVACIteration,
                                              int ControlledZoneNum,
                                              Real64 QZnReq,
                                              int AirInNode,
                                              int WaterControlNode,
                                              Real64 MinWaterFlow);

    int getEqIndex(EnergyPlusData &state, std::string_view CompName);
} // namespace FanCoilUnits

struct FanCoilUnitsData : BaseGlobalStruct
{

    std::string const cMO_FanCoil = "ZoneHVAC:FourPipeFanCoil";
    int NumFanCoils = 0;
    int Num4PipeFanCoils = 0;
    Array1D_bool MySizeFlag;
    Array1D_bool CheckEquipName;
    bool GetFanCoilInputFlag = true; // First time, input is "gotten"
    Real64 FanFlowRatio = 0.0;
    bool HeatingLoad = false; // True when zone needs heating
    bool CoolingLoad = false; // True when zone needs cooling
    Array1D<FanCoilUnits::FanCoilData> FanCoil;
    Array1D<FanCoilUnits::FanCoilNumericFieldData> FanCoilNumericFields;
    bool InitFanCoilUnitsOneTimeFlag = true;
    bool InitFanCoilUnitsCheckInZoneEquipmentListFlag = false; // True after the Zone Equipment List has been checked for items

    // static variables extracted from functions
    bool ErrorsFound = false;                               // Set to true if errors in input, fatal at end of routine
    bool errFlag = false;                                   // Local error flag for GetOAMixerNodeNums
    int TotalArgs = 0;                                      // Total number of alpha and numeric arguments (max) for a
    bool ZoneExNodeNotFound = false;                        // used in error checking
    bool ZoneInNodeNotFound = false;                        // used in error checking
    int ATMixerNum = 0;                                     // index of air terminal mixer in the air terminal mixer data array
    HVAC::MixerType ATMixerType = HVAC::MixerType::Invalid; // type of air terminal mixer (1=inlet side; 2=supply side)
    int ATMixerPriNode = 0;                                 // node number of the air terminal mixer primary air inlet
    int ATMixerSecNode = 0;                                 // node number of the air terminal mixer secondary air inlet
    int ATMixerOutNode = 0;                                 // node number of the air terminal mixer secondary air inlet
    Array1D_bool MyEnvrnFlag;
    Array1D_bool MyPlantScanFlag;
    Array1D_bool MyZoneEqFlag; // used to set up zone equipment availability managers
    int CoilWaterInletNode = 0;
    int CoilWaterOutletNode = 0;
    int ATMixOutNode = 0; // outlet node of ATM Mixer
    int ZoneNode = 0;     // zone node

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) FanCoilUnitsData();
    }
};

} // namespace EnergyPlus

#endif
