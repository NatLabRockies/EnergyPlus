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

#include <EnergyPlus/DataGlobals.hh>

// EnergyPlus Headers
#include <EnergyPlus/TARCOGArgs.hh>
#include <EnergyPlus/TARCOGCommon.hh>
#include <EnergyPlus/TARCOGGassesParams.hh>
#include <EnergyPlus/TARCOGParams.hh>

namespace EnergyPlus::TARCOGArgs {

// MODULE INFORMATION:
//       AUTHOR         Simon Vidanovic
//       DATE WRITTEN   June/22/2010
//       MODIFIED       na
//       RE-ENGINEERED  na
//  Revision: 6.0.36  (June/22/2010)
//   - Initial setup, extracted from TARCOG.for

// PURPOSE OF THIS MODULE:
// A module which contains common functions for error checking and
//    preparation of arguments and intermediate variables

// Using/Aliasing
using namespace TARCOGCommon;
using namespace TARCOGGassesParams;
using namespace TARCOGOutput;
using namespace TARCOGParams;

int ArgCheck(EnergyPlusData &state,
             Files &files,
             int const nlayer,
             int const iwd,
             Real64 const tout,
             Real64 const tind,
             Real64 const trmin,
             Real64 const wso,
             Real64 const wsi,
             Real64 const dir,
             Real64 const outir,
             int const isky,
             Real64 const tsky,
             Real64 const esky,
             Real64 const fclr,
             Real64 const VacuumPressure,
             Real64 const VacuumMaxGapThickness,
             DeflectionCalculation const CalcDeflection,
             Real64 const Pa,
             Real64 const Pini,
             Real64 const Tini,
             const Array1D<Real64> &gap,
             const Array1D<Real64> &GapDef,
             const Array1D<Real64> &thick,
             const Array1D<Real64> &scon,
             const Array1D<Real64> &YoungsMod,
             const Array1D<Real64> &PoissonsRat,
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
             const Array1D_int &SupportPillar,     // Shows whether or not gap have support pillar
             const Array1D<Real64> &PillarSpacing, // Pillar spacing for each gap (used in case there is support pillar)
             const Array1D<Real64> &PillarRadius,  // Pillar radius for each gap (used in case there is support pillar)
             Real64 &hin,
             Real64 &hout,
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
             const Array1D<TARCOGParams::TARCOGLayerType> &LayerType,
             const Array1D_int &nslice,
             const Array1D<Real64> &LaminateA,
             const Array1D<Real64> &LaminateB,
             const Array1D<Real64> &sumsol,
             TARCOGGassesParams::Stdrd const standard,
             TARCOGThermalModel const ThermalMod,
             Real64 const SDScalar,
             std::string &ErrorMessage)
{

    // Return value
    int ArgCheck;

    // Argument array dimensioning
    EP_SIZE_CHECK(gap, maxlay);
    EP_SIZE_CHECK(GapDef, MaxGap);
    EP_SIZE_CHECK(thick, maxlay);
    EP_SIZE_CHECK(scon, maxlay);
    EP_SIZE_CHECK(YoungsMod, maxlay);
    EP_SIZE_CHECK(PoissonsRat, maxlay);
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

    // bi...Write debug output files - if debug flag = 1:

    if (files.WriteDebugOutput) {

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
                            hout,
                            hin,
                            standard,
                            ThermalMod,
                            SDScalar,
                            height,
                            heightt,
                            width,
                            tilt,
                            totsol,
                            nlayer,
                            LayerType,
                            thick,
                            scon,
                            asol,
                            tir,
                            emis,
                            Atop,
                            Abot,
                            Al,
                            Ar,
                            Ah,
                            SlatThick,
                            SlatWidth,
                            SlatAngle,
                            SlatCond,
                            SlatSpacing,
                            SlatCurve,
                            nslice,
                            LaminateA,
                            LaminateB,
                            sumsol,
                            gap,
                            vvent,
                            tvent,
                            presure,
                            nmix,
                            iprop,
                            frct,
                            xgcon,
                            xgvis,
                            xgcp,
                            xwght);

        std::string const VersionNumber(" 7.0.15.00 ");
        WriteTARCOGInputFile(state,
                             files,
                             VersionNumber,
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
                             CalcDeflection,
                             Pa,
                             Pini,
                             Tini,
                             ibc,
                             hout,
                             hin,
                             standard,
                             ThermalMod,
                             SDScalar,
                             height,
                             heightt,
                             width,
                             tilt,
                             totsol,
                             nlayer,
                             LayerType,
                             thick,
                             scon,
                             YoungsMod,
                             PoissonsRat,
                             asol,
                             tir,
                             emis,
                             Atop,
                             Abot,
                             Al,
                             Ar,
                             Ah,
                             SupportPillar,
                             PillarSpacing,
                             PillarRadius,
                             SlatThick,
                             SlatWidth,
                             SlatAngle,
                             SlatCond,
                             SlatSpacing,
                             SlatCurve,
                             nslice,
                             gap,
                             GapDef,
                             vvent,
                             tvent,
                             presure,
                             nmix,
                             iprop,
                             frct,
                             xgcon,
                             xgvis,
                             xgcp,
                             xwght,
                             gama);

    } // if debug=1 - write dbg output file

    // bi...assume All OK
    ArgCheck = 0;

    // dr...check for error messages
    if (nlayer < 1) {
        ArgCheck = 17;
        ErrorMessage = "Number of layers must be >0.";
        return ArgCheck;
    }

    if ((static_cast<int>(standard) < MinStandard) || (static_cast<int>(standard) > MaxStandard)) {
        ArgCheck = 28;
        ErrorMessage = "Invalid code for standard.";
        return ArgCheck;
    }

    if ((ThermalMod != TARCOGThermalModel::ISO15099) && (ThermalMod != TARCOGThermalModel::SCW) && (ThermalMod != TARCOGThermalModel::CSM)) {
        ArgCheck = 29;
        ErrorMessage = "Invalid code for thermal mode.";
        return ArgCheck;
    }

    if ((iwd != 0) && (iwd != 1)) {
        ArgCheck = 18;
        ErrorMessage = "Wind direction can be windward (=0) or leeward (=1).";
        return ArgCheck;
    }

    if ((fclr < 0.0) || (fclr > 1.0)) {
        ArgCheck = 19;
        ErrorMessage = "Fraction of sky that is clear can be in range between 0 and 1.";
        return ArgCheck;
    }

    for (int i = 1; i <= nlayer - 1; ++i) {
        if (gap(i) <= 0.0) {
            ArgCheck = 20;
            ErrorMessage = format("Gap width is less than (or equal to) zero. Gap #{:3}", i);
            return ArgCheck;
        }
    }

    for (int i = 1; i <= nlayer; ++i) {
        if (thick(i) <= 0.0) {
            ArgCheck = 21;
            ErrorMessage = format("Layer width is less than (or equal to) zero. Layer #{:3}", i);
            return ArgCheck;
        }
        if ((i < nlayer) && IsShadingLayer(LayerType(i)) && IsShadingLayer(LayerType(i + 1))) {
            ArgCheck = 37;
            ErrorMessage = "Cannot handle two consecutive shading layers.";
            return ArgCheck;
        }
        // Deflection cannot be calculated with IGU containing shading layer. This error check is to be
        // removed once that extension is programmed
        if ((CalcDeflection != TARCOGParams::DeflectionCalculation::NONE) && (LayerType(i) != TARCOGParams::TARCOGLayerType::SPECULAR)) {
            ArgCheck = 42;
            ErrorMessage = "Cannot calculate deflection with IGU containing shading devices.";
            return ArgCheck;
        }
    }

    if (height <= 0.0) {
        ArgCheck = 23;
        ErrorMessage = "IGU cavity height must be greater than zero.";
        return ArgCheck;
    }

    if (heightt <= 0.0) {
        ArgCheck = 24;
        ErrorMessage = "Total window height must be greater than zero.";
        return ArgCheck;
    }

    if (width <= 0.0) {
        ArgCheck = 25;
        ErrorMessage = "Window width must be greater than zero.";
        return ArgCheck;
    }

    if ((SDScalar < 0.0) || (SDScalar > 1.0)) {
        ArgCheck = 30;
        ErrorMessage = "SDscalar is out of range (<0.0 or >1.0).";
        return ArgCheck;
    }

    // bi...Check layers and update Venetian blinds properties:
    for (int i = 1; i <= nlayer; ++i) {
        if (scon(i) <= 0.0) {
            ArgCheck = 26;
            ErrorMessage = format("Layer {:3} has conductivity which is less or equal to zero.", i);
            return ArgCheck;
        }

        if (LayerType(i) != TARCOGLayerType::SPECULAR && LayerType(i) != TARCOGLayerType::WOVSHADE &&
            LayerType(i) != TARCOGLayerType::VENETBLIND_HORIZ && LayerType(i) != TARCOGLayerType::PERFORATED &&
            LayerType(i) != TARCOGLayerType::DIFFSHADE && LayerType(i) != TARCOGLayerType::BSDF && LayerType(i) != TARCOGLayerType::VENETBLIND_VERT)

        {
            ArgCheck = 22;
            ErrorMessage = format("Incorrect layer type for layer #{:3}"
                                  ".  Layer type can either be 0 (glazing layer), 1 (Venetian blind), 2 (woven shade), 3 (perforated), 4 (diffuse "
                                  "shade) or 5 (bsdf).",
                                  i);
            return ArgCheck;
        }

        // bi...TEMPORARY! Don't allow CSW and CSM method for outdoor and indoor SD layers
        if ((IsShadingLayer(LayerType(1))) && ((ThermalMod == TARCOGThermalModel::SCW) || (ThermalMod == TARCOGThermalModel::CSM))) {
            ArgCheck = 39;
            ErrorMessage = "CSM and SCW thermal models cannot be used for outdoor and indoor SD layers.";
            return ArgCheck;
        }
        if ((IsShadingLayer(LayerType(nlayer))) && ((ThermalMod == TARCOGThermalModel::SCW) || (ThermalMod == TARCOGThermalModel::CSM))) {
            ArgCheck = 39;
            ErrorMessage = "CSM and SCW thermal models cannot be used for outdoor and indoor SD layers.";
            return ArgCheck;
        }

        if (LayerType(i) == TARCOGParams::TARCOGLayerType::VENETBLIND_HORIZ ||
            LayerType(i) == TARCOGParams::TARCOGLayerType::VENETBLIND_VERT) { // Venetian blind specific:
            if (SlatThick(i) <= 0) {
                ArgCheck = 31;
                ErrorMessage = format("Invalid slat thickness (must be >0). Layer #{:3}", i);
                return ArgCheck;
            }
            if (SlatWidth(i) <= 0.0) {
                ArgCheck = 32;
                ErrorMessage = format("Invalid slat width (must be >0). Layer #{:3}", i);
                return ArgCheck;
            }
            if ((SlatAngle(i) < -90.0) || (SlatAngle(i) > 90.0)) {
                ArgCheck = 33;
                ErrorMessage = format("Invalid slat angle (must be between -90 and 90). Layer #{:3}", i);
                return ArgCheck;
            }
            if (SlatCond(i) <= 0.0) {
                ArgCheck = 34;
                ErrorMessage = format("Invalid conductivity of slat material (must be >0). Layer #{:3}", i);
                return ArgCheck;
            }
            if (SlatSpacing(i) <= 0.0) {
                ArgCheck = 35;
                ErrorMessage = format("Invalid slat spacing (must be >0). Layer #{:3}", i);
                return ArgCheck;
            }
            if ((SlatCurve(i) != 0.0) && (std::abs(SlatCurve(i)) <= (SlatWidth(i) / 2.0))) {
                ArgCheck = 36;
                ErrorMessage = format("Invalid curvature radius (absolute value must be >SlatWidth/2, or 0 for flat slats). Layer #{:3}", i);
                return ArgCheck;
            }

        } //  LayerType is Venetian

    } // Layers...

    for (int i = 1; i <= nlayer + 1; ++i) {
        if (presure(i) < 0.0) {
            ArgCheck = 27;
            if ((i == 1) || (i == (nlayer + 1))) {
                ErrorMessage = "One of environments (inside or outside) has pressure which is less than zero.";
            } else {
                ErrorMessage = format("One of gaps has pressure which is less than zero. Gap #{:3}", i);
            }
            return ArgCheck;
        }
    }

    return ArgCheck;
}

void PrepVariablesISO15099(int const nlayer,
                           Real64 const tout,
                           Real64 const tind,
                           Real64 &trmin,
                           int const isky,
                           Real64 const outir,
                           // IR radiance of window's exterior/interior surround (W/m2)
                           Real64 const tsky,
                           Real64 &esky,
                           Real64 const fclr,
                           Array1D<Real64> &gap,
                           Array1D<Real64> &thick,
                           Array1D<Real64> &scon,
                           const Array1D<Real64> &tir,
                           const Array1D<Real64> &emis,
                           Real64 const tilt,
                           Real64 &hin,
                           Real64 &hout,
                           const Array1D_int &ibc,
                           const Array1D<Real64> &SlatThick,
                           const Array1D<Real64> &SlatWidth,
                           const Array1D<Real64> &SlatAngle,
                           const Array1D<Real64> &SlatCond,
                           const Array1D<TARCOGLayerType> &LayerType,
                           TARCOGThermalModel const ThermalMod,
                           Real64 const SDScalar,
                           Real64 &ShadeEmisRatioOut,
                           Real64 &ShadeEmisRatioIn,
                           Real64 &ShadeHcRatioOut,
                           Real64 &ShadeHcRatioIn,
                           Array1D<Real64> &Keff,
                           Array1D<Real64> &ShadeGapKeffConv,
                           Real64 &sc,
                           Real64 &shgc,
                           Real64 &ufactor,
                           Real64 &flux,
                           Array1D<Real64> &LaminateAU,
                           Array1D<Real64> &sumsolU,
                           Array1D<Real64> &sol0,
                           Real64 &hint,
                           Real64 &houtt,
                           Real64 &trmout,
                           Real64 &ebsky,
                           Real64 &ebroom,
                           Real64 &Gout,
                           Real64 &Gin,
                           Array1D<Real64> &rir,
                           Array1D<Real64> &vfreevent,
                           int &nperr,
                           std::string &ErrorMessage)
{

    // Argument array dimensioning
    EP_SIZE_CHECK(gap, MaxGap);
    EP_SIZE_CHECK(thick, maxlay);
    EP_SIZE_CHECK(scon, maxlay);
    EP_SIZE_CHECK(tir, maxlay2);
    EP_SIZE_CHECK(emis, maxlay2);
    EP_SIZE_CHECK(ibc, 2);
    EP_SIZE_CHECK(SlatThick, maxlay);
    EP_SIZE_CHECK(SlatWidth, maxlay);
    EP_SIZE_CHECK(SlatAngle, maxlay);
    EP_SIZE_CHECK(SlatCond, maxlay);
    EP_SIZE_CHECK(LayerType, maxlay);
    EP_SIZE_CHECK(Keff, maxlay);
    EP_SIZE_CHECK(ShadeGapKeffConv, MaxGap);
    EP_SIZE_CHECK(LaminateAU, maxlay);
    EP_SIZE_CHECK(sumsolU, maxlay);
    EP_SIZE_CHECK(sol0, maxlay);
    EP_SIZE_CHECK(rir, maxlay2);
    EP_SIZE_CHECK(vfreevent, maxlay1);

    int k1;
    Real64 tiltr;
    Real64 Rsky;
    Real64 Fsky;
    Real64 Fground;
    Real64 e0;
    std::string a;

    //! Scalars:
    ShadeEmisRatioOut = 1.0;
    ShadeEmisRatioIn = 1.0;
    ShadeHcRatioOut = 1.0;
    ShadeHcRatioIn = 1.0;

    //! re-initialize iteration parameters:
    sc = 0.0;
    shgc = 0.0;
    ufactor = 0.0;
    flux = 0.0;

    //! Vectors:
    LaminateAU = 0.0;
    sumsolU = 0.0;
    vfreevent = 0.0;
    sol0 = 0.0;
    // bi...    Clear keff, keffc elements:
    Keff = 0.0;
    ShadeGapKeffConv = 0.0;

    // Adjust shading layer properties
    for (int i = 1; i <= nlayer; ++i) {
        if ((TARCOGLayerType)LayerType(i) == TARCOGParams::TARCOGLayerType::VENETBLIND_HORIZ ||
            (TARCOGLayerType)LayerType(i) == TARCOGParams::TARCOGLayerType::VENETBLIND_VERT) {
            scon(i) = SlatCond(i);
            if (ThermalMod == TARCOGThermalModel::SCW) {
                // bi...the idea here is to have glass-to-glass width the same as before scaling
                // bi...TODO: check for outdoor and indoor blinds! SCW model is only applicable to in-between SDs!!!
                thick(i) = SlatWidth(i) * std::cos(SlatAngle(i) * Constant::Pi / 180.0);
                if (i > 1) {
                    gap(i - 1) += (1.0 - SDScalar) / 2.0 * thick(i); // Autodesk:BoundsViolation gap(i-1) @ i=1: Added if condition
                }
                gap(i) += (1.0 - SDScalar) / 2.0 * thick(i);
                thick(i) *= SDScalar;
                if (thick(i) < SlatThick(i)) {
                    thick(i) = SlatThick(i);
                }
            } else if ((ThermalMod == TARCOGThermalModel::ISO15099) || (ThermalMod == TARCOGThermalModel::CSM)) {
                thick(i) = SlatThick(i);
                const Real64 slatAngRad = SlatAngle(i) * 2.0 * Constant::Pi / 360.0;
                Real64 C4_VENET(0);
                if ((TARCOGLayerType)LayerType(i) == TARCOGParams::TARCOGLayerType::VENETBLIND_HORIZ) {
                    C4_VENET = C4_VENET_HORIZONTAL;
                }
                if ((TARCOGLayerType)LayerType(i) == TARCOGParams::TARCOGLayerType::VENETBLIND_VERT) {
                    C4_VENET = C4_VENET_VERTICAL;
                }
                thick(i) = C4_VENET * (SlatWidth(i) * cos(slatAngRad) + thick(i) * sin(slatAngRad));
            }
        } // Venetian
    }

    hint = hin;
    houtt = hout;
    tiltr = tilt * 2.0 * Constant::Pi / 360.0; // convert tilt in degrees to radians

    // external radiation term
    switch (isky) {
    case 3:
        Gout = outir;
        trmout = root_4(Gout / Constant::StefanBoltzmann);
        break;
    case 2: // effective clear sky emittance from swinbank (SPC142/ISO15099 equations 131, 132, ...)
        Rsky = 5.31e-13 * pow_6(tout);
        esky = Rsky / (Constant::StefanBoltzmann * pow_4(tout)); // check esky const, also check what esky to use when tsky input...
        break;
    case 1:
        esky = pow_4(tsky) / pow_4(tout);
        break;
    case 0: // for isky=0 it is assumed that actual values for esky and Tsky are specified
        esky *= pow_4(tsky) / pow_4(tout);
        break;
    default:
        nperr = 1; // error 2010: isky can be: 0(esky,Tsky input), 1(Tsky input), or 2(Swinbank model)
        return;
    }

    // Simon: In this case we do not need to recalculate Gout and Trmout again
    if (isky != 3) {
        Fsky = (1.0 + std::cos(tiltr)) / 2.0;
        Fground = 1.0 - Fsky;
        e0 = Fground + (1.0 - fclr) * Fsky + Fsky * fclr * esky;
        //  Trmout = Tout * e0**0.25d0

        // bi   Set mean radiant temps for fixed combined film coef. case:

        if (ibc(1) == 1) { // outside BC - fixed combined film coef.
            trmout = tout;
        } else {
            trmout = tout * root_4(e0);
        }

        Gout = Constant::StefanBoltzmann * pow_4(trmout);
    } // if (isky.ne.3) then

    ebsky = Gout;

    if (ibc(2) == 1) { // inside BC - fixed combined film coef.
        trmin = tind;
    }

    Gin = Constant::StefanBoltzmann * pow_4(trmin);
    ebroom = Gin;

    // calculate ir reflectance:
    for (int k = 1; k <= nlayer; ++k) {
        k1 = 2 * k - 1;
        rir(k1) = 1 - tir(k1) - emis(k1);
        rir(k1 + 1) = 1 - tir(k1) - emis(k1 + 1);
        if ((tir(k1) < 0.0) || (tir(k1) > 1.0) || (tir(k1 + 1) < 0.0) || (tir(k1 + 1) > 1.0)) {
            nperr = 4;
            ErrorMessage = format("Layer transmissivity is our of range (<0 or >1). Layer #{:3}", k);
            return;
        }
        if ((emis(k1) < 0.0) || (emis(k1) > 1.0) || (emis(k1 + 1) < 0.0) || (emis(k1 + 1) > 1.0)) {
            nperr = 14;
            ErrorMessage = format("Layer emissivity is our of range (<0 or >1). Layer #{:3}", k);
            return;
        }
        if ((rir(k1) < 0.0) || (rir(k1) > 1.0) || (rir(k1 + 1) < 0.0) || (rir(k1 + 1) > 1.0)) {
            nperr = 3;
            ErrorMessage = format("Layer reflectivity is our of range (<0 or >1). Layer #{:3}", k);
            return;
        }
    }
}

bool GoAhead(int const nperr)
{
    return !(((nperr > 0) && (nperr < 1000)) || ((nperr > 2000) && (nperr < 3000)));
}

} // namespace EnergyPlus::TARCOGArgs
