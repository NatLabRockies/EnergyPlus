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

#ifndef DataAirLoop_hh_INCLUDED
#define DataAirLoop_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHVACSystems.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/SimAirServingZones.hh>

namespace EnergyPlus {

namespace DataAirLoop {

    struct AirLoopZoneEquipConnectData
    {
        // Members
        std::string AirLoopName;                   // Name of Primary Air System
        int NumReturnNodes = 0;                    // Number of return nodes entering primary air system (currently limited to 1 node)
        int NumSupplyNodes = 0;                    // number of supply nodes exiting primary air system
        int NumZonesCooled = 0;                    // number of zones cooled by this primary air system
        int NumZonesHeated = 0;                    // number of zones heated by this primary air system
        Array1D_int ZoneEquipReturnNodeNum;        // Zone Equip side return air node numbers (currently limited to 1 node)
        Array1D_int ZoneEquipSupplyNodeNum;        // Zone equip side supply air node numbers
        Array1D_int AirLoopReturnNodeNum;          // Air loop side return air node numbers
        Array1D_int AirLoopSupplyNodeNum;          // Air loop side supply air node numbers
        Array1D_int CoolCtrlZoneNums;              // Controlled zone numbers of zones cooled by this air loop
        Array1D_int HeatCtrlZoneNums;              // Controlled zone numbers of zones heated by this air loop
        Array1D_int CoolZoneInletNodes;            // Zone inlet node numbers of zones cooled by this air loop
        Array1D_int HeatZoneInletNodes;            // Zone inlet node numbers of zones heated by this air loop
        Array1D_int TermUnitCoolInletNodes;        // Air terminal unit cooling inlet node numbers for this air loop
        Array1D_int TermUnitHeatInletNodes;        // Air terminal unit heating inlet node numbers for this air loop
        Array1D_int TermUnitCoolSizingIndex;       // Air terminal sizing numbers for zones cooled by this air loop
        Array1D_int TermUnitHeatSizingIndex;       // Air terminal sizing numbers for zones heated by this air loop
        Array1D<HVAC::AirDuctType> SupplyDuctType; // 1=main, 2=cooling, 3=heating, 4=other
        EPVector<int> SupplyDuctBranchNum;         // Supply duct branch number (airloop branchnum, not the actual branch index)
        EPVector<int> SupplyAirPathNum;            // Supply air path indexes
        EPVector<int> ReturnAirPathNum;            // Return air path indexes
    };

    struct AirLoopOutsideAirConnectData
    {
        // Members
        bool OASysExists = false;   // true if there is an Outside Air Sys
        int OASysInletNodeNum = 0;  // node number of return air inlet to OA sys
        int OASysOutletNodeNum = 0; // node number of mixed air outlet of OA sys
    };

    struct DefinePriAirSysAvailMgrs
    {
        // Members
        int NumAvailManagers = 0;                            // number of availability managers for this system
        Avail::Status availStatus = Avail::Status::NoAction; // system availability status
        int StartTime = 0;                                   // cycle on time (in SimTimeSteps)
        int StopTime = 0;                                    // cycle off time (in SimTimeSteps)
        Real64 ReqSupplyFrac = 0.0;                          // required system flow rate (as a fraction)
        Array1D<Avail::AvailManagerNTN> availManagers;       // type of availability manager
    };

    struct AirLooptoZoneData // Derived type for air loop connection to zones on air loop
    {
        // Members
        int NumZones = 0;
        Array1D_int Zone;
        Array1D_int ActualZoneNumber;
    };

    struct AirLoopControlData // Derived type for air control information
    {
        // Members
        std::string OACtrlName;                     // name of OA controller
        int OACtrlNum = 0;                          // index of OA controller
        int OASysNum = 0;                           // index of OA System
        bool CyclingFan = false;                    // TRUE if currently the air loop supply fan is cycling
        bool AnyContFan = false;                    // TRUE if at any time supply fan is continuous
        Sched::Schedule *cycFanSched = nullptr;     // schedule indicating whether fan is cycling or continuous in a unitary system
        HVAC::FanOp fanOp = HVAC::FanOp::Invalid;   // 1=cycling fan cycling compressor; 2=constant fan cycling comptressor
        bool UnitarySys = false;                    // TRUE if a unitary system
        bool UnitarySysSimulating = true;           // set FALSE for AirloopUnitarySystem after simulating to downstream coils can size independently
        bool Simple = false;                        // TRUE if system has 1 branch and 1 component
        bool CanNotLockoutEcono = false;            // user input says econo lockout not allowed
        bool CanLockoutEconoWithHeating = false;    // user input says econo lockout with heating is allowed
        bool CanLockoutEconoWithCompressor = false; // user input says econo lockout with compressor is allowed
        bool ReqstEconoLockoutWithHeating = false;  // there is a request to lockout the economizer due to heating
        bool ReqstEconoLockoutWithCompressor = false; // there is a request to lockout the economizer due to compressor operation
        bool EconoActive = false;                     // if true economizer is active
        bool HeatRecoveryBypass = false;              // if true heat recovery is bypassed (not active)
        bool ResimAirLoopFlag = false;                // Same as SimAir, will trigger re-sim of air loops
        bool HeatRecoveryResimFlag = true;            // Used to trigger new air loop sim when HX is used in OA system
        bool HeatRecoveryResimFlag2 = false;          // Used to trigger new air loop sim when HX is used in OA system
        bool CheckHeatRecoveryBypassStatus = false;   // determines when heat recovery bypass is set
        bool EconomizerFlowLocked = false;            // locks economizer flow for custom ERV operation
        bool HighHumCtrlActive = false;               // if true high humidity control is active
        bool EconoLockout = false;                    // if true the economizer will be locked out (OA flow set to minimum)
        bool LoopFlowRateSet = false;                 // if true then the air loop flow rate should be set using ReqSupplyFrac
        bool NightVent = false;                       // if true then air loop is in night ventilation mode
        bool AllowWarmRestartFlag = false;            // if true then speculative warm restart is attempted after first HVAC iteration
        bool NewFlowRateFlag = false;                 // true whenever the air mass flow rates have changed since last air loop sim
        bool ConvergedFlag = false;                   // true whenever the air loop sim was converged overall
        bool CoolingActiveFlag = false;               // true whenever the air loop cooling coil is operating
        bool HeatingActiveFlag = false;               // true whenever the air loop heating coil is operating
        bool OASysComponentsSimulated = false;        // - true after OA components have been simulated
        Real64 ZoneExhMassFlow = 0.0;                 // zone exhaust flow rate not accounted for by zone inlet flow
        bool AirLoopDCVFlag = true;                   // TRUE if the air loop has OA Controller specifying a Mechanical controller with DCV
        int AirLoopPass = 0;                          // number of air loop passes during iteration
        // - internal flag only
    };

    struct AirLoopFlowData // Derived type for air loop flow information
    {
        // Members
        Real64 DesSupply = 0.0;             // design supply air mass flow rate for loop [kg/s]
        Real64 DesReturnFrac = 1.0;         // the design return flow rate as a fraction of supply flow assuming no exhaust (0 to 1)
        Real64 SysToZoneDesFlowRatio = 0.0; // System design flow divided by the sum of the zone design flows
        Real64 ReqSupplyFrac = 1.0;         // required flow (as a fraction of DesSupply) set by a manager
        Real64 MinOutAir = 0.0;             // minimum outside air mass flow rate [kg/s]
        Real64 MaxOutAir = 0.0;             // current maximum available outside air mass flow rate [kg/s]
        Real64 OAMinFrac = 0.0;             // minimum outside air flow fraction this time step
        Real64 Previous = 0.0;              // Previous mass air flow rate for this loop [kg/s]
        Real64 SupFlow = 0.0;               // supply air flow rate (includes LeakFlow) [kg/s]
        Real64 ZoneRetFlow = 0.0;           // return air flow rate at all zone return air nodes (includes RecircFlow, excludes LeakFlow) [kg/s]
        Real64 ZoneRetFlowRatio = 1.0;      // ratio for adjusting zone return flows for excess zone exhaust
        Real64 SysRetFlow = 0.0;            // return air flow rate back to central return (excludes RecircFlow, includes LeakFlow) [kg/s]
        Real64 RecircFlow = 0.0;            // sum of zone plenum recirculated flows [kg/s]
        Real64 LeakFlow = 0.0;              // sum of air distribution leak flows to return plenum [kg/s]
        Real64 ExcessZoneExhFlow = 0.0;     // excess zone exhuast flows made up by reduced return flow in other zones on same airloop [kg/s]
        Real64 FanPLR = 1.0;                // Operating PLR of air loop fan
        Real64 OAFrac = 0.0;                // fraction of outside air to mixed air mass flow rate
        Real64 OAFlow = 0.0;                // oa flow rate this time step [kg/s]
        bool FlowError = false;             // error flag for flow error message
        Real64 BypassMassFlow = 0.0;        // air loop bypass mass flow NOT entering splitter but included in mixer or plenum
    };

    enum class ControllerKind
    {
        Invalid = -1,
        WaterCoil,  // Controller:WaterCoil
        OutdoorAir, // Controller:OutdoorAir
        Num
    };

    struct OutsideAirSysProps
    {
        // Members
        std::string Name;
        std::string ControllerListName;
        std::string ComponentListName;
        int ControllerListNum = 0; // index of the Controller List
        int NumComponents = 0;
        int NumControllers = 0;
        int NumSimpleControllers = 0; // number of CONTROLLER:SIMPLE objects in OA Sys controller list
        std::string OAControllerName; // OA controller name
        int OAControllerIndex = 0;    // OA controller index in OAController
        Array1D_string ComponentName;
        Array1D_string ComponentType;
        Array1D<SimAirServingZones::CompType> ComponentTypeEnum; // Parameterized (see above) Component Types this
        // module can address
        Array1D_int ComponentIndex; // Which one in list -- updated by routines called from here
        std::vector<HVACSystemData *> compPointer;
        Array1D_string ControllerName;
        Array1D_string ControllerType;
        EPVector<ControllerKind> controllerTypeEnum; // Controller:OutdoorAir or Controller:WaterCoil
        Array1D_int ControllerIndex;                 // Which one in list -- updated by routines called from here
        Array1D_int InletNodeNum;                    // component inlet node number
        Array1D_int OutletNodeNum;                   // component outlet node number
        bool HeatExchangerFlag = false;              // True to have a heat exchanger in the equipment list
        int AirLoopDOASNum = -1;                     // AirLoopHVAC:DedicatedOutdoorAirSystem number
    };

    struct AirLoopAFNData
    {
        // Members
        HVAC::FanOp LoopFanOperationMode = HVAC::FanOp::Invalid; // OnOff fan operation mode
        Real64 LoopSystemOnMassFlowrate = 0.0;                   // Loop mass flow rate during on cycle using an OnOff fan
        Real64 LoopSystemOffMassFlowrate = 0.0;                  // Loop mass flow rate during off cycle using an OnOff fan
        Real64 LoopOnOffFanPartLoadRatio = 0.0;                  // OnOff fan part load ratio
        Real64 LoopCompCycRatio = 0.0;                           // Loop compressor cycling ratio for multispeed heat pump
        Real64 AFNLoopOnOffFanRTF = 0.0;                         // OnOff fan run time fraction in an HVAC Air Loop
    };

} // namespace DataAirLoop

struct DataAirLoopData : BaseGlobalStruct
{

    int NumOASystems = 0;             // Number of Outdoor Air Systems
    bool AirLoopInputsFilled = false; // Set to TRUE after first pass through air loop
    Real64 LoopDXCoilRTF = 0.0;       // OnOff fan run time fraction in an HVAC Air Loop

    EPVector<DataAirLoop::AirLoopZoneEquipConnectData> AirToZoneNodeInfo;
    EPVector<DataAirLoop::AirLoopOutsideAirConnectData> AirToOANodeInfo;
    EPVector<DataAirLoop::DefinePriAirSysAvailMgrs> PriAirSysAvailMgr;
    EPVector<DataAirLoop::AirLooptoZoneData> AirLoopZoneInfo;
    EPVector<DataAirLoop::AirLoopControlData> AirLoopControlInfo;
    EPVector<DataAirLoop::AirLoopFlowData> AirLoopFlow;
    EPVector<DataAirLoop::OutsideAirSysProps> OutsideAirSys;
    EPVector<DataAirLoop::AirLoopAFNData> AirLoopAFNInfo;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) DataAirLoopData();
    }
};

} // namespace EnergyPlus

#endif
