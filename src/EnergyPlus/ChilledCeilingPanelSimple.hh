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

#ifndef ChilledCeilingPanelSimple_hh_INCLUDED
#define ChilledCeilingPanelSimple_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace CoolingPanelSimple {

    // Control types:
    enum class ClgPanelCtrlType
    {
        Invalid = -1,
        MAT,                // Controls system using mean air temperature
        MRT,                // Controls system using mean radiant temperature
        Operative,          // Controls system using operative temperature
        ODB,                // Controls system using outside air dry-bulb temperature
        OWB,                // Controls system using outside air wet-bulb temperature
        ZoneTotalLoad,      // Controls system using zone total remaining load
        ZoneConvectiveLoad, // Controls system using zone convective remaining load
        Num
    };

    // Condensation control types:
    enum class CondCtrl
    {
        Invalid = -1,
        NONE,      // Condensation control--none, so system never shuts down
        SIMPLEOFF, // Condensation control--simple off, system shuts off when condensation predicted
        VARIEDOFF, // Condensation control--variable off, system modulates to keep running if possible
        Num
    };

    struct CoolingPanelParams
    {
        // Members
        std::string Name;
        DataPlant::PlantEquipmentType EquipType = DataPlant::PlantEquipmentType::Invalid;
        std::string Schedule;
        Array1D_string SurfaceName;
        Array1D_int SurfacePtr;
        int ZonePtr = 0;
        Sched::Schedule *availSched = nullptr;
        int WaterInletNode = 0;
        int WaterOutletNode = 0;
        int TotSurfToDistrib = 0;
        int ControlCompTypeNum = 0;
        int CompErrIndex = 0;
        ClgPanelCtrlType controlType = ClgPanelCtrlType::Invalid;
        std::string ColdSetptSchedName;
        Sched::Schedule *coldSetptSched = nullptr;
        CondCtrl CondCtrlType = CondCtrl::NONE;
        Real64 CondDewPtDeltaT = 0.0;
        int CondErrIndex = 0;
        Real64 ColdThrottlRange = 0.0;
        Real64 RatedWaterTemp = 0.0;
        int CoolingCapMethod = 0;
        Real64 ScaledCoolingCapacity = 0.0;
        Real64 UA = 0.0;
        Real64 Offset = 0.0;
        Real64 WaterMassFlowRate = 0.0;
        Real64 WaterMassFlowRateMax = 0.0;
        Real64 RatedWaterFlowRate = 0.0;
        Real64 WaterVolFlowRateMax = 0.0;
        Real64 WaterInletTempStd = 0.0;
        Real64 WaterInletTemp = 0.0;
        Real64 WaterInletEnthalpy = 0.0;
        Real64 WaterOutletTempStd = 0.0;
        Real64 WaterOutletTemp = 0.0;
        Real64 WaterOutletEnthalpy = 0.0;
        Real64 RatedZoneAirTemp = 0.0;
        Real64 FracRadiant = 0.0;
        Real64 FracConvect = 0.0;
        Real64 FracDistribPerson = 0.0;
        Array1D<Real64> FracDistribToSurf;
        Real64 TotPower = 0.0;
        Real64 Power = 0.0;
        Real64 ConvPower = 0.0;
        Real64 RadPower = 0.0;
        Real64 TotEnergy = 0.0;
        Real64 Energy = 0.0;
        Real64 ConvEnergy = 0.0;
        Real64 RadEnergy = 0.0;
        PlantLocation plantLoc;
        int CoolingPanelLoadReSimIndex = 0;
        int CoolingPanelMassFlowReSimIndex = 0;
        int CoolingPanelInletTempFlowReSimIndex = 0;
        bool MyEnvrnFlag = true;
        Real64 ZeroCPSourceSumHATsurf = 0.0;
        Real64 CoolingPanelSource = 0.0;
        Real64 CoolingPanelSrcAvg = 0.0;
        Real64 LastCoolingPanelSrc = 0.0;
        Real64 LastSysTimeElapsed = 0.0;
        Real64 LastTimeStepSys = 0.0;
        bool SetLoopIndexFlag = true;
        bool MySizeFlagCoolPanel = true;
        bool CheckEquipName = true;
        Array1D_string FieldNames;
        bool ZoneEquipmentListChecked = false;

        void CalcCoolingPanel(EnergyPlusData &state, int CoolingPanelNum);

        Real64 getCoolingPanelControlTemp(EnergyPlusData &state, int ZoneNum) const;

        bool SizeCoolingPanelUA(EnergyPlusData &state);

        void ReportCoolingPanel(EnergyPlusData &state);
    };

    void SimCoolingPanel(
        EnergyPlusData &state, std::string const &EquipName, int ControlledZoneNum, bool FirstHVACIteration, Real64 &PowerMet, int &CompIndex);

    void GetCoolingPanelInput(EnergyPlusData &state);

    void InitCoolingPanel(EnergyPlusData &state, int CoolingPanelNum, int ControlledZoneNum, bool FirstHVACIteration);

    void SizeCoolingPanel(EnergyPlusData &state, int CoolingPanelNum);

    void UpdateCoolingPanel(EnergyPlusData &state, int CoolingPanelNum);

    void UpdateCoolingPanelSourceValAvg(EnergyPlusData &state,
                                        bool &CoolingPanelSysOn); // .TRUE. if the radiant system has run this zone time step

    void DistributeCoolingPanelRadGains(EnergyPlusData &state);

} // namespace CoolingPanelSimple

struct ChilledCeilingPanelSimpleData : BaseGlobalStruct
{
    bool GetInputFlag = true;
    Array1D<CoolingPanelSimple::CoolingPanelParams> CoolingPanel;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) ChilledCeilingPanelSimpleData();
    }
};

} // namespace EnergyPlus

#endif
