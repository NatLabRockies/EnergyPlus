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

#ifndef SimAirServingZones_hh_INCLUDED
#define SimAirServingZones_hh_INCLUDED

// C++ Headers
#include <string>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACControllers.hh>
#include <EnergyPlus/DataHVACSystems.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace SimAirServingZones {

    constexpr int BeforeBranchSim(1);
    constexpr int AfterBranchSim(2);

    // CompType numerics -- for this module
    // component types addressed by this module
    enum class CompType
    {
        Invalid = -1,
        OAMixer_Num,
        Fan_Simple_CV,
        Fan_Simple_VAV,
        WaterCoil_Cooling,
        WaterCoil_SimpleHeat,
        SteamCoil_AirHeat,
        WaterCoil_DetailedCool,
        Coil_ElectricHeat,
        Coil_GasHeat,
        WaterCoil_CoolingHXAsst,
        Coil_DeSuperHeat,
        DXSystem,
        HeatXchngr,
        Desiccant,
        Unglazed_SolarCollector,
        EvapCooler,
        Furnace_UnitarySys_HeatOnly,
        Furnace_UnitarySys_HeatCool,
        Humidifier,
        Duct,
        UnitarySystem_BypassVAVSys,
        UnitarySystem_MSHeatPump,
        Fan_ComponentModel,
        DXHeatPumpSystem,
        CoilUserDefined,
        Fan_System_Object,
        UnitarySystemModel,
        ZoneVRFasAirLoopEquip,
        PVT_AirBased,
        CoilSystemWater,
        Num
    };

    void ManageAirLoops(EnergyPlusData &state,
                        bool FirstHVACIteration, // TRUE if first full HVAC iteration in an HVAC timestep
                        bool &SimAir,            // TRUE means air loops must be (re)simulated
                        bool &SimZoneEquipment   // TRUE means zone equipment must be (re) simulated
    );

    void GetAirPathData(EnergyPlusData &state);

    void InitAirLoops(EnergyPlusData &state, bool FirstHVACIteration); // TRUE if first full HVAC iteration in an HVAC timestep

    void ConnectReturnNodes(EnergyPlusData &state);

    void SimAirLoops(EnergyPlusData &state, bool FirstHVACIteration, bool &SimZoneEquipment);

    void SimAirLoop(EnergyPlusData &state,
                    bool FirstHVACIteration,
                    int AirLoopNum,
                    int AirLoopPass,
                    int &AirLoopIterMax,
                    int &AirLoopIterTot,
                    int &AirLoopNumCalls);

    void SolveAirLoopControllers(
        EnergyPlusData &state, bool FirstHVACIteration, int AirLoopNum, bool &AirLoopConvergedFlag, int &IterMax, int &IterTot, int &NumCalls);

    void SolveWaterCoilController(EnergyPlusData &state,
                                  bool FirstHVACIteration,
                                  int AirLoopNum,
                                  std::string const &CompName,
                                  int &CompIndex,
                                  std::string const &ControllerName,
                                  int ControllerIndex,
                                  bool HXAssistedWaterCoil);

    void ReSolveAirLoopControllers(
        EnergyPlusData &state, bool FirstHVACIteration, int AirLoopNum, bool &AirLoopConvergedFlag, int &IterMax, int &IterTot, int &NumCalls);

    void SimAirLoopComponents(EnergyPlusData &state,
                              int AirLoopNum,         // Index of the air loop being currently simulated
                              bool FirstHVACIteration // TRUE if first full HVAC iteration in an HVAC timestep
    );

    void SimAirLoopComponent(EnergyPlusData &state,
                             std::string const &CompName, // the component Name
                             CompType CompType_Num,       // numeric equivalent for component type
                             bool FirstHVACIteration,     // TRUE if first full HVAC iteration in an HVAC timestep
                             int AirLoopNum,              // Primary air loop number
                             int &CompIndex,              // numeric pointer for CompType/CompName -- passed back from other routines
                             HVACSystemData *CompPointer, // equipment actual pointer
                             int airLoopNum,              // index to AirloopHVAC
                             int branchNum,               // index to AirloopHVAC branch
                             int compNum                  // index to AirloopHVAC branch component
    );

    void UpdateBranchConnections(EnergyPlusData &state,
                                 int AirLoopNum, // primary air system number
                                 int BranchNum,  // branch reference number
                                 int Update      // 1=BeforeBranchSim; 2=AfterBranchSim
    );

    void ResolveSysFlow(EnergyPlusData &state,
                        int SysNum,    // the primary air system number
                        bool &SysReSim // Set to TRUE if mass balance fails and re-simulation is needed
    );

    void SizeAirLoops(EnergyPlusData &state);

    void SizeAirLoopBranches(EnergyPlusData &state, int AirLoopNum, int BranchNum);

    void SetUpSysSizingArrays(EnergyPlusData &state);

    void SizeSysOutdoorAir(EnergyPlusData &state);

    void UpdateSysSizing(EnergyPlusData &state, Constant::CallIndicator CallIndicator);

    void UpdateSysSizingForScalableInputs(EnergyPlusData &state, int AirLoopNum);

    Real64 GetHeatingSATempForSizing(EnergyPlusData &state, int IndexAirLoop);

    Real64 GetHeatingSATempHumRatForSizing(EnergyPlusData &state, int IndexAirLoop);

    void LimitZoneVentEff(EnergyPlusData &state,
                          Real64 Xs,              // ratio of uncorrected system outdoor air flow rate to the design system supply flow rate
                          Real64 Voz,             // corrected (divided by distribution efficiency) zone outside air flow rate [m3/s]
                          int CtrlZoneNum,        // controlled zone number
                          Real64 &SystemCoolingEv // system ventilation efficiency
    );

    void CheckWaterCoilIsOnAirLoop(EnergyPlusData &state,
                                   SimAirServingZones::CompType CompTypeNum,
                                   std::string const &CompType,
                                   std::string const &CompName,
                                   bool &WaterCoilOnAirLoop);

    bool CheckWaterCoilOnPrimaryAirLoopBranch(EnergyPlusData &state, SimAirServingZones::CompType CompTypeNum, std::string const &CompName);

    bool CheckWaterCoilOnOASystem(EnergyPlusData &state, SimAirServingZones::CompType CompTypeNum, std::string const &CompName);

    bool CheckWaterCoilSystemOnAirLoopOrOASystem(EnergyPlusData &state, SimAirServingZones::CompType CoilTypeNum, std::string const &CompName);

} // namespace SimAirServingZones

struct SimAirServingZonesData : BaseGlobalStruct
{

    bool GetAirLoopInputFlag = true; // Flag set to make sure you get input once
    int NumOfTimeStepInDay = 0;      // number of zone time steps in a day
    bool InitAirLoopsOneTimeFlag = true;
    int TestUniqueNodesNum = 0;
    bool SizeAirLoopsOneTimeFlag = true;
    bool InitAirLoopsBranchSizingFlag = true;
    bool OutputSetupFlag = false;
    bool MyEnvrnFlag = true;

    Array1D<Real64> EvBySysCool; // saved value of SysCoolingEv used in 62.1 tabular report
    Array1D<Real64> EvBySysHeat; // saved value of SysHeatingEv used in 62.1 tabular report
    Real64 Ep = 1.0;             // zone primary air fraction
    Real64 Er = 0.0;             // zone secondary recirculation fraction
    Real64 Fa = 1.0;             // temporary variable used in multi-path VRP calc
    Real64 Fb = 1.0;             // temporary variable used in multi-path VRP calc
    Real64 Fc = 1.0;             // temporary variable used in multi-path VRP calc
    Real64 Xs = 1.0;             // uncorrected system outdoor air fraction
    Real64 MinHeatingEvz = 1.0;  // minimum zone ventilation efficiency for heating (to be used as system efficiency)
    Real64 MinCoolingEvz = 1.0;  // minimum zone ventilation efficiency for cooling (to be used as system efficiency)
    Real64 ZoneOAFrac = 0.0;     // zone OA fraction
    Real64 ZoneEz = 1.0;         // zone air distribution effectiveness
    Real64 Vou = 0.0;            // Uncorrected outdoor air intake for all zones per ASHRAE std 62.1
    Real64 Vot = 0.0;            // Required outdoor air intake at primary AHU per ASHRAE std 62.1

    int TUInNode = 0;            // inlet node number of a terminal unit
    Real64 SumZoneDesFlow = 0.0; // sum of the zone design air mass flow rates for zones served by a system
    Real64 OAReliefDiff = 0.0;   // local for massflow change across OA system, kg/s
    Real64 MassFlowSetToler;
    int salIterMax = 0;   // Maximum of iteration counters across all air loops
    int salIterTot = 0;   // Aggregated number of iterations across all air loops
    int NumCallsTot = 0;  // Aggregated number fo times SimAirLoopComponents() has been invoked across all air loops
    int IterMaxSAL2 = 0;  // Maximum number of iterations performed by each controller on this air loop
    int IterTotSAL2 = 0;  // Aggregated number of iterations performed by each controller on this air loop
    int NumCallsSAL2 = 0; // Number of times SimAirLoopComponents() has been invoked per air loop for either Solve or ReSolve operations
    // TRUE when primary air system & controllers simulation has converged;
    bool AirLoopConvergedFlagSAL = false;
    // TRUE when speculative warm restart is allowed; FALSE otherwise.
    bool DoWarmRestartFlagSAL = false;
    // If Status<0, no speculative warm restart attempted.
    // If Status==0, warm restart failed.
    // If Status>0, warm restart succeeded.
    DataHVACControllers::ControllerWarmRestart WarmRestartStatusSAL = DataHVACControllers::ControllerWarmRestart::None;
    int IterSALC = 0;        // Iteration counter
    int ErrCountSALC = 0;    // Number of times that the maximum iterations was exceeded
    int MaxErrCountSALC = 0; // Number of times that the maximum iterations was exceeded
    // A character string equivalent of ErrCount
    bool BypassOAControllerSALC; // logical to tell ManageControllers to sim or not sim controller in OA System (don't sim here)
    // Iteration counter
    int IterSWCC = 0;
    // Number of times that the maximum iterations was exceeded
    int ErrCountSWCC = 0;
    // Number of times that the maximum iterations was exceeded
    int MaxErrCountSWCC = 0;
    int AirLoopPassSWCC;
    bool BypassOAControllerSWCC;
    bool BypassOAControllerRSALC; // logical to bypass HVAC controller calcs
    Real64 EpSSOA = 1.0;          // zone primary air fraction

    // Last saved HVAC time stamp at beginning of step in seconds.
    // Used to control when to reset the statistic counters for each new HVAC step.
    Real64 SavedPreviousHVACTime = 0.0;

    // Placeholder for environment name used in error reporting
    std::string ErrEnvironmentName;
    std::string ErrEnvironmentNameSolveWaterCoilController;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->GetAirLoopInputFlag = true;
        this->NumOfTimeStepInDay = 0;
        this->InitAirLoopsOneTimeFlag = true;
        this->TestUniqueNodesNum = 0;
        this->SizeAirLoopsOneTimeFlag = true;
        this->InitAirLoopsBranchSizingFlag = true;
        this->OutputSetupFlag = false;
        this->MyEnvrnFlag = true;

        this->EvBySysCool.clear();
        this->EvBySysHeat.clear();
        this->Ep = 1.0;
        this->Er = 0.0;
        this->Fa = 1.0;
        this->Fb = 1.0;
        this->Fc = 1.0;
        this->Xs = 1.0;
        this->MinHeatingEvz = 1.0;
        this->MinCoolingEvz = 1.0;
        this->ZoneOAFrac = 0.0;
        this->ZoneEz = 1.0;
        this->Vou = 0.0;
        this->Vot = 0.0;

        this->TUInNode = 0;         // inlet node number of a terminal unit
        this->SumZoneDesFlow = 0.0; // sum of the zone design air mass flow rates for zones served by a system
        this->OAReliefDiff = 0.0;   // local for massflow change across OA system, kg/s
        this->salIterMax = 0;       // Maximum of iteration counters across all air loops
        this->salIterTot = 0;       // Aggregated number of iterations across all air loops
        this->NumCallsTot = 0;      // Aggregated number fo times SimAirLoopComponents() has been invoked across all air loops
        this->IterMaxSAL2 = 0;      // Maximum number of iterations performed by each controller on this air loop
        this->IterTotSAL2 = 0;      // Aggregated number of iterations performed by each controller on this air loop
        this->NumCallsSAL2 = 0;     // Number of times SimAirLoopComponents() has been invoked per air loop for either Solve or ReSolve operations
        this->AirLoopConvergedFlagSAL = false;
        this->DoWarmRestartFlagSAL = false;
        this->WarmRestartStatusSAL = DataHVACControllers::ControllerWarmRestart::None;
        this->IterSALC = 0;        // Iteration counter
        this->ErrCountSALC = 0;    // Number of times that the maximum iterations was exceeded
        this->MaxErrCountSALC = 0; // Number of times that the maximum iterations was exceeded
        this->IterSWCC = 0;
        this->ErrCountSWCC = 0;
        this->MaxErrCountSWCC = 0;
        this->EpSSOA = 1.0; // zone primary air fraction
        this->SavedPreviousHVACTime = 0.0;
        this->ErrEnvironmentName.clear();
        this->ErrEnvironmentNameSolveWaterCoilController.clear();
    }
};

} // namespace EnergyPlus

#endif
