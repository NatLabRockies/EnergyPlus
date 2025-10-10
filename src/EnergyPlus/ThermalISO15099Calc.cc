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

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/TARCOGArgs.hh>
#include <EnergyPlus/TARCOGCommon.hh>
#include <EnergyPlus/TARCOGGasses90.hh>
#include <EnergyPlus/TARCOGGassesParams.hh>
#include <EnergyPlus/TARCOGParams.hh>
#include <EnergyPlus/TarcogShading.hh>
#include <EnergyPlus/ThermalISO15099Calc.hh>

namespace EnergyPlus::ThermalISO15099Calc {
//***********************************************************************
// ThermalISO15099Calc: a TARCOG module
//    module For Calculation of Thermal Performance Indices For Center
//     of Glass According to ISO 15099
// History of Revisions:
//  Revision: 6.0.36  (June/22/2010)
//   - Initial setup, extracted and refactored from TARCOG.for
//***********************************************************************

// MODULE INFORMATION:
//       AUTHOR         D. Charlie Curcija
//       DATE WRITTEN   July/2000
//       MODIFIED       na
//       RE-ENGINEERED  March/27/2012, Simon Vidanovic

//  Revision: 7.0.13  (March/27/2012), Simon Vidanovic
//   - feature: New set of equations is set instead of hhat coefficients and new approach to solution which improves
//               speed and stability.  Note that this solution does not include laminates

// PURPOSE OF THIS MODULE:
// Module For Calculation of Thermal Performance Indices For Center
//  of Glass According to ISO 15099

// Using/Aliasing
using namespace TARCOGGassesParams;
using namespace TARCOGParams;
using namespace TARCOGArgs;
using namespace TARCOGCommon;
using namespace TARCOGOutput;
using namespace TARCOGGasses90;
using namespace TarcogShading;

//  Calculation outcome
enum class CalculationOutcome
{
    Invalid = -1,
    OK,
    Num
};

void film(Real64 const tex, Real64 const tw, Real64 const ws, int const iwd, Real64 &hcout, int const ibc)
{
    //***********************************************************************
    // purpose - to find outdoor film coeff
    //***********************************************************************
    // Inputs -
    //   tex - outdoor air temp [k]
    //   tw - outside surface temp
    //   ws - wind speed [m/s]
    //   iwd - wind direction [0 - windward; 1 - leeward]
    // Outputs
    //   hcout - convective film coeff [w m-2 k-1]

    // Locals
    Real64 constexpr conv(5.6783);

    Real64 vc;
    Real64 acoef;
    Real64 bexp;

    // calculation of convection component of exterior film coefficient using the :
    switch (ibc) {
    case 0: { // ISO 15099
        hcout = 4.0 + 4.0 * ws;
    } break;
    case -1: {          // old ASHRAE SPC142 correlation
        if (iwd == 0) { // windward
            if (ws > 2.0) {
                vc = 0.25 * ws;
            } else {
                vc = 0.5;
            }
        } else { // leeward
            vc = 0.3 + 0.05 * ws;
        }
        hcout = 3.28 * std::pow(vc, 0.605);
        hcout *= conv; // convert to metric
    } break;
    case -2: {          // Yazdanian-Klems correlation:
        if (iwd == 0) { // windward
            acoef = 2.38;
            bexp = 0.89;
        } else { // leeward
            acoef = 2.86;
            bexp = 0.617;
        }
        hcout = std::sqrt(pow_2(0.84 * std::pow(tw - tex, 0.33)) + pow_2(acoef * std::pow(ws, bexp)));
    } break;
    case -3: {          // Kimura correlation (Section 8.4.2.3 in ISO 15099-2001):
        if (iwd == 0) { // windward
            if (ws > 2.0) {
                vc = 0.25 * ws;
            } else {
                vc = 0.5 * ws;
            }
        } else { // leeward
            vc = 0.3 + 0.05 * ws;
        }
        hcout = 4.7 + 7.6 * vc;
    } break;
    default:
        break;
    }
}

void Calc_ISO15099(EnergyPlusData &state,
                   Files &files,
                   int const nlayer,
                   int const iwd,
                   Real64 &tout,
                   Real64 &tind,
                   Real64 &trmin,
                   Real64 const wso,
                   Real64 const wsi,
                   Real64 const dir,
                   Real64 const outir,
                   int const isky,
                   Real64 const tsky,
                   Real64 &esky,
                   Real64 const fclr,
                   Real64 const VacuumPressure,
                   Real64 const VacuumMaxGapThickness,
                   Array1D<Real64> &gap,
                   Array1D<Real64> &thick,
                   Array1D<Real64> &scon,
                   const Array1D<Real64> &tir,
                   const Array1D<Real64> &emis,
                   Real64 const totsol,
                   Real64 const tilt,
                   const Array1D<Real64> &asol,
                   Real64 const height,
                   Real64 const heightt,
                   Real64 const width,
                   const Array1D<Real64> &presure,
                   Array2A_int const iprop,
                   Array2A<Real64> const frct,
                   Array2A<Real64> const xgcon,
                   Array2A<Real64> const xgvis,
                   Array2A<Real64> const xgcp,
                   const Array1D<Real64> &xwght,
                   const Array1D<Real64> &gama,
                   const Array1D_int &nmix,
                   const Array1D_int &SupportPillar,
                   const Array1D<Real64> &PillarSpacing,
                   const Array1D<Real64> &PillarRadius,
                   Array1D<Real64> &theta,
                   Array1D<Real64> &q,
                   Array1D<Real64> &qv,
                   Real64 &ufactor,
                   Real64 &sc,
                   Real64 &hflux,
                   Real64 &hcin,
                   Real64 &hcout,
                   Real64 &hrin,
                   Real64 &hrout,
                   Real64 &hin,
                   Real64 &hout,
                   Array1D<Real64> &hcgas,
                   Array1D<Real64> &hrgas,
                   Real64 &shgc,
                   int &nperr,
                   std::string &ErrorMessage,
                   Real64 &shgct,
                   Real64 &tamb,
                   Real64 &troom,
                   const Array1D_int &ibc,
                   const Array1D<Real64> &Atop,
                   const Array1D<Real64> &Abot,
                   const Array1D<Real64> &Al,
                   const Array1D<Real64> &Ar,
                   const Array1D<Real64> &Ah,
                   const Array1D<Real64> &SlatThick,
                   const Array1D<Real64> &SlatWidth,
                   const Array1D<Real64> &SlatAngle,
                   const Array1D<Real64> &SlatCond,
                   const Array1D<Real64> &SlatSpacing,
                   const Array1D<Real64> &SlatCurve,
                   const Array1D<Real64> &vvent,
                   const Array1D<Real64> &tvent,
                   const Array1D<TARCOGLayerType> &LayerType,
                   const Array1D_int &nslice,
                   const Array1D<Real64> &LaminateA,
                   const Array1D<Real64> &LaminateB,
                   const Array1D<Real64> &sumsol,
                   Array1D<Real64> &Ra,
                   Array1D<Real64> &Nu,
                   TARCOGThermalModel const ThermalMod,
                   int const Debug_mode,
                   Real64 &ShadeEmisRatioOut,
                   Real64 &ShadeEmisRatioIn,
                   Real64 &ShadeHcRatioOut,
                   Real64 &ShadeHcRatioIn,
                   Real64 &HcUnshadedOut,
                   Real64 &HcUnshadedIn,
                   Array1D<Real64> &Keff,
                   Array1D<Real64> &ShadeGapKeffConv,
                   Real64 const SDScalar,
                   int const SHGCCalc,
                   int &NumOfIterations,
                   Real64 const edgeGlCorrFac)
{

    // Argument array dimensioning
    EP_SIZE_CHECK(gap, MaxGap);
    EP_SIZE_CHECK(thick, maxlay);
    EP_SIZE_CHECK(scon, maxlay);
    EP_SIZE_CHECK(tir, maxlay2);
    EP_SIZE_CHECK(emis, maxlay2);
    EP_SIZE_CHECK(asol, maxlay);
    EP_SIZE_CHECK(presure, maxlay1);
    iprop.dim(maxgas, maxlay1);
    frct.dim(maxgas, maxlay1);
    xgcon.dim(3, maxgas);
    xgvis.dim(3, maxgas);
    xgcp.dim(3, maxgas);
    EP_SIZE_CHECK(xwght, maxgas);
    EP_SIZE_CHECK(gama, maxgas);
    EP_SIZE_CHECK(nmix, maxlay1);
    EP_SIZE_CHECK(SupportPillar, maxlay);
    EP_SIZE_CHECK(PillarSpacing, maxlay);
    EP_SIZE_CHECK(PillarRadius, maxlay);
    EP_SIZE_CHECK(theta, maxlay2);
    EP_SIZE_CHECK(q, maxlay3);
    EP_SIZE_CHECK(qv, maxlay1);
    EP_SIZE_CHECK(hcgas, maxlay1);
    EP_SIZE_CHECK(hrgas, maxlay1);
    EP_SIZE_CHECK(ibc, 2);
    EP_SIZE_CHECK(Atop, maxlay);
    EP_SIZE_CHECK(Abot, maxlay);
    EP_SIZE_CHECK(Al, maxlay);
    EP_SIZE_CHECK(Ar, maxlay);
    EP_SIZE_CHECK(Ah, maxlay);
    EP_SIZE_CHECK(SlatThick, maxlay);
    EP_SIZE_CHECK(SlatWidth, maxlay);
    EP_SIZE_CHECK(SlatAngle, maxlay);
    EP_SIZE_CHECK(SlatCond, maxlay);
    EP_SIZE_CHECK(SlatSpacing, maxlay);
    EP_SIZE_CHECK(SlatCurve, maxlay);
    EP_SIZE_CHECK(vvent, maxlay1);
    EP_SIZE_CHECK(tvent, maxlay1);
    EP_SIZE_CHECK(LayerType, maxlay);
    EP_SIZE_CHECK(nslice, maxlay);
    EP_SIZE_CHECK(LaminateA, maxlay);
    EP_SIZE_CHECK(LaminateB, maxlay);
    EP_SIZE_CHECK(sumsol, maxlay);
    EP_SIZE_CHECK(Ra, maxlay);
    EP_SIZE_CHECK(Nu, maxlay);
    EP_SIZE_CHECK(Keff, maxlay);
    EP_SIZE_CHECK(ShadeGapKeffConv, MaxGap);

    //  REAL(r64) :: grho(maxgas,3)
    Real64 shgct_NOSD;
    Real64 trmout;

    Real64 Gout;
    Real64 Gin;
    Real64 AchievedErrorTolerance;
    Real64 AchievedErrorToleranceSolar;
    int NumOfIter;
    int NumOfIterSolar;

    Real64 tgg;
    Real64 qc1;
    Real64 qc2;
    Real64 qcgg;

    Real64 ShadeHcModifiedOut;
    Real64 ShadeHcModifiedIn;

    // cbi...Variables for "unshaded" run:

    bool NeedUnshadedRun;
    int nlayer_NOSD;
    Real64 AchievedErrorTolerance_NOSD;
    int NumOfIter_NOSD;
    Real64 hin_NOSD;
    Real64 flux_NOSD;
    Real64 hcin_NOSD;
    Real64 hrin_NOSD;
    Real64 hcout_NOSD;
    Real64 hrout_NOSD;
    Real64 tamb_NOSD;
    Real64 troom_NOSD;
    Real64 ufactor_NOSD;
    Real64 sc_NOSD;
    Real64 hflux_NOSD;
    Real64 shgc_NOSD;
    Real64 hout_NOSD;
    // REAL(r64) ::  rs_NOSD(maxlay3)!,sol(maxlay)
    Real64 ShadeEmisRatioOut_NOSD;
    Real64 ShadeEmisRatioIn_NOSD;
    Real64 ShadeHcRatioOut_NOSD;
    Real64 ShadeHcRatioIn_NOSD;
    Real64 ShadeHcModifiedOut_NOSD;
    Real64 ShadeHcModifiedIn_NOSD;

    int FirstSpecularLayer;
    int LastSpecularLayer;

    // cbi...Other variables:
    Real64 flux;
    Real64 hint;
    Real64 houtt;
    Real64 ebsky;
    Real64 ebroom;
    int i;
    int j;
    int OriginalIndex;
    int UnshadedDebug;

    // Autodesk:Uninit Initialize variables used uninitialized
    shgc_NOSD = 0.0;            // Autodesk:Uninit Force default initialization
    sc_NOSD = 0.0;              // Autodesk:Uninit Force default initialization
    hflux_NOSD = 0.0;           // Autodesk:Uninit Force default initialization
    ShadeHcRatioIn_NOSD = 0.0;  // Autodesk:Uninit Force default initialization
    ShadeHcRatioOut_NOSD = 0.0; // Autodesk:Uninit Force default initialization

    AchievedErrorTolerance = 0.0;
    AchievedErrorToleranceSolar = 0.0;
    AchievedErrorTolerance_NOSD = 0.0;

    PrepVariablesISO15099(nlayer,
                          tout,
                          tind,
                          trmin,
                          isky,
                          outir,
                          tsky,
                          esky,
                          fclr,
                          gap,
                          thick,
                          scon,
                          tir,
                          emis,
                          tilt,
                          hin,
                          hout,
                          ibc,
                          SlatThick,
                          SlatWidth,
                          SlatAngle,
                          SlatCond,
                          LayerType,
                          ThermalMod,
                          SDScalar,
                          ShadeEmisRatioOut,
                          ShadeEmisRatioIn,
                          ShadeHcRatioOut,
                          ShadeHcRatioIn,
                          Keff,
                          ShadeGapKeffConv,
                          sc,
                          shgc,
                          ufactor,
                          flux,
                          state.dataThermalISO15099Calc->LaminateAU,
                          state.dataThermalISO15099Calc->sumsolU,
                          state.dataThermalISO15099Calc->sol0,
                          hint,
                          houtt,
                          trmout,
                          ebsky,
                          ebroom,
                          Gout,
                          Gin,
                          state.dataThermalISO15099Calc->rir,
                          state.dataThermalISO15099Calc->vfreevent,
                          nperr,
                          ErrorMessage);

    for (int i = 1; i <= nlayer; ++i) {
        state.dataThermalISO15099Calc->EffectiveOpenness(i) = Ah(i) / (width * height);
    }

    updateEffectiveMultipliers(nlayer,
                               width,
                               height,
                               Atop,
                               Abot,
                               Al,
                               Ar,
                               Ah,
                               state.dataThermalISO15099Calc->Atop_eff,
                               state.dataThermalISO15099Calc->Abot_eff,
                               state.dataThermalISO15099Calc->Al_eff,
                               state.dataThermalISO15099Calc->Ar_eff,
                               state.dataThermalISO15099Calc->Ah_eff,
                               LayerType,
                               SlatAngle);

    // No option to take hardcoded variables.  All gas coefficients are now passed from outside.
    // if (GoAhead(nperr)) call propcon90(ISO15099,mgas,xgcon,xgvis,xgcp,xgrho,xwght,nperr)

    // exit on error
    if (!(GoAhead(nperr))) {
        return;
    }

    // bi...Write intermediate results to output file:
    if (files.WriteDebugOutput) {
        WriteModifiedArguments(files.DebugOutputFile,
                               files.DBGD,
                               esky,
                               trmout,
                               trmin,
                               ebsky,
                               ebroom,
                               Gout,
                               Gin,
                               nlayer,
                               LayerType,
                               nmix,
                               frct,
                               thick,
                               scon,
                               gap,
                               xgcon,
                               xgvis,
                               xgcp,
                               xwght);
    }

    // cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
    //     This is "solar radiation" pass
    // cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

    // This is main calculation in case UFactor calculations will not be performed
    if ((dir > 0.0) || (SHGCCalc == 0)) {
        // call therm1d to calculate heat flux with solar radiation

        therm1d(state,
                files,
                nlayer,
                iwd,
                tout,
                tind,
                wso,
                wsi,
                VacuumPressure,
                VacuumMaxGapThickness,
                dir,
                ebsky,
                Gout,
                trmout,
                trmin,
                ebroom,
                Gin,
                tir,
                state.dataThermalISO15099Calc->rir,
                emis,
                gap,
                thick,
                scon,
                tilt,
                asol,
                height,
                heightt,
                width,
                iprop,
                frct,
                presure,
                nmix,
                xwght,
                xgcon,
                xgvis,
                xgcp,
                gama,
                SupportPillar,
                PillarSpacing,
                PillarRadius,
                theta,
                q,
                qv,
                flux,
                hcin,
                hrin,
                hcout,
                hrout,
                hin,
                hout,
                hcgas,
                hrgas,
                ufactor,
                nperr,
                ErrorMessage,
                tamb,
                troom,
                ibc,
                state.dataThermalISO15099Calc->Atop_eff,
                state.dataThermalISO15099Calc->Abot_eff,
                state.dataThermalISO15099Calc->Al_eff,
                state.dataThermalISO15099Calc->Ar_eff,
                state.dataThermalISO15099Calc->Ah_eff,
                state.dataThermalISO15099Calc->EffectiveOpenness,
                vvent,
                tvent,
                LayerType,
                Ra,
                Nu,
                state.dataThermalISO15099Calc->vfreevent,
                state.dataThermalISO15099Calc->qcgas,
                state.dataThermalISO15099Calc->qrgas,
                state.dataThermalISO15099Calc->Ebf,
                state.dataThermalISO15099Calc->Ebb,
                state.dataThermalISO15099Calc->Rf,
                state.dataThermalISO15099Calc->Rb,
                ShadeEmisRatioOut,
                ShadeEmisRatioIn,
                ShadeHcModifiedOut,
                ShadeHcModifiedIn,
                ThermalMod,
                Debug_mode,
                AchievedErrorToleranceSolar,
                NumOfIterSolar,
                edgeGlCorrFac);

        NumOfIterations = NumOfIterSolar;
        // exit on error:

        if (nlayer > 1) {
            for (i = 1; i <= nlayer - 1; ++i) {
                Keff(i) = gap(i) * q(2 * i + 1) / (theta(2 * i + 1) - theta(2 * i));
                if (IsShadingLayer(LayerType(i))) {
                    Keff(i) = gap(i) * q(2 * i + 1) / (theta(2 * i + 1) - theta(2 * i));
                }
                if (IsShadingLayer(LayerType(i + 1))) {
                    Keff(i) = gap(i) * q(2 * i + 1) / (theta(2 * i + 1) - theta(2 * i));
                }
            }
        }

        if (!(GoAhead(nperr))) {
            return;
        }

        // No need to store results in case of non-ufactor run
        if ((SHGCCalc > 0) && (dir > 0.0)) {
            solarISO15099(
                totsol, state.dataThermalISO15099Calc->rtot, state.dataThermalISO15099Calc->rs, nlayer, asol, state.dataThermalISO15099Calc->sft);
            shgct = state.dataThermalISO15099Calc->sft;
            shgct_NOSD = 0.0;
            state.dataThermalISO15099Calc->hcins = hcin;
            state.dataThermalISO15099Calc->hrins = hrin;
            state.dataThermalISO15099Calc->hins = hin;
            state.dataThermalISO15099Calc->hcouts = hcout;
            state.dataThermalISO15099Calc->hrouts = hrout;
            state.dataThermalISO15099Calc->houts = hout;
            state.dataThermalISO15099Calc->ufactors = ufactor;
            state.dataThermalISO15099Calc->fluxs = flux;
            for (i = 1; i <= nlayer; ++i) {
                state.dataThermalISO15099Calc->thetas(2 * i - 1) = theta(2 * i - 1);
                state.dataThermalISO15099Calc->thetas(2 * i) = theta(2 * i);
                state.dataThermalISO15099Calc->Ebbs(i) = state.dataThermalISO15099Calc->Ebb(i);
                state.dataThermalISO15099Calc->Ebfs(i) = state.dataThermalISO15099Calc->Ebf(i);
                state.dataThermalISO15099Calc->Rbs(i) = state.dataThermalISO15099Calc->Rb(i);
                state.dataThermalISO15099Calc->Rfs(i) = state.dataThermalISO15099Calc->Rf(i);
                state.dataThermalISO15099Calc->qs(2 * i - 1) = q(2 * i - 1);
                state.dataThermalISO15099Calc->qs(2 * i) = q(2 * i);
                // qprims(2*i - 1) = qprim(2*i - 1)
                // qprims(2*i) = qprim(2*i)
                state.dataThermalISO15099Calc->qvs(2 * i - 1) = qv(2 * i - 1);
                state.dataThermalISO15099Calc->qvs(2 * i) = qv(2 * i);
                state.dataThermalISO15099Calc->hcgass(i) = hcgas(i);
                state.dataThermalISO15099Calc->hrgass(i) = hrgas(i);
                state.dataThermalISO15099Calc->qrgaps(i) = state.dataThermalISO15099Calc->qrgas(i);
                state.dataThermalISO15099Calc->qcgaps(i) = state.dataThermalISO15099Calc->qcgas(i);
            }
            //    CHECK THIS!
            state.dataThermalISO15099Calc->qs(2 * nlayer + 1) = q(2 * nlayer + 1);
        } // if (UFactorCalc.gt.0) then
    }

    // No solar radiation pass is not needed to be calculated
    // if ((SHGCCalc.gt.0).or.(dir.eq.0)) then
    if (SHGCCalc > 0) {

        // cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
        //      This is "no solar radiation" pass
        // cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

        hin = hint;
        hout = houtt;

        // call therm1d to calculate heat flux without solar radiation
        therm1d(state,
                files,
                nlayer,
                iwd,
                tout,
                tind,
                wso,
                wsi,
                VacuumPressure,
                VacuumMaxGapThickness,
                0.0,
                ebsky,
                Gout,
                trmout,
                trmin,
                ebroom,
                Gin,
                tir,
                state.dataThermalISO15099Calc->rir,
                emis,
                gap,
                thick,
                scon,
                tilt,
                state.dataThermalISO15099Calc->sol0,
                height,
                heightt,
                width,
                iprop,
                frct,
                presure,
                nmix,
                xwght,
                xgcon,
                xgvis,
                xgcp,
                gama,
                SupportPillar,
                PillarSpacing,
                PillarRadius,
                theta,
                q,
                qv,
                flux,
                hcin,
                hrin,
                hcout,
                hrout,
                hin,
                hout,
                hcgas,
                hrgas,
                ufactor,
                nperr,
                ErrorMessage,
                tamb,
                troom,
                ibc,
                state.dataThermalISO15099Calc->Atop_eff,
                state.dataThermalISO15099Calc->Abot_eff,
                state.dataThermalISO15099Calc->Al_eff,
                state.dataThermalISO15099Calc->Ar_eff,
                state.dataThermalISO15099Calc->Ah_eff,
                state.dataThermalISO15099Calc->EffectiveOpenness,
                vvent,
                tvent,
                LayerType,
                Ra,
                Nu,
                state.dataThermalISO15099Calc->vfreevent,
                state.dataThermalISO15099Calc->qcgas,
                state.dataThermalISO15099Calc->qrgas,
                state.dataThermalISO15099Calc->Ebf,
                state.dataThermalISO15099Calc->Ebb,
                state.dataThermalISO15099Calc->Rf,
                state.dataThermalISO15099Calc->Rb,
                ShadeEmisRatioOut,
                ShadeEmisRatioIn,
                ShadeHcModifiedOut,
                ShadeHcModifiedIn,
                ThermalMod,
                Debug_mode,
                AchievedErrorTolerance,
                NumOfIter,
                edgeGlCorrFac);

        NumOfIterations = NumOfIter;

        // exit on error:
        if (!(GoAhead(nperr))) {
            return;
        }

        // bi...Keep hcout, hcin in case this is an unshaded system:
        HcUnshadedOut = hcout;
        HcUnshadedIn = hcin;

        // bi...do an Unshaded run if necessary (Uvalue/Winter conditions):
        // bi...Prepare variables for UNSHADED (NO SD) run:

        NeedUnshadedRun = false;
        FirstSpecularLayer = 1;
        LastSpecularLayer = nlayer;
        nlayer_NOSD = nlayer;
        if (IsShadingLayer(LayerType(1))) {
            --nlayer_NOSD;
            FirstSpecularLayer = 2;
            NeedUnshadedRun = true;
        }

        //  if (LayerType(nlayer).eq.VENETBLIND) then
        if (IsShadingLayer(LayerType(nlayer))) {
            --nlayer_NOSD;
            LastSpecularLayer = nlayer - 1;
            NeedUnshadedRun = true;
        }

        // no unshaded run for now
        NeedUnshadedRun = false;
        // bi...Set outdoor & indoor gas properties:
        if (NeedUnshadedRun) {
            state.dataThermalISO15099Calc->nmix_NOSD(1) = nmix(1);
            state.dataThermalISO15099Calc->presure_NOSD(1) = presure(1);
            state.dataThermalISO15099Calc->nmix_NOSD(nlayer_NOSD + 1) = nmix(nlayer + 1);
            state.dataThermalISO15099Calc->presure_NOSD(nlayer_NOSD + 1) = presure(nlayer + 1);
            for (j = 1; j <= nmix(1); ++j) {
                state.dataThermalISO15099Calc->iprop_NOSD(j, 1) = iprop(j, 1);
                state.dataThermalISO15099Calc->frct_NOSD(j, 1) = frct(j, 1);
            }
            for (j = 1; j <= nmix(nlayer_NOSD + 1); ++j) {
                state.dataThermalISO15099Calc->iprop_NOSD(j, nlayer_NOSD + 1) = iprop(j, nlayer + 1);
                state.dataThermalISO15099Calc->frct_NOSD(j, nlayer_NOSD + 1) = frct(j, nlayer + 1);
            }
            for (i = 1; i <= nlayer_NOSD; ++i) {
                OriginalIndex = FirstSpecularLayer + i - 1;
                state.dataThermalISO15099Calc->Atop_NOSD(i) = state.dataThermalISO15099Calc->Atop_eff(OriginalIndex);
                state.dataThermalISO15099Calc->Abot_NOSD(i) = state.dataThermalISO15099Calc->Abot_eff(OriginalIndex);
                state.dataThermalISO15099Calc->Al_NOSD(i) = state.dataThermalISO15099Calc->Al_eff(OriginalIndex);
                state.dataThermalISO15099Calc->Ar_NOSD(i) = state.dataThermalISO15099Calc->Ar_eff(OriginalIndex);
                state.dataThermalISO15099Calc->Ah_NOSD(i) = state.dataThermalISO15099Calc->Ah_eff(OriginalIndex);

                state.dataThermalISO15099Calc->SlatThick_NOSD(i) = SlatThick(OriginalIndex);
                state.dataThermalISO15099Calc->SlatWidth_NOSD(i) = SlatWidth(OriginalIndex);
                state.dataThermalISO15099Calc->SlatAngle_NOSD(i) = SlatAngle(OriginalIndex);
                state.dataThermalISO15099Calc->SlatCond_NOSD(i) = SlatCond(OriginalIndex);
                state.dataThermalISO15099Calc->SlatSpacing_NOSD(i) = SlatSpacing(OriginalIndex);
                state.dataThermalISO15099Calc->SlatCurve_NOSD(i) = SlatCurve(OriginalIndex);

                // cbi...    TO do when Forced Ventilation is implemented: take care of appropriate arguments!!!
                //      vvent_NOSD
                //      tvent_NOSD

                state.dataThermalISO15099Calc->LayerType_NOSD(i) = LayerType(OriginalIndex);

                state.dataThermalISO15099Calc->thick_NOSD(i) = thick(OriginalIndex);
                state.dataThermalISO15099Calc->scon_NOSD(i) = scon(OriginalIndex);
                state.dataThermalISO15099Calc->tir_NOSD(2 * i - 1) = tir(2 * OriginalIndex - 1);
                state.dataThermalISO15099Calc->emis_NOSD(2 * i - 1) = emis(2 * OriginalIndex - 1);
                state.dataThermalISO15099Calc->emis_NOSD(2 * i) = emis(2 * OriginalIndex);
                state.dataThermalISO15099Calc->rir_NOSD(2 * i - 1) = state.dataThermalISO15099Calc->rir(2 * OriginalIndex - 1);
                state.dataThermalISO15099Calc->rir_NOSD(2 * i) = state.dataThermalISO15099Calc->rir(2 * OriginalIndex);

                state.dataThermalISO15099Calc->gap_NOSD(i) = gap(OriginalIndex);

                if (i < nlayer_NOSD) {
                    state.dataThermalISO15099Calc->nmix_NOSD(i + 1) = nmix(OriginalIndex + 1);
                    state.dataThermalISO15099Calc->presure_NOSD(i + 1) = presure(OriginalIndex + 1);
                    for (j = 1; j <= state.dataThermalISO15099Calc->nmix_NOSD(i + 1); ++j) {
                        state.dataThermalISO15099Calc->iprop_NOSD(j, i + 1) = iprop(j, OriginalIndex + 1);
                        state.dataThermalISO15099Calc->frct_NOSD(j, i + 1) = frct(j, OriginalIndex + 1);
                    }
                }

                state.dataThermalISO15099Calc->LaminateA_NOSD(i) = LaminateA(OriginalIndex);
                state.dataThermalISO15099Calc->LaminateB_NOSD(i) = LaminateB(OriginalIndex);
                state.dataThermalISO15099Calc->sumsol_NOSD(i) = sumsol(OriginalIndex);

                state.dataThermalISO15099Calc->nslice_NOSD(i) = nslice(OriginalIndex);
            }

            //    This is UNSHADED pass - no solar radiation:
            hin_NOSD = hint;
            hout_NOSD = houtt;

            // Simon: Removed unshaded debug output for now
            UnshadedDebug = 0;
            if (files.WriteDebugOutput && (UnshadedDebug == 1)) {
                print(files.DebugOutputFile, "\n");
                print(files.DebugOutputFile, "UNSHADED RUN:\n");
                print(files.DebugOutputFile, "\n");

                WriteInputArguments(state,
                                    files.DebugOutputFile,
                                    files.DBGD,
                                    tout,
                                    tind,
                                    trmin,
                                    wso,
                                    iwd,
                                    wsi,
                                    dir,
                                    outir,
                                    isky,
                                    tsky,
                                    esky,
                                    fclr,
                                    VacuumPressure,
                                    VacuumMaxGapThickness,
                                    ibc,
                                    hout_NOSD,
                                    hin_NOSD,
                                    TARCOGGassesParams::Stdrd::ISO15099,
                                    ThermalMod,
                                    SDScalar,
                                    height,
                                    heightt,
                                    width,
                                    tilt,
                                    totsol,
                                    nlayer_NOSD,
                                    state.dataThermalISO15099Calc->LayerType_NOSD,
                                    state.dataThermalISO15099Calc->thick_NOSD,
                                    state.dataThermalISO15099Calc->scon_NOSD,
                                    asol,
                                    state.dataThermalISO15099Calc->tir_NOSD,
                                    state.dataThermalISO15099Calc->emis_NOSD,
                                    state.dataThermalISO15099Calc->Atop_NOSD,
                                    state.dataThermalISO15099Calc->Abot_NOSD,
                                    state.dataThermalISO15099Calc->Al_NOSD,
                                    state.dataThermalISO15099Calc->Ar_NOSD,
                                    state.dataThermalISO15099Calc->Ah_NOSD,
                                    state.dataThermalISO15099Calc->SlatThick_NOSD,
                                    state.dataThermalISO15099Calc->SlatWidth_NOSD,
                                    state.dataThermalISO15099Calc->SlatAngle_NOSD,
                                    state.dataThermalISO15099Calc->SlatCond_NOSD,
                                    state.dataThermalISO15099Calc->SlatSpacing_NOSD,
                                    state.dataThermalISO15099Calc->SlatCurve_NOSD,
                                    state.dataThermalISO15099Calc->nslice_NOSD,
                                    state.dataThermalISO15099Calc->LaminateA_NOSD,
                                    state.dataThermalISO15099Calc->LaminateB_NOSD,
                                    state.dataThermalISO15099Calc->sumsol_NOSD,
                                    state.dataThermalISO15099Calc->gap_NOSD,
                                    state.dataThermalISO15099Calc->vvent_NOSD,
                                    state.dataThermalISO15099Calc->tvent_NOSD,
                                    state.dataThermalISO15099Calc->presure_NOSD,
                                    state.dataThermalISO15099Calc->nmix_NOSD,
                                    state.dataThermalISO15099Calc->iprop_NOSD,
                                    state.dataThermalISO15099Calc->frct_NOSD,
                                    xgcon,
                                    xgvis,
                                    xgcp,
                                    xwght);

            } // end if UnshadedDebug = 1

            // cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
            //      This is "Unshaded, No solar radiation" pass
            // cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
            // call therm1d to calculate heat flux with solar radiation
            therm1d(state,
                    files,
                    nlayer_NOSD,
                    iwd,
                    tout,
                    tind,
                    wso,
                    wsi,
                    VacuumPressure,
                    VacuumMaxGapThickness,
                    0.0,
                    ebsky,
                    Gout,
                    trmout,
                    trmin,
                    ebroom,
                    Gin,
                    state.dataThermalISO15099Calc->tir_NOSD,
                    state.dataThermalISO15099Calc->rir_NOSD,
                    state.dataThermalISO15099Calc->emis_NOSD,
                    state.dataThermalISO15099Calc->gap_NOSD,
                    state.dataThermalISO15099Calc->thick_NOSD,
                    state.dataThermalISO15099Calc->scon_NOSD,
                    tilt,
                    state.dataThermalISO15099Calc->sol0,
                    height,
                    heightt,
                    width,
                    state.dataThermalISO15099Calc->iprop_NOSD,
                    state.dataThermalISO15099Calc->frct_NOSD,
                    state.dataThermalISO15099Calc->presure_NOSD,
                    state.dataThermalISO15099Calc->nmix_NOSD,
                    xwght,
                    xgcon,
                    xgvis,
                    xgcp,
                    gama,
                    SupportPillar,
                    PillarSpacing,
                    PillarRadius,
                    state.dataThermalISO15099Calc->theta_NOSD,
                    state.dataThermalISO15099Calc->q_NOSD,
                    state.dataThermalISO15099Calc->qv_NOSD,
                    flux_NOSD,
                    hcin_NOSD,
                    hrin_NOSD,
                    hcout_NOSD,
                    hrout_NOSD,
                    hin_NOSD,
                    hout_NOSD,
                    state.dataThermalISO15099Calc->hcgas_NOSD,
                    state.dataThermalISO15099Calc->hrgas_NOSD,
                    ufactor_NOSD,
                    nperr,
                    ErrorMessage,
                    tamb_NOSD,
                    troom_NOSD,
                    ibc,
                    state.dataThermalISO15099Calc->Atop_NOSD,
                    state.dataThermalISO15099Calc->Abot_NOSD,
                    state.dataThermalISO15099Calc->Al_NOSD,
                    state.dataThermalISO15099Calc->Ar_NOSD,
                    state.dataThermalISO15099Calc->Ah_NOSD,
                    state.dataThermalISO15099Calc->EffectiveOpenness_NOSD,
                    state.dataThermalISO15099Calc->vvent_NOSD,
                    state.dataThermalISO15099Calc->tvent_NOSD,
                    state.dataThermalISO15099Calc->LayerType_NOSD,
                    state.dataThermalISO15099Calc->Ra_NOSD,
                    state.dataThermalISO15099Calc->Nu_NOSD,
                    state.dataThermalISO15099Calc->vfreevent_NOSD,
                    state.dataThermalISO15099Calc->qcgas_NOSD,
                    state.dataThermalISO15099Calc->qrgas_NOSD,
                    state.dataThermalISO15099Calc->Ebf_NOSD,
                    state.dataThermalISO15099Calc->Ebb_NOSD,
                    state.dataThermalISO15099Calc->Rf_NOSD,
                    state.dataThermalISO15099Calc->Rb_NOSD,
                    ShadeEmisRatioOut_NOSD,
                    ShadeEmisRatioIn_NOSD,
                    ShadeHcModifiedOut_NOSD,
                    ShadeHcModifiedIn_NOSD,
                    ThermalMod,
                    Debug_mode,
                    AchievedErrorTolerance_NOSD,
                    NumOfIter_NOSD,
                    edgeGlCorrFac);

            NumOfIterations = NumOfIter_NOSD;
            // exit on error
            if (!(GoAhead(nperr))) {
                return;
            }

            // bi...  Keep these values:
            HcUnshadedOut = hcout_NOSD;
            HcUnshadedIn = hcin_NOSD;

            ShadeHcRatioOut = ShadeHcModifiedOut / HcUnshadedOut;
            ShadeHcRatioIn = ShadeHcModifiedIn / HcUnshadedIn;

            // bi...unshaded results:
            if (files.WriteDebugOutput && (UnshadedDebug == 1)) {
                WriteOutputArguments(files.DebugOutputFile,
                                     files.DBGD,
                                     nlayer_NOSD,
                                     tamb,
                                     state.dataThermalISO15099Calc->q_NOSD,
                                     state.dataThermalISO15099Calc->qv_NOSD,
                                     state.dataThermalISO15099Calc->qcgas_NOSD,
                                     state.dataThermalISO15099Calc->qrgas_NOSD,
                                     state.dataThermalISO15099Calc->theta_NOSD,
                                     state.dataThermalISO15099Calc->vfreevent_NOSD,
                                     state.dataThermalISO15099Calc->vvent_NOSD,
                                     state.dataThermalISO15099Calc->Keff_NOSD,
                                     state.dataThermalISO15099Calc->ShadeGapKeffConv_NOSD,
                                     troom_NOSD,
                                     ufactor_NOSD,
                                     shgc_NOSD,
                                     sc_NOSD,
                                     hflux_NOSD,
                                     shgct_NOSD,
                                     hcin_NOSD,
                                     hrin_NOSD,
                                     hcout_NOSD,
                                     hrout_NOSD,
                                     state.dataThermalISO15099Calc->Ra_NOSD,
                                     state.dataThermalISO15099Calc->Nu_NOSD,
                                     state.dataThermalISO15099Calc->LayerType_NOSD,
                                     state.dataThermalISO15099Calc->Ebf_NOSD,
                                     state.dataThermalISO15099Calc->Ebb_NOSD,
                                     state.dataThermalISO15099Calc->Rf_NOSD,
                                     state.dataThermalISO15099Calc->Rb_NOSD,
                                     ebsky,
                                     Gout,
                                     ebroom,
                                     Gin,
                                     ShadeEmisRatioIn_NOSD,
                                     ShadeEmisRatioOut_NOSD,
                                     ShadeHcRatioIn_NOSD,
                                     ShadeHcRatioOut_NOSD,
                                     hcin_NOSD,
                                     hcout_NOSD,
                                     state.dataThermalISO15099Calc->hcgas_NOSD,
                                     state.dataThermalISO15099Calc->hrgas_NOSD,
                                     AchievedErrorTolerance_NOSD,
                                     NumOfIter_NOSD); // Autodesk:Uninit shgc_NOSD, sc_NOSD, hflux_NOSD,
                                                      // ShadeHcRatioIn_NOSD, ShadeHcRatioOut_NOSD were
                                                      // uninitialized
            } // end if UnshadedDebug = 1
        } // end if NeedUnshadedRun...

        // bi Set T6-related quantities keff, keffc: (using non-solar pass results)
        if (nlayer > 1) {
            for (i = 1; i <= nlayer - 1; ++i) {
                Keff(i) = gap(i) * q(2 * i + 1) / (theta(2 * i + 1) - theta(2 * i));
                if (IsShadingLayer(LayerType(i))) {
                    Keff(i) = gap(i) * q(2 * i + 1) / (theta(2 * i + 1) - theta(2 * i));
                }
                if (IsShadingLayer(LayerType(i + 1))) {
                    Keff(i) = gap(i) * q(2 * i + 1) / (theta(2 * i + 1) - theta(2 * i));
                }
                if (IsShadingLayer(LayerType(i))) {
                    // Keff(i)   = gap(i)   * qprim(2*i+1) / (theta(2*i+1) - theta(2*i))
                    if ((i > 1) && (i < nlayer)) {
                        tgg = gap(i - 1) + gap(i) + thick(i);
                        qc1 = state.dataThermalISO15099Calc->qcgas(i - 1);
                        qc2 = state.dataThermalISO15099Calc->qcgas(i);
                        qcgg = (qc1 + qc2) / 2.0;
                        ShadeGapKeffConv(i) = tgg * qcgg / (theta(2 * i + 1) - theta(2 * i - 2));
                    }
                }
            }
        }

    } // if (UFactorCalc.ne.0) then

    // bi...  For debugging purposes:
    state.dataThermalISO15099Calc->qeff = ufactor * std::abs(tout - tind);
    state.dataThermalISO15099Calc->flux_nonsolar = flux;

    if ((SHGCCalc > 0) && (dir > 0.0)) {
        shgc = totsol - (state.dataThermalISO15099Calc->fluxs - flux) / dir;
        sc = shgc / 0.87;
        hcin = state.dataThermalISO15099Calc->hcins;
        hrin = state.dataThermalISO15099Calc->hrins;
        hin = state.dataThermalISO15099Calc->hins;
        hcout = state.dataThermalISO15099Calc->hcouts;
        hrout = state.dataThermalISO15099Calc->hrouts;
        hout = state.dataThermalISO15099Calc->houts;
        flux = state.dataThermalISO15099Calc->fluxs; // <--- ???
        for (i = 1; i <= nlayer; ++i) {
            theta(2 * i - 1) = state.dataThermalISO15099Calc->thetas(2 * i - 1);
            theta(2 * i) = state.dataThermalISO15099Calc->thetas(2 * i);
            state.dataThermalISO15099Calc->Ebb(i) = state.dataThermalISO15099Calc->Ebbs(i);
            state.dataThermalISO15099Calc->Ebf(i) = state.dataThermalISO15099Calc->Ebfs(i);
            state.dataThermalISO15099Calc->Rb(i) = state.dataThermalISO15099Calc->Rbs(i);
            state.dataThermalISO15099Calc->Rf(i) = state.dataThermalISO15099Calc->Rfs(i);
            q(2 * i - 1) = state.dataThermalISO15099Calc->qs(2 * i - 1);
            q(2 * i) = state.dataThermalISO15099Calc->qs(2 * i);
            // qprim(2*i - 1) = qprims(2*i - 1)
            // qprim(2*i) = qprims(2*i)
            qv(2 * i - 1) = state.dataThermalISO15099Calc->qvs(2 * i - 1);
            qv(2 * i) = state.dataThermalISO15099Calc->qvs(2 * i);
            hcgas(i) = state.dataThermalISO15099Calc->hcgass(i);
            hrgas(i) = state.dataThermalISO15099Calc->hrgass(i);
            state.dataThermalISO15099Calc->qcgas(i) = state.dataThermalISO15099Calc->qcgaps(i);
            state.dataThermalISO15099Calc->qrgas(i) = state.dataThermalISO15099Calc->qrgaps(i);
            AchievedErrorTolerance = AchievedErrorToleranceSolar;
            NumOfIter = NumOfIterSolar;
        }

        // bi    CHECK THIS!
        q(2 * nlayer + 1) = state.dataThermalISO15099Calc->qs(2 * nlayer + 1);
    }

    hflux = flux; // save flux value for output table

    // bi...  Write results to debug output file:
    if (files.WriteDebugOutput) {
        WriteOutputArguments(files.DebugOutputFile,
                             files.DBGD,
                             nlayer,
                             tamb,
                             q,
                             qv,
                             state.dataThermalISO15099Calc->qcgas,
                             state.dataThermalISO15099Calc->qrgas,
                             theta,
                             state.dataThermalISO15099Calc->vfreevent,
                             vvent,
                             Keff,
                             ShadeGapKeffConv,
                             troom,
                             ufactor,
                             shgc,
                             sc,
                             hflux,
                             shgct,
                             hcin,
                             hrin,
                             hcout,
                             hrout,
                             Ra,
                             Nu,
                             LayerType,
                             state.dataThermalISO15099Calc->Ebf,
                             state.dataThermalISO15099Calc->Ebb,
                             state.dataThermalISO15099Calc->Rf,
                             state.dataThermalISO15099Calc->Rb,
                             ebsky,
                             Gout,
                             ebroom,
                             Gin,
                             ShadeEmisRatioIn,
                             ShadeEmisRatioOut,
                             ShadeHcRatioIn,
                             ShadeHcRatioOut,
                             HcUnshadedIn,
                             HcUnshadedOut,
                             hcgas,
                             hrgas,
                             AchievedErrorTolerance,
                             NumOfIter);
    } // if WriteDebugOutput.eq.true - writing output file
}

void therm1d(EnergyPlusData &state,
             Files &files,
             int const nlayer,
             int const iwd,
             Real64 &tout,
             Real64 &tind,
             Real64 const wso,
             Real64 const wsi,
             Real64 const VacuumPressure,
             Real64 const VacuumMaxGapThickness,
             Real64 const dir,
             Real64 &ebsky,
             Real64 const Gout,
             Real64 const trmout,
             Real64 const trmin,
             Real64 &ebroom,
             Real64 const Gin,
             const Array1D<Real64> &tir,
             const Array1D<Real64> &rir,
             const Array1D<Real64> &emis,
             const Array1D<Real64> &gap,
             const Array1D<Real64> &thick,
             const Array1D<Real64> &scon,
             Real64 const tilt,
             const Array1D<Real64> &asol,
             Real64 const height,
             Real64 const heightt,
             Real64 const width,
             Array2_int const &iprop,
             Array2<Real64> const &frct,
             const Array1D<Real64> &presure,
             const Array1D_int &nmix,
             const Array1D<Real64> &wght,
             Array2<Real64> const &gcon,
             Array2<Real64> const &gvis,
             Array2<Real64> const &gcp,
             const Array1D<Real64> &gama,
             const Array1D_int &SupportPillar,
             const Array1D<Real64> &PillarSpacing,
             const Array1D<Real64> &PillarRadius,
             Array1D<Real64> &theta,
             Array1D<Real64> &q,
             Array1D<Real64> &qv,
             Real64 &flux,
             Real64 &hcin,
             Real64 &hrin,
             Real64 &hcout,
             Real64 &hrout,
             Real64 const hin,
             Real64 const hout,
             Array1D<Real64> &hcgas,
             Array1D<Real64> &hrgas,
             Real64 &ufactor,
             int &nperr,
             std::string &ErrorMessage,
             Real64 &tamb,
             Real64 &troom,
             const Array1D_int &ibc,
             const Array1D<Real64> &Atop,
             const Array1D<Real64> &Abot,
             const Array1D<Real64> &Al,
             const Array1D<Real64> &Ar,
             const Array1D<Real64> &Ah,
             const Array1D<Real64> &EffectiveOpenness,
             const Array1D<Real64> &vvent,
             const Array1D<Real64> &tvent,
             const Array1D<TARCOGLayerType> &LayerType,
             Array1D<Real64> &Ra,
             Array1D<Real64> &Nu,
             Array1D<Real64> &vfreevent,
             Array1D<Real64> &qcgas,
             Array1D<Real64> &qrgas,
             Array1D<Real64> &Ebf,
             Array1D<Real64> &Ebb,
             Array1D<Real64> &Rf,
             Array1D<Real64> &Rb,
             Real64 &ShadeEmisRatioOut,
             Real64 &ShadeEmisRatioIn,
             Real64 &ShadeHcModifiedOut,
             Real64 &ShadeHcModifiedIn,
             TARCOGThermalModel ThermalMod,
             [[maybe_unused]] int const Debug_mode,
             Real64 &AchievedErrorTolerance,
             int &TotalIndex,
             Real64 const edgeGlCorrFac)
{
    //********************************************************************************
    // Main subroutine for calculation of 1-D heat transfer in the center of glazing.
    //********************************************************************************
    // Inputs
    //   nlayer    number of solid layers
    //   iwd   wind direction
    //   tout  outside temp in k
    //   tind  inside temp in k
    //   wso   wind speed in m/s
    //   wsi   inside forced air speed m/s
    //   Ebsky     ir flux from outside
    //   Gout  back facing radiosity from outside
    //   Trmout    Mean outdoor radiant temperature
    //   Trmin     Mean indoor radiant temperature
    //   Ebroom    ir flux from room
    //   Gin   front facing radiosity from room
    //   tir   ir transmittance of each layer
    //   rir   ir reflectance of each surface
    //   emis  ir emittances of each surface
    //   gap   array of gap widths in meters
    //   thick     thickness of glazing layers (m)
    //   scon  Vector of conductivities of 'glazing' layers
    //   tilt  Window tilt (deg). vert: tilt=90, hor out up: tilt=0, hor out down: tilt=180
    //   sol   absorbed solar energy for each layer in w/m2
    //   height    glazing cavity height
    //   heightt
    //   iprop
    //   frct
    //   presure
    //   nmix  vector of number of gasses in a mixture for each gap
    //   hin  convective indoor film coefficient (if non-zero hin input)
    //   hout     convective outdoor film coeff. (if non-zero hout input)
    // outputs
    //   theta     temp distribution in k
    //   flux  net heat flux between room and window
    //   rtot  overall thermal resistance
    //   rs    ?
    //   hcin  convective indoor film coeff.
    //   hrin  radiative part of indoor film coeff.
    //   hcout     convective outdoor film coeff.
    //   hrout     radiative part of outdoor film coeff.
    //   hin   convective indoor film coefficient
    //   hout  convective outdoor film coeff.
    //   ufactor   overall u-factor
    //   qcgap     vector of convective/conductive parts of flux in gaps
    //   qrgap     vector of radiative parts of flux in gaps
    //   nperr
    // *Inactives**
    //   wa - window azimuth (degrees, clockwise from south)
    //   hgas  matrix of gap film coefficients
    // Locals
    //   Ebb   Vector
    //   Ebf   Vector
    //   Rb    Vector
    //   Rf    Vector
    //   a     Array
    //   b     Array
    //   hhat  Vector
    //   err   iteration tolerance
    //   dtmax     max temp difference after iteration
    //   index     iteration step

    // Using
    // Locals
    //    0 - don't create debug output files
    //    1 - append results to existing debug output file
    //    2 - store results in new debug output file
    //   3 - save in-between results (in all iterations) to existing debug file

    Array2D<Real64> a(4 * nlayer, 4 * nlayer);
    Array1D<Real64> b(4 * nlayer);
    // REAL(r64) :: hhatv(maxlay3),hcv(maxlay3), Ebgap(maxlay3), Tgap(maxlay1)

    // REAL(r64) ::  alpha
    int maxiter;

    Real64 qr_gap_out;
    Real64 qr_gap_in;

    Array1D<Real64> told(2 * nlayer);

    // Simon: parameters used in case of JCFN iteration method
    Array1D<Real64> FRes({1, 4 * nlayer});      // store function results from current iteration
    Array1D<Real64> FResOld({1, 4 * nlayer});   // store function results from previous iteration
    Array1D<Real64> FResDiff({1, 4 * nlayer});  // save difference in results between iterations
    Array1D<Real64> Radiation({1, 2 * nlayer}); // radiation on layer surfaces.  used as temporary storage during iterations

    Array1D<Real64> x({1, 4 * nlayer});       // temporary vector for storing results (theta and Radiation).  used for easier handling
    Array1D<Real64> dX({1, 4 * nlayer}, 0.0); // difference in results
    Array2D<Real64> Jacobian({1, 4 * nlayer}, {1, 4 * nlayer}); // diagonal vector for jacobian computation-free newton method
    Array1D<Real64> DRes({1, 4 * nlayer});                      // used in jacobian forward-difference approximation

    // This is used to store matrix before equation solver.  It is important because solver destroys
    // content of matrices
    Array2D<Real64> LeftHandSide({1, 4 * nlayer}, {1, 4 * nlayer});
    Array1D<Real64> RightHandSide({1, 4 * nlayer});

    // Simon: Keep best achieved convergence
    Real64 prevDifference;
    Real64 Relaxation;
    Array1D<Real64> RadiationSave({1, 2 * nlayer});
    Array1D<Real64> thetaSave({1, 2 * nlayer});
    int currentTry;

    int CSMFlag;
    int i;
    int j;
    int k;
    Real64 curDifference;
    int index;
    int curTempCorrection;

    Real64 qc_gap_in;
    Real64 hc_modified_in;

    CalculationOutcome CalcOutcome;

    bool iterationsFinished; // To mark whether or not iterations are finished
    bool saveIterationResults;
    bool updateGapTemperature;
    // logical :: TurnOnNewton

    int SDLayerIndex = -1;

    Array1D<Real64> sconScaled(maxlay);

    // Simon: This is set to zero until it is resolved what to do with modifier
    ShadeHcModifiedOut = 0.0;
    CSMFlag = 0;
    CalcOutcome = CalculationOutcome::Invalid;
    curTempCorrection = 0;
    AchievedErrorTolerance = 0.0;
    curDifference = 0.0;
    currentTry = 0;
    index = 0;
    TotalIndex = 0;
    iterationsFinished = false;
    qv = 0.0;
    Ebb = 0.0;
    Ebf = 0.0;
    Rb = 0.0;
    Rf = 0.0;
    a = 0.0;
    b = 0.0;

    FRes = 0.0;
    FResOld = 0.0;
    FResDiff = 0.0;
    Radiation = 0.0;
    Relaxation = RelaxationStart;

    maxiter = NumOfIterations;

    saveIterationResults = false;

    for (i = 1; i <= nlayer; ++i) {
        k = 2 * i;
        Radiation(k) = Ebb(i);
        Radiation(k - 1) = Ebf(i);
        told(k - 1) = 0.0;
        told(k) = 0.0;
    }

    // bi...Set LayerTypeSpec array - need to treat venetians AND woven shades as glass:
    if (ThermalMod == TARCOGThermalModel::CSM) {
        for (i = 1; i <= nlayer; ++i) {
            if (IsShadingLayer(LayerType(i))) {
                //                    LayerTypeSpec( i ) = 0; //Unused
                SDLayerIndex = i;
            } else {
                //                    LayerTypeSpec( i ) = LayerType( i ); //Unused
            }
        }
    }

    // first store results before iterations begin
    if (saveIterationResults) {
        storeIterationResults(state,
                              files,
                              nlayer,
                              index,
                              theta,
                              trmout,
                              tamb,
                              trmin,
                              troom,
                              ebsky,
                              ebroom,
                              hcin,
                              hcout,
                              hrin,
                              hrout,
                              hin,
                              hout,
                              Ebb,
                              Ebf,
                              Rb,
                              Rf,
                              nperr);
    }

    state.dataThermalISO15099Calc->Tgap(1) = tout;
    state.dataThermalISO15099Calc->Tgap(nlayer + 1) = tind;
    for (i = 2; i <= nlayer; ++i) {
        state.dataThermalISO15099Calc->Tgap(i) = (theta(2 * i - 1) + theta(2 * i - 2)) / 2;
    }
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //!!! MAIN ITERATION LOOP
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    while (!(iterationsFinished)) {

        for (i = 1; i <= 2 * nlayer; ++i) {
            if (theta(i) < 0) {
                theta(i) = 1.0 * i;
            }
        }

        // do i=1,nlayer+1
        //  if (i == 1) then
        //    Tgap(i) = tout
        //  else if (i == nlayer+1) then
        //    Tgap(i) = tind
        //  else
        //    Tgap(i) = (theta(2*i-2) + theta(2*i-1)) / 2.0d0
        //  end if
        // end do

        // skip updating gap temperatures for shading devices. Gap temperature in that case is not simply average
        // between two layer temperatures
        for (i = 2; i <= nlayer; ++i) {
            updateGapTemperature = false;
            if ((!(IsShadingLayer(LayerType(i - 1)))) && (!(IsShadingLayer(LayerType(i))))) {
                updateGapTemperature = true;
            }
            if (updateGapTemperature) {
                state.dataThermalISO15099Calc->Tgap(i) = (theta(2 * i - 1) + theta(2 * i - 2)) / 2;
            }
        }

        // evaluate convective/conductive components of gap
        hatter(state,
               nlayer,
               iwd,
               tout,
               tind,
               wso,
               wsi,
               VacuumPressure,
               VacuumMaxGapThickness,
               ebsky,
               tamb,
               ebroom,
               troom,
               gap,
               height,
               heightt,
               scon,
               tilt,
               theta,
               state.dataThermalISO15099Calc->Tgap,
               Radiation,
               trmout,
               trmin,
               iprop,
               frct,
               presure,
               nmix,
               wght,
               gcon,
               gvis,
               gcp,
               gama,
               SupportPillar,
               PillarSpacing,
               PillarRadius,
               state.dataThermalISO15099Calc->hgas,
               hcgas,
               hrgas,
               hcin,
               hcout,
               hin,
               hout,
               index,
               ibc,
               nperr,
               ErrorMessage,
               hrin,
               hrout,
               Ra,
               Nu);

        effectiveLayerCond(state,
                           nlayer,
                           LayerType,
                           scon,
                           thick,
                           iprop,
                           frct,
                           nmix,
                           presure,
                           wght,
                           gcon,
                           gvis,
                           gcp,
                           EffectiveOpenness,
                           theta,
                           sconScaled,
                           nperr,
                           ErrorMessage);

        // exit on error
        if (!(GoAhead(nperr))) {
            return;
        }

        // bi...Override hhat values near SHADING DEVICE layer(s), but only for CSM thermal model:
        if ((ThermalMod == TARCOGThermalModel::CSM) && (SDLayerIndex > 0)) {
            // adjust hhat values
            // call adjusthhat(SDLayerIndex, ibc, tout, tind, nlayer, theta, wso, wsi, iwd, height, heightt, tilt,  &
            //               &  thick, gap, hout, hrout, hin, hrin, iprop, frct, presure, nmix, wght, gcon, gvis, gcp, &
            //               index, SDScalar, Ebf, Ebb, hgas, hhat, nperr, ErrorMessage)
            // do i = 1, maxlay3
            // hhatv(i) = 0.0d0
            // Ebgap(i) = 0.0d0
            // qv(i)    = 0.0d0
            // hcv(i)   = 0.0d0
            // end do
            matrixQBalance(nlayer,
                           a,
                           b,
                           sconScaled,
                           hcgas,
                           state.dataThermalISO15099Calc->hcgapMod,
                           asol,
                           qv,
                           state.dataThermalISO15099Calc->hcv,
                           tind,
                           tout,
                           Gin,
                           Gout,
                           theta,
                           tir,
                           rir,
                           emis,
                           edgeGlCorrFac);
        } else {
            // bi...There are no Venetian layers, or ThermalMod is not CSM, so carry on as usual:
            shading(state,
                    theta,
                    gap,
                    state.dataThermalISO15099Calc->hgas,
                    hcgas,
                    hrgas,
                    frct,
                    iprop,
                    presure,
                    nmix,
                    wght,
                    gcon,
                    gvis,
                    gcp,
                    nlayer,
                    width,
                    height,
                    tilt,
                    tout,
                    tind,
                    Atop,
                    Abot,
                    Al,
                    Ar,
                    Ah,
                    vvent,
                    tvent,
                    LayerType,
                    state.dataThermalISO15099Calc->Tgap,
                    qv,
                    state.dataThermalISO15099Calc->hcv,
                    nperr,
                    ErrorMessage,
                    vfreevent);

            // exit on error
            if (!(GoAhead(nperr))) {
                return;
            }

            matrixQBalance(nlayer,
                           a,
                           b,
                           sconScaled,
                           hcgas,
                           state.dataThermalISO15099Calc->hcgapMod,
                           asol,
                           qv,
                           state.dataThermalISO15099Calc->hcv,
                           tind,
                           tout,
                           Gin,
                           Gout,
                           theta,
                           tir,
                           rir,
                           emis,
                           edgeGlCorrFac);

        } //  end if

        FResOld = FRes;

        // Pack results in one array
        for (i = 1; i <= nlayer; ++i) {
            k = 4 * i - 3;
            j = 2 * i - 1;

            x(k) = theta(j);
            x(k + 1) = Radiation(j);
            x(k + 2) = Radiation(j + 1);
            x(k + 3) = theta(j + 1);
        }

        CalculateFuncResults(nlayer, a, b, x, FRes);

        FResDiff = FRes - FResOld;

        LeftHandSide = a;
        RightHandSide = b;
        EquationsSolver(state, LeftHandSide, RightHandSide, 4 * nlayer, nperr, ErrorMessage);

        // Simon: This is much better, but also much slower convergence criteria.  Think of how to make this flexible and allow
        // user to change this from outside (through argument passing)
        // curDifference = ABS(FRes(1))
        // do i = 2, 4*nlayer
        // curDifference = MAX(curDifference, ABS(FRes(i)))
        // curDifference = curDifference + ABS(FRes(i))
        // end do

        curDifference = std::abs(theta(1) - told(1));
        // curDifference = ABS(FRes(1))
        for (i = 2; i <= 2 * nlayer; ++i) {
            // do i = 2, 4*nlayer
            curDifference = max(curDifference, std::abs(theta(i) - told(i)));
            // curDifference = MAX(ABS(FRes(i)), curDifference)
        }

        for (i = 1; i <= nlayer; ++i) {
            k = 4 * i - 3;
            j = 2 * i - 1;
            // if (TurnOnNewton) then
            //  theta(j) = theta(j) + Relaxation*dx(k)
            //  theta(j+1) = theta(j+1) + Relaxation*dx(k+1)
            //  Radiation(j) = Radiation(j) + Relaxation*dx(k+2)
            //  Radiation(j+1) = Radiation(j+1) + Relaxation*dx(k+3)
            // else
            //  dX(k) = RightHandSide(k) - theta(j)
            //  dX(k+1) = RightHandSide(k + 1) - theta(j+1)
            //  dX(k+2) = RightHandSide(k + 2) - Radiation(j)
            //  dX(k+3) = RightHandSide(k + 3) - Radiation(j+1)
            told(j) = theta(j);
            told(j + 1) = theta(j + 1);
            theta(j) = (1 - Relaxation) * theta(j) + Relaxation * RightHandSide(k);
            Radiation(j) = (1 - Relaxation) * Radiation(j) + Relaxation * RightHandSide(k + 1);
            Radiation(j + 1) = (1 - Relaxation) * Radiation(j + 1) + Relaxation * RightHandSide(k + 2);
            theta(j + 1) = (1 - Relaxation) * theta(j + 1) + Relaxation * RightHandSide(k + 3);
            // end if
        }

        // it is important not to update gaps around shading layers since that is already calculated by
        // shading routines
        for (i = 1; i <= nlayer + 1; ++i) {
            updateGapTemperature = true;
            if ((i == 1) || (i == nlayer + 1)) {
                // update gap array with interior and exterior temperature
                updateGapTemperature = true;
            } else {
                // update gap temperature only if gap on both sides
                updateGapTemperature = false;
                if ((!(IsShadingLayer(LayerType(i - 1)))) && (!(IsShadingLayer(LayerType(i))))) {
                    updateGapTemperature = true;
                }
            }
            j = 2 * (i - 1);
            if (updateGapTemperature) {
                if (i == 1) {
                    state.dataThermalISO15099Calc->Tgap(1) = tout;
                } else if (i == (nlayer + 1)) {
                    state.dataThermalISO15099Calc->Tgap(i) = tind;
                } else {
                    state.dataThermalISO15099Calc->Tgap(i) = (theta(j) + theta(j + 1)) / 2;
                }
            }
        }

        // and store results during iterations
        if (saveIterationResults) {
            storeIterationResults(state,
                                  files,
                                  nlayer,
                                  index + 1,
                                  theta,
                                  trmout,
                                  tamb,
                                  trmin,
                                  troom,
                                  ebsky,
                                  ebroom,
                                  hcin,
                                  hcout,
                                  hrin,
                                  hrout,
                                  hin,
                                  hout,
                                  Ebb,
                                  Ebf,
                                  Rb,
                                  Rf,
                                  nperr);
        }

        if (!(GoAhead(nperr))) {
            return;
        }

        prevDifference = curDifference;

        if ((index == 0) || (curDifference < AchievedErrorTolerance)) {
            AchievedErrorTolerance = curDifference;
            currentTry = 0;
            for (i = 1; i <= 2 * nlayer; ++i) {
                RadiationSave(i) = Radiation(i);
                thetaSave(i) = theta(i);
            }
        } else {
            // This is case when program solution diverged
            ++currentTry;
            if (currentTry >= NumOfTries) {
                currentTry = 0;
                for (i = 1; i <= 2 * nlayer; ++i) {
                    Radiation(i) = RadiationSave(i);
                    theta(i) = thetaSave(i);
                }
                // if (.not.TurnOnNewton) then
                //  TurnOnNewton = .TRUE.
                // else
                Relaxation -= RelaxationDecrease;
                TotalIndex += index;
                index = 0;
                // Start from best achieved convergence
                if (Relaxation <= 0.0) { // cannot continue with relaxation equal to zero
                    iterationsFinished = true;
                }
                // TurnOnNewton = .TRUE.
                // end if ! if (.not.TurnOnNewton) then
            } // f (currentTry == NumOfTries) then
        }

        // Check if results were found:
        if (curDifference < ConvergenceTolerance) {
            CalcOutcome = CalculationOutcome::OK;
            TotalIndex += index;
            iterationsFinished = true;
        }

        if (index >= maxiter) {
            Relaxation -= RelaxationDecrease;
            TotalIndex += index;
            index = 0;
            // TurnOnNewton = .TRUE.

            // Start from best achieved convergence
            for (i = 1; i <= 2 * nlayer; ++i) {
                Radiation(i) = RadiationSave(i);
                theta(i) = thetaSave(i);
            }
            if (Relaxation <= 0.0) { // cannot continue with relaxation equal to zero
                iterationsFinished = true;
            }
        }

        ++index;
    }

    // Get results from closest iteration and store it
    if (CalcOutcome == CalculationOutcome::OK) {
        for (i = 1; i <= 2 * nlayer; ++i) {
            Radiation(i) = RadiationSave(i);
            theta(i) = thetaSave(i);
        }

        for (i = 2; i <= nlayer; ++i) {
            updateGapTemperature = false;
            if ((!(IsShadingLayer(LayerType(i - 1)))) && (!(IsShadingLayer(LayerType(i))))) {
                updateGapTemperature = true;
            }

            if (updateGapTemperature) {
                state.dataThermalISO15099Calc->Tgap(i) = (theta(2 * i - 1) + theta(2 * i - 2)) / 2;
            }
        }

        // Simon: It is important to recalculate coefficients from most accurate run
        hatter(state,
               nlayer,
               iwd,
               tout,
               tind,
               wso,
               wsi,
               VacuumPressure,
               VacuumMaxGapThickness,
               ebsky,
               tamb,
               ebroom,
               troom,
               gap,
               height,
               heightt,
               scon,
               tilt,
               theta,
               state.dataThermalISO15099Calc->Tgap,
               Radiation,
               trmout,
               trmin,
               iprop,
               frct,
               presure,
               nmix,
               wght,
               gcon,
               gvis,
               gcp,
               gama,
               SupportPillar,
               PillarSpacing,
               PillarRadius,
               state.dataThermalISO15099Calc->hgas,
               hcgas,
               hrgas,
               hcin,
               hcout,
               hin,
               hout,
               index,
               ibc,
               nperr,
               ErrorMessage,
               hrin,
               hrout,
               Ra,
               Nu);

        shading(state,
                theta,
                gap,
                state.dataThermalISO15099Calc->hgas,
                hcgas,
                hrgas,
                frct,
                iprop,
                presure,
                nmix,
                wght,
                gcon,
                gvis,
                gcp,
                nlayer,
                width,
                height,
                tilt,
                tout,
                tind,
                Atop,
                Abot,
                Al,
                Ar,
                Ah,
                vvent,
                tvent,
                LayerType,
                state.dataThermalISO15099Calc->Tgap,
                qv,
                state.dataThermalISO15099Calc->hcv,
                nperr,
                ErrorMessage,
                vfreevent);
    }

    if (CalcOutcome == CalculationOutcome::Invalid) {
        ErrorMessage = "Tarcog failed to converge";
        nperr = 2; // error 2: failed to converge...
    }

    // Get radiation results first
    // if (curEquationsApproach.eq.eaQBalance) then
    for (i = 1; i <= nlayer; ++i) {
        k = 2 * i - 1;
        Rf(i) = Radiation(k);
        Rb(i) = Radiation(k + 1);
        Ebf(i) = Constant::StefanBoltzmann * pow_4(theta(k));
        Ebb(i) = Constant::StefanBoltzmann * pow_4(theta(k + 1));
    }
    // end if

    // Finishing calcs:
    resist(nlayer, trmout, tout, trmin, tind, hcgas, hrgas, theta, q, qv, LayerType, thick, scon, ufactor, flux, qcgas, qrgas);

    // bi...  Set T6-related quantities - ratios for modified epsilon, hc for modelling external SDs:
    //    (using non-solar pass results)
    if ((dir == 0.0) && (nlayer > 1)) {

        qr_gap_out = Rf(2) - Rb(1);
        qr_gap_in = Rf(nlayer) - Rb(nlayer - 1);

        if (IsShadingLayer(LayerType(1))) {
            ShadeEmisRatioOut = qr_gap_out / (emis(3) * Constant::StefanBoltzmann * (pow_4(theta(3)) - pow_4(trmout)));
            // qc_gap_out = qprim(3) - qr_gap_out
            // qcgapout2 = qcgas(1)
            // Hc_modified_out = (qc_gap_out / (theta(3) - tout))
            // ShadeHcModifiedOut = Hc_modified_out
        }

        if (IsShadingLayer(LayerType(nlayer))) {
            ShadeEmisRatioIn = qr_gap_in / (emis(2 * nlayer - 2) * Constant::StefanBoltzmann * (pow_4(trmin) - pow_4(theta(2 * nlayer - 2))));
            qc_gap_in = q(2 * nlayer - 1) - qr_gap_in;
            hc_modified_in = (qc_gap_in / (tind - theta(2 * nlayer - 2)));
            ShadeHcModifiedIn = hc_modified_in;
        }
    } // IF dir = 0
}

void guess(Real64 const tout,
           Real64 const tind,
           int const nlayer,
           const Array1D<Real64> &gap,
           const Array1D<Real64> &thick,
           Real64 &width,
           Array1D<Real64> &theta,
           Array1D<Real64> &Ebb,
           Array1D<Real64> &Ebf,
           Array1D<Real64> &Tgap)
{
    //***********************************************************************
    // purpose - initializes temperature distribution assuming
    //   a constant temperature gradient across the window
    //***********************************************************************
    // Input
    //   tout    outdoor air temperature (k)
    //   tind     indoor air temperature (k)
    //   nlayer  number of solid layers in window output
    //   gap     thickness of gas gaps (m)
    //   thick   thickness of glazing layers (m)
    // Output
    //   width   total width of the glazing system
    //   theta   array of surface temps starting from outdoor layer (k)
    //   Ebb     vector of emissive power (?) of the back surface (# of layers)
    //   Ebf     vector of emissive power (?) of the front surface (# of layers)
    // Locals
    //   x   Vector of running width
    //   delta   delta T per unit length

    // Using
    // Argument array dimensioning
    EP_SIZE_CHECK(gap, MaxGap);
    EP_SIZE_CHECK(thick, maxlay);
    EP_SIZE_CHECK(theta, maxlay2);
    EP_SIZE_CHECK(Ebb, maxlay);
    EP_SIZE_CHECK(Ebf, maxlay);
    EP_SIZE_CHECK(Tgap, maxlay1);

    // Locals
    Array1D<Real64> x(maxlay2);
    Real64 delta;
    int i;
    int j;
    int k;

    x(1) = 0.001;
    x(2) = x(1) + thick(1);

    for (i = 2; i <= nlayer; ++i) {
        j = 2 * i - 1;
        k = 2 * i;
        x(j) = x(j - 1) + gap(i - 1);
        x(k) = x(k - 1) + thick(i);
    }

    width = x(nlayer * 2) + 0.01;
    delta = (tind - tout) / width;

    if (delta == 0.0) {
        delta = TemperatureQuessDiff / width;
    }

    for (i = 1; i <= nlayer; ++i) {
        j = 2 * i;
        theta(j - 1) = tout + x(j - 1) * delta;
        theta(j) = tout + x(j) * delta;
        Ebf(i) = Constant::StefanBoltzmann * pow_4(theta(j - 1));
        Ebb(i) = Constant::StefanBoltzmann * pow_4(theta(j));
    }

    for (i = 1; i <= nlayer + 1; ++i) {
        if (i == 1) {
            Tgap(1) = tout;
        } else if (i == (nlayer + 1)) {
            Tgap(nlayer + 1) = tind;
        } else {
            Tgap(i) = (theta(2 * i - 1) + theta(2 * i - 2)) / 2;
        }
    }
}

void solarISO15099(Real64 const totsol, Real64 const rtot, const Array1D<Real64> &rs, int const nlayer, const Array1D<Real64> &absol, Real64 &sf)
{
    //***********************************************************************
    //   This subroutine calculates the shading coefficient for a window.
    //***********************************************************************
    //  Inputs:
    //    absol     array of absorbed fraction of solar radiation in lites
    //    totsol    total solar transmittance
    //    rtot  total thermal resistance of window
    //    rs    array of thermal resistances of each gap and layer
    //    layer     number of layers
    //     dir  direct solar radiation
    //  Outputs:
    //    sf    solar gain of space

    // Argument array dimensioning
    EP_SIZE_CHECK(rs, maxlay3);
    EP_SIZE_CHECK(absol, maxlay);

    // Locals
    Real64 flowin;
    Real64 fract;
    int i;
    int j;

    fract = 0.0;
    flowin = 0.0;
    sf = 0.0;

    if (rtot == 0.0) {
        return;
    }

    // evaluate inward flowing fraction of absorbed radiation:
    flowin = (rs(1) + 0.5 * rs(2)) / rtot;
    fract = absol(1) * flowin;

    for (i = 2; i <= nlayer; ++i) {
        j = 2 * i;
        flowin += (0.5 * (rs(j - 2) + rs(j)) + rs(j - 1)) / rtot;
        fract += absol(i) * flowin;
    }
    sf = totsol + fract; // add inward fraction to directly transmitted fraction
}

void resist(int const nlayer,
            Real64 const trmout,
            Real64 const Tout,
            Real64 const trmin,
            Real64 const tind,
            const Array1D<Real64> &hcgas,
            const Array1D<Real64> &hrgas,
            Array1D<Real64> const &Theta,
            Array1D<Real64> &qlayer,
            const Array1D<Real64> &qv,
            const Array1D<TARCOGLayerType> &LayerType,
            const Array1D<Real64> &thick,
            const Array1D<Real64> &scon,
            Real64 &ufactor,
            Real64 &flux,
            Array1D<Real64> &qcgas,
            Array1D<Real64> &qrgas)
{
    //***********************************************************************
    // subroutine to calculate total thermal resistance of the glazing system
    //***********************************************************************

    // Locals
    int i;

    // Simon: calculation of heat flow through gaps and layers as well as ventilation speed and heat flow
    // are kept just for reporting purposes.  U-factor calculation is performed by calculating heat flow transfer
    // at indoor layer

    // calculate heat flow for external and internal environments and gaps
    for (i = 1; i <= nlayer + 1; ++i) {
        if (i == 1) {
            qcgas(i) = hcgas(i) * (Theta(2 * i - 1) - Tout);
            qrgas(i) = hrgas(i) * (Theta(2 * i - 1) - trmout);
            qlayer(2 * i - 1) = qcgas(i) + qrgas(i);
            //    rs(2*i-1) = 1/hgas(i)
        } else if (i == (nlayer + 1)) {
            qcgas(i) = hcgas(i) * (tind - Theta(2 * i - 2));
            qrgas(i) = hrgas(i) * (trmin - Theta(2 * i - 2));
            qlayer(2 * i - 1) = qcgas(i) + qrgas(i);
            //    rs(2*i-1) = 1/hgas(i)
        } else {
            qcgas(i) = hcgas(i) * (Theta(2 * i - 1) - Theta(2 * i - 2));
            qrgas(i) = hrgas(i) * (Theta(2 * i - 1) - Theta(2 * i - 2));
            qlayer(2 * i - 1) = qcgas(i) + qrgas(i);
            //    rs(2*i-1) = 1/hgas(i)
        }
    }

    //.....Calculate thermal resistances for glazing layers:
    for (i = 1; i <= nlayer; ++i) {
        //  rs(2*i) = thick(i)/scon(i)
        qlayer(2 * i) = scon(i) / thick(i) * (Theta(2 * i) - Theta(2 * i - 1));
    }

    flux = qlayer(2 * nlayer + 1);
    if (IsShadingLayer(LayerType(nlayer))) {
        flux += qv(nlayer);
    }

    ufactor = 0.0;
    if (tind != Tout) {
        ufactor = flux / (tind - Tout);
    }
}

void hatter(EnergyPlusData &state,
            int const nlayer,
            int const iwd,
            Real64 const tout,
            Real64 const tind,
            Real64 const wso,
            Real64 const wsi,
            Real64 const VacuumPressure,
            Real64 const VacuumMaxGapThickness,
            Real64 const ebsky,
            Real64 &tamb,
            Real64 const ebroom,
            Real64 &troom,
            const Array1D<Real64> &gap,
            Real64 const height,
            Real64 const heightt,
            const Array1D<Real64> &scon,
            Real64 const tilt,
            Array1D<Real64> const &theta,
            const Array1D<Real64> &Tgap,
            Array1D<Real64> const &Radiation,
            Real64 const trmout,
            Real64 const trmin,
            Array2_int const &iprop,
            Array2<Real64> const &frct,
            const Array1D<Real64> &presure,
            const Array1D_int &nmix,
            const Array1D<Real64> &wght,
            Array2<Real64> const &gcon,
            Array2<Real64> const &gvis,
            Array2<Real64> const &gcp,
            const Array1D<Real64> &gama,
            const Array1D_int &SupportPillar,
            const Array1D<Real64> &PillarSpacing,
            const Array1D<Real64> &PillarRadius,
            Array1D<Real64> &hgas,
            Array1D<Real64> &hcgas,
            Array1D<Real64> &hrgas,
            Real64 &hcin,
            Real64 &hcout,
            Real64 const hin,
            Real64 const hout,
            int const index,
            const Array1D_int &ibc,
            int &nperr,
            std::string &ErrorMessage,
            Real64 &hrin,
            Real64 &hrout,
            Array1D<Real64> &Ra,
            Array1D<Real64> &Nu)
{
    //***********************************************************************
    //  This subroutine calculates the array of conductances/film coefficients used to model convection.  The conductances/film
    //  coefficients are calculated as functions of temperature defined with the usual variable h and THEN are converted into an
    //  equivalent value interms of the black body emittance based on the surface
    //***********************************************************************
    // Inputs
    //   nlayer   number of solid layers
    //   iwd  wind direction
    //   tout     outside temp in k
    //   tind  inside temp in k
    //   wso  wind speed in m/s
    //   wsi  inside forced air speed m/s
    //   Ebsky    ir flux from outside
    //   Ebroom   ir flux from room
    //   Gout     radiosity (ir flux) of the combined environment (sky+ground)
    //   Gin
    //   gap  vector of gap widths in meters
    //   height   IGU cavity height
    //   heightt
    //   thick    glazing layer thickness
    //   scon   Vector of conductivities of each glazing layer
    //   tilt   Window tilt (in degrees)
    //   theta  Vector of average temperatures
    //   Ebb
    //   Ebf
    //   iprop    array of gap mixtures
    //   frct     vector of mixture fractions
    //   presure
    //   hin   Indoor Indoor combined film coefficient (if non-zero)
    //   hout  Outdoor combined film coefficient (if non-zero)
    //   nmix  vector of number of gasses in a mixture for each gap
    // Ouputs
    //   hhat     vector of all film coefficients (maxlay3)
    //   hgas     vector of gap 'film' coeff.
    //   hcin  Indoor convective surface heat transfer coefficient
    //   hcout     Outdoor convective heat transfer coeff
    //   hrin    Indoor radiative surface heat transfer coefficient
    //   hrout   Outdoor radiative surface heat transfer coefficient
    //   hin   Indoor combined film coefficient
    //   hout  Outdoor combined film coefficient
    //   index    iteration step
    //   ibc
    // Inactives**
    //   wa - window azimuth (degrees, clockwise from south)

    // Locals
    int i;
    int k;
    int nface;

    // evaluate convective/conductive components of gap grashof number, thermal conductivity and their derivatives:
    nface = 2 * nlayer;

    filmg(state,
          tilt,
          theta,
          Tgap,
          nlayer,
          height,
          gap,
          iprop,
          frct,
          VacuumPressure,
          presure,
          nmix,
          wght,
          gcon,
          gvis,
          gcp,
          gama,
          hcgas,
          Ra,
          Nu,
          nperr,
          ErrorMessage);

    if (!(GoAhead(nperr))) {
        return;
    }

    // this is adding influence of pillar to hgas
    filmPillar(state, SupportPillar, scon, PillarSpacing, PillarRadius, nlayer, gap, hcgas, VacuumMaxGapThickness, nperr, ErrorMessage);

    if (!(GoAhead(nperr))) {
        return;
    }

    // adjust radiation coefficients
    // hrgas = 0.0d0
    for (i = 2; i <= nlayer; ++i) {
        k = 2 * i - 1;
        // if ((theta(k)-theta(k-1)) == 0) then
        //  theta(k-1) = theta(k-1) + tempCorrection
        // end if
        if ((theta(k) - theta(k - 1)) != 0) {
            hrgas(i) = (Radiation(k) - Radiation(k - 1)) / (theta(k) - theta(k - 1));
        }

        hgas(i) = hcgas(i) + hrgas(i);
    }

    // convective indoor film coeff:
    if (ibc(2) <= 0) {
        filmi(state,
              tind,
              theta(nface),
              nlayer,
              tilt,
              wsi,
              heightt,
              iprop,
              frct,
              presure,
              nmix,
              wght,
              gcon,
              gvis,
              gcp,
              hcin,
              ibc(2),
              nperr,
              ErrorMessage);
    } else if (ibc(2) == 1) {
        hcin = hin - hrin;
        // Simon: First iteration is with index = 0 and that means it should reenter iteration with whatever is provided as input
        // else if (ibc(2).eq.2.and.index.eq.1) then
    } else if ((ibc(2) == 2) && (index == 0)) {
        hcin = hin;
    }
    if (hcin < 0) {
        nperr = 8;
        ErrorMessage = "Hcin is less then zero.";
        return;
    }

    hcgas(nlayer + 1) = hcin;
    // hrin = 0.95d0*(Ebroom - Radiation(2*nlayer))/(Trmin-theta(2*nlayer))+0.05d0*hrin
    hrin = (ebroom - Radiation(2 * nlayer)) / (trmin - theta(2 * nlayer));
    // if ((Theta(2*nlayer) - Trmin).ne.0) then
    //  hrin =  sigma * emis(2*nlayer) * (Theta(2*nlayer)**4 - Trmin**4)/(Theta(2*nlayer) - Trmin)
    // else
    //  Theta(2*nlayer) = Theta(2*nlayer) + tempCorrection
    //  hrin =  sigma * emis(2*nlayer) * (Theta(2*nlayer)**4 - Trmin**4)/(Theta(2*nlayer) - Trmin)
    // end if
    hrgas(nlayer + 1) = hrin;
    // hgas(nlayer+1)  = hcgas(nlayer+1) + hrgas(nlayer+1)
    troom = (hcin * tind + hrin * trmin) / (hcin + hrin);

    // convective outdoor film coeff:
    if (ibc(1) <= 0) {
        film(tout, theta(1), wso, iwd, hcout, ibc(1));
    } else if (ibc(1) == 1) {
        hcout = hout - hrout;
        // Simon: First iteration is with index = 0 and that means it should reenter iteration with whatever is provided as input
        // else if (ibc(1).eq.2.and.index.eq.1) then
    } else if ((ibc(1) == 2) && (index == 0)) {
        hcout = hout;
    }
    if (hcout < 0) {
        nperr = 9;
        ErrorMessage = "Hcout is less than zero.";
        return;
    }

    hcgas(1) = hcout;
    hrout = (Radiation(1) - ebsky) / (theta(1) - trmout);
    // if ((Theta(1) - Trmout).ne.0) then
    //  hrout = sigma * emis(1) * (Theta(1)**4 - Trmout**4)/(Theta(1) - Trmout)
    // else
    //  Theta(1) = Theta(1) + tempCorrection
    //  hrout = sigma * emis(1) * (Theta(1)**4 - Trmout**4)/(Theta(1) - Trmout)
    // end if
    hrgas(1) = hrout;
    // hgas(1)  = hrout + hcout
    tamb = (hcout * tout + hrout * trmout) / (hcout + hrout);
}

void effectiveLayerCond(EnergyPlusData &state,
                        int const nlayer,
                        const Array1D<TARCOGLayerType> &LayerType, // Layer type
                        const Array1D<Real64> &scon,               // Layer thermal conductivity
                        const Array1D<Real64> &thick,              // Layer thickness
                        Array2A_int const iprop,                   // Gas type in gaps
                        Array2A<Real64> const frct,                // Fraction of gas
                        const Array1D_int &nmix,                   // Gas mixture
                        const Array1D<Real64> &pressure,           // Gas pressure [Pa]
                        const Array1D<Real64> &wght,               // Molecular weight
                        Array2A<Real64> const gcon,                // Gas specific conductivity
                        Array2A<Real64> const gvis,                // Gas specific viscosity
                        Array2A<Real64> const gcp,                 // Gas specific heat
                        const Array1D<Real64> &EffectiveOpenness,  // Layer effective openness [m2]
                        Array1D<Real64> const &theta,              // Layer surface temperatures [K]
                        Array1D<Real64> &sconScaled,               // Layer conductivity divided by thickness
                        int &nperr,                                // Error message flag
                        std::string &ErrorMessage                  // Error message
)
{
    for (int i = 1; i <= nlayer; ++i) {
        if (LayerType(i) != TARCOGLayerType::SPECULAR) {
            Real64 tLayer = (theta(2 * i - 1) + theta(2 * i)) / 2;
            Real64 nmix1 = nmix(i);
            Real64 press1 = (pressure(i) + pressure(i + 1)) / 2.0;
            for (int j = 1; j <= maxgas; ++j) {
                state.dataThermalISO15099Calc->iprop1(j) = iprop(j, i);
                state.dataThermalISO15099Calc->frct1(j) = frct(j, i);
            }

            Real64 con;
            Real64 visc;
            Real64 dens;
            Real64 cp;
            Real64 pr;
            GASSES90(state,
                     tLayer,
                     state.dataThermalISO15099Calc->iprop1,
                     state.dataThermalISO15099Calc->frct1,
                     press1,
                     nmix1,
                     wght,
                     gcon,
                     gvis,
                     gcp,
                     con,
                     visc,
                     dens,
                     cp,
                     pr,
                     TARCOGGassesParams::Stdrd::ISO15099,
                     nperr,
                     ErrorMessage);
            sconScaled(i) = (EffectiveOpenness(i) * con + (1 - EffectiveOpenness(i)) * scon(i)) / thick(i);
        } else {
            sconScaled(i) = scon(i) / thick(i);
        }
    }
}

void filmi(EnergyPlusData &state,
           Real64 const tair,
           Real64 const t,
           int const nlayer,
           Real64 const tilt,
           Real64 const wsi,
           Real64 const height,
           Array2A_int const iprop,
           Array2A<Real64> const frct,
           const Array1D<Real64> &presure,
           const Array1D_int &nmix,
           const Array1D<Real64> &wght,
           Array2A<Real64> const gcon,
           Array2A<Real64> const gvis,
           Array2A<Real64> const gcp,
           Real64 &hcin,
           int const ibc,
           int &nperr,
           std::string &ErrorMessage)
{
    //***********************************************************************
    //  purpose to evaluate heat flux at indoor surface of window using still air correlations (Curcija and Goss 1993)
    //  found in SPC142 equations 5.43 - 5.48.
    //***********************************************************************
    // Input
    //   tair - room air temperature
    //   t - inside surface temperature
    //   nlayer  number of glazing layers
    //   tilt - the tilt of the glazing in degrees
    //   wsi - room wind speed (m/s)
    //   height - window height
    //   iprop
    //   frct
    //   presure
    //   nmix  vector of number of gasses in a mixture for each gap
    // Output
    //   hcin - indoor convective heat transfer coeff

    // If there is forced air in the room than use SPC142 correlation 5.49 to calculate the room side film coefficient.

    // Using
    // Argument array dimensioning
    iprop.dim(maxgas, maxlay1);
    frct.dim(maxgas, maxlay1);
    EP_SIZE_CHECK(presure, maxlay1);
    EP_SIZE_CHECK(nmix, maxlay1);
    EP_SIZE_CHECK(wght, maxgas);
    gcon.dim(3, maxgas);
    gvis.dim(3, maxgas);
    gcp.dim(3, maxgas);

    // Locals
    int j;
    Real64 tiltr;
    Real64 tmean;
    Real64 delt;
    Real64 con;
    Real64 visc;
    Real64 dens;
    Real64 cp;
    Real64 pr;
    Real64 gr;
    Real64 RaCrit;
    Real64 RaL;
    Real64 Gnui(0.0);

    if (wsi > 0.0) { // main IF
        switch (ibc) {
        case 0: {
            hcin = 4.0 + 4.0 * wsi;
        } break;
        case -1: {
            hcin = 5.6 + 3.8 * wsi; // SPC142 correlation
            return;
        } break;
        default:
            break;
        }
    } else {                                       // main IF - else
        tiltr = tilt * 2.0 * Constant::Pi / 360.0; // convert tilt in degrees to radians
        tmean = tair + 0.25 * (t - tair);
        delt = std::abs(tair - t);

        for (j = 1; j <= nmix(nlayer + 1); ++j) {
            state.dataThermalISO15099Calc->ipropi(j) = iprop(j, nlayer + 1);
            state.dataThermalISO15099Calc->frcti(j) = frct(j, nlayer + 1);
        }

        GASSES90(state,
                 tmean,
                 state.dataThermalISO15099Calc->ipropi,
                 state.dataThermalISO15099Calc->frcti,
                 presure(nlayer + 1),
                 nmix(nlayer + 1),
                 wght,
                 gcon,
                 gvis,
                 gcp,
                 con,
                 visc,
                 dens,
                 cp,
                 pr,
                 TARCOGGassesParams::Stdrd::ISO15099,
                 nperr,
                 ErrorMessage);

        //   Calculate grashoff number:
        //   The grashoff number is the Rayleigh Number (equation 5.29) in SPC142 divided by the Prandtl Number (prand):
        gr = Constant::Gravity * pow_3(height) * delt * pow_2(dens) / (tmean * pow_2(visc));

        RaL = gr * pr;
        //   write(*,*)' RaCrit,RaL,gr,pr '
        //   write(*,*) RaCrit,RaL,gr,pr

        if ((0.0 <= tilt) && (tilt < 15.0)) { // IF no. 1
            Gnui = 0.13 * std::pow(RaL, 1.0 / 3.0);
        } else if ((15.0 <= tilt) && (tilt <= 90.0)) {
            //   if the room air is still THEN use equations 5.43 - 5.48:
            RaCrit = 2.5e5 * std::pow(std::exp(0.72 * tilt) / std::sin(tiltr), 0.2);
            if (RaL <= RaCrit) { // IF no. 2
                Gnui = 0.56 * root_4(RaL * std::sin(tiltr));
                // write(*,*) ' Nu ', Gnui
            } else {
                // Gnui = 0.13d0*(RaL**0.3333d0 - RaCrit**0.3333d0) + 0.56d0*(RaCrit*sin(tiltr))**0.25d0
                Gnui = 0.13 * (std::pow(RaL, 1.0 / 3.0) - std::pow(RaCrit, 1.0 / 3.0)) + 0.56 * root_4(RaCrit * std::sin(tiltr));
            } // end if no. 2
        } else if ((90.0 < tilt) && (tilt <= 179.0)) {
            Gnui = 0.56 * root_4(RaL * std::sin(tiltr));
        } else if ((179.0 < tilt) && (tilt <= 180.0)) {
            Gnui = 0.58 * std::pow(RaL, 1 / 3.0);
        } else {
            assert(false);
        } // end if no. 1
        //   write(*,*) ' RaL   ', RaL, '   RaCrit', RaCrit
        //   write(*,*)'   Nusselt Number   ',Gnui

        hcin = Gnui * (con / height);
        //   hin = 1.77d0*(ABS(t-tair))**0.25d0

    } // end main IF
}

void filmg(EnergyPlusData &state,
           Real64 const tilt,
           const Array1D<Real64> &theta,
           const Array1D<Real64> &Tgap,
           int const nlayer,
           Real64 const height,
           const Array1D<Real64> &gap,
           Array2A_int const iprop,
           Array2A<Real64> const frct,
           Real64 const VacuumPressure,
           const Array1D<Real64> &presure,
           const Array1D_int &nmix,
           const Array1D<Real64> &wght,
           Array2A<Real64> const gcon,
           Array2A<Real64> const gvis,
           Array2A<Real64> const gcp,
           const Array1D<Real64> &gama,
           Array1D<Real64> &hcgas,
           Array1D<Real64> &Rayleigh,
           Array1D<Real64> &Nu,
           int &nperr,
           std::string &ErrorMessage)
{
    //***********************************************************************
    // subroutine to calculate effective conductance of gaps
    //***********************************************************************
    // Inputs:
    //   tilt  window angle (deg)
    //   theta     vector of surface temperatures [K]
    //   nlayer    total number of glazing layers
    //   height    glazing cavity height
    //   gap   vector of gap widths [m]
    //   iprop
    //   frct
    //   presure
    //   nmix  vector of number of gasses in a mixture for each gap
    // Output:
    //   hgas  vector of gap coefficients
    //   nperr     error code
    // Locals:
    //   gr    gap grashof number
    //   con   gap gas conductivity
    //   visc  dynamic viscosity @ mean temperature [g/m*s]
    //   dens  density @ mean temperature [kg/m^3]
    //   cp    specific heat @ mean temperature [J/g*K]
    //   pr    gap gas Prandtl number
    //   tmean     average film temperature
    //   delt  temperature difference

    // Using
    // Argument array dimensioning
    EP_SIZE_CHECK(theta, maxlay2);
    EP_SIZE_CHECK(Tgap, maxlay1);
    EP_SIZE_CHECK(gap, MaxGap);
    iprop.dim(maxgas, maxlay1);
    frct.dim(maxgas, maxlay1);
    EP_SIZE_CHECK(presure, maxlay1);
    EP_SIZE_CHECK(nmix, maxlay1);
    EP_SIZE_CHECK(wght, maxgas);
    gcon.dim(3, maxgas);
    gvis.dim(3, maxgas);
    gcp.dim(3, maxgas);
    EP_SIZE_CHECK(gama, maxgas);
    EP_SIZE_CHECK(hcgas, maxlay1);
    EP_SIZE_CHECK(Rayleigh, maxlay);
    EP_SIZE_CHECK(Nu, maxlay);

    // Locals
    Real64 con;
    Real64 visc;
    Real64 dens;
    Real64 cp;
    Real64 pr;
    Real64 delt;
    Real64 tmean;
    Real64 ra;
    Real64 asp;
    Real64 gnu;
    int i;
    int j;
    int k;
    int l;

    hcgas = 0.0;

    for (i = 1; i <= nlayer - 1; ++i) {
        j = 2 * i;
        k = j + 1;
        // determine the gas properties of each gap:
        // tmean = (theta(j)+theta(k))/2.0d0
        tmean = Tgap(i + 1); // Tgap(1) is exterior environment
        delt = std::abs(theta(j) - theta(k));
        // Temperatures should not be equal. This can happen in initial temperature guess before iterations started
        if (delt == 0.0) {
            delt = 1.0e-6;
        }
        for (l = 1; l <= nmix(i + 1); ++l) {
            state.dataThermalISO15099Calc->ipropg(l) = iprop(l, i + 1);
            state.dataThermalISO15099Calc->frctg(l) = frct(l, i + 1);
        }

        if (presure(i + 1) > VacuumPressure) {
            GASSES90(state,
                     tmean,
                     state.dataThermalISO15099Calc->ipropg,
                     state.dataThermalISO15099Calc->frctg,
                     presure(i + 1),
                     nmix(i + 1),
                     wght,
                     gcon,
                     gvis,
                     gcp,
                     con,
                     visc,
                     dens,
                     cp,
                     pr,
                     TARCOGGassesParams::Stdrd::ISO15099,
                     nperr,
                     ErrorMessage);

            // Calculate grashoff number:
            // The grashoff number is the Rayleigh Number (equation 5.29) in SPC142 divided by the Prandtl Number (prand):
            ra = Constant::Gravity * pow_3(gap(i)) * delt * cp * pow_2(dens) / (tmean * visc * con);
            Rayleigh(i) = ra;
            // write(*,*) 'height,gap(i),asp',height,gap(i),asp
            // asp = 1
            // if (gap(i).ne.0) then
            asp = height / gap(i);
            // end if
            // determine the Nusselt number:
            nusselt(tilt, ra, asp, gnu, nperr, ErrorMessage);

            Nu(i) = gnu;
            // calculate effective conductance of the gap
            hcgas(i + 1) = con / gap(i) * gnu;

            // write(*,*)'theta(j),theta(k),j,k',j,theta(j),k,theta(k)
            // write(*,*)'Nusselt,Rayleigh,Prandtl,hgas(k),k'
            // write(*,*) gnu,gr*pr,pr,hgas(k),k
        } else { // low pressure calculations
            GassesLow(tmean, wght(iprop(1, i + 1)), presure(i + 1), gama(iprop(1, i + 1)), con, nperr, ErrorMessage);
            hcgas(i + 1) = con;
        } // if (pressure(i+1).gt.VacuumPressure) then
    }
}

void filmPillar(EnergyPlusData &state,
                const Array1D_int &SupportPillar,     // Shows whether or not gap have support pillar
                const Array1D<Real64> &scon,          // Conductivity of glass layers
                const Array1D<Real64> &PillarSpacing, // Pillar spacing for each gap (used in case there is support pillar)
                const Array1D<Real64> &PillarRadius,  // Pillar radius for each gap (used in case there is support pillar)
                int const nlayer,
                const Array1D<Real64> &gap,
                Array1D<Real64> &hcgas,
                [[maybe_unused]] Real64 const VacuumMaxGapThickness,
                [[maybe_unused]] int &nperr,
                [[maybe_unused]] std::string &ErrorMessage)
{
    //***********************************************************************
    // subroutine to calculate effective conductance of support pillars
    //***********************************************************************

    // Using
    // Argument array dimensioning
    EP_SIZE_CHECK(SupportPillar, maxlay);
    EP_SIZE_CHECK(scon, maxlay);
    EP_SIZE_CHECK(PillarSpacing, maxlay);
    EP_SIZE_CHECK(PillarRadius, maxlay);
    EP_SIZE_CHECK(gap, MaxGap);
    EP_SIZE_CHECK(hcgas, maxlay1);

    // Locals
    //   0 - does not have support pillar
    //   1 - have support pillar

    for (state.dataThermalISO15099Calc->iFP = 1; state.dataThermalISO15099Calc->iFP <= nlayer - 1; ++state.dataThermalISO15099Calc->iFP) {
        state.dataThermalISO15099Calc->kFP = 2 * state.dataThermalISO15099Calc->iFP + 1;
        if (SupportPillar(state.dataThermalISO15099Calc->iFP) == YES_SupportPillar) {

            // Average glass conductivity is taken as average from both glass surrounding gap
            state.dataThermalISO15099Calc->aveGlassConductivity =
                (scon(state.dataThermalISO15099Calc->iFP) + scon(state.dataThermalISO15099Calc->iFP + 1)) / 2;

            state.dataThermalISO15099Calc->cpa =
                2.0 * state.dataThermalISO15099Calc->aveGlassConductivity * PillarRadius(state.dataThermalISO15099Calc->iFP) /
                (pow_2(PillarSpacing(state.dataThermalISO15099Calc->iFP)) *
                 (1.0 + 2.0 * gap(state.dataThermalISO15099Calc->iFP) / (Constant::Pi * PillarRadius(state.dataThermalISO15099Calc->iFP))));

            // It is important to add on previous values calculated for gas
            hcgas(state.dataThermalISO15099Calc->iFP + 1) += state.dataThermalISO15099Calc->cpa;
        } // if (SupportPillar(i).eq.YES_SupportPillar) then
    }
}

void nusselt(Real64 const tilt, Real64 const ra, Real64 const asp, Real64 &gnu, int &nperr, std::string &ErrorMessage)
{
    //***********************************************************************
    // purpose to calculate nusselt modulus for air gaps (ISO15099)
    //***********************************************************************
    // Input
    //   tilt   tilt in degrees
    //   ra     rayleigh number
    //   asp    Aspect ratio
    // Output
    //   gnu    nusselt number
    //   nperr

    // Using
    // Locals
    Real64 subNu1;
    Real64 subNu2;
    Real64 subNu3;
    Real64 Nu1;
    Real64 Nu2;
    Real64 G;
    Real64 Nu60;
    Real64 Nu90;
    Real64 tiltr;

    subNu1 = 0.0;
    subNu2 = 0.0;
    subNu3 = 0.0;
    Nu1 = 0.0;
    Nu2 = 0.0;
    Nu90 = 0.0;
    Nu60 = 0.0;
    G = 0.0;
    tiltr = tilt * 2.0 * Constant::Pi / 360.0; // convert tilt in degrees to radians
    if ((tilt >= 0.0) && (tilt < 60.0)) {      // ISO/DIS 15099 - chapter 5.3.3.1
        subNu1 = 1.0 - 1708.0 / (ra * std::cos(tiltr));
        subNu1 = pos(subNu1);
        subNu2 = 1.0 - (1708.0 * std::pow(std::sin(1.8 * tiltr), 1.6)) / (ra * std::cos(tiltr));
        subNu3 = std::pow(ra * std::cos(tiltr) / 5830.0, 1.0 / 3.0) - 1.0;
        subNu3 = pos(subNu3);
        gnu = 1.0 + 1.44 * subNu1 * subNu2 + subNu3; // equation 42
        if (ra >= 1.0e5) {
            nperr = 1001; // Rayleigh number is out of range
            ErrorMessage = "Rayleigh number out of range in Nusselt num. calc. for gaps (angle between 0 and 60 deg).";
        }
        if (asp <= 20.0) {
            nperr = 1002; // Aspect Ratio is out of range
            ErrorMessage = "Aspect Ratio out of range in Nusselt num. calc. for gaps (angle between 0 and 60 deg).";
        }
    } else if (tilt == 60.0) {                                                              // ISO/DIS 15099 - chapter 5.3.3.2
        G = 0.5 / std::pow(1.0 + std::pow(ra / 3160.0, 20.6), 0.1);                         // equation 47
        Nu1 = std::pow(1.0 + pow_7((0.0936 * std::pow(ra, 0.314)) / (1.0 + G)), 0.1428571); // equation 45
        Nu2 = (0.104 + 0.175 / asp) * std::pow(ra, 0.283);                                  // equation 46
        gnu = max(Nu1, Nu2);                                                                // equation 44
    } else if ((tilt > 60.0) && (tilt < 90.0)) {                                            // ISO/DIS 15099 - chapter 5.3.3.3
        if ((ra > 100.0) && (ra < 2.0e7) && (asp > 5.0) && (asp < 100.0)) {
            G = 0.5 / std::pow(1.0 + std::pow(ra / 3160.0, 20.6), 0.1);                         // equation 47
            Nu1 = std::pow(1.0 + pow_7((0.0936 * std::pow(ra, 0.314)) / (1.0 + G)), 0.1428571); // equation 45
            Nu2 = (0.104 + 0.175 / asp) * std::pow(ra, 0.283);                                  // equation 46
            Nu60 = max(Nu1, Nu2);                                                               // equation 44
            Nu2 = 0.242 * std::pow(ra / asp, 0.272);                                            // equation 52
            if (ra > 5.0e4) {
                Nu1 = 0.0673838 * std::pow(ra, 1.0 / 3.0); // equation 49
            } else if ((ra > 1.0e4) && (ra <= 5.0e4)) {
                Nu1 = 0.028154 * std::pow(ra, 0.4134); // equation 50
            } else if (ra <= 1.0e4) {
                Nu1 = 1.0 + 1.7596678e-10 * std::pow(ra, 2.2984755); // equation 51
            }
        } else if (ra <= 100.0) {
            G = 0.5 / std::pow(1.0 + std::pow(ra / 3160.0, 20.6), 0.1);                         // equation 47
            Nu1 = std::pow(1.0 + pow_7((0.0936 * std::pow(ra, 0.314)) / (1.0 + G)), 0.1428571); // equation 45
            Nu2 = (0.104 + 0.175 / asp) * std::pow(ra, 0.283);                                  // equation 46
            Nu60 = max(Nu1, Nu2);                                                               // equation 44
            Nu2 = 0.242 * std::pow(ra / asp, 0.272);                                            // equation 52
            Nu1 = 1.0 + 1.7596678e-10 * std::pow(ra, 2.2984755);                                // equation 51
            nperr = 1003;                                                                       // Rayleigh number is less than 100
            ErrorMessage = "Rayleigh number is less than 100 in Nusselt number calculations for gaps (angle between 60 and 90 degrees).";
        } else if (ra > 2.0e7) {
            G = 0.5 / std::pow(1.0 + std::pow(ra / 3160.0, 20.6), 0.1);                         // equation 47
            Nu1 = std::pow(1.0 + pow_7((0.0936 * std::pow(ra, 0.314)) / (1.0 + G)), 0.1428571); // equation 45
            Nu2 = (0.104 + 0.175 / asp) * std::pow(ra, 0.283);                                  // equation 46
            Nu60 = max(Nu1, Nu2);                                                               // equation 44
            Nu2 = 0.242 * std::pow(ra / asp, 0.272);                                            // equation 52
            Nu1 = 0.0673838 * std::pow(ra, 1.0 / 3.0);                                          // equation 49
            nperr = 1004;                                                                       // Rayleigh number is great from 2e7
            ErrorMessage = "Rayleigh number is greater than 2e7 in Nusselt number calculations for gaps (angle between 60 and 90 degrees).";
        } else if ((asp <= 5.0) || (asp >= 100.0)) {
            G = 0.5 / std::pow(1.0 + std::pow(ra / 3160.0, 20.6), 0.1);                         // equation 47
            Nu1 = std::pow(1.0 + pow_7((0.0936 * std::pow(ra, 0.314)) / (1.0 + G)), 0.1428571); // equation 45
            Nu2 = (0.104 + 0.175 / asp) * std::pow(ra, 0.283);                                  // equation 46
            Nu60 = max(Nu1, Nu2);                                                               // equation 44
            Nu2 = 0.242 * std::pow(ra / asp, 0.272);                                            // equation 52
            if (ra > 5.0e4) {
                Nu1 = 0.0673838 * std::pow(ra, 1.0 / 3.0); // equation 49
            } else if ((ra > 1.0e4) && (ra <= 5.0e4)) {
                Nu1 = 0.028154 * std::pow(ra, 0.4134); // equation 50
            } else if (ra <= 1.0e4) {
                Nu1 = 1.0 + 1.7596678e-10 * std::pow(ra, 2.2984755); // equation 51
            }
            nperr = 1005; // Aspect Ratio is out of range
            ErrorMessage = "Aspect Ratio is out of range in Nusselt number calculations for gaps (angle between 60 and 90 degrees).";
        }
        Nu90 = max(Nu1, Nu2);                                         // equation 48
        gnu = ((Nu90 - Nu60) / (90.0 - 60.0)) * (tilt - 60.0) + Nu60; // linear interpolation between 60 and 90 degrees
    } else if (tilt == 90.0) {                                        // ISO/DIS 15099 - chapter 5.3.3.4
        Nu2 = 0.242 * std::pow(ra / asp, 0.272);                      // equation 52
        if (ra > 5.0e4) {
            Nu1 = 0.0673838 * std::pow(ra, 1.0 / 3.0); // equation 49
        } else if ((ra > 1.0e4) && (ra <= 5.0e4)) {
            Nu1 = 0.028154 * std::pow(ra, 0.4134); // equation 50
            // Nu1 = 0.028154d0 * ra ** 0.414d0                       !equation 50 - DISCONTINUITY CORRECTED
        } else if (ra <= 1.0e4) {
            Nu1 = 1.0 + 1.7596678e-10 * std::pow(ra, 2.2984755); // equation 51
        }
        gnu = max(Nu1, Nu2); // equation 48
    } else if ((tilt > 90.0) && (tilt <= 180.0)) {
        Nu2 = 0.242 * std::pow(ra / asp, 0.272); // equation 52
        if (ra > 5.0e4) {
            Nu1 = 0.0673838 * std::pow(ra, 1.0 / 3.0); // equation 49
        } else if ((ra > 1.0e4) && (ra <= 5.0e4)) {
            Nu1 = 0.028154 * std::pow(ra, 0.4134); // equation 50
        } else if (ra <= 1.0e4) {
            Nu1 = 1.0 + 1.7596678e-10 * std::pow(ra, 2.2984755); // equation 51
        }
        gnu = max(Nu1, Nu2);                       // equation 48
        gnu = 1.0 + (gnu - 1.0) * std::sin(tiltr); // equation 53
    } else {
        nperr = 10; // error flag: angle is out of range
        ErrorMessage = "Window tilt angle is out of range.";
        return;
    }
}

void storeIterationResults(EnergyPlusData &state,
                           Files &files,
                           int const nlayer,
                           int const index,
                           const Array1D<Real64> &theta,
                           Real64 const trmout,
                           Real64 const tamb,
                           Real64 const trmin,
                           Real64 const troom,
                           Real64 const ebsky,
                           Real64 const ebroom,
                           Real64 const hcin,
                           Real64 const hcout,
                           Real64 const hrin,
                           Real64 const hrout,
                           Real64 const hin,
                           Real64 const hout,
                           const Array1D<Real64> &Ebb,
                           const Array1D<Real64> &Ebf,
                           const Array1D<Real64> &Rb,
                           const Array1D<Real64> &Rf,
                           int &)
{
    auto &dynFormat = state.dataThermalISO15099Calc->dynFormat;
    int i;

    // write(a,1000) index
    print(files.TarcogIterationsFile, "*************************************************************************************************\n");
    print(files.TarcogIterationsFile, "Iteration number: {:5}\n", index);

    print(files.TarcogIterationsFile, "Trmin = {:8.4F}\n", trmin - Constant::Kelvin);
    print(files.TarcogIterationsFile, "Troom = {:12.6F}\n", troom - Constant::Kelvin);
    print(files.TarcogIterationsFile, "Trmout = {:8.4F}\n", trmout - Constant::Kelvin);
    print(files.TarcogIterationsFile, "Tamb = {:12.6F}\n", tamb - Constant::Kelvin);

    print(files.TarcogIterationsFile, "Ebsky = {:8.4F}\n", ebsky);
    print(files.TarcogIterationsFile, "Ebroom = {:8.4F}\n", ebroom);

    print(files.TarcogIterationsFile, "hcin = {:8.4F}\n", hcin);
    print(files.TarcogIterationsFile, "hcout = {:8.4F}\n", hcout);
    print(files.TarcogIterationsFile, "hrin = {:8.4F}\n", hrin);
    print(files.TarcogIterationsFile, "hrout = {:8.4F}\n", hrout);
    print(files.TarcogIterationsFile, "hin = {:8.4F}\n", hin);
    print(files.TarcogIterationsFile, "hout = {:8.4F}\n", hout);

    // Write headers for Ebb and Ebf
    for (i = 1; i <= 2 * nlayer; ++i) {
        if (i == 1) {
            dynFormat = "";
        }
        if (mod(i, 2) == 1) {
            dynFormat += fmt::format("Ebf({:3})", (i + 1) / 2);
        } else {
            dynFormat += fmt::format("Ebb({:3})", (i + 1) / 2);
        }
        if (i != 2 * nlayer) {
            dynFormat += "===";
        }
    }
    print(files.TarcogIterationsFile, dynFormat);
    print(files.TarcogIterationsFile, "\n");

    // write Ebb and Ebf
    print(files.TarcogIterationsFile, "{:16.8F}   {:16.8F}", Ebf(1), Ebb(1));
    for (i = 2; i <= nlayer; ++i) {
        print(files.TarcogIterationsFile, "   {:16.8F}   {:16.8F}", Ebf(i), Ebb(i));
    }
    print(files.TarcogIterationsFile, "\n");

    // Write headers for Rb and Rf
    for (i = 1; i <= 2 * nlayer; ++i) {
        const std::string a = fmt::format("{:3}", (i + 1) / 2); // this is just to simulate correct integer in brackets
        if (i == 1) {
            dynFormat = "";
        }
        if (mod(i, 2) == 1) {
            dynFormat += "Rf(" + a + ')';
        } else {
            dynFormat += "Rb(" + a + ')';
        }
        if (i != 2 * nlayer) {
            dynFormat += "===";
        }
    }
    print(files.TarcogIterationsFile, dynFormat);
    print(files.TarcogIterationsFile, "\n");
    // write Rb and Rf
    print(files.TarcogIterationsFile, "{:16.8F}   {:16.8F}", Rf(1), Rb(1));
    for (i = 1; i <= nlayer; ++i) {
        print(files.TarcogIterationsFile, "   {:16.8F}   {:16.8F}", Rf(i), Rb(i));
    }
    print(files.TarcogIterationsFile, "\n");

    // Write header for temperatures
    for (i = 1; i <= 2 * nlayer; ++i) {
        const std::string a = fmt::format("{:3}", i);
        if (i == 1) {
            dynFormat = "";
        }
        dynFormat += "theta(" + a + ')';
        if (i != (2 * nlayer)) {
            dynFormat += "==";
        }
    }
    print(files.TarcogIterationsFile, dynFormat);
    print(files.TarcogIterationsFile, "\n");

    // write temperatures
    print(files.TarcogIterationsFile, "{:16.8F}   \n", theta(1) - Constant::Kelvin);
    for (i = 2; i <= 2 * nlayer; ++i) {
        print(files.TarcogIterationsFile, "   {:16.8F}   \n", theta(i) - Constant::Kelvin);
    }
    print(files.TarcogIterationsFile, "\n");

    // close(TarcogIterationsFileNumber)

    // write results in csv file
    if (index == 0) {
        dynFormat = "  ";
        for (i = 1; i <= 2 * nlayer; ++i) {
            const std::string a = fmt::format("{:3}", i);
            if (i != 2 * nlayer) {
                dynFormat += "theta(" + a + "),";
            } else {
                dynFormat += "theta(" + a + ')';
            }
        }
        print(files.IterationCSVFile, dynFormat);
        print(files.IterationCSVFile, "\n");
    }
    print(files.IterationCSVFile, "{:16.8F}   \n", theta(1) - Constant::Kelvin);
    for (i = 2; i <= 2 * nlayer; ++i) {
        print(files.IterationCSVFile, "   {:16.8F}   \n", theta(i) - Constant::Kelvin);
    }
    print(files.IterationCSVFile, "\n");

    // close(IterationCSVFileNumber)
}

void CalculateFuncResults(int const nlayer, Array2<Real64> const &a, const Array1D<Real64> &b, const Array1D<Real64> &x, Array1D<Real64> &FRes)
{
    // Tuned Rewritten to traverse a in unit stride order
    int const nlayer4(4 * nlayer);
    for (int i = 1; i <= nlayer4; ++i) {
        FRes(i) = -b(i);
    }
    for (int j = 1; j <= nlayer4; ++j) {
        Real64 const x_j(x(j));
        for (int i = 1; i <= nlayer4; ++i) {
            FRes(i) += a(j, i) * x_j;
        }
    }
}

} // namespace EnergyPlus::ThermalISO15099Calc
