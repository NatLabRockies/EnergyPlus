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

// C++ Headers
#include <cassert>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/TARCOGGasses90.hh>
#include <EnergyPlus/TARCOGGassesParams.hh>

namespace EnergyPlus::TARCOGGasses90 {

// MODULE INFORMATION:
//       AUTHOR         D. Charlie Curcija
//       DATE WRITTEN   June/2000
//       MODIFIED       (see revision history bellow)
//       RE-ENGINEERED  na
//  Revision: 7.0.02  (November/8/2011), Simon Vidanovic
//   - feature: Error message (string) return from gasses
//  Revision: 7.0.00  (September/6/2011), Simon Vidanovic
//   - Introduction of vacuum coefficients and routine to calculate low gas pressure conductance
//  Revision: 6.3.09  (August/23/2011), Simon Vidanovic
//   - Removed GetGasIndex function which could cause a double usage of gas coefficients
//     and therefore introducing new bugs.

// PURPOSE OF THIS MODULE:
// A module containing functions for gas properties calculation

// Using/Aliasing
using namespace TARCOGGassesParams;

void GASSES90(EnergyPlusData &state,
              Real64 const tmean,
              const Array1D_int &iprop,
              const Array1D<Real64> &frct,
              Real64 const pres,
              int const nmix,
              const Array1D<Real64> &xwght,
              Array2<Real64> const &xgcon,
              Array2<Real64> const &xgvis,
              Array2<Real64> const &xgcp,
              Real64 &con,
              Real64 &visc,
              Real64 &dens,
              Real64 &cp,
              Real64 &pr,
              TARCOGGassesParams::Stdrd const standard,
              int &nperr,
              std::string &ErrorMessage)
{

    static Real64 const two_sqrt_2(2.0 * std::sqrt(2.0));
    Real64 molmix;
    Real64 cpmixm;
    Real64 phimup;
    Real64 downer;
    Real64 psiup;
    Real64 psiterm;
    Real64 phikup;

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr ENpressure(1.0e5); // Gap gas pressure (Pa)
    Real64 constexpr gaslaw(8314.51);   // Molar gas constant (J/kMol-K)

    //!! Body of GASSES90

    // Autodesk:Uninit Initialize variables used uninitialized
    // xgrho = 0.0d0 //Autodesk:Uninit Force default initialization

    Real64 const tmean_2(pow_2(tmean));
    state.dataTARCOGGasses90->fcon(1) = xgcon(1, iprop(1)) + xgcon(2, iprop(1)) * tmean + xgcon(3, iprop(1)) * tmean_2;
    state.dataTARCOGGasses90->fvis(1) = xgvis(1, iprop(1)) + xgvis(2, iprop(1)) * tmean + xgvis(3, iprop(1)) * tmean_2;
    state.dataTARCOGGasses90->fcp(1) = xgcp(1, iprop(1)) + xgcp(2, iprop(1)) * tmean + xgcp(3, iprop(1)) * tmean_2;
    // Density using ideal gas law: rho=(presure*mol. weight)/(gas const*Tmean)
    state.dataTARCOGGasses90->fdens(1) = pres * xwght(iprop(1)) / (Constant::UniversalGasConst * tmean);
    // Mollecular weights in kg/kmol
    if ((standard == TARCOGGassesParams::Stdrd::EN673) || (standard == TARCOGGassesParams::Stdrd::EN673Design)) {
        // fdens( 1 ) = xgrho( iprop( 1 ), 1 ) + xgrho( iprop( 1 ), 2 ) * tmean + xgrho( iprop( 1 ), 3 ) * pow_2( tmean ); //Autodesk:Uninit xgrho
        // was uninitialized
        state.dataTARCOGGasses90->fdens(1) = ENpressure * xwght(iprop(1)) / (gaslaw * tmean);
    }

    if (frct(1) == 1.0) {                          // Single gas properties
        visc = state.dataTARCOGGasses90->fvis(1);  // viscosity in kg/(m*s)
        con = state.dataTARCOGGasses90->fcon(1);   // conductivity in W/(m*K)
        cp = state.dataTARCOGGasses90->fcp(1);     // SpecIFic heat in J/(kg*K)
        dens = state.dataTARCOGGasses90->fdens(1); // density in kg/m^3
    } else {                                       // Mixture properties
        bool const stdISO15099(standard == TARCOGGassesParams::Stdrd::ISO15099);
        bool const stdEN673((standard == TARCOGGassesParams::Stdrd::EN673) || (standard == TARCOGGassesParams::Stdrd::EN673Design));
        if (stdISO15099) {
            molmix = frct(1) * xwght(iprop(1));                 // initialize equation 56
            cpmixm = molmix * state.dataTARCOGGasses90->fcp(1); // initialize equation 58
            state.dataTARCOGGasses90->kprime(1) =
                3.75 * Constant::UniversalGasConst / xwght(iprop(1)) * state.dataTARCOGGasses90->fvis(1);                   // equation 67
            state.dataTARCOGGasses90->kdblprm(1) = state.dataTARCOGGasses90->fcon(1) - state.dataTARCOGGasses90->kprime(1); // equation 67
            // initialize sumations for eqns 60-66:
            state.dataTARCOGGasses90->mukpdwn(1) = 1.0;
            state.dataTARCOGGasses90->kpdown(1) = 1.0;
            state.dataTARCOGGasses90->kdpdown(1) = 1.0;
        }
        for (int i = 2; i <= nmix; ++i) {
            if (frct(i) == 0.0) {
                nperr = 2011; // error 2011: component fraction in a mixture is 0%
                ErrorMessage = "Component fraction in mixture is 0%";
                return;
            }
            // calculate properties of mixture constituents:
            state.dataTARCOGGasses90->fcon(i) = xgcon(1, iprop(i)) + xgcon(2, iprop(i)) * tmean + xgcon(3, iprop(i)) * tmean_2;
            state.dataTARCOGGasses90->fvis(i) = xgvis(1, iprop(i)) + xgvis(2, iprop(i)) * tmean + xgvis(3, iprop(i)) * tmean_2;
            state.dataTARCOGGasses90->fcp(i) = xgcp(1, iprop(i)) + xgcp(2, iprop(i)) * tmean + xgcp(3, iprop(i)) * tmean_2;
            if (stdEN673) {
                // fdens( i ) = grho( iprop( i ), 1 ) + grho( iprop( i ), 2 ) * tmean + grho( iprop( i ), 3 ) * pow_2( tmean );
                state.dataTARCOGGasses90->fdens(i) =
                    ENpressure * xwght(iprop(i)) / (gaslaw * tmean); // Density using ideal gas law: rho=(presure*mol. weight)/(gas const*Tmean)
            }
            if (stdISO15099) {
                molmix += frct(i) * xwght(iprop(i));                                    // equation 56
                cpmixm += frct(i) * state.dataTARCOGGasses90->fcp(i) * xwght(iprop(i)); // equation 58-59
                state.dataTARCOGGasses90->kprime(i) =
                    3.75 * Constant::UniversalGasConst / xwght(iprop(i)) * state.dataTARCOGGasses90->fvis(i);                   // equation 67
                state.dataTARCOGGasses90->kdblprm(i) = state.dataTARCOGGasses90->fcon(i) - state.dataTARCOGGasses90->kprime(i); // equation 68
                state.dataTARCOGGasses90->mukpdwn(i) = 1.0; // initialize denominator of equation 60
                state.dataTARCOGGasses90->kpdown(i) = 1.0;  // initialize denominator of equation 63
                state.dataTARCOGGasses90->kdpdown(i) = 1.0; // initialize denominator of equation 65
            }
        }

        if (stdISO15099) {
            Real64 mumix(0.0);
            Real64 kpmix(0.0);
            Real64 kdpmix(0.0);
            for (int i = 1; i <= nmix; ++i) {
                Real64 const kprime_i(state.dataTARCOGGasses90->kprime(i));
                Real64 const xwght_i(xwght(iprop(i)));
                for (int j = 1; j <= nmix; ++j) {
                    Real64 const xwght_j(xwght(iprop(j)));

                    // numerator of equation 61
                    Real64 const x_pow(root_4(xwght_j / xwght_i));
                    phimup = pow_2(1.0 + std::sqrt(state.dataTARCOGGasses90->fvis(i) / state.dataTARCOGGasses90->fvis(j)) * x_pow);

                    // denominator of equation 61, 64 and 66
                    downer = two_sqrt_2 * std::sqrt(1.0 + (xwght_i / xwght_j));

                    // calculate the denominator of equation 60
                    if (i != j) {
                        state.dataTARCOGGasses90->mukpdwn(i) += phimup / downer * frct(j) / frct(i);
                    }

                    // numerator of equation 64, psiterm is the multiplied term in brackets
                    psiup = pow_2(1.0 + std::sqrt(kprime_i / state.dataTARCOGGasses90->kprime(j)) / x_pow);

                    psiterm = 1.0 + 2.41 * (xwght_i - xwght_j) * (xwght_i - 0.142 * xwght_j) / pow_2(xwght_i + xwght_j);

                    // using the common denominator downer calculate the denominator for equation 63
                    if (i != j) {
                        state.dataTARCOGGasses90->kpdown(i) += psiup * psiterm / downer * frct(j) / frct(i);
                    }

                    // calculate the numerator of equation 66
                    phikup = psiup; // Tuned Was pow_2( 1.0 + std::sqrt( kprime_i / kprime( j ) ) * std::pow( xwght_i / xwght_j, 0.25 ) );

                    // using the common denominator downer calculate the denominator for equation 65
                    if (i != j) {
                        state.dataTARCOGGasses90->kdpdown(i) += phikup / downer * frct(j) / frct(i);
                    }
                }
                mumix += state.dataTARCOGGasses90->fvis(i) / state.dataTARCOGGasses90->mukpdwn(i);     // equation 60
                kpmix += state.dataTARCOGGasses90->kprime(i) / state.dataTARCOGGasses90->kpdown(i);    // equation 63
                kdpmix += state.dataTARCOGGasses90->kdblprm(i) / state.dataTARCOGGasses90->kdpdown(i); // equation 65
            }

            // calculate the density of the mixture assuming an ideal gas:
            Real64 const rhomix = pres * molmix / (Constant::UniversalGasConst * tmean); // equation 57
            Real64 const kmix = kpmix + kdpmix;                                          // equation 68-a

            // final mixture properties:
            visc = mumix;
            con = kmix;
            dens = rhomix;
            if (molmix > 0) {
                cp = cpmixm / molmix;
            } else {
                cp = 0;
            }
        } else if (stdEN673) {
            con = 0.0;
            visc = 0.0;
            dens = 0.0;
            cp = 0.0;
            for (int i = 1; i <= nmix; ++i) {
                Real64 const frct_i(frct(i));
                con += state.dataTARCOGGasses90->fcon(i) * frct_i;
                visc += state.dataTARCOGGasses90->fvis(i) * frct_i;
                dens += state.dataTARCOGGasses90->fdens(i) * frct_i;
                cp += state.dataTARCOGGasses90->fcp(i) * frct_i;
            }
        } else {
            assert(false); // should never come here - unsupported standard
        }
    }

    pr = cp * visc / con; // calculate the Prandtl number
}

void GassesLow(Real64 const tmean, Real64 const mwght, Real64 const pressure, Real64 const gama, Real64 &cond, int &nperr, std::string &ErrorMessage)
{

    Real64 const alpha = alpha1 * alpha2 / (alpha2 + alpha1 * (1 - alpha2));

    if ((gama) == 1) {
        nperr = 40; // supplied gamma coefficient is incorrect
        ErrorMessage = "Supplied gamma coefficient is incorrect.";
        return;
    }

    Real64 const B = alpha * (gama + 1) / (gama - 1) * std::sqrt(Constant::UniversalGasConst / (8 * Constant::Pi * mwght * tmean));

    cond = B * pressure;
}

} // namespace EnergyPlus::TARCOGGasses90
