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

#ifndef ZonePlenum_hh_INCLUDED
#define ZonePlenum_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace ZonePlenum {

    struct ZoneReturnPlenumConditions
    {
        std::string ZonePlenumName;
        std::string ZoneName;
        std::string ZoneNodeName;
        Real64 ZoneTemp = 0.0;
        Real64 ZoneHumRat = 0.0;
        Real64 ZoneEnthalpy = 0.0;
        Real64 OutletTemp = 0.0;
        Real64 OutletHumRat = 0.0;
        Real64 OutletEnthalpy = 0.0;
        Real64 OutletPressure = 0.0;
        int ZoneNodeNum = 0;
        int ActualZoneNum = 0;
        int OutletNode = 0;
        Real64 OutletMassFlowRate = 0.0;         // MassFlow through the ZonePlenum being Simulated [kg/Sec]
        Real64 OutletMassFlowRateMaxAvail = 0.0; // [kg/Sec]
        Real64 OutletMassFlowRateMinAvail = 0.0; // [kg/Sec]
        int NumInducedNodes = 0;
        Array1D_int InducedNode;
        Array1D<Real64> InducedMassFlowRate;
        Array1D<Real64> InducedMassFlowRateMaxAvail;
        Array1D<Real64> InducedMassFlowRateMinAvail;
        Array1D<Real64> InducedTemp;
        Array1D<Real64> InducedHumRat;
        Array1D<Real64> InducedEnthalpy;
        Array1D<Real64> InducedPressure;
        Array1D<Real64> InducedCO2;
        Array1D<Real64> InducedGenContam;
        bool InitFlag = false;
        int NumInletNodes = 0;
        Array1D_int InletNode;
        Array1D<Real64> InletMassFlowRate;
        Array1D<Real64> InletMassFlowRateMaxAvail;
        Array1D<Real64> InletMassFlowRateMinAvail;
        Array1D<Real64> InletTemp;
        Array1D<Real64> InletHumRat;
        Array1D<Real64> InletEnthalpy;
        Array1D<Real64> InletPressure;
        Array1D_int ADUIndex;  // index to AirDistUnit leaking to this plenum
        int NumADUs;           // number of ADU's that can leak to this plenum
        Array1D_int ZoneEqNum; // list of zone equip config indices for this plenum
        bool checkEquipName = true;
    };

    struct ZoneSupplyPlenumConditions
    {
        std::string ZonePlenumName;
        std::string ZoneName;
        std::string ZoneNodeName;
        Real64 ZoneTemp = 0.0;
        Real64 ZoneHumRat = 0.0;
        Real64 ZoneEnthalpy = 0.0;
        Real64 InletTemp = 0.0;
        Real64 InletHumRat = 0.0;
        Real64 InletEnthalpy = 0.0;
        Real64 InletPressure = 0.0;
        int ZoneNodeNum = 0;
        int ActualZoneNum = 0;
        int InletNode = 0;
        Real64 InletMassFlowRate = 0.0;         // MassFlow through the ZonePlenum being Simulated [kg/Sec]
        Real64 InletMassFlowRateMaxAvail = 0.0; // [kg/Sec]
        Real64 InletMassFlowRateMinAvail = 0.0; // [kg/Sec]
        bool InitFlag = false;
        int NumOutletNodes = 0;
        Array1D_int OutletNode;
        Array1D<Real64> OutletMassFlowRate;
        Array1D<Real64> OutletMassFlowRateMaxAvail;
        Array1D<Real64> OutletMassFlowRateMinAvail;
        Array1D<Real64> OutletTemp;
        Array1D<Real64> OutletHumRat;
        Array1D<Real64> OutletEnthalpy;
        Array1D<Real64> OutletPressure;
        bool checkEquipName = true;
    };

    void SimAirZonePlenum(EnergyPlusData &state,
                          std::string_view CompName,
                          DataZoneEquipment::AirLoopHVACZone iCompType,
                          int &CompIndex,
                          ObjexxFCL::Optional_bool_const FirstHVACIteration = _, // Autodesk:OPTIONAL Used without PRESENT check
                          ObjexxFCL::Optional_bool_const FirstCall = _,          // Autodesk:OPTIONAL Used without PRESENT check
                          ObjexxFCL::Optional_bool PlenumInletChanged = _        // Autodesk:OPTIONAL Used without PRESENT check
    );

    void GetZonePlenumInput(EnergyPlusData &state);

    void InitAirZoneReturnPlenum(EnergyPlusData &state, int ZonePlenumNum);

    void InitAirZoneSupplyPlenum(EnergyPlusData &state, int ZonePlenumNum, bool FirstHVACIteration, bool FirstCall);

    void CalcAirZoneReturnPlenum(EnergyPlusData &state, int ZonePlenumNum);

    void CalcAirZoneSupplyPlenum(EnergyPlusData &state, int ZonePlenumNum, bool FirstCall);

    void UpdateAirZoneReturnPlenum(EnergyPlusData &state, int ZonePlenumNum);

    void UpdateAirZoneSupplyPlenum(EnergyPlusData &state, int ZonePlenumNum, bool &PlenumInletChanged, bool FirstCall);

    int GetReturnPlenumIndex(EnergyPlusData &state, int ExNodeNum);

    void GetReturnPlenumName(EnergyPlusData &state, int ReturnPlenumIndex, std::string &ReturnPlenumName);

    int getReturnPlenumIndexFromInletNode(EnergyPlusData &state, int InNodeNum);

    bool ValidateInducedNode(EnergyPlusData &state, int InduceNodeNum, int NumReturnNodes, Array1D<int> const &ReturnNode);

} // namespace ZonePlenum

struct ZonePlenumData : BaseGlobalStruct
{

    bool GetInputFlag = true;     // Flag set to make sure you get input once
    int NumZoneReturnPlenums = 0; // The Number of ZoneReturnPlenums found in the Input
    int NumZoneSupplyPlenums = 0; // The Number of ZoneSupplyPlenums found in the Input

    // these should be removed from state and just made individual flags on each plenum
    // however, this will require going into a few functions and changing them from looping over all plenums to only operating on the current plenum
    // this is the right step, but will possibly cause diffs, so pushing this for later.
    bool InitAirZoneReturnPlenumEnvrnFlag = true;
    bool InitAirZoneReturnPlenumOneTimeFlag = true;
    bool MyEnvrnFlag = true;

    EPVector<ZonePlenum::ZoneReturnPlenumConditions> ZoneRetPlenCond;
    EPVector<ZonePlenum::ZoneSupplyPlenumConditions> ZoneSupPlenCond;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) ZonePlenumData();
    }
};

} // namespace EnergyPlus

#endif
