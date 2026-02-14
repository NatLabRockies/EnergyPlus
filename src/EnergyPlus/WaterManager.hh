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

#ifndef WaterManager_hh_INCLUDED
#define WaterManager_hh_INCLUDED

// C++ Headers
#include <string>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace WaterManager {

    void ManageWater(EnergyPlusData &state);

    void ManageWaterInits(EnergyPlusData &state);

    void GetWaterManagerInput(EnergyPlusData &state);

    void UpdatePrecipitation(EnergyPlusData &state);

    void UpdateIrrigation(EnergyPlusData &state);

    void CalcWaterStorageTank(EnergyPlusData &state, int const TankNum); // Index of storage tank

    void SetupTankSupplyComponent(EnergyPlusData &state,
                                  std::string_view CompName,
                                  std::string_view CompType,
                                  std::string_view TankName,
                                  bool &ErrorsFound,
                                  int &TankIndex,
                                  int &WaterSupplyIndex);

    void InternalSetupTankSupplyComponent(EnergyPlusData &state,
                                          std::string_view CompName,
                                          std::string_view CompType,
                                          std::string_view TankName,
                                          bool &ErrorsFound,
                                          int &TankIndex,
                                          int &WaterSupplyIndex);

    void SetupTankDemandComponent(EnergyPlusData &state,
                                  std::string_view CompName,
                                  std::string_view const CompType,
                                  std::string_view TankName,
                                  bool &ErrorsFound,
                                  int &TankIndex,
                                  int &WaterDemandIndex);

    void InternalSetupTankDemandComponent(EnergyPlusData &state,
                                          std::string_view CompName,
                                          std::string_view const CompType,
                                          std::string_view TankName,
                                          bool &ErrorsFound,
                                          int &TankIndex,
                                          int &WaterDemandIndex);

    void CalcRainCollector(EnergyPlusData &state, int const RainColNum); // Index of rain collector

    void CalcGroundwaterWell(EnergyPlusData &state, int const WellNum); // Index of well

    void UpdateWaterManager(EnergyPlusData &state);

    void ReportRainfall(EnergyPlusData &state);

} // namespace WaterManager

struct WaterManagerData : BaseGlobalStruct
{

    bool MyOneTimeFlag;
    bool GetInputFlag; // First time, input is "gotten"
    bool MyEnvrnFlag;  // flag for init once at start of environment
    bool MyWarmupFlag; // flag for init after warmup complete
    bool MyTankDemandCheckFlag;
    Real64 overflowTwater = 0.0;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->MyOneTimeFlag = true;
        this->GetInputFlag = true;
        this->MyEnvrnFlag = true;
        this->MyWarmupFlag = false;
        this->MyTankDemandCheckFlag = true;
        this->overflowTwater = 0.0;
    }

    // Default Constructor
    WaterManagerData()
        : MyOneTimeFlag(true), GetInputFlag(true), MyEnvrnFlag(true), MyWarmupFlag(false), MyTankDemandCheckFlag(true), overflowTwater(0.0)
    {
    }
};

} // namespace EnergyPlus

#endif
