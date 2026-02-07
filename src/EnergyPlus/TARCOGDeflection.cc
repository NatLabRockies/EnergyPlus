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
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/TARCOGCommon.hh>
#include <EnergyPlus/TARCOGDeflection.hh>
#include <EnergyPlus/TARCOGParams.hh>

namespace EnergyPlus::TARCOGDeflection {

// MODULE INFORMATION:
//       AUTHOR         Simon Vidanovic
//       DATE WRITTEN   October/22/2011
//       MODIFIED       na
//       RE-ENGINEERED  na
//  Revision: 7.0.02  (October 22, 2011)
//   - Initial setup

// PURPOSE OF THIS MODULE:
// A module which contains functions for deflection calculation

// Using/Aliasing
using namespace TARCOGParams;
using namespace TARCOGCommon;

void PanesDeflection(DeflectionCalculation const DeflectionStandard,
                     Real64 const W,
                     Real64 const H,
                     int const nlayer,
                     Real64 const Pa,
                     Real64 const Pini,
                     Real64 const Tini,
                     const Array1D<Real64> &PaneThickness,
                     const Array1D<Real64> &NonDeflectedGapWidth,
                     Array1D<Real64> &DeflectedGapWidthMax,
                     Array1D<Real64> &DeflectedGapWidthMean,
                     const Array1D<Real64> &PanelTemps,
                     const Array1D<Real64> &YoungsMod,
                     const Array1D<Real64> &PoissonsRat,
                     Array1D<Real64> &LayerDeflection,
                     int &nperr,
                     std::string &ErrorMessage)
{
    //***********************************************************************
    // PanesDeflection - calculates deflection of panes and recalculate gap
    //                   widths at maximal point of deflection
    //***********************************************************************

    // Argument array dimensioning
    EP_SIZE_CHECK(PaneThickness, maxlay);
    EP_SIZE_CHECK(NonDeflectedGapWidth, MaxGap);
    EP_SIZE_CHECK(DeflectedGapWidthMax, MaxGap);
    EP_SIZE_CHECK(DeflectedGapWidthMean, MaxGap);
    EP_SIZE_CHECK(PanelTemps, maxlay2);
    EP_SIZE_CHECK(YoungsMod, maxlay);
    EP_SIZE_CHECK(PoissonsRat, maxlay);
    EP_SIZE_CHECK(LayerDeflection, maxlay);

    // Locally used
    Array1D<Real64> DCoeff(maxlay);

    // first calculate D coefficients since that will be necessary for any of selected standards
    for (int i = 1; i <= nlayer; ++i) {
        DCoeff(i) = YoungsMod(i) * pow_3(PaneThickness(i)) / (12 * (1 - pow_2(PoissonsRat(i))));
    }
    if (DeflectionStandard == DeflectionCalculation::TEMPERATURE) {
        DeflectionTemperatures(nlayer,
                               W,
                               H,
                               Pa,
                               Pini,
                               Tini,
                               NonDeflectedGapWidth,
                               DeflectedGapWidthMax,
                               DeflectedGapWidthMean,
                               PanelTemps,
                               DCoeff,
                               LayerDeflection,
                               nperr,
                               ErrorMessage);
    } else if (DeflectionStandard == DeflectionCalculation::GAP_WIDTHS) {
        DeflectionWidths(nlayer, W, H, DCoeff, NonDeflectedGapWidth, DeflectedGapWidthMax, DeflectedGapWidthMean, LayerDeflection);
    } else { // including NO_DEFLECTION_CALCULATION
        return;
    }
}

void DeflectionTemperatures(int const nlayer,
                            Real64 const W,
                            Real64 const H,
                            Real64 const Pa,
                            Real64 const Pini,
                            Real64 const Tini,
                            const Array1D<Real64> &NonDeflectedGapWidth,
                            Array1D<Real64> &DeflectedGapWidthMax,
                            Array1D<Real64> &DeflectedGapWidthMean,
                            const Array1D<Real64> &PanelTemps,
                            Array1D<Real64> &DCoeff,
                            Array1D<Real64> &LayerDeflection,
                            int &nperr,
                            std::string &ErrorMessage)
{
    //***********************************************************************************************************
    // DeflectionTemperatures - calculates deflection of panes and recalculate gap
    //                          widths at maximal point of deflection based on gap pressures and temperatures
    //***********************************************************************************************************

    // Argument array dimensioning
    EP_SIZE_CHECK(NonDeflectedGapWidth, MaxGap);
    EP_SIZE_CHECK(DeflectedGapWidthMax, MaxGap);
    EP_SIZE_CHECK(DeflectedGapWidthMean, MaxGap);
    EP_SIZE_CHECK(PanelTemps, maxlay2);
    EP_SIZE_CHECK(DCoeff, maxlay);
    EP_SIZE_CHECK(LayerDeflection, maxlay);

    // Static constants
    static Real64 const Pi_6(pow_6(Constant::Pi));

    // locally used
    Array1D<Real64> DPressure(maxlay); // delta pressure at each glazing layer
    Array1D<Real64> Vini(MaxGap);
    Array1D<Real64> Vgap(MaxGap);
    Array1D<Real64> Pgap(MaxGap);
    Array1D<Real64> Tgap(MaxGap);
    Real64 MaxLDSum;
    Real64 MeanLDSum;
    Real64 Ratio;

    // calculate Vini for each gap
    for (int i = 1; i <= nlayer - 1; ++i) {
        Vini(i) = NonDeflectedGapWidth(i) * W * H;
    } // do i = 1, nlayer

    MaxLDSum = LDSumMax(W, H);
    MeanLDSum = LDSumMean(W, H);
    Ratio = MeanLDSum / MaxLDSum;

    // calculate Vgap for each gap
    Real64 const W_H_Ratio(W * H * Ratio);
    for (int i = 1; i <= nlayer - 1; ++i) {
        Vgap(i) = Vini(i) + W_H_Ratio * (LayerDeflection(i) - LayerDeflection(i + 1));
    } // do i = 1, nlayer

    // calculate Tgap for each gap
    for (int i = 1; i <= nlayer - 1; ++i) {
        int const j = 2 * i;
        Tgap(i) = (PanelTemps(j) + PanelTemps(j + 1)) / 2;
    } // do i = 1, nlayer

    for (int i = 1; i <= nlayer - 1; ++i) {
        Pgap(i) = Pini * Vini(i) * Tgap(i) / (Tini * Vgap(i));
    } // do i = 1, nlayer

    DPressure(1) = Pgap(1) - Pa;
    if (nlayer > 1) {
        DPressure(nlayer) = Pa - Pgap(nlayer - 1);
    }

    for (int i = 2; i <= nlayer - 1; ++i) {
        DPressure(i) = Pgap(i) - Pgap(i - 1);
    } // do i = 1, nlayer

    Real64 const deflection_fac(DeflectionRelaxation * MaxLDSum * 16);
    for (int i = 1; i <= nlayer; ++i) {
        LayerDeflection(i) += deflection_fac * DPressure(i) / (Pi_6 * DCoeff(i));
    }

    for (int i = 1; i <= nlayer - 1; ++i) {
        DeflectedGapWidthMax(i) = NonDeflectedGapWidth(i) + LayerDeflection(i) - LayerDeflection(i + 1);
        if (DeflectedGapWidthMax(i) < 0.0) {
            nperr = 2001; // glazing panes collapsed
            ErrorMessage = "Glazing panes collapsed";
        }
    }

    for (int i = 1; i <= nlayer - 1; ++i) {
        DeflectedGapWidthMean(i) = NonDeflectedGapWidth(i) + Ratio * (DeflectedGapWidthMax(i) - NonDeflectedGapWidth(i));
    }
}

void DeflectionWidths(int const nlayer,
                      Real64 const W,
                      Real64 const H,
                      Array1D<Real64> &DCoeff,
                      const Array1D<Real64> &NonDeflectedGapWidth,
                      const Array1D<Real64> &DeflectedGapWidthMax,
                      Array1D<Real64> &DeflectedGapWidthMean,
                      Array1D<Real64> &LayerDeflection)
{
    // Argument array dimensioning
    EP_SIZE_CHECK(DCoeff, maxlay);
    EP_SIZE_CHECK(NonDeflectedGapWidth, MaxGap);
    EP_SIZE_CHECK(DeflectedGapWidthMax, MaxGap);
    EP_SIZE_CHECK(DeflectedGapWidthMean, MaxGap);
    EP_SIZE_CHECK(LayerDeflection, maxlay);

    Real64 nominator = 0.0;
    for (int i = 1; i <= nlayer - 1; ++i) {
        Real64 SumL = 0.0;
        for (int j = i; j <= nlayer - 1; ++j) {
            SumL += NonDeflectedGapWidth(j) - DeflectedGapWidthMax(j);
        }
        nominator += SumL * DCoeff(i);
    }

    Real64 denominator = 0.0;
    for (int i = 1; i <= nlayer; ++i) {
        denominator += DCoeff(i);
    }

    LayerDeflection(nlayer) = nominator / denominator;

    for (int i = nlayer - 1; i >= 1; --i) {
        LayerDeflection(i) = DeflectedGapWidthMax(i) - NonDeflectedGapWidth(i) + LayerDeflection(i + 1);
    }

    Real64 MaxLDSum = LDSumMax(W, H);
    Real64 MeanLDSum = LDSumMean(W, H);
    Real64 Ratio = MeanLDSum / MaxLDSum;

    for (int i = 1; i <= nlayer - 1; ++i) {
        DeflectedGapWidthMean(i) = NonDeflectedGapWidth(i) + Ratio * (DeflectedGapWidthMax(i) - NonDeflectedGapWidth(i));
    }
}

} // namespace EnergyPlus::TARCOGDeflection
