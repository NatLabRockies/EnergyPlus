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

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/TARCOGCommon.hh>
#include <EnergyPlus/TARCOGParams.hh>

namespace EnergyPlus {

namespace TARCOGCommon {

    // MODULE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   June/22/2010
    //       MODIFIED       na
    //       RE-ENGINEERED  na
    //  Revision: 6.0.36  (June/22/2010)
    //   - Initial setup, extracted from TARCOG.for

    // PURPOSE OF THIS MODULE:
    // A module which contains common TARCOG functions and subroutines

    Real64 LDSumMax(Real64 const Width, Real64 const Height)
    {
        // LDSumMax function calculates sum part of equation for maximum deflection
        // Width - glazing system width
        // Height - glazing system height

        // Using/Aliasing
        using namespace TARCOGParams;

        // Return value
        Real64 LDSumMax;

        // Locals
        int i;
        int j;

        LDSumMax = 0.0;
        for (i = 1; i <= TARCOGParams::MMax; i += 2) {
            Real64 const sin_i(std::sin(i * Constant::PiOvr2));
            Real64 const pow_i_W(pow_2(i / Width));
            for (j = 1; j <= TARCOGParams::NMax; j += 2) {
                LDSumMax += (sin_i * std::sin(j * Constant::PiOvr2)) / (i * j * pow_2(pow_i_W + pow_2(j / Height)));
            } // do j = 1, DeflectionParameters::NMax, 2
        } // do i = 1, DeflectionParameters::MMax, 2

        return LDSumMax;
    }

    Real64 LDSumMean(Real64 const Width, Real64 const Height)
    {
        // LDSumMean function calculates sum part of equation for mean deflection
        // Width - glazing system width
        // Height - glazing system height

        // Using/Aliasing
        using namespace TARCOGParams;

        // Return value
        Real64 LDSumMean;

        // Locals
        Real64 constexpr Pi_squared(Constant::Pi * Constant::Pi);
        int i;
        int j;

        LDSumMean = 0.0;
        for (i = 1; i <= TARCOGParams::MMax; i += 2) {
            Real64 const pow_i_Pi_2(i * i * Pi_squared);
            Real64 const pow_i_W(pow_2(i / Width));
            for (j = 1; j <= TARCOGParams::NMax; j += 2) {
                LDSumMean += 4.0 / (pow_i_Pi_2 * pow_2(j) * pow_2(pow_i_W + pow_2(j / Height)));
            } // do j = 1, DeflectionParameters::NMax, 2
        } // do i = 1, MMax, 2

        return LDSumMean;
    }

    void modifyHcGap(Array1D<Real64> const &hcgap, // Convective coefficient for gap
                     Array1D<Real64> const &qv,    // Heat flow from ventilation [W/m2]
                     Array1D<Real64> const &hcv,   // Convective heat flow coefficient due to ventilation
                     Array1D<Real64> &hcgapMod,    // Modified heat flow coefficient for gap
                     int const nlayer,             // Number of layers
                     Real64 const edgeGlCorFac     // Edge of glass correction factor
    )
    {
        for (int i = 1; i <= nlayer + 1; ++i) {
            if (qv(i) != 0) {
                hcgapMod(i) = 0.5 * hcv(i);
            } else {
                hcgapMod(i) = hcgap(i) * edgeGlCorFac;
            }
        }
    }

    void matrixQBalance(int const nlayer,
                        Array2<Real64> &a,
                        Array1D<Real64> &b,
                        Array1D<Real64> const &sconScaled, // Solid layer coduction coefficient divided by thickness
                        Array1D<Real64> const &hcgas,
                        Array1D<Real64> &hcgapMod, // Modified heat flow coefficient for gap
                        Array1D<Real64> const &asol,
                        Array1D<Real64> const &qv,
                        Array1D<Real64> const &hcv, // Convective heat flow coefficient due to ventilation
                        Real64 const Tin,
                        Real64 const Tout,
                        Real64 const Gin,
                        Real64 const Gout,
                        Array1D<Real64> const &theta,
                        Array1D<Real64> const &tir,
                        Array1D<Real64> const &rir,
                        Array1D<Real64> const &emis,
                        Real64 const edgeGlCorrFac // Edge of glass correction factor
    )
    {

        // Using/Aliasing
        using namespace TARCOGParams;

        // Locals
        // local variables
        int i;
        int j;
        int k;
        int front;
        int back;
        int vent;

        for (i = 1; i <= 4 * nlayer; ++i) {
            b(i) = 0.0;
            for (j = 1; j <= 4 * nlayer; ++j) {
                a(j, i) = 0.0;
            }
        }

        modifyHcGap(hcgas, qv, hcv, hcgapMod, nlayer, edgeGlCorrFac);

        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //!!!!!!!!!!!!!!!!!!!!  build matrix a   !!!!!!!!!!!!!!!!!!!!!!!!!
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        for (i = 1; i <= nlayer; ++i) {
            k = 4 * i - 3;
            front = 2 * i - 1;
            back = 2 * i;

            // first row
            if (i != 1) {
                a(k - 1, k) = -hcgapMod(i);
                a(k - 2, k) = tir(front) - 1.0;
            }
            a(k, k) = hcgapMod(i) + sconScaled(i);
            a(k + 1, k) = 1.0;
            a(k + 3, k) = -sconScaled(i);
            if (i != nlayer) {
                a(k + 5, k) = -tir(back);
            }

            // second row
            a(k, k + 1) = emis(front) * Constant::StefanBoltzmann * pow_3(theta(front));
            a(k + 1, k + 1) = -1.0;
            if (i != 1) {
                a(k - 2, k + 1) = rir(front);
            }
            if (i != nlayer) {
                a(k + 5, k + 1) = tir(back);
            }

            // third row
            a(k + 2, k + 2) = -1.0;
            a(k + 3, k + 2) = emis(back) * Constant::StefanBoltzmann * pow_3(theta(back));
            if (i != 1) {
                a(k - 2, k + 2) = tir(front);
            }
            if (i != nlayer) {
                a(k + 5, k + 2) = rir(back);
            }

            // fourth row
            a(k, k + 3) = sconScaled(i);
            a(k + 2, k + 3) = -1.0;
            a(k + 3, k + 3) = -hcgapMod(i + 1) - sconScaled(i);
            if (i != 1) {
                a(k - 2, k + 3) = tir(front);
            }
            if (i != nlayer) {
                a(k + 4, k + 3) = hcgapMod(i + 1);
                a(k + 5, k + 3) = 1.0 - tir(back);
            }
        }

        // build matrix b

        for (i = 1; i <= nlayer; ++i) {
            k = 4 * i - 3;
            front = 2 * i - 1;
            back = 2 * i;
            vent = i + 1;

            b(k) = 0.5 * asol(i) + 0.5 * qv(vent - 1);
            b(k + 3) = -0.5 * asol(i) - 0.5 * qv(vent);

            if (i == 1) {
                b(k) = b(k) + hcgapMod(i) * Tout + Gout - tir(front) * Gout;
                b(k + 1) = b(k + 1) - rir(front) * Gout;
                b(k + 2) = b(k + 2) - tir(front) * Gout;
                b(k + 3) = b(k + 3) - tir(front) * Gout;
            }

            if (i == nlayer) {
                b(k) = b(k) + tir(back) * Gin;
                b(k + 1) = b(k + 1) - tir(back) * Gin;
                b(k + 2) = b(k + 2) - rir(back) * Gin;
                b(k + 3) = b(k + 3) - Gin - hcgapMod(i + 1) * Tin + tir(back) * Gin;
            }
        }
    }

    void EquationsSolver(EnergyPlusData &state, Array2<Real64> &a, Array1D<Real64> &b, int const n, int &nperr, std::string &ErrorMessage)
    {
        //***********************************************************************
        // Purpose: solves the main system of energy balance equations
        //***********************************************************************
        // Input:
        //   a - matrix, radiositied
        //   b - vector of known quantities
        //   n - ???
        // Output:
        //   b - solutions
        //   nperr - error code

        // Using/Aliasing
        using namespace TARCOGParams;

        // Locals
        Array1D_int indx(n);
        Real64 d;

        ludcmp(state, a, n, indx, d, nperr, ErrorMessage);

        // Exit on error
        if ((nperr > 0) && (nperr <= 1000)) {
            return;
        }

        lubksb(a, n, indx, b);
    }

    void ludcmp(EnergyPlusData &state, Array2<Real64> &a, int const n, Array1D_int &indx, Real64 &d, int &nperr, std::string &ErrorMessage)
    {

        // Locals
        Real64 constexpr TINY(1.0e-20);

        int i;
        int imax;
        int j;
        int k;
        Real64 aamax;
        Real64 dum;
        Real64 sum;

        d = 1.0;
        for (i = 1; i <= n; ++i) {
            aamax = 0.0;
            for (j = 1; j <= n; ++j) {
                if (std::abs(a(j, i)) > aamax) {
                    aamax = std::abs(a(j, i));
                }
            } // j
            if (aamax == 0.0) {
                nperr = 13;
                ErrorMessage = "Singular matrix in ludcmp.";
                return;
            }
            state.dataTARCOGCommon->vv(i) = 1.0 / aamax;
        } // i

        for (j = 1; j <= n; ++j) {
            for (i = 1; i <= j - 1; ++i) {
                sum = a(j, i);
                for (k = 1; k <= i - 1; ++k) {
                    sum -= a(k, i) * a(j, k);
                } // k
                a(j, i) = sum;
            } // i
            aamax = 0.0;
            for (i = j; i <= n; ++i) {
                sum = a(j, i);
                for (k = 1; k <= j - 1; ++k) {
                    sum -= a(k, i) * a(j, k);
                } // k
                a(j, i) = sum;
                dum = state.dataTARCOGCommon->vv(i) * std::abs(sum);
                if (dum >= aamax) {
                    imax = i;
                    aamax = dum;
                }
            } // i
            if (j != imax) {
                for (k = 1; k <= n; ++k) {
                    dum = a(k, imax);
                    a(k, imax) = a(k, j);
                    a(k, j) = dum;
                } // k
                d = -d;
                state.dataTARCOGCommon->vv(imax) = state.dataTARCOGCommon->vv(j);
            }
            indx(j) = imax;
            if (a(j, j) == 0.0) {
                a(j, j) = TINY;
            }
            if (j != n) {
                dum = 1.0 / a(j, j);
                for (i = j + 1; i <= n; ++i) {
                    a(j, i) *= dum;
                } // i
            }
        } // j
    }

    void lubksb(Array2A<Real64> const a, int const n, const Array1D_int &indx, Array1D<Real64> &b)
    {
        // Argument array dimensioning
        a.dim(n, n);
        EP_SIZE_CHECK(indx, n);
        EP_SIZE_CHECK(b, n);

        // Locals
        int i;
        int ii;
        int j;
        int ll;
        Real64 sum;

        ii = 0;
        for (i = 1; i <= n; ++i) {
            ll = indx(i);
            sum = b(ll);
            b(ll) = b(i);
            if (ii != 0) {
                for (j = ii; j <= i - 1; ++j) {
                    sum -= a(j, i) * b(j);
                } // j
            } else if (sum != 0.0) {
                ii = i;
            }
            b(i) = sum;
        } // i

        for (i = n; i >= 1; --i) {
            sum = b(i);
            for (j = i + 1; j <= n; ++j) {
                sum -= a(j, i) * b(j);
            } // j
            b(i) = sum / a(i, i);
        } // i
    }

    Real64 pos(Real64 const x)
    {
        return (x + std::abs(x)) / 2.0;
    }

} // namespace TARCOGCommon

} // namespace EnergyPlus
