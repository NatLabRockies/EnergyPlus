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

#ifndef IndoorGreen_hh_INCLUDED
#define IndoorGreen_hh_INCLUDED
// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

struct EnergyPlusData;

namespace IndoorGreen {

    enum class ETCalculationMethod
    {
        Invalid = -1,
        PenmanMonteith,
        Stanghellini,
        Num
    };
    enum class LightingMethod
    {
        Invalid = -1,
        LED,
        Daylighting,
        LEDDaylighting,
        Num
    };
    struct IndoorGreenParams
    {
        std::string Name;
        std::string ZoneName;
        std::string SurfName;
        Sched::Schedule *sched = nullptr;
        Sched::Schedule *ledSched = nullptr;
        int LightRefPtr = 0;                               // daylight reference point number;
        int LightControlPtr = 0;                           // daylight control point number;
        Sched::Schedule *ledDaylightTargetSched = nullptr; // LED-Daylight PPFD setpoint schedule
        Real64 LeafArea = 0.0;                             // one-sided leaf area
        Real64 LEDNominalPPFD = 0.0;                       // nominal PPFD for LED grow light (PPFD)
        Real64 LEDNominalEleP = 0.0;                       // nominal power for total LED grow light (W)
        Real64 LEDRadFraction = 0.0;                       // radiant fraction of LED grow light (0-1)
        Real64 ZCO2 = 400;
        Real64 ZVPD = 0.0;            // vapor pressure deficit (pa)
        Real64 ZPPFD = 0;             // PPFD
        Real64 SensibleRate = 0.0;    // w, living wall
        Real64 SensibleRateLED = 0.0; // w, lED
        Real64 LatentRate = 0.0;      // w
        Real64 ETRate = 0.0;          // kg/(m2s)
        Real64 LambdaET = 0.0;        // J/(kg m2)
        Real64 LEDActualPPFD = 0.0;   // LED operational PPFD
        Real64 LEDActualEleP = 0.0;   // LED operational power (W)
        Real64 LEDActualEleCon = 0.0; // LED operational electricity consumption (J)
        int SurfPtr = 0;
        int ZoneListPtr = 0;
        int ZonePtr = 0;                         // point to the zone where the indoor greenery system is located
        int SpacePtr = 0;                        // point to the space where the indoor greenery system is located
        ETCalculationMethod etCalculationMethod; // Enum for ETCalculationMethod
        LightingMethod lightingMethod;           // Enum for LightingMethod
        bool CheckIndoorGreenName = true;
        Real64 EMSET = 0.0;
        bool EMSETCalOverrideOn = false;
        Array1D_string FieldNames;
    };

    void SimIndoorGreen(EnergyPlusData &state);
    void GetIndoorGreenInput(EnergyPlusData &state, bool &ErrorsFound);
    void SetIndoorGreenOutput(EnergyPlusData &state);
    void InitIndoorGreen(EnergyPlusData const &state);
    void ETModel(EnergyPlusData &state);
    Real64 ETBaseFunction(EnergyPlusData &state, Real64 ZonePreTemp, Real64 ZonePreHum, Real64 ZonePPFD, Real64 VPD, Real64 LAI, Real64 SwitchF);

} // namespace IndoorGreen

struct IndoorGreenData : BaseGlobalStruct
{
    int NumIndoorGreen = 0; // Number of Indoor Greenery Systems found in input
    bool getInputFlag = true;
    Array1D<IndoorGreen::IndoorGreenParams> indoorGreens;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) IndoorGreenData();
    }
};

} // namespace EnergyPlus

#endif
