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

#ifndef HVACSingleDuctInduc_hh_INCLUDED
#define HVACSingleDuctInduc_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/Enums.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HVACSingleDuctInduc {

    enum class SingleDuct_CV
    {
        Invalid = -1,
        TwoPipeInduc,
        FourPipeInduc,
        Num
    };

    struct IndUnitData
    {
        // Members
        // input data
        std::string Name;                      // name of unit
        std::string UnitType;                  // type of unit
        SingleDuct_CV UnitType_Num;            // index to type of unit
        Sched::Schedule *availSched = nullptr; // index to schedule
        Real64 MaxTotAirVolFlow;               // m3/s (autosizable)
        Real64 MaxTotAirMassFlow;              // kg/s
        Real64 InducRatio;                     // ratio of induced air flow to primary air flow
        int PriAirInNode;                      // unit primary air inlet node number
        int SecAirInNode;                      // unit induced air inlet node number
        int OutAirNode;                        // unit air outlet node number
        int HWControlNode;                     // hot water control node
        int CWControlNode;                     // cold water control node
        std::string HCoilType;                 // type of heating coil component
        std::string HCoil;                     // name of heating coil component
        int HCoil_Num;                         // index to this coil
        DataPlant::PlantEquipmentType HeatingCoilType;
        Real64 MaxVolHotWaterFlow; // m3/s (autosizable)
        Real64 MaxHotWaterFlow;    // kg/s
        Real64 MinVolHotWaterFlow; // m3/s
        Real64 MinHotWaterFlow;    // kg/s
        Real64 HotControlOffset;   // control tolerance
        PlantLocation HWPlantLoc;  // index for plant location for hot water coil
        int HWCoilFailNum1;        // index for errors
        int HWCoilFailNum2;        // index for errors
        std::string CCoilType;     // type of cooling coil component
        std::string CCoil;         // name of cooling coil component
        int CCoil_Num;             // index to this coil
        DataPlant::PlantEquipmentType CoolingCoilType;
        Real64 MaxVolColdWaterFlow; // m3/s (autosizable)
        Real64 MaxColdWaterFlow;    // kg/s
        Real64 MinVolColdWaterFlow; // m3/s
        Real64 MinColdWaterFlow;    // kg/s
        Real64 ColdControlOffset;   // control tolerance
        PlantLocation CWPlantLoc;   // index for plant component for chilled water coil
        int CWCoilFailNum1;         // index for errors
        int CWCoilFailNum2;         // index for errors
        std::string MixerName;      // name of air mixer component
        int Mixer_Num;              // index to this mixer
        Real64 MaxPriAirMassFlow;   // kg/s
        Real64 MaxSecAirMassFlow;   // kg/s
        int ADUNum;                 // index of corresponding air distribution unit
        Real64 DesCoolingLoad;      // used for reporting during coil sizing
        Real64 DesHeatingLoad;      // used for reporting during coil sizing
        int CtrlZoneNum;            // Pointer to CtrlZone data structure
        int CtrlZoneInNodeIndex;    // which controlled zone inlet node number corresponds with this unit
        int AirLoopNum;             // index to airloop that this terminal unit is connected to
        Real64 OutdoorAirFlowRate;  // zone outdoor air volume flow rate

        // Default Constructor
        IndUnitData()
            : UnitType_Num(SingleDuct_CV::Invalid), MaxTotAirVolFlow(0.0), MaxTotAirMassFlow(0.0), InducRatio(2.5), PriAirInNode(0), SecAirInNode(0),
              OutAirNode(0), HWControlNode(0), CWControlNode(0), HCoil_Num(0), HeatingCoilType(DataPlant::PlantEquipmentType::Invalid),
              MaxVolHotWaterFlow(0.0), MaxHotWaterFlow(0.0), MinVolHotWaterFlow(0.0), MinHotWaterFlow(0.0), HotControlOffset(0.0), HWPlantLoc{},
              HWCoilFailNum1(0), HWCoilFailNum2(0), CCoil_Num(0), CoolingCoilType(DataPlant::PlantEquipmentType::Invalid), MaxVolColdWaterFlow(0.0),
              MaxColdWaterFlow(0.0), MinVolColdWaterFlow(0.0), MinColdWaterFlow(0.0), ColdControlOffset(0.0), CWPlantLoc{}, CWCoilFailNum1(0),
              CWCoilFailNum2(0), Mixer_Num(0), MaxPriAirMassFlow(0.0), MaxSecAirMassFlow(0.0), ADUNum(0), DesCoolingLoad(0.0), DesHeatingLoad(0.0),
              CtrlZoneNum(0), CtrlZoneInNodeIndex(0), AirLoopNum(0), OutdoorAirFlowRate(0.0)
        {
        }
        void ReportIndUnit(EnergyPlusData &state);
        void CalcOutdoorAirVolumeFlowRate(EnergyPlusData &state);
        void reportTerminalUnit(EnergyPlusData &state);
    };

    void SimIndUnit(EnergyPlusData &state,
                    std::string_view CompName,     // name of the terminal unit
                    bool const FirstHVACIteration, // TRUE if first HVAC iteration in time step
                    int const ZoneNum,             // index of zone served by the terminal unit
                    int const ZoneNodeNum,         // zone node number of zone served by the terminal unit
                    int &CompIndex                 // which terminal unit in data structure
    );

    void GetIndUnits(EnergyPlusData &state);

    void InitIndUnit(EnergyPlusData &state,
                     int const IUNum,              // number of the current induction unit being simulated
                     bool const FirstHVACIteration // TRUE if first air loop solution this HVAC step
    );

    void SizeIndUnit(EnergyPlusData &state, int const IUNum);

    void SimFourPipeIndUnit(EnergyPlusData &state,
                            int const IUNum,              // number of the current unit being simulated
                            int const ZoneNum,            // number of zone being served
                            int const ZoneNodeNum,        // zone node number
                            bool const FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
    );

    void CalcFourPipeIndUnit(EnergyPlusData &state,
                             int const IUNum,               // Unit index
                             bool const FirstHVACIteration, // flag for 1st HVAV iteration in the time step
                             int const ZoneNode,            // zone node number
                             Real64 const HWFlow,           // hot water flow (kg/s)
                             Real64 const CWFlow,           // cold water flow (kg/s)
                             Real64 &LoadMet                // load met by unit (watts)
    );

    bool FourPipeInductionUnitHasMixer(EnergyPlusData &state, std::string_view CompName); // component (mixer) name

} // namespace HVACSingleDuctInduc

struct HVACSingleDuctInducData : BaseGlobalStruct
{
    int NumIndUnits = 0;
    int NumFourPipes = 0;
    Array1D_bool CheckEquipName;
    bool GetIUInputFlag = true; // First time, input is "gotten"
    bool MyOneTimeFlag = true;
    Array1D_bool MyEnvrnFlag;
    Array1D_bool MySizeFlag;
    Array1D_bool MyPlantScanFlag;
    Array1D_bool MyAirDistInitFlag;
    Array1D<HVACSingleDuctInduc::IndUnitData> IndUnit;
    bool ZoneEquipmentListChecked = false;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumIndUnits = 0;
        this->IndUnit.deallocate();
        this->GetIUInputFlag = true;
        this->NumFourPipes = 0;
        this->MyOneTimeFlag = true;
        this->MyEnvrnFlag.deallocate();
        this->MySizeFlag.deallocate();
        this->MyPlantScanFlag.deallocate();
        this->MyAirDistInitFlag.deallocate();
        this->CheckEquipName.deallocate();
        this->ZoneEquipmentListChecked = false;
    }
};

} // namespace EnergyPlus

#endif
