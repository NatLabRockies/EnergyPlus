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

#ifndef ThermalEN673Calc_hh_INCLUDED
#define ThermalEN673Calc_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array2A.hh>

// EnergyPlus Headers
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace ThermalEN673Calc {

    // Functions

    void Calc_EN673(EnergyPlusData &state,
                    TARCOGOutput::Files &files,
                    TARCOGGassesParams::Stdrd const standard,
                    int const nlayer,
                    Real64 const tout,
                    Real64 const tind,
                    Array1D<Real64> &gap,
                    Array1D<Real64> &thick,
                    Array1D<Real64> &scon,
                    const Array1D<Real64> &emis,
                    Real64 const totsol,
                    Real64 const tilt,
                    Real64 const dir,
                    const Array1D<Real64> &asol,
                    const Array1D<Real64> &presure,
                    Array2A_int const iprop,
                    Array2A<Real64> const frct,
                    const Array1D_int &nmix,
                    Array2A<Real64> const xgcon,
                    Array2A<Real64> const xgvis,
                    Array2A<Real64> const xgcp,
                    const Array1D<Real64> &xwght,
                    Array1D<Real64> &theta,
                    Real64 &ufactor,
                    Real64 &hcin,
                    Real64 &hin,
                    Real64 &hout,
                    Real64 &shgc,
                    int &nperr,
                    std::string &ErrorMessage,
                    const Array1D_int &ibc,
                    Array1D<Real64> &hg,
                    Array1D<Real64> &hr,
                    Array1D<Real64> &hs,
                    Array1D<Real64> &Ra,
                    Array1D<Real64> &Nu);

    void EN673ISO10292(EnergyPlusData &state,
                       int const nlayer,
                       Real64 const tout,
                       Real64 const tind,
                       const Array1D<Real64> &emis,
                       const Array1D<Real64> &gap,
                       const Array1D<Real64> &thick,
                       const Array1D<Real64> &scon,
                       Real64 const tilt,
                       Array2A_int const iprop,
                       Array2A<Real64> const frct,
                       Array2A<Real64> const xgcon,
                       Array2A<Real64> const xgvis,
                       Array2A<Real64> const xgcp,
                       const Array1D<Real64> &xwght,
                       const Array1D<Real64> &presure,
                       const Array1D_int &nmix,
                       Array1D<Real64> &theta,
                       TARCOGGassesParams::Stdrd const standard,
                       Array1D<Real64> &hg,
                       Array1D<Real64> &hr,
                       Array1D<Real64> &hs,
                       Real64 &hin,
                       Real64 const hout,
                       Real64 &hcin,
                       const Array1D_int &ibc,
                       Array1D<Real64> &rs,
                       Real64 &ufactor,
                       Array1D<Real64> &Ra,
                       Array1D<Real64> &Nu,
                       int &nperr,
                       std::string &ErrorMessage);

    void linint(Real64 const x1, Real64 const x2, Real64 const y1, Real64 const y2, Real64 const x, Real64 &y);

    void solar_EN673(Real64 const dir,
                     Real64 const totsol,
                     Real64 const rtot,
                     const Array1D<Real64> &rs,
                     int const nlayer,
                     const Array1D<Real64> &absol,
                     Real64 &sf,
                     TARCOGGassesParams::Stdrd const standard,
                     int &nperr,
                     std::string &ErrorMessage);

} // namespace ThermalEN673Calc

} // namespace EnergyPlus

#endif
