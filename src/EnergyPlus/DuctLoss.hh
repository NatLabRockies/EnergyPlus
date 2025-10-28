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

#ifndef DuctLoss_hh_INCLUDED
#define DuctLoss_hh_INCLUDED

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

namespace DuctLoss {

    enum class EnvironmentType
    {
        Invalid = -1,
        Zone,
        Schedule,
        Num
    };

    enum class DuctLossType
    {
        Invalid = -1,
        Conduction,
        Leakage,
        MakeupAir,
        Num
    };

    enum class DuctLossSubType
    {
        Invalid = -1,
        SupplyBranch,
        SupplyTrunk,
        ReturnBranch,
        ReturnTrunk,
        SupLeakTrunk,
        SupLeakBranch,
        RetLeakTrunk,
        RetLeakBranch,
        Num
    };

    enum class AirPath
    {
        Invalid = -1,
        Supply,
        Return,
        Num
    };

    struct DuctLossComp
    {
        std::string Name;        // Name of the DuctLoss
        std::string AirLoopName; // Name of the Airloop
        EnvironmentType EnvType = EnvironmentType::Invalid;
        std::string ZoneName;                 // Name of the zone
        std::string ScheduleNameT;            // Name of the schedule
        std::string ScheduleNameW;            // Name of the schedule
        Sched::Schedule *tambSched = nullptr; // ambient temperature schedule pointer
        Sched::Schedule *wambSched = nullptr; // ambient humidity ratio schedule pointer
        DuctLossType LossType = DuctLossType::Invalid;
        int AirLoopNum; // AirLoop number
        int LinkageNum; // Linkage number
        int ZoneNum;    // Zone number
        DuctLossSubType LossSubType = DuctLossSubType::Invalid;
        Real64 Qsen;        // Sensible load
        Real64 Qlat;        // Latent load
        Real64 QsenSL;      // Sensible load
        Real64 QlatSL;      // Latent load
        int RetLeakZoneNum; // Return branch leak source zone number

        void CalcDuctLoss(EnergyPlusData &state, int Index);

        void CalcConduction(EnergyPlusData &state);

        void CalcLeakage(EnergyPlusData &state);

        void CalcMakeupAir(EnergyPlusData &state);
    };

    void SimulateDuctLoss(EnergyPlusData &state, AirPath AirPathWay = AirPath::Invalid, int PathNum = 0);

    void GetDuctLossInput(EnergyPlusData &state);

    void InitDuctLoss(EnergyPlusData &state);

    void ReportDuctLoss(EnergyPlusData &state);

    void ReturnPathUpdate(EnergyPlusData &state, int MixerNum);

    void SupplyPathUpdate(EnergyPlusData &state, int SplitterNum);

} // namespace DuctLoss

struct DuctLossData : BaseGlobalStruct
{
    bool DuctLossSimu = false;
    int NumOfDuctLosses = 0;           // Number of duct loss objects
    bool GetDuctLossInputFlag = true;  // Flag set to make sure you get input once
    bool AirLoopConnectionFlag = true; // Flag set to make sure you get input once
    int AirLoopInNodeNum = 0;
    Array1D<DuctLoss::DuctLossComp> ductloss;
    Array1D<Real64> ZoneSen;          // Sensible zone load
    Array1D<Real64> ZoneLat;          // Latent zone load
    Real64 SysSen = 0.0;              // System sensible load
    Real64 SysLat = 0.0;              // System latent load
    int CtrlZoneNum;                  // Controlled zone number
    Array1D<bool> SubTypeSimuFlag;    // Sub duct loss type simulation flag
    Array1D<int> ZoneEquipInletNodes; // Zone inlet nodes used without supply branch
    int SplitterNum = 0;              // Splitter component number
    int MixerNum = 0;                 // Mixer component number

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) DuctLossData();
    }
};

} // namespace EnergyPlus

#endif
