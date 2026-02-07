// EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
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

#ifndef TarcogShading_hh_INCLUDED
#define TarcogShading_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array2A.hh>

// EnergyPlus Headers
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/TARCOGGassesParams.hh>
#include <EnergyPlus/TARCOGParams.hh>

namespace EnergyPlus {

// Using/Aliasing
using namespace TARCOGGassesParams;
using namespace TARCOGParams;

namespace TarcogShading {

    // Functions

    void shading(EnergyPlusData &state,
                 Array1D<Real64> const &theta,
                 Array1D<Real64> const &gap,
                 Array1D<Real64> &hgas,
                 Array1D<Real64> &hcgas,
                 Array1D<Real64> &hrgas,
                 Array2<Real64> const &frct,
                 Array2_int const &iprop,
                 Array1D<Real64> const &pressure,
                 Array1D_int const &nmix,
                 const Array1D<Real64> &xwght,
                 Array2<Real64> const &xgcon,
                 Array2<Real64> const &xgvis,
                 Array2<Real64> const &xgcp,
                 int const nlayer,
                 Real64 const width,
                 Real64 const height,
                 Real64 const angle,
                 Real64 const Tout,
                 Real64 const Tin,
                 Array1D<Real64> const &Atop,
                 Array1D<Real64> const &Abot,
                 Array1D<Real64> const &Al,
                 Array1D<Real64> const &Ar,
                 Array1D<Real64> const &Ah,
                 Array1D<Real64> const &vvent,
                 Array1D<Real64> const &tvent,
                 Array1D<TARCOGLayerType> LayerType,
                 Array1D<Real64> &Tgaps,
                 Array1D<Real64> &qv,
                 Array1D<Real64> &hcv,
                 int &nperr,
                 std::string &ErrorMessage,
                 Array1D<Real64> &vfreevent);

    void forcedventilation(EnergyPlusData &state,
                           const Array1D_int &iprop,
                           const Array1D<Real64> &frct,
                           Real64 const press,
                           int const nmix,
                           const Array1D<Real64> &xwght,
                           Array2A<Real64> const xgcon,
                           Array2A<Real64> const xgvis,
                           Array2A<Real64> const xgcp,
                           Real64 const s,
                           Real64 const H,
                           Real64 const hc,
                           Real64 const forcedspeed,
                           Real64 const Tinlet,
                           Real64 &Toutlet,
                           Real64 const Tav,
                           Real64 &hcv,
                           Real64 &qv,
                           int &nperr,
                           std::string &ErrorMessage);

    void shadingin(EnergyPlusData &state,
                   const Array1D_int &iprop1,
                   const Array1D<Real64> &frct1,
                   Real64 const press1,
                   int const nmix1,
                   const Array1D_int &iprop2,
                   const Array1D<Real64> &frct2,
                   Real64 const press2,
                   int const nmix2,
                   const Array1D<Real64> &xwght,
                   Array2A<Real64> const xgcon,
                   Array2A<Real64> const xgvis,
                   Array2A<Real64> const xgcp,
                   Real64 &Atop,
                   Real64 &Abot,
                   Real64 const Al,
                   Real64 const Ar,
                   Real64 const Ah,
                   Real64 const s1,
                   Real64 const s2,
                   Real64 const H,
                   Real64 const L,
                   Real64 const angle,
                   Real64 const hc1,
                   Real64 const hc2,
                   Real64 &speed1,
                   Real64 &speed2,
                   Real64 &Tgap1,
                   Real64 &Tgap2,
                   Real64 const Tav1,
                   Real64 const Tav2,
                   Real64 &hcv1,
                   Real64 &hcv2,
                   Real64 &qv1,
                   Real64 &qv2,
                   int &nperr,
                   std::string &ErrorMessage);

    void shadingedge(EnergyPlusData &state,
                     const Array1D_int &iprop1,
                     const Array1D<Real64> &frct1,
                     Real64 const press1,
                     int const nmix1,
                     const Array1D_int &iprop2,
                     const Array1D<Real64> &frct2,
                     Real64 const press2,
                     int const nmix2,
                     const Array1D<Real64> &xwght,
                     Array2A<Real64> const xgcon,
                     Array2A<Real64> const xgvis,
                     Array2A<Real64> const xgcp,
                     Real64 &Atop,
                     Real64 &Abot,
                     Real64 const Al,
                     Real64 const Ar,
                     Real64 &Ah,
                     Real64 const s,
                     Real64 const H,
                     Real64 const L,
                     Real64 const angle,
                     Real64 const forcedspeed,
                     Real64 const hc,
                     Real64 const Tenv,
                     Real64 const Tav,
                     Real64 &Tgap,
                     Real64 &hcv,
                     Real64 &qv,
                     int &nperr,
                     std::string &ErrorMessage,
                     Real64 &speed);

    void updateEffectiveMultipliers(int const nlayer,                          // Number of layers
                                    Real64 const width,                        // IGU width [m]
                                    Real64 const height,                       // IGU height [m]
                                    const Array1D<Real64> &Atop,               // Top opening area [m2]
                                    const Array1D<Real64> &Abot,               // Bottom opening area [m2]
                                    const Array1D<Real64> &Al,                 // Left side opening area [m2]
                                    const Array1D<Real64> &Ar,                 // Right side opening area [m2]
                                    const Array1D<Real64> &Ah,                 // Front side opening area [m2]
                                    Array1D<Real64> &Atop_eff,                 // Output - Effective top opening area [m2]
                                    Array1D<Real64> &Abot_eff,                 // Output - Effective bottom opening area [m2]
                                    Array1D<Real64> &Al_eff,                   // Output - Effective left side opening area [m2]
                                    Array1D<Real64> &Ar_eff,                   // Output - Effective right side opening area [m2]
                                    Array1D<Real64> &Ah_eff,                   // Output - Effective front side opening area [m2]
                                    const Array1D<TARCOGLayerType> &LayerType, // Layer type
                                    const Array1D<Real64> &SlatAngle           // Venetian layer slat angle [deg]
    );

} // namespace TarcogShading

struct TarcogShadingData : BaseGlobalStruct
{

    Array1D<Real64> frct1 = Array1D<Real64>(TARCOGGassesParams::maxgas);
    Array1D<Real64> frct2 = Array1D<Real64>(TARCOGGassesParams::maxgas);
    Array1D_int iprop1 = Array1D_int(TARCOGGassesParams::maxgas);
    Array1D_int iprop2 = Array1D_int(TARCOGGassesParams::maxgas);

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        frct1 = Array1D<Real64>(TARCOGGassesParams::maxgas);
        frct2 = Array1D<Real64>(TARCOGGassesParams::maxgas);
        iprop1 = Array1D_int(TARCOGGassesParams::maxgas);
        iprop2 = Array1D_int(TARCOGGassesParams::maxgas);
    }
};

} // namespace EnergyPlus

#endif
