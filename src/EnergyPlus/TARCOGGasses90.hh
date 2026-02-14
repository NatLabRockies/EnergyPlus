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

#ifndef TARCOGGasses90_hh_INCLUDED
#define TARCOGGasses90_hh_INCLUDED

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

namespace TARCOGGasses90 {

    void GASSES90(EnergyPlusData &state,
                  Real64 tmean,
                  const Array1D_int &iprop,
                  const Array1D<Real64> &frct,
                  Real64 pres,
                  int nmix,
                  const Array1D<Real64> &xwght,
                  Array2<Real64> const &xgcon,
                  Array2<Real64> const &xgvis,
                  Array2<Real64> const &xgcp,
                  Real64 &con,
                  Real64 &visc,
                  Real64 &dens,
                  Real64 &cp,
                  Real64 &pr,
                  TARCOGGassesParams::Stdrd standard,
                  int &nperr,
                  std::string &ErrorMessage);

    void GassesLow(Real64 tmean, Real64 mwght, Real64 pressure, Real64 gama, Real64 &cond, int &nperr, std::string &ErrorMessage);
} // namespace TARCOGGasses90
struct TARCOGGasses90Data : BaseGlobalStruct
{

    Array1D<Real64> fvis = Array1D<Real64>(TARCOGGassesParams::maxgas);
    Array1D<Real64> fcon = Array1D<Real64>(TARCOGGassesParams::maxgas);
    Array1D<Real64> fdens = Array1D<Real64>(TARCOGGassesParams::maxgas);
    Array1D<Real64> fcp = Array1D<Real64>(TARCOGGassesParams::maxgas);
    Array1D<Real64> kprime = Array1D<Real64>(TARCOGGassesParams::maxgas);
    Array1D<Real64> kdblprm = Array1D<Real64>(TARCOGGassesParams::maxgas);
    Array1D<Real64> mukpdwn = Array1D<Real64>(TARCOGGassesParams::maxgas);
    Array1D<Real64> kpdown = Array1D<Real64>(TARCOGGassesParams::maxgas);
    Array1D<Real64> kdpdown = Array1D<Real64>(TARCOGGassesParams::maxgas);

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        fvis = Array1D<Real64>(TARCOGGassesParams::maxgas);
        fcon = Array1D<Real64>(TARCOGGassesParams::maxgas);
        fdens = Array1D<Real64>(TARCOGGassesParams::maxgas);
        fcp = Array1D<Real64>(TARCOGGassesParams::maxgas);
        kprime = Array1D<Real64>(TARCOGGassesParams::maxgas);
        kdblprm = Array1D<Real64>(TARCOGGassesParams::maxgas);
        mukpdwn = Array1D<Real64>(TARCOGGassesParams::maxgas);
        kpdown = Array1D<Real64>(TARCOGGassesParams::maxgas);
        kdpdown = Array1D<Real64>(TARCOGGassesParams::maxgas);
    }
};

} // namespace EnergyPlus

#endif
