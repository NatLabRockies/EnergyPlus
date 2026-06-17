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

#ifndef DataDefineEquip_hh_INCLUDED
#define DataDefineEquip_hh_INCLUDED

// C++ Headers
#include <memory>
// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/AirTerminalUnit.hh>
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace DataDefineEquip {

    constexpr int MaxZoneAirComponents = 1;

    // Equipment Types covered by ZoneAirLoopEquipment:
    enum class ZnAirLoopEquipType
    {
        Invalid = -1,
        DualDuctConstVolume,
        DualDuctVAV,
        SingleDuctVAVReheat,
        SingleDuctVAVNoReheat,
        SingleDuctConstVolReheat,
        SingleDuctConstVolNoReheat,
        SingleDuct_SeriesPIU_Reheat,
        SingleDuct_ParallelPIU_Reheat,
        SingleDuct_ConstVol_4PipeInduc,
        SingleDuctVAVReheatVSFan,
        SingleDuctCBVAVReheat,
        SingleDuctCBVAVNoReheat,
        SingleDuctConstVolCooledBeam,
        DualDuctVAVOutdoorAir,
        SingleDuctUserDefined,
        SingleDuctATMixer,
        SingleDuctConstVolFourPipeBeam,
        Num
    };

    struct ZoneAirEquip
    {
        // Members
        std::string Name;         // Name or identifier of this piece of equipment
        int OutletNodeNum = 0;    // index of outlet node
        int NumComponents = 0;    // number of subcomponents (=1)
        int NumControls = 0;      // number of controls (not used; =0)
        Array1D_string EquipType; // Pointer identifying type of subcomponent
        Array1D<DataDefineEquip::ZnAirLoopEquipType> EquipTypeEnum;
        ///// Note use of shared_ptr here is not a good pattern, not to be replicated without further discussion.
        std::shared_ptr<AirTerminalUnit> airTerminalPtr = nullptr;
        Array1D_string EquipName; // name of subcomponent
        Array1D_int EquipIndex;
        int AirTerminalSizingSpecIndex = 0;       // index to DesignSpecification:AirTerminal:Sizing obect
        int TermUnitSizingNum = 0;                // index to TermUnitSizing and TermUnitFinalZoneSizing for this air distribution unit
        Real64 UpStreamLeakFrac = 0.0;            // upstream nominal leakage fraction
        Real64 DownStreamLeakFrac = 0.0;          // downstream constant leakage fraction
        Real64 parallelPIUTerminalLeakFrac = 0.0; // PIU terminal leakage fraction
        Real64 MassFlowRateUpStrLk = 0.0;         // current air mass flow rate of the upstream leak [kg/s]
        Real64 MassFlowRateDnStrLk = 0.0;         // current air mass flow rate of the downstream leak [kg/s]
        Real64 MassFlowRateTU = 0.0;              // current air mass flow rate through the terminal unit [kg/s]
        Real64 MassFlowRateZSup = 0.0;            // current air mass flow rate of zone supply air [kg/s]
        Real64 massFlowRateParallelPIULk = 0.0;   // current air mass flow rate of parallel piu leak [kg/s]
        Real64 MassFlowRateSup = 0.0;             // current air mass flow rate of supply air upstream of upstream leak [kg/s]
        Real64 MassFlowRatePlenInd = 0.0;         // current air mass flow rate of induced air from plenum [kg/s]
        Real64 MaxAvailDelta = 0.0;               // change in max avail mass low rate due to leaks [kg/s]
        Real64 MinAvailDelta = 0.0;               // change in min avail mass low rate due to leaks [kg/s]
        int InletNodeNum = 0;                     // index of inlet node 1
        int InletNodeNum2 = 0;                    // index of inlet node 2 (used for dual duct airterminals)
        int ZoneEqNum = 0;                        // index of zone equipment object for this terminal unit
        int AirLoopNum = 0;                       // index to airloop that this terminal unit is connected to
        Real64 LeakLoadMult = 0.0;                // zome load multiplier to adjust for downstream leak
        bool UpStreamLeak = false;                // if true, there is an upstream leak
        bool DownStreamLeak = false;              // if true, there is an downstream leak
        int RetPlenumNum = 0;                     // return plenum number that this ADU can leak to, zero if none
        int ZoneNum = 0;                          // index of the zone object for this terminal unit
        bool AccountForDOAS = false;              // if true user has asked for DOAS
        Real64 HeatRate = 0.0;                    // [W]
        Real64 CoolRate = 0.0;                    // [W]
        Real64 HeatGain = 0.0;                    // [J]
        Real64 CoolGain = 0.0;                    // [J]
        bool EachOnceFlag = true;
        bool IsConstLeakageRate = false; // if true, constant leakage rate, if false proportional leakage rate will be calculated
        int piuLkZoneNum = 0;            // zone index designated as the destination for the PIU backdraft damper leaks

        // Default Constructor
        ZoneAirEquip()
            : EquipType(MaxZoneAirComponents), EquipTypeEnum(MaxZoneAirComponents, DataDefineEquip::ZnAirLoopEquipType::Invalid),
              EquipName(MaxZoneAirComponents), EquipIndex(MaxZoneAirComponents, 0)
        {
        }
    };

} // namespace DataDefineEquip

struct DefineEquipData : BaseGlobalStruct
{
    Array1D<DataDefineEquip::ZoneAirEquip> AirDistUnit; // Used to specify zone related

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) DefineEquipData();
    }
};

} // namespace EnergyPlus

#endif
