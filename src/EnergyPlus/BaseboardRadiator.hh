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

#ifndef BaseboardRadiator_hh_INCLUDED
#define BaseboardRadiator_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/DataPlant.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace BaseboardRadiator {

    struct BaseboardParams
    {
        // Members
        std::string EquipID;
        std::string Schedule;
        Sched::Schedule *availSched = nullptr;
        DataPlant::PlantEquipmentType EquipType = DataPlant::PlantEquipmentType::Invalid;
        int ZonePtr = 0;
        int WaterInletNode = 0;
        int WaterOutletNode = 0;
        int ControlCompTypeNum = 0;
        int CompErrIndex = 0;
        Real64 UA = 0.0;
        Real64 WaterMassFlowRate = 0.0;
        Real64 WaterVolFlowRateMax = 0.0;  // m3/s
        Real64 WaterMassFlowRateMax = 0.0; // kg/s
        Real64 Offset = 0.0;
        Real64 AirMassFlowRate = 0.0;    // kg/s
        Real64 DesAirMassFlowRate = 0.0; // kg/s
        Real64 WaterInletTemp = 0.0;
        Real64 WaterOutletTemp = 0.0;
        Real64 WaterInletEnthalpy = 0.0;
        Real64 WaterOutletEnthalpy = 0.0;
        Real64 AirInletTemp = 0.0;
        Real64 AirInletHumRat = 0.0;
        Real64 AirOutletTemp = 0.0;
        Real64 Power = 0.0;
        Real64 Energy = 0.0;
        PlantLocation plantLoc;
        std::vector<std::string> FieldNames;
        int HeatingCapMethod = 0; // - Method for water baseboard Radiator system heating capacity scaledsizing calculation (HeatingDesignCapacity,
                                  // CapacityPerFloorArea, FracOfAutosizedHeatingCapacity)
        Real64 ScaledHeatingCapacity = 0.0; // -  water baseboard Radiator system scaled maximum heating capacity {W} or scalable variable of zone
                                            // HVAC equipment, {-}, or {W/m2}

        bool MySizeFlag = true;
        bool CheckEquipName = true;
        bool SetLoopIndexFlag = true;
        bool MyEnvrnFlag = true;

        void InitBaseboard(EnergyPlusData &state, int baseboardNum);

        void SizeBaseboard(EnergyPlusData &state, int baseboardNum);

        void checkForZoneSizing(EnergyPlusData &state);
    };

    void SimBaseboard(
        EnergyPlusData &state, std::string const &EquipName, int ControlledZoneNum, bool FirstHVACIteration, Real64 &PowerMet, int &CompIndex);

    void GetBaseboardInput(EnergyPlusData &state);

    void SimHWConvective(EnergyPlusData &state, int &BaseboardNum, Real64 &LoadMet);

    void UpdateBaseboard(EnergyPlusData &state, int &BaseboardNum);

} // namespace BaseboardRadiator

struct BaseboardRadiatorData : BaseGlobalStruct
{

    bool getInputFlag = true;
    EPVector<BaseboardRadiator::BaseboardParams> baseboards;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) BaseboardRadiatorData();
    }
};

} // namespace EnergyPlus

#endif
