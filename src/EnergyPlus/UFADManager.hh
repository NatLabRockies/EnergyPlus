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

#ifndef UFADManager_hh_INCLUDED
#define UFADManager_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace RoomAir {

    struct UFADConvCoef
    {
        Real64 HAT_MX = 0.0;    // HAT_MX Convection Coefficient times Area times Temperature for the upper subzone
        Real64 HAT_MXWin = 0.0; // HAT_MX Convection Coefficient times Area times Temperature for the upper subzone (windows only)
        Real64 HA_MX = 0.0;     // HA_MX Convection Coefficient times Area for the upper subzone
        Real64 HA_MXWin = 0.0;  // HA_MX Convection Coefficient times Area for the upper subzone (windows only)
        Real64 HAT_OC = 0.0;    // HAT_OC Convection Coefficient times Area times Temperature for the lower subzone
        Real64 HAT_OCWin = 0.0; // HAT_OC Convection Coefficient times Area times Temperature for the lower subzone (windows only)
        Real64 HA_OC = 0.0;     // HA_OC Convection Coefficient times Area for the lower subzone
        Real64 HA_OCWin = 0.0;  // HA_OC Convection Coefficient times Area for the lower subzone (windows only)
        Real64 HAT_FLOOR = 0.0; // HAT_FLOOR Convection Coefficient times Area times Temperature for the floor(?) subzone
        Real64 HA_FLOOR = 0.0;  // HA_FLOOR Convection Coefficient times Area for the floor(?) subzone
    };

    void ManageUFAD(EnergyPlusData &state,
                    int const ZoneNum,                        // index number for the specified zone
                    RoomAir::RoomAirModel const ZoneModelType // type of zone model; UCSDUFI = 6
    );

    void InitUFAD(EnergyPlusData &state,
                  int const ZoneNum,
                  RoomAir::RoomAirModel const ZoneModelType // type of zone model; UCSDUFI = 6
    );

    void SizeUFAD(EnergyPlusData &state,
                  int const ZoneNum,
                  RoomAir::RoomAirModel const ZoneModelType // type of zone model; UCSDUFI = 6
    );

    Real64 sumUFADConvGainPerPlume(EnergyPlusData const &state, int const zoneNum, Real64 const numOccupants);

    void HcUFAD(EnergyPlusData &state, int const ZoneNum, Real64 const FractionHeight, UFADConvCoef &ufadCC);

    void CalcUFADInt(EnergyPlusData &state, int const ZoneNum); // index number for the specified zone

    void CalcUFADExt(EnergyPlusData &state, int const ZoneNum); // index number for the specified zone

} // namespace RoomAir

struct UFADManagerData : BaseGlobalStruct
{

    Real64 HeightFloorSubzoneTop = 0.2;   // Assumed thickness of floor subzone
    Real64 ThickOccupiedSubzoneMin = 0.2; // Minimum thickness of occupied subzone
    Real64 HeightIntMass = 0.0;           // Height of internal mass surfaces, assumed vertical, cannot exceed ceiling height
    Real64 HeightIntMassDefault = 2.0;    // Default height of internal mass surfaces

    bool MyOneTimeFlag = true;
    Array1D_bool MySizeFlag;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->MyOneTimeFlag = true;
        this->MySizeFlag.deallocate();
    }

    // Default Constructor
    UFADManagerData() = default;
};
} // namespace EnergyPlus

#endif
