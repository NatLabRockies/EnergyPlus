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

#ifndef ThermalISO15099Calc_hh_INCLUDED
#define ThermalISO15099Calc_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array2A.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/TARCOGGassesParams.hh>
#include <EnergyPlus/TARCOGParams.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

// Using/Aliasing
using namespace TARCOGGassesParams;
using namespace TARCOGParams;

namespace ThermalISO15099Calc {

    void film(EnergyPlusData &state, Real64 tex, Real64 tw, Real64 ws, int iwd, Real64 &hcout, int ibc);

    void Calc_ISO15099(EnergyPlusData &state,
                       TARCOGOutput::Files &files,
                       int nlayer,
                       int iwd,
                       Real64 &tout,
                       Real64 &tind,
                       Real64 &trmin,
                       Real64 wso,
                       Real64 wsi,
                       Real64 dir,
                       Real64 outir,
                       int isky,
                       Real64 tsky,
                       Real64 &esky,
                       Real64 fclr,
                       Real64 VacuumPressure,
                       Real64 VacuumMaxGapThickness,
                       Array1D<Real64> &gap,
                       Array1D<Real64> &thick,
                       Array1D<Real64> &scon,
                       const Array1D<Real64> &tir,
                       const Array1D<Real64> &emis,
                       Real64 totsol,
                       Real64 tilt,
                       const Array1D<Real64> &asol,
                       Real64 height,
                       Real64 heightt,
                       Real64 width,
                       const Array1D<Real64> &presure,
                       Array2A_int iprop,
                       Array2A<Real64> frct,
                       Array2A<Real64> xgcon,
                       Array2A<Real64> xgvis,
                       Array2A<Real64> xgcp,
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
                       TARCOGThermalModel ThermalMod,
                       int Debug_mode,
                       Real64 &ShadeEmisRatioOut,
                       Real64 &ShadeEmisRatioIn,
                       Real64 &ShadeHcRatioOut,
                       Real64 &ShadeHcRatioIn,
                       Real64 &HcUnshadedOut,
                       Real64 &HcUnshadedIn,
                       Array1D<Real64> &Keff,
                       Array1D<Real64> &ShadeGapKeffConv,
                       Real64 SDScalar,
                       int SHGCCalc,
                       int &NumOfIterations,
                       Real64 egdeGlCorrFac);

    void therm1d(EnergyPlusData &state,
                 TARCOGOutput::Files &files,
                 int nlayer,
                 int iwd,
                 Real64 &tout,
                 Real64 &tind,
                 Real64 wso,
                 Real64 wsi,
                 Real64 VacuumPressure,
                 Real64 VacuumMaxGapThickness,
                 Real64 dir,
                 Real64 &ebsky,
                 Real64 Gout,
                 Real64 trmout,
                 Real64 trmin,
                 Real64 &ebroom,
                 Real64 Gin,
                 const Array1D<Real64> &tir,
                 const Array1D<Real64> &rir,
                 const Array1D<Real64> &emis,
                 const Array1D<Real64> &gap,
                 const Array1D<Real64> &thick,
                 const Array1D<Real64> &scon,
                 Real64 tilt,
                 const Array1D<Real64> &asol,
                 Real64 height,
                 Real64 heightt,
                 Real64 width,
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
                 int Debug_mode,
                 Real64 &AchievedErrorTolerance,
                 int &TotalIndex,
                 Real64 edgeGlCorrFac);

    void guess(EnergyPlusData &state,
               Real64 tout,
               Real64 tind,
               int nlayer,
               const Array1D<Real64> &gap,
               const Array1D<Real64> &thick,
               Real64 &width,
               Array1D<Real64> &theta,
               Array1D<Real64> &Ebb,
               Array1D<Real64> &Ebf,
               Array1D<Real64> &Tgap);

    void solarISO15099(Real64 totsol, Real64 rtot, const Array1D<Real64> &rs, int nlayer, const Array1D<Real64> &absol, Real64 &sf);

    void resist(int nlayer,
                Real64 trmout,
                Real64 Tout,
                Real64 trmin,
                Real64 tind,
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
                Array1D<Real64> &qrgas);

    void hatter(EnergyPlusData &state,
                int nlayer,
                int iwd,
                Real64 tout,
                Real64 tind,
                Real64 wso,
                Real64 wsi,
                Real64 VacuumPressure,
                Real64 VacuumMaxGapThickness,
                Real64 const ebsky,
                Real64 &tamb,
                Real64 const ebroom,
                Real64 &troom,
                const Array1D<Real64> &gap,
                Real64 height,
                Real64 heightt,
                const Array1D<Real64> &scon,
                Real64 tilt,
                Array1D<Real64> const &theta,
                const Array1D<Real64> &Tgap,
                Array1D<Real64> const &Radiation,
                Real64 trmout,
                Real64 trmin,
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
                Real64 hin,
                Real64 hout,
                int index,
                const Array1D_int &ibc,
                int &nperr,
                std::string &ErrorMessage,
                Real64 &hrin,
                Real64 &hrout,
                Array1D<Real64> &Ra,
                Array1D<Real64> &Nu);

    void effectiveLayerCond(EnergyPlusData &state,
                            int nlayer,
                            const Array1D<TARCOGLayerType> &LayerType, // Layer type
                            const Array1D<Real64> &scon,               // Layer thermal conductivity
                            const Array1D<Real64> &thick,              // Layer thickness
                            Array2A_int iprop,                         // Gas type in gaps
                            Array2A<Real64> frct,                      // Fraction of gas
                            const Array1D_int &nmix,                   // Gas mixture
                            const Array1D<Real64> &pressure,           // Gas pressure [Pa]
                            const Array1D<Real64> &wght,               // Molecular weight
                            Array2A<Real64> gcon,                      // Gas specific conductivity
                            Array2A<Real64> gvis,                      // Gas specific viscosity
                            Array2A<Real64> gcp,                       // Gas specific heat
                            const Array1D<Real64> &EffectiveOpenness,  // Layer effective openneess [m2]
                            Array1D<Real64> const &theta,              // Layer surface temperatures [K]
                            Array1D<Real64> &sconScaled,               // Layer conductivity divided by thickness
                            int &nperr,                                // Error message flag
                            std::string &ErrorMessage                  // Error message
    );

    void filmi(EnergyPlusData &state,
               Real64 tair,
               Real64 t,
               int nlayer,
               Real64 tilt,
               Real64 wsi,
               Real64 height,
               Array2A_int iprop,
               Array2A<Real64> frct,
               const Array1D<Real64> &presure,
               const Array1D_int &nmix,
               const Array1D<Real64> &wght,
               Array2A<Real64> gcon,
               Array2A<Real64> gvis,
               Array2A<Real64> gcp,
               Real64 &hcin,
               int ibc,
               int &nperr,
               std::string &ErrorMessage);

    void filmg(EnergyPlusData &state,
               Real64 tilt,
               const Array1D<Real64> &theta,
               const Array1D<Real64> &Tgap,
               int nlayer,
               Real64 height,
               const Array1D<Real64> &gap,
               Array2A_int iprop,
               Array2A<Real64> frct,
               Real64 VacuumPressure,
               const Array1D<Real64> &presure,
               const Array1D_int &nmix,
               const Array1D<Real64> &wght,
               Array2A<Real64> gcon,
               Array2A<Real64> gvis,
               Array2A<Real64> gcp,
               const Array1D<Real64> &gama,
               Array1D<Real64> &hcgas,
               Array1D<Real64> &Rayleigh,
               Array1D<Real64> &Nu,
               int &nperr,
               std::string &ErrorMessage);

    void filmPillar(EnergyPlusData &state,
                    const Array1D_int &SupportPillar,     // Shows whether or not gap have support pillar
                    const Array1D<Real64> &scon,          // Conductivity of glass layers
                    const Array1D<Real64> &PillarSpacing, // Pillar spacing for each gap (used in case there is support pillar)
                    const Array1D<Real64> &PillarRadius,  // Pillar radius for each gap (used in case there is support pillar)
                    int nlayer,
                    const Array1D<Real64> &gap,
                    Array1D<Real64> &hcgas,
                    Real64 VacuumMaxGapThickness,
                    int &nperr,
                    std::string &ErrorMessage);

    void nusselt(Real64 tilt, Real64 ra, Real64 asp, Real64 &gnu, int &nperr, std::string &ErrorMessage);

    void storeIterationResults(EnergyPlusData &state,
                               TARCOGOutput::Files &files,
                               int nlayer,
                               int index,
                               const Array1D<Real64> &theta,
                               Real64 trmout,
                               Real64 tamb,
                               Real64 trmin,
                               Real64 troom,
                               Real64 ebsky,
                               Real64 ebroom,
                               Real64 hcin,
                               Real64 hcout,
                               Real64 hrin,
                               Real64 hrout,
                               Real64 hin,
                               Real64 hout,
                               const Array1D<Real64> &Ebb,
                               const Array1D<Real64> &Ebf,
                               const Array1D<Real64> &Rb,
                               const Array1D<Real64> &Rf,
                               int &);

    void CalculateFuncResults(int nlayer, Array2<Real64> const &a, const Array1D<Real64> &b, const Array1D<Real64> &x, Array1D<Real64> &FRes);
} // namespace ThermalISO15099Calc
struct ThermalISO15099CalcData : BaseGlobalStruct
{
    Array1D<Real64> thetas = Array1D<Real64>(maxlay2);
    Array1D<Real64> rir = Array1D<Real64>(maxlay2);
    Array1D<Real64> hcgass = Array1D<Real64>(maxlay1);
    Array1D<Real64> hrgass = Array1D<Real64>(maxlay1);
    Array1D<Real64> rs = Array1D<Real64>(maxlay3, 0.0);
    Array1D<Real64> qs = Array1D<Real64>(maxlay3);
    Array1D<Real64> qvs = Array1D<Real64>(maxlay1);
    Array1D<Real64> LaminateAU = Array1D<Real64>(maxlay);
    Array1D<Real64> sumsolU = Array1D<Real64>(maxlay);
    Array1D<Real64> sol0 = Array1D<Real64>(maxlay);
    Array1D<Real64> qcgas = Array1D<Real64>(maxlay1);
    Array1D<Real64> qcgaps = Array1D<Real64>(maxlay1);
    Array1D<Real64> qrgas = Array1D<Real64>(maxlay1);
    Array1D<Real64> qrgaps = Array1D<Real64>(maxlay1);
    Array1D<Real64> Atop_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> Abot_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> Al_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> Ar_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> Ah_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> EffectiveOpenness_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> SlatThick_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> SlatWidth_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> SlatAngle_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> SlatCond_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> SlatSpacing_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> SlatCurve_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> vvent_NOSD = Array1D<Real64>(maxlay1);
    Array1D<Real64> tvent_NOSD = Array1D<Real64>(maxlay1);
    Array1D<Real64> qv_NOSD = Array1D<Real64>(maxlay1);
    Array1D<Real64> q_NOSD = Array1D<Real64>(maxlay3);
    Array1D<TARCOGParams::TARCOGLayerType> LayerType_NOSD = Array1D<TARCOGParams::TARCOGLayerType>(maxlay);
    Array1D<Real64> gap_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> thick_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> scon_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> emis_NOSD = Array1D<Real64>(maxlay2);
    Array1D<Real64> rir_NOSD = Array1D<Real64>(maxlay2);
    Array1D<Real64> tir_NOSD = Array1D<Real64>(maxlay2);
    Array1D<Real64> theta_NOSD = Array1D<Real64>(maxlay2);
    Array2D<Real64> frct_NOSD = Array2D<Real64>(maxgas, maxlay1);
    Array2D_int iprop_NOSD = Array2D_int(maxgas, maxlay1);
    Array1D_int nmix_NOSD = Array1D_int(maxlay1);
    Array1D<Real64> presure_NOSD = Array1D<Real64>(maxlay1);
    Array1D<Real64> hcgas_NOSD = Array1D<Real64>(maxlay1);
    Array1D<Real64> hrgas_NOSD = Array1D<Real64>(maxlay1);
    Array1D<Real64> LaminateA_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> LaminateB_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> sumsol_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> Ra_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> Nu_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> Ebb = Array1D<Real64>(maxlay);
    Array1D<Real64> Ebf = Array1D<Real64>(maxlay);
    Array1D<Real64> Rb = Array1D<Real64>(maxlay);
    Array1D<Real64> Rf = Array1D<Real64>(maxlay);
    Array1D<Real64> Ebbs = Array1D<Real64>(maxlay);
    Array1D<Real64> Ebfs = Array1D<Real64>(maxlay);
    Array1D<Real64> Rbs = Array1D<Real64>(maxlay);
    Array1D<Real64> Rfs = Array1D<Real64>(maxlay);
    Array1D<Real64> Ebb_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> Ebf_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> Rb_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> Rf_NOSD = Array1D<Real64>(maxlay);
    Array1D<Real64> ShadeGapKeffConv_NOSD = Array1D<Real64>(MaxGap);
    Array1D<Real64> qcgas_NOSD = Array1D<Real64>(maxlay1);
    Array1D<Real64> Keff_NOSD = Array1D<Real64>(maxlay1);
    Array1D<Real64> qrgas_NOSD = Array1D<Real64>(maxlay1);
    Array1D_int nslice_NOSD = Array1D_int(maxlay);
    Array1D<Real64> vfreevent_NOSD = Array1D<Real64>(maxlay1);
    Array1D<Real64> vfreevent = Array1D<Real64>(maxlay1);
    Array1D<Real64> Atop_eff = Array1D<Real64>(maxlay, 0.0);
    Array1D<Real64> Abot_eff = Array1D<Real64>(maxlay, 0.0);
    Array1D<Real64> Al_eff = Array1D<Real64>(maxlay, 0.0);
    Array1D<Real64> Ar_eff = Array1D<Real64>(maxlay, 0.0);
    Array1D<Real64> Ah_eff = Array1D<Real64>(maxlay, 0.0);
    Array1D<Real64> EffectiveOpenness = Array1D<Real64>(maxlay, 0.0);
    Array1D<Real64> hgas = Array1D<Real64>(maxlay1);
    Array1D<Real64> Tgap = Array1D<Real64>(maxlay1);
    Array1D<Real64> hcgapMod = Array1D<Real64>(maxlay1);
    Array1D<Real64> hcv = Array1D<Real64>(maxlay1);
    Array1D_int iprop1 = Array1D_int(maxgas);
    Array1D<Real64> frct1 = Array1D<Real64>(maxgas);
    Array1D<Real64> frcti = Array1D<Real64>(maxgas);
    Array1D_int ipropi = Array1D_int(maxgas);
    Array1D<Real64> frctg = Array1D<Real64>(maxgas);
    Array1D_int ipropg = Array1D_int(maxgas);

    Real64 rtot = 0.0;
    Real64 sft = 0.0;
    Real64 hcins = 0.0;
    Real64 hrins = 0.0;
    Real64 hins = 0.0;
    Real64 hcouts = 0.0;
    Real64 hrouts = 0.0;
    Real64 houts = 0.0;
    Real64 ufactors = 0.0;
    Real64 fluxs = 0.0;
    Real64 qeff = 0.0;
    Real64 flux_nonsolar = 0.0;
    Real64 cpa = 0.0;
    Real64 aveGlassConductivity = 0.0;

    int iFP = 0;
    int kFP = 0;
    std::string dynFormat;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->thetas = Array1D<Real64>(maxlay2);
        this->rir = Array1D<Real64>(maxlay2);
        this->hcgass = Array1D<Real64>(maxlay1);
        this->hrgass = Array1D<Real64>(maxlay1);
        this->rs = Array1D<Real64>(maxlay3, 0.0);
        this->qs = Array1D<Real64>(maxlay3);
        this->qvs = Array1D<Real64>(maxlay1);
        this->LaminateAU = Array1D<Real64>(maxlay);
        this->sumsolU = Array1D<Real64>(maxlay);
        this->sol0 = Array1D<Real64>(maxlay);
        this->qcgas = Array1D<Real64>(maxlay1);
        this->qcgaps = Array1D<Real64>(maxlay1);
        this->qrgas = Array1D<Real64>(maxlay1);
        this->qrgaps = Array1D<Real64>(maxlay1);
        this->Atop_NOSD = Array1D<Real64>(maxlay);
        this->Abot_NOSD = Array1D<Real64>(maxlay);
        this->Al_NOSD = Array1D<Real64>(maxlay);
        this->Ar_NOSD = Array1D<Real64>(maxlay);
        this->Ah_NOSD = Array1D<Real64>(maxlay);
        this->EffectiveOpenness_NOSD = Array1D<Real64>(maxlay);
        this->SlatThick_NOSD = Array1D<Real64>(maxlay);
        this->SlatWidth_NOSD = Array1D<Real64>(maxlay);
        this->SlatAngle_NOSD = Array1D<Real64>(maxlay);
        this->SlatCond_NOSD = Array1D<Real64>(maxlay);
        this->SlatSpacing_NOSD = Array1D<Real64>(maxlay);
        this->SlatCurve_NOSD = Array1D<Real64>(maxlay);
        this->vvent_NOSD = Array1D<Real64>(maxlay1);
        this->tvent_NOSD = Array1D<Real64>(maxlay1);
        this->qv_NOSD = Array1D<Real64>(maxlay1);
        this->q_NOSD = Array1D<Real64>(maxlay3);
        this->LayerType_NOSD = Array1D<TARCOGLayerType>(maxlay);
        this->gap_NOSD = Array1D<Real64>(maxlay);
        this->thick_NOSD = Array1D<Real64>(maxlay);
        this->scon_NOSD = Array1D<Real64>(maxlay);
        this->emis_NOSD = Array1D<Real64>(maxlay2);
        this->rir_NOSD = Array1D<Real64>(maxlay2);
        this->tir_NOSD = Array1D<Real64>(maxlay2);
        this->theta_NOSD = Array1D<Real64>(maxlay2);
        this->frct_NOSD = Array2D<Real64>(maxgas, maxlay1);
        this->iprop_NOSD = Array2D_int(maxgas, maxlay1);
        this->nmix_NOSD = Array1D_int(maxlay1);
        this->presure_NOSD = Array1D<Real64>(maxlay1);
        this->hcgas_NOSD = Array1D<Real64>(maxlay1);
        this->hrgas_NOSD = Array1D<Real64>(maxlay1);
        this->LaminateA_NOSD = Array1D<Real64>(maxlay);
        this->LaminateB_NOSD = Array1D<Real64>(maxlay);
        this->sumsol_NOSD = Array1D<Real64>(maxlay);
        this->Ra_NOSD = Array1D<Real64>(maxlay);
        this->Nu_NOSD = Array1D<Real64>(maxlay);
        this->Ebb = Array1D<Real64>(maxlay);
        this->Ebf = Array1D<Real64>(maxlay);
        this->Rb = Array1D<Real64>(maxlay);
        this->Rf = Array1D<Real64>(maxlay);
        this->Ebbs = Array1D<Real64>(maxlay);
        this->Ebfs = Array1D<Real64>(maxlay);
        this->Rbs = Array1D<Real64>(maxlay);
        this->Rfs = Array1D<Real64>(maxlay);
        this->Ebb_NOSD = Array1D<Real64>(maxlay);
        this->Ebf_NOSD = Array1D<Real64>(maxlay);
        this->Rb_NOSD = Array1D<Real64>(maxlay);
        this->Rf_NOSD = Array1D<Real64>(maxlay);
        this->ShadeGapKeffConv_NOSD = Array1D<Real64>(MaxGap);
        this->qcgas_NOSD = Array1D<Real64>(maxlay1);
        this->Keff_NOSD = Array1D<Real64>(maxlay1);
        this->qrgas_NOSD = Array1D<Real64>(maxlay1);
        this->nslice_NOSD = Array1D_int(maxlay);
        this->vfreevent_NOSD = Array1D<Real64>(maxlay1);
        this->vfreevent = Array1D<Real64>(maxlay1);
        this->Atop_eff = Array1D<Real64>(maxlay, 0.0);
        this->Abot_eff = Array1D<Real64>(maxlay, 0.0);
        this->Al_eff = Array1D<Real64>(maxlay, 0.0);
        this->Ar_eff = Array1D<Real64>(maxlay, 0.0);
        this->Ah_eff = Array1D<Real64>(maxlay, 0.0);
        this->EffectiveOpenness = Array1D<Real64>(maxlay, 0.0);
        this->hgas = Array1D<Real64>(maxlay1);
        this->Tgap = Array1D<Real64>(maxlay1);
        this->hcgapMod = Array1D<Real64>(maxlay1);
        this->hcv = Array1D<Real64>(maxlay1);
        this->iprop1 = Array1D_int(maxgas);
        this->frct1 = Array1D<Real64>(maxgas);
        this->frcti = Array1D<Real64>(maxgas);
        this->ipropi = Array1D_int(maxgas);
        this->frctg = Array1D<Real64>(maxgas);
        this->ipropg = Array1D_int(maxgas);

        this->rtot = 0.0;
        this->sft = 0.0;
        this->hcins = 0.0;
        this->hrins = 0.0;
        this->hins = 0.0;
        this->hcouts = 0.0;
        this->hrouts = 0.0;
        this->houts = 0.0;
        this->ufactors = 0.0;
        this->fluxs = 0.0;
        this->qeff = 0.0;
        this->flux_nonsolar = 0.0;
        this->cpa = 0.0;
        this->aveGlassConductivity = 0.0;

        this->iFP = 0;
        this->kFP = 0;

        this->dynFormat = "";
    }
};

} // namespace EnergyPlus

#endif
