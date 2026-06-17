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
#ifndef HybridUnitaryAirConditioners_hh_INCLUDED
#define HybridUnitaryAirConditioners_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/HybridEvapCoolingModel.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HybridUnitaryAirConditioners {

    using HybridEvapCoolingModel::CSetting;
    using HybridEvapCoolingModel::Model;

    // MODULE PARAMETER DEFINITIONS
    void SimZoneHybridUnitaryAirConditioners(EnergyPlusData &state,
                                             std::string_view CompName,      // name of the packaged terminal heat pump
                                             int ZoneNum,                    // number of zone being served
                                             Real64 &SensibleOutputProvided, // sensible capacity delivered to zone
                                             Real64 &LatentOutputProvided,   // Latent add/removal  (kg/s), dehumid = negative
                                             int &CompIndex                  // index to zone hvac unit
    );

    void GetInputZoneHybridUnitaryAirConditioners(EnergyPlusData &state, bool &Errors);

    void InitZoneHybridUnitaryAirConditioners(EnergyPlusData &state,
                                              int UnitNum, // unit number
                                              int ZoneNum  // number of zone being served
    );

    void CalcZoneHybridUnitaryAirConditioners(EnergyPlusData &state,
                                              int UnitNum,                    // unit number
                                              int ZoneNum,                    // number of zone being served
                                              Real64 &SensibleOutputProvided, // sensible capacity delivered to zone
                                              Real64 &LatentOutputProvided    // Latent add/removal  (kg/s), dehumid = negative
    );

    void ReportZoneHybridUnitaryAirConditioners(EnergyPlusData &state, int UnitNum);

    int GetHybridUnitaryACOutAirNode(EnergyPlusData &state, int UnitNum);

    int GetHybridUnitaryACZoneInletNode(EnergyPlusData &state, int UnitNum);

    int GetHybridUnitaryACReturnAirNode(EnergyPlusData &state, int UnitNum);

    int getHybridUnitaryACIndex(EnergyPlusData &state, std::string_view CompName);

} // namespace HybridUnitaryAirConditioners

struct HybridUnitaryAirConditionersData : BaseGlobalStruct
{

    int NumZoneHybridEvap = 0;
    bool GetInputZoneHybridEvap = true;
    bool ZoneEquipmentListChecked = false;
    bool HybridCoolOneTimeFlag = true;
    Array1D_bool CheckZoneHybridEvapName;
    EPVector<HybridUnitaryAirConditioners::Model> ZoneHybridUnitaryAirConditioner;
    Array1D_bool MySizeFlag;
    Array1D_bool MyEnvrnFlag;
    Array1D_bool MyFanFlag;
    Array1D_bool MyZoneEqFlag; // used to set up zone equipment availability managers

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumZoneHybridEvap = 0;
        this->GetInputZoneHybridEvap = true;
        this->ZoneEquipmentListChecked = false;
        this->HybridCoolOneTimeFlag = true;
        this->CheckZoneHybridEvapName.deallocate();
        this->ZoneHybridUnitaryAirConditioner.deallocate();
        this->MySizeFlag.clear();
        this->MyEnvrnFlag.clear();
        this->MyFanFlag.clear();
        this->MyZoneEqFlag.clear();
    }
};

} // namespace EnergyPlus
#endif
