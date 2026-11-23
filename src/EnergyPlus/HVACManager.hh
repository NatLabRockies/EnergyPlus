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

#ifndef HVACManager_hh_INCLUDED
#define HVACManager_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HVACManager {

    // SUBROUTINE SPECIFICATIONS FOR MODULE PrimaryPlantLoops
    // and zone equipment simulations

    enum class ConvErrorCallType
    {
        Invalid = -1,
        MassFlow,
        HumidityRatio,
        Temperature,
        Energy,
        CO2,
        Generic,
        Num
    };

    // Functions
    void ManageHVAC(EnergyPlusData &state);

    void SimHVAC(EnergyPlusData &state);

    void SimSelectedEquipment(EnergyPlusData &state,
                              bool &SimAirLoops,         // True when the air loops need to be (re)simulated
                              bool &SimZoneEquipment,    // True when zone equipment components need to be (re)simulated
                              bool &SimNonZoneEquipment, // True when non-zone equipment components need to be (re)simulated
                              bool &SimPlantLoops,       // True when the main plant loops need to be (re)simulated
                              bool &SimElecCircuits,     // True when electric circuits need to be (re)simulated
                              bool &FirstHVACIteration,  // True when solution technique on first iteration
                              bool LockPlantFlows);

    void ResetTerminalUnitFlowLimits(EnergyPlusData &state);

    void ResolveAirLoopFlowLimits(EnergyPlusData &state);

    void ResolveLockoutFlags(EnergyPlusData &state, bool &SimAir); // TRUE means air loops must be (re)simulated

    void ResetHVACControl(EnergyPlusData const &state);

    void ResetNodeData(EnergyPlusData &state);

    void UpdateZoneListAndGroupLoads(EnergyPlusData &state);

    void ReportAirHeatBalance(EnergyPlusData &state);

    void reportAirHeatBal1(EnergyPlusData &state,
                           DataHeatBalance::AirReportVars &szAirRpt,
                           DataZoneEquipment::EquipConfiguration const &szEquipConfig,
                           int const zoneNum,
                           int const spaceNum = 0);

    void reportAirHeatBal2(EnergyPlusData &state,
                           DataHeatBalance::AirReportVars &szAirRpt,
                           DataZoneEquipment::EquipConfiguration const &szEquipConfig,
                           ZoneTempPredictorCorrector::ZoneSpaceHeatBalanceData const &szHeatBal,
                           int const zoneNum,
                           int const spaceNum = 0);

    void SetHeatToReturnAirFlag(EnergyPlusData &state);

    void UpdateZoneInletConvergenceLog(EnergyPlusData &state);

    void CheckAirLoopFlowBalance(EnergyPlusData &state);

    void ConvergenceErrors(EnergyPlusData &state,
                           std::array<bool, 3> &HVACNotConverged,
                           std::array<Real64, 10> &DemandToSupply,
                           std::array<Real64, 10> &SupplyDeck1ToDemand,
                           std::array<Real64, 10> &SupplyDeck2ToDemand,
                           int AirSysNum,
                           ConvErrorCallType index);

} // namespace HVACManager

struct HVACManagerData : BaseGlobalStruct
{

    int HVACManageIteration = 0; // counts iterations to enforce maximum iteration limit
    int RepIterAir = 0;
    bool SimHVACIterSetup = false;
    bool TriggerGetAFN = true;
    bool MyOneTimeFlag = true;
    bool PrintedWarmup = false;
    bool MyEnvrnFlag = true;
    bool DebugNamesReported = false;
    bool MySetPointInit = true;
    bool MyEnvrnFlag2 = true;
    bool FlowMaxAvailAlreadyReset = false;
    bool FlowResolutionNeeded = false;
    int ErrCount = 0; // Number of times that the maximum iterations was exceeded
    int MaxErrCount = 0;
    std::string ErrEnvironmentName;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) HVACManagerData();
    }
};

} // namespace EnergyPlus

#endif
