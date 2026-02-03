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

#ifndef DaylightingDevices_hh_INCLUDED
#define DaylightingDevices_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/DataDaylightingDevices.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Dayltg {

    void InitDaylightingDevices(EnergyPlusData &state);

    void GetTDDInput(EnergyPlusData &state);

    void GetShelfInput(EnergyPlusData &state);

    Real64 CalcPipeTransBeam(Real64 const R,    // Reflectance of surface, constant (can be made R = f(theta) later)
                             Real64 const A,    // Aspect ratio, L / d
                             Real64 const Theta // Angle of entry in radians
    );

    Real64 CalcTDDTransSolIso(EnergyPlusData &state, int const PipeNum); // TDD pipe object number

    Real64 CalcTDDTransSolHorizon(EnergyPlusData &state, int const PipeNum); // TDD pipe object number

    Real64 CalcTDDTransSolAniso(EnergyPlusData &state,
                                int const PipeNum, // TDD pipe object number
                                Real64 const COSI  // Cosine of the incident angle
    );

    Real64 TransTDD(EnergyPlusData &state,
                    int const PipeNum,                  // TDD pipe object number
                    Real64 const COSI,                  // Cosine of the incident angle
                    Dayltg::RadType const RadiationType // Radiation type flag
    );

    Real64 InterpolatePipeTransBeam(EnergyPlusData &state,
                                    Real64 const COSI,               // Cosine of the incident angle
                                    const Array1D<Real64> &transBeam // Table of beam transmittance vs. cosine angle
    );

    int FindTDDPipe(EnergyPlusData &state, int const WinNum);

    void DistributeTDDAbsorbedSolar(EnergyPlusData &state);

    void CalcViewFactorToShelf(EnergyPlusData &state, int const ShelfNum); // Daylighting shelf object number

    void adjustViewFactorsWithShelf(
        EnergyPlusData &state, Real64 &viewFactorToShelf, Real64 &viewFactorToSky, Real64 &viewFactorToGround, int WinSurf, int ShelfNum);

    void FigureTDDZoneGains(EnergyPlusData &state);

} // namespace Dayltg

struct DaylightingDevicesData : BaseGlobalStruct
{

    Array1D<Real64> COSAngle = Array1D<Real64>(Dayltg::NumOfAngles); // List of cosines of incident angle
    bool ShelfReported = false;
    bool GetTDDInputErrorsFound = false;   // Set to true if errors in input, fatal at end of routine
    bool GetShelfInputErrorsFound = false; // Set to true if errors in input, fatal at end of routine
    bool MyEnvrnFlag = true;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->COSAngle = Array1D<Real64>(Dayltg::NumOfAngles);
        this->ShelfReported = false;
        this->GetTDDInputErrorsFound = false;
        this->GetShelfInputErrorsFound = false;
        this->MyEnvrnFlag = true;
    }
};

} // namespace EnergyPlus

#endif
