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

#ifndef ExhaustAirSystemManager_hh_INCLUDED
#define ExhaustAirSystemManager_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace ExhaustAirSystemManager {

    struct ExhaustAir
    {
        // Members
        std::string Name;

        Sched::Schedule *availSched = nullptr;
        std::string ZoneMixerName;
        int ZoneMixerIndex = 0;
        HVAC::FanType centralFanType = HVAC::FanType::Invalid;
        std::string CentralFanName;
        int CentralFanIndex = 0;

        bool SizingFlag = true;

        // output variables
        Real64 centralFan_MassFlowRate = 0.0;
        Real64 centralFan_VolumeFlowRate_Std = 0.0;
        Real64 centralFan_VolumeFlowRate_Cur = 0.0;
        Real64 centralFan_Power = 0.0;
        Real64 centralFan_Energy = 0.0;

        // Output acc variable for heat rejection outputs
        Real64 exhTotalHVACReliefHeatLoss = 0.0; // feed to state.dataHeatBal->SysTotalHVACReliefHeatLoss
    };

    struct ZoneExhaustControl
    {
        enum class FlowControlType
        {
            Invalid = -1,
            Scheduled,
            FollowSupply,
            Num
        };

        std::string Name;

        Sched::Schedule *availSched = nullptr;

        std::string ZoneName;
        int ZoneNum = 0;
        int ControlledZoneNum = 0;

        int InletNodeNum = 0;
        int OutletNodeNum = 0;

        Real64 DesignExhaustFlowRate = 0.0;
        FlowControlType FlowControlOption = FlowControlType::Scheduled;
        Sched::Schedule *exhaustFlowFractionSched = nullptr;
        std::string SupplyNodeOrNodelistName;
        int SupplyNodeOrNodelistNum = 0; // may not need this one
        Sched::Schedule *minZoneTempLimitSched = nullptr;
        Sched::Schedule *minExhFlowFracSched = nullptr;
        Sched::Schedule *balancedExhFracSched = nullptr;
        Real64 BalancedFlow = 0.0;
        Real64 UnbalancedFlow = 0.0;

        Array1D_int SuppNodeNums;
    };

    void SimExhaustAirSystem(EnergyPlusData &state, bool FirstHVACIteration);

    void GetExhaustAirSystemInput(EnergyPlusData &state);

    void CalcExhaustAirSystem(EnergyPlusData &state, int const ExhaustAirSystemNum, bool FirstHVACIteration);

    void GetZoneExhaustControlInput(EnergyPlusData &state);

    void SimZoneHVACExhaustControls(EnergyPlusData &state);

    void CalcZoneHVACExhaustControl(EnergyPlusData &state, int const ZoneHVACExhaustControlNum, Real64 const FlowRatio = -1.0);

    void SizeExhaustSystem(EnergyPlusData &state, int const exhSysNum);

    void SizeExhaustControlFlow(EnergyPlusData &state, int const zoneExhCtrlNum, Array1D_int &NodeNums);

    void UpdateZoneExhaustControl(EnergyPlusData &state);

    void CheckForSupplyNode(EnergyPlusData &state, int const ExhCtrlNum, bool &NodeNotFound);

    bool ExhaustSystemHasMixer(EnergyPlusData &state, std::string_view CompName); // component (mixer) name
} // namespace ExhaustAirSystemManager

struct ExhaustAirSystemMgr : BaseGlobalStruct
{
    bool GetInputFlag = true;
    std::map<int, int> mixerIndexMap;
    bool mappingDone = false;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) ExhaustAirSystemMgr();
    }
};

struct ExhaustControlSystemMgr : BaseGlobalStruct
{

    bool GetInputFlag = true;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) ExhaustControlSystemMgr();
    }
};

} // namespace EnergyPlus

#endif
