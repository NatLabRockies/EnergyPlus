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

#ifndef SetPointManager_hh_INCLUDED
#define SetPointManager_hh_INCLUDED

#include <map>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace SetPointManager {

    enum class SupplyFlowTempStrategy
    {
        Invalid = -1,
        MaxTemp,
        MinTemp,
        Num
    };

    enum class ControlStrategy
    {
        Invalid = -1,
        TempFirst,
        FlowFirst,
        Num
    };

    enum class AirTempType
    {
        Invalid = -1,
        WetBulb,
        DryBulb,
        Num
    };

    enum class ReturnTempType
    {
        Invalid = -1,
        Scheduled,
        Constant,
        Setpoint,
        Num
    };

    enum class SPMType
    {
        Invalid = -1,
        Scheduled,
        ScheduledDual,
        OutsideAir,
        SZReheat,
        SZHeating,
        SZCooling,
        SZMinHum,
        SZMaxHum,
        MixedAir,
        OutsideAirPretreat,
        Warmest,
        Coldest,
        WarmestTempFlow,
        ReturnAirBypass,
        MZCoolingAverage,
        MZHeatingAverage,
        MZMinHumAverage,
        MZMaxHumAverage,
        MZMinHum,
        MZMaxHum,
        FollowOutsideAirTemp,
        FollowSystemNodeTemp,
        FollowGroundTemp,
        CondenserEnteringTemp,
        IdealCondenserEnteringTemp,
        SZOneStageCooling,
        SZOneStageHeating,
        ChilledWaterReturnTemp,
        HotWaterReturnTemp,
        TESScheduled,
        SystemNodeTemp,
        SystemNodeHum,
        Num
    };

    struct SPMBase
    {
        std::string Name;
        SPMType type = SPMType::Invalid;

        HVAC::CtrlVarType ctrlVar = HVAC::CtrlVarType::Invalid; // set to CtrlVarType::*

        std::vector<int> ctrlNodeNums; // index to control node

        std::string airLoopName; // name of air loop
        int airLoopNum = 0;      // index to air loop
        int refNodeNum = 0;      // index to reference node

        Real64 minSetTemp = 0.0; // minimum supply air setpoint temperature
        Real64 maxSetTemp = 0.0; // maximum supply air setpoint temperature

        Real64 minSetHum = 0.0; // minimum supply air setpoint humidity ratio [kg/kg]
        Real64 maxSetHum = 0.0; // maximum supply air setpoint humidity ratio [kg/kg]
        Real64 setPt = 0.0;     // the setpoint

        virtual ~SPMBase() = default;

        virtual void calculate(EnergyPlusData &state) = 0;
    };

    struct SPMScheduled : SPMBase // Derived type for Scheduled Setpoint Manager data
    {
        Sched::Schedule *sched = nullptr;

        void calculate(EnergyPlusData &state) override;
    };

    struct SPMScheduledDual : SPMBase // Derived type for Scheduled Dual Setpoint Manager
    {
        Sched::Schedule *hiSched = nullptr;
        Sched::Schedule *loSched = nullptr;
        Real64 setPtHi = 0.0;
        Real64 setPtLo = 0.0;

        void calculate(EnergyPlusData &state) override;
    };

    struct SPMOutsideAir : SPMBase // Derived type for Outside Air Setpoint Manager Data
    {
        // Members
        Sched::Schedule *sched = nullptr;

        Real64 lowSetPt1 = 0.0;            // 1st setpoint at outside low
        Real64 low1 = 0.0;                 // 1st Outside low
        Real64 highSetPt1 = 0.0;           // 1st setpoint at outside high
        Real64 high1 = 0.0;                // 1st Outside high
        int invalidSchedValErrorIndex = 0; // index for recurring error when schedule is not 1 or 2
        int setPtErrorCount = 0;           // countfor recurring error when schedule is not 1 or 2
        Real64 lowSetPt2 = 0.0;            // 2nd setpoint at outside low (optional)
        Real64 low2 = 0.0;                 // 2nd Outside low (optional)
        Real64 highSetPt2 = 0.0;           // 2nd setpoint at outside high (optional)
        Real64 high2 = 0.0;                // 2nd Outside high (optional)

        void calculate(EnergyPlusData &state) override;
    };

    struct SPMSingleZoneReheat : SPMBase // Derived type for the Single Zone Reheat Setpoint Manager data
    {
        // Members
        std::string ctrlZoneName; // name of the control zone (zone with main thermostat)
        int ctrlZoneNum = 0;      // number (index into Zone array) of control zone
        int zoneNodeNum = 0;      // zone node number
        int zoneInletNodeNum = 0; // inlet node number for the SZRH air
        int mixedAirNodeNum = 0;  // mixed air node number
        int fanInNodeNum = 0;     // fan inlet node number
        int fanOutNodeNum = 0;    // fan outlet node number
        int oaInNodeNum = 0;      // outside airstream inlet node to the OA mixer
        int retNodeNum = 0;       // return node inlet to OA mixer
        int loopInNodeNum = 0;    // Primary Air System inlet node

        void calculate(EnergyPlusData &state) override;
    };

    // The one can be used for both SZ Heating and SZCooling SPMs
    struct SPMSingleZoneTemp : SPMBase
    {
        // Members
        std::string ctrlZoneName; // name of the control zone (zone with main thermostat)
        int ctrlZoneNum = 0;      // number (index into Zone array) of control zone
        int zoneNodeNum = 0;      // zone node number
        int zoneInletNodeNum = 0; // inlet node number for the supply air

        void calculate(EnergyPlusData &state) override;
    };

    // Can be used for both SZMinHum and SZMaxHum SPMs
    struct SPMSingleZoneHum : SPMBase // Derived Type for Single Zone Minimum Humidity Setpoint Manager data
    {
        // Members
        int zoneNodeNum = 0; // zone node numbers of zones being controlled
        int ctrlZoneNum = 0; // index into ZoneEquipConfig

        void calculate(EnergyPlusData &state) override;
    };

    struct SPMMixedAir : SPMBase

    {
        // Members
        int fanInNodeNum = 0;            // supply fan inlet node number
        int fanOutNodeNum = 0;           // Supplt fan outlet node number
        bool mySetPointCheckFlag = true; // used for mixed air SPM test for missing SP
        bool freezeCheckEnable = true;   // Enable freezing check
        int coolCoilInNodeNum = 0;       // Cooling coil inlet node number
        int coolCoilOutNodeNum = 0;      // Cooling coil outlet node number
        Real64 minCoolCoilOutTemp = 7.2; // The minimum temperature at cooling coil outlet node

        void calculate(EnergyPlusData &state) override;
    };

    struct SPMOutsideAirPretreat : SPMBase
    {
        // Members
        int mixedOutNodeNum = 0;         // mixed air outlet node number
        int oaInNodeNum = 0;             // outside air inlet node number
        int returnInNodeNum = 0;         // return air inlet node number
        bool mySetPointCheckFlag = true; // used for DOAS SPM test for missing SP

        void calculate(EnergyPlusData &state) override;
    };

    // Works for both coldest and warmest SPMs
    struct SPMTempest : SPMBase
    {
        // Members
        SupplyFlowTempStrategy strategy = SupplyFlowTempStrategy::Invalid; // supply flow and temperature set strategy

        void calculate(EnergyPlusData &state) override;
    };

    struct SPMWarmestTempFlow : SPMBase
    {
        // Members
        ControlStrategy strategy = ControlStrategy::Invalid; // supply flow and temperature set strategy
        Real64 minTurndown = 0.0;                            // minimum fractional flow rate
        Real64 turndown = 0.0;                               // fractional flow rate
        int critZoneNum = 0;
        bool simReady = false;

        void calculate(EnergyPlusData &state) override;
    };

    struct SPMReturnAirBypassFlow : SPMBase
    {
        // Members
        Sched::Schedule *sched = nullptr;
        Real64 FlowSetPt = 0; // mass flow rate setpoint (kg/s)
        int rabMixInNodeNum = 0;
        int supMixInNodeNum = 0;
        int mixOutNodeNum = 0;
        int rabSplitOutNodeNum = 0;
        int sysOutNodeNum = 0;

        void calculate(EnergyPlusData &state) override;
    };

    struct SPMMultiZoneTemp : SPMBase // derived type for SetpointManager:Multizone:Cooling:Average data
    {
        void calculate(EnergyPlusData &state) override;
    };

    struct SPMMultiZoneHum : SPMBase // derived type for SetpointManager:MultiZone:MinimumHumidity:Average data
    {
        void calculate(EnergyPlusData &state) override;
    };

    struct SPMFollowOutsideAirTemp : SPMBase
    {
        // Members
        AirTempType refTempType = AirTempType::Invalid; // set to iRefTempType_WetBulb or iRefTempType_DryBulb
        Real64 offset = 0.0;                            // Offset temperature difference

        void calculate(EnergyPlusData &state) override;
    };

    struct SPMFollowSysNodeTemp : SPMBase
    {
        // Members
        AirTempType refTempType = AirTempType::Invalid; // set to iRefTempType_WetBulb or iRefTempType_DryBulb
        Real64 offset = 0.0;                            // Offset temperature difference

        void calculate(EnergyPlusData &state) override;
    };

    struct SPMFollowGroundTemp : SPMBase
    {
        // Members
        // Site:GroundTemperature:BuildingSurface
        // Site:GroundTemperature:Shallow
        // Site:GroundTemperature:Deep
        // Site:GroundTemperature:FCfactorMethod
        DataEnvironment::GroundTempType refTempType = DataEnvironment::GroundTempType::Invalid;
        Real64 offset = 0.0; // Offset temperature difference

        void calculate(EnergyPlusData &state) override;
    };

    struct SPMCondenserEnteringTemp : SPMBase // derived type for SetpointManager:CondenserEnteringReset data
    {
        // Members
        Sched::Schedule *condenserEnteringTempSched = nullptr; // default condenser entering water temperature schedule Index
        Real64 towerDesignInletAirWetBulbTemp = 0;             // cooling tower design inlet air wetbulb temperature
        int minTowerDesignWetBulbCurveNum = 0;                 // minimum design wetbulb temperature curve name
        int minOAWetBulbCurveNum = 0;                          // minimum outside air wetbulb temperature curve name
        int optCondenserEnteringTempCurveNum = 0;              // optimized condenser entering water temperature curve name
        Real64 minLift = 0.0;                                  // minimum lift
        Real64 maxCondenserEnteringTemp = 0.0;                 // maximum condenser entering water temp
        PlantLocation plantPloc;                               // plant side chiller index
        PlantLocation demandPloc;
        DataPlant::PlantEquipmentType chillerType; // chiller type number

        void calculate(EnergyPlusData &state) override;
    };

    struct SPMVar
    {
        OutputProcessor::VariableType Type = OutputProcessor::VariableType::Invalid;
        int Num = 0;
    };

    struct SPMIdealCondenserEnteringTemp : SPMBase // derived type for SetpointManager:CondenserEnteringReset:Ideal data
    {
        // Members
        Real64 minLift = 0.0;                  // minimum lift
        Real64 maxCondenserEnteringTemp = 0.0; // maximum condenser entering water temp
        PlantLocation chillerPloc;             // plant side chiller index
        SPMVar chillerVar;
        SPMVar chilledWaterPumpVar;
        Array1D<SPMVar> towerVars;
        SPMVar condenserPumpVar;
        DataPlant::PlantEquipmentType chillerType = DataPlant::PlantEquipmentType::Invalid; // chiller type number
        Array1D<PlantLocation> towerPlocs;
        int numTowers = 0;                      // number of towers to query
        PlantLocation condenserPumpPloc;        // condenser pump number
        PlantLocation chilledWaterPumpPloc;     // chilled water pump number
        bool setupIdealCondEntSetPtVars = true; // flag for initialization of meters and such

        void calculate(EnergyPlusData &state) override;

        void SetupMeteredVarsForSetPt(EnergyPlusData &state);

        Real64 calculateCurrentEnergyUsage(EnergyPlusData &state);

        void setupSetPointAndFlags(Real64 &TotEnergy,
                                   Real64 &TotEnergyPre,
                                   Real64 &CondWaterSetPoint,
                                   Real64 &CondTempLimit,
                                   bool &RunOptCondEntTemp,
                                   bool &RunSubOptCondEntTemp,
                                   bool &RunFinalOptCondEntTemp) const;
    };

    struct SPMSingleZoneOneStageCooling : SPMBase // Derived type for the Single Zone One Stage Cooling Setpoint Manager data
    {
        // Members
        int ctrlZoneNum = 0;          // number (index into Zone array) of control zone
        int zoneNodeNum = 0;          // zone node number
        Real64 coolingOnSetPt = 0.0;  // minimum supply air setpoint temperature
        Real64 coolingOffSetPt = 0.0; // maximum supply air setpoint temperature

        void calculate(EnergyPlusData &state) override;
    };

    struct SPMSingleZoneOneStageHeating : SPMBase // Derived type for the Single Zone One Stage Cooling Setpoint Manager data
    {
        // Members
        int ctrlZoneNum = 0;          // number (index into Zone array) of control zone
        int zoneNodeNum = 0;          // zone node number
        Real64 heatingOnSetPt = 0.0;  // minimum supply air setpoint temperature
        Real64 heatingOffSetPt = 0.0; // maximum supply air setpoint temperature

        void calculate(EnergyPlusData &state) override;
    };

    struct SPMReturnWaterTemp : SPMBase // derived type for SetpointManager:SupplyResetForReturnTemperature:ChilledWater data
    {
        // Members
        int returnNodeNum = 0;                      // node ID for the plant supply-side return node
        int supplyNodeNum = 0;                      // node ID for the plant supply-side supply node
        Sched::Schedule *returnTempSched = nullptr; // scheduled return temperature
        Real64 returnTempConstantTarget = 0.0;      // the constant value used as the return temperature target; used if schedule index is zero
        Real64 currentSupplySetPt = 0.0;            // the current supply setpoint temperature
        int plantLoopNum = 0;                       // the index for the plant loop for this manager, zero if not initialized
        int plantSetPtNodeNum = 0; // the index for the node where the plant setpoint is set, need to look up after Plant is established
        ReturnTempType returnTempType = ReturnTempType::Invalid;

        // Calculation method
        void calculate(EnergyPlusData &state) override;
        // void calculate(EnergyPlusData &state, DataLoopNode::NodeData &returnNode, DataLoopNode::NodeData &supplyNode);
    };

    struct SPMTESScheduled : SPMBase // Derived type for Scheduled TES Setpoint Manager data
    {
        // Members
        Sched::Schedule *sched = nullptr;       // Default is AlwaysOff
        Sched::Schedule *chargeSched = nullptr; // Default is AlwaysOff
        int ctrlNodeNum = 0;
        Real64 nonChargeCHWTemp = 0.0;
        Real64 chargeCHWTemp = 0.0;
        DataPlant::CtrlType compOpType = DataPlant::CtrlType::Invalid;

        // Calculation method
        void calculate(EnergyPlusData &state) override;
    };

    struct SPMSystemNode : SPMBase // Derived type for System Node Reset Setpoint Manager Data
    {
        // Members
        Real64 lowRefSetPt = 0.0;  // Setpoint at Low Reference Temperature or Humidity Ratio (i.e., Maximum Temperature/Humidity Ratio Setpoint)
        Real64 highRefSetPt = 0.0; // Setpoint at High Reference Temperature or Humidity Ratio (i.e., Maximum Temperature/Humidity Ratio Setpoint)
        Real64 lowRef = 0.0;       // Low Reference Temperature or Humidity Ratio
        Real64 highRef = 0.0;      // High Reference Temperature or Humidity Ratio

        void calculate(EnergyPlusData &state) override;
    };

    int GetSetPointManagerIndex(EnergyPlusData const &state, std::string const &Name);

    void ManageSetPoints(EnergyPlusData &state);

    void GetSetPointManagerInputs(EnergyPlusData &state); // wrapper for GetInput to accomodate unit testing

    void GetSetPointManagerInputData(EnergyPlusData &state, bool &ErrorsFound);

    void VerifySetPointManagers(EnergyPlusData &state, bool &ErrorsFound); // flag to denote node conflicts in input. !unused1208

    void InitSetPointManagers(EnergyPlusData &state);

    void SimSetPointManagers(EnergyPlusData &state);

    void UpdateSetPointManagers(EnergyPlusData &state);

    void UpdateMixedAirSetPoints(EnergyPlusData &state);

    void UpdateOAPretreatSetPoints(EnergyPlusData &state);

    int GetSetPointManagerIndexByNode(EnergyPlusData &state, int NodeNum, HVAC::CtrlVarType ctrlVar, SPMType spmType, bool isRefNode);

    bool IsNodeOnSetPtManager(EnergyPlusData &state, int NodeNum, HVAC::CtrlVarType ctrlVar);

    bool NodeHasSPMCtrlVarType(EnergyPlusData &state, int NodeNum, HVAC::CtrlVarType ctrlVar);

    void ResetHumidityRatioCtrlVarType(EnergyPlusData &state, int NodeNum);

    void CheckIfAnyIdealCondEntSetPoint(EnergyPlusData &state);

    HVAC::CtrlVarType GetHumidityRatioVariableType(EnergyPlusData &state, int CtrlNodeNum);

    void SetUpNewScheduledTESSetPtMgr(EnergyPlusData &state,
                                      Sched::Schedule *onPeakSched,
                                      Sched::Schedule *chargeSched,
                                      Real64 NonChargeCHWTemp,
                                      Real64 ChargeCHWTemp,
                                      DataPlant::CtrlType CompOpType,
                                      int ControlNodeNum);

    bool GetCoilFreezingCheckFlag(EnergyPlusData &state, int MixedAirSPMNum);

    int GetMixedAirNumWithCoilFreezingCheck(EnergyPlusData &state, int MixedAirNode);

    Real64 interpSetPoint(Real64 LowVal, Real64 HighVal, Real64 RefVal, Real64 SetptAtLowVal, Real64 SetptAtHighVal);

    void FillPredefinedTablesForSetPointManagers(EnergyPlusData &state);
} // namespace SetPointManager

struct SetPointManagerData : BaseGlobalStruct
{
    bool ManagerOn = false;
    bool GetInputFlag = true; // First time, input is "gotten"

    bool InitSetPointManagersOneTimeFlag = true;
    bool InitSetPointManagersOneTimeFlag2 = true;
    // This will cause a warning to be issued if no "surface" ground temperature object was input.

    std::array<bool, (int)DataEnvironment::GroundTempType::Num> NoGroundTempObjWarning = {true, true, true, true};
    bool InitSetPointManagersMyEnvrnFlag = true; // flag for init once at start of environment

    // Object Data
    Array1D<SetPointManager::SPMBase *> spms;
    std::map<std::string, int> spmMap;

    // IdealCondenserEnteringTemp::calculate() state variables
    bool ICET_RunSubOptCondEntTemp = false;
    bool ICET_RunFinalOptCondEntTemp = false;
    Real64 ICET_CondenserWaterSetPt = 0; // Condenser entering water temperature setpoint this timestep, C
    Real64 ICET_TotEnergyPre = 0;        // Total energy consumptions at the previous time step

    // Condenser Entering Temp::calculate() state variables
    Real64 CET_ActualLoadSum = 0.0;
    Real64 CET_DesignLoadSum = 0.0;
    Real64 CET_WeightedActualLoadSum = 0.0;
    Real64 CET_WeightedDesignLoadSum = 0.0;
    Real64 CET_WeightedLoadRatio = 0.0;
    Real64 CET_DesignMinCondenserSetPt = 0.0;
    Real64 CET_DesignEnteringCondenserTemp = 0.0;
    Real64 CET_DesignMinWetBulbTemp = 0.0;
    Real64 CET_MinActualWetBulbTemp = 0.0;
    Real64 CET_OptCondenserEnteringTemp = 0.0;
    Real64 CET_CurMinLift = 0.0;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        ManagerOn = false;
        GetInputFlag = true; // First time, input is "gotten"
        // Object Data
        InitSetPointManagersOneTimeFlag = true;
        InitSetPointManagersOneTimeFlag2 = true;

        for (int iSPM = 1; iSPM <= (int)spms.size(); ++iSPM) {
            delete spms(iSPM);
        }
        spms.deallocate();
        spmMap.clear();

        NoGroundTempObjWarning = {true, true, true, true};
        InitSetPointManagersMyEnvrnFlag = true;

        ICET_RunSubOptCondEntTemp = false;
        ICET_RunFinalOptCondEntTemp = false;
        ICET_CondenserWaterSetPt = 0;
        ICET_TotEnergyPre = 0;

        CET_ActualLoadSum = 0.0;
        CET_DesignLoadSum = 0.0;
        CET_WeightedActualLoadSum = 0.0;
        CET_WeightedDesignLoadSum = 0.0;
        CET_WeightedLoadRatio = 0.0;
        CET_DesignMinCondenserSetPt = 0.0;
        CET_DesignEnteringCondenserTemp = 0.0;
        CET_DesignMinWetBulbTemp = 0.0;
        CET_MinActualWetBulbTemp = 0.0;
        CET_OptCondenserEnteringTemp = 0.0;
        CET_CurMinLift = 0.0;
    }
};

} // namespace EnergyPlus

#endif
