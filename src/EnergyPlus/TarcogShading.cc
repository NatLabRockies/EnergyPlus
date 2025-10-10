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
#include <EnergyPlus/TARCOGGasses90.hh>
#include <EnergyPlus/TARCOGGassesParams.hh>
#include <EnergyPlus/TARCOGParams.hh>
#include <EnergyPlus/TarcogShading.hh>

namespace EnergyPlus {

namespace TarcogShading {

    // MODULE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   June/22/2010
    //       MODIFIED       na
    //       RE-ENGINEERED  na
    //  Revision: 6.0.36  (June/22/2010)
    //   - Initial setup, extracted from TARCOG.for

    // PURPOSE OF THIS MODULE:
    // Module which contains subroutines used for handling shading
    // device layers according to ISO15099

    // METHODOLOGY EMPLOYED:
    // <description>

    // REFERENCES:
    // na

    // OTHER NOTES:
    // na

    // USE STATEMENTS:

    // Using/Aliasing
    using namespace TARCOGGassesParams;
    using namespace TARCOGGasses90;
    using namespace TARCOGParams;

    enum class CalcForcedVentilation
    {
        Invalid = -1,
        Skip,
        Allow,
        Num
    };

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
                 Array1D<Real64> &hcv, // Heat transfer coefficient in gaps including airflow
                 int &nperr,
                 std::string &ErrorMessage,
                 Array1D<Real64> &vfreevent)
    {
        //**************************************************************************************************************
        //  Input:
        // theta   Vector of average temperatures
        //  gap      Vector of gap widths (maxlay) [m]
        //  hgas    Convective part of gap effective conductivity
        //  frct    Fraction of gasses in a mixture (maxlay1,maxgas)
        //  iprop    Vector of gas identifiers (maxlay1,maxgas)
        //  pressure  Vector of gas pressures [N/m^2]
        //  nmix    Vector of number of gasses for each mixture (maxgas=10)
        //  nlayer  Number of glazing layers
        //  width    IGU cavity width [m]
        //  height  IGU cavity height [m]
        //  angle    Window angle [degrees]
        //  Tout    Outdoor temperature [K]
        //  Tin      Indoor temperature [K]
        //  Atop    Opening between top of shading device and top of glazing cavity [m^2]
        //  Abot    Opening between bottom of shading device and bottom of glazing cavity [m^2]
        //  Al      Opening between left of shading device and left end of glazing cavity [m^2]
        //  Ar      Opening between right of shading device and right end of glazing cavity [m^2]
        //  Ah      Total area holes in the shading device [m^2]
        //  LayerType    Vector of layer types (0 - glazing; 1 - shading)
        //  Ebf      Vector of emissive power of the front surface (# of layers)
        //  Input/Output:
        //  Tgaps    Vector of gap temperatures [K]
        //  Output:
        //  qv      Vector of heat transfer to the gap by ventilation [W/m^2]
        //  hhatv    Vector of all film coefficients for vented cavities (maxlay3)
        //  hcv      Vector of surface-to-air heat transfer coefficients by conduction/convection for vented cavities [W/(m^2*K)]
        //  Ebgap    Vector of emissive power of the vented cavities (maxlay3)
        //  nperr    Error flag
        // vfreevent   Vector of free ventilation velocities in gaps
        //**************************************************************************************************************

        // Using/Aliasing
        using namespace TARCOGCommon;

        // Locals
        // REAL(r64), intent(in) :: Ebf(maxlay)

        Real64 Atops;
        Real64 Abots;
        Real64 Als;
        Real64 Ars;
        Real64 Ahs;
        Real64 press1;
        Real64 press2;
        Real64 s1;
        Real64 s2;
        Real64 s;
        Real64 hcvs;
        Real64 qvs;
        Real64 hc;
        Real64 hc1;
        Real64 hc2;
        Real64 speed;
        Real64 Tav;
        Real64 Tgap;
        Real64 Temp;
        Real64 speed1;
        Real64 speed2;
        Real64 Tav1;
        Real64 Tav2;
        Real64 Tgap1;
        Real64 Tgap2;
        Real64 hcv1;
        Real64 hcv2;
        Real64 qv1;
        Real64 qv2;

        int i;
        int j;
        int k;
        int nmix1;
        int nmix2;

        // init vectors:
        qv = 0.0;
        hcv = 0.0;
        // hhatv = 0.0d0
        // Ebgap = 0.0d0
        // hcv = 0.0d0

        // main loop:
        for (i = 1; i <= nlayer; ++i) {
            k = 2 * i + 1;
            // if (LayerType(i).eq.VENETBLIND) then
            if (IsShadingLayer(LayerType(i))) {
                // dr.........set Shading device geometry
                Atops = Atop(i);
                Abots = Abot(i);
                Als = Al(i);
                Ars = Ar(i);
                Ahs = Ah(i);

                // dr.....setting gas properties for two adjacent gaps (or environment)
                nmix1 = nmix(i);
                nmix2 = nmix(i + 1);
                press1 = pressure(i);
                press2 = pressure(i + 1);
                for (j = 1; j <= maxgas; ++j) {
                    state.dataTarcogShading->iprop1(j) = iprop(j, i);
                    state.dataTarcogShading->iprop2(j) = iprop(j, i + 1);
                    state.dataTarcogShading->frct1(j) = frct(j, i);
                    state.dataTarcogShading->frct2(j) = frct(j, i + 1);
                } // j

                // dr.......shading on outdoor side
                if (i == 1) {
                    s = gap(1);
                    hc = hcgas(2);
                    // Tenv = tvent(1)
                    Tav = (theta(2) + theta(3)) / 2.0;
                    Tgap = Tgaps(2);

                    // bi......use Tout as temp of the air at inlet
                    shadingedge(state,
                                state.dataTarcogShading->iprop1,
                                state.dataTarcogShading->frct1,
                                press1,
                                nmix1,
                                state.dataTarcogShading->iprop2,
                                state.dataTarcogShading->frct2,
                                press2,
                                nmix2,
                                xwght,
                                xgcon,
                                xgvis,
                                xgcp,
                                Atops,
                                Abots,
                                Als,
                                Ars,
                                Ahs,
                                s,
                                height,
                                width,
                                angle,
                                vvent(2),
                                hc,
                                Tout,
                                Tav,
                                Tgap,
                                hcvs,
                                qvs,
                                nperr,
                                ErrorMessage,
                                speed);

                    // exit on error
                    if ((nperr > 0) && (nperr < 1000)) {
                        return;
                    }

                    Tgaps(2) = Tgap;
                    // Ebgap(3) = sigma * Tgap ** 4

                    hcgas(2) = hcvs / 2.0;
                    hgas(2) = hcgas(2) + hrgas(2);
                    hcv(2) = hcvs;
                    qv(2) = qvs;

                    // bi.........Add free ventilation velocity
                    vfreevent(2) = speed;
                } // if (i.eq.1) then

                // dr.......shading on indoor side
                if (i == nlayer) {
                    if (nlayer > 1) {
                        s = gap(nlayer - 1); // Autodesk:BoundsViolation gap(nlayer - 1) @ nlayer=1: Fixed with if block
                        Tav = (theta(2 * nlayer - 1) + theta(2 * nlayer - 2)) /
                              2.0; // Autodesk:BoundsViolation theta(2 * nlayer - 2) @ nlayer=1: Fixed with if block in 8.2
                    } else {
                        s = 0.0;
                        Tav = 273.15;
                    }
                    hc = hcgas(nlayer);
                    // Tenv = tvent(nlayer + 1)

                    Tgap = Tgaps(nlayer);

                    // bi.........use Tin as temp of the air at inlet
                    shadingedge(state,
                                state.dataTarcogShading->iprop2,
                                state.dataTarcogShading->frct2,
                                press2,
                                nmix2,
                                state.dataTarcogShading->iprop1,
                                state.dataTarcogShading->frct1,
                                press1,
                                nmix1,
                                xwght,
                                xgcon,
                                xgvis,
                                xgcp,
                                Atops,
                                Abots,
                                Als,
                                Ars,
                                Ahs,
                                s,
                                height,
                                width,
                                angle,
                                vvent(nlayer),
                                hc,
                                Tin,
                                Tav,
                                Tgap,
                                hcvs,
                                qvs,
                                nperr,
                                ErrorMessage,
                                speed);

                    // exit on error
                    if ((nperr > 0) && (nperr < 1000)) {
                        return;
                    }

                    Tgaps(nlayer) = Tgap;
                    hcgas(nlayer) = hcvs / 2.0;
                    hgas(nlayer) = hcgas(nlayer) + hrgas(nlayer);
                    hcv(nlayer) = hcvs;
                    qv(nlayer) = qvs;

                    // bi.........Add free ventilation velocity
                    vfreevent(i) = speed;
                } // if (i.eq.nlayer) then

                // dr.......shading between glass layers
                if ((i > 1) && (i < nlayer)) {
                    // dr.........average temperatures
                    Tav1 = (theta(2 * i - 2) + theta(2 * i - 1)) / 2.0;
                    Tav2 = (theta(2 * i) + theta(2 * i + 1)) / 2.0;
                    Tgap1 = Tgaps(i);
                    Tgap2 = Tgaps(i + 1);

                    hc1 = hcgas(i);
                    hc2 = hcgas(i + 1);
                    if (i > 1) {
                        s1 = gap(i - 1);
                    }
                    s2 = gap(i);

                    // speed1 = vvent(i)
                    // speed2 = vvent(i+1)

                    if ((static_cast<int>(CalcForcedVentilation::Allow)) && ((vvent(i) != 0) || (vvent(i + 1) != 0))) {
                        forcedventilation(state,
                                          state.dataTarcogShading->iprop1,
                                          state.dataTarcogShading->frct1,
                                          press1,
                                          nmix1,
                                          xwght,
                                          xgcon,
                                          xgvis,
                                          xgcp,
                                          s1,
                                          height,
                                          hc1,
                                          vvent(i),
                                          tvent(i),
                                          Temp,
                                          Tav1,
                                          hcv1,
                                          qv1,
                                          nperr,
                                          ErrorMessage);
                        forcedventilation(state,
                                          state.dataTarcogShading->iprop2,
                                          state.dataTarcogShading->frct2,
                                          press2,
                                          nmix1,
                                          xwght,
                                          xgcon,
                                          xgvis,
                                          xgcp,
                                          s2,
                                          height,
                                          hc1,
                                          vvent(i + 1),
                                          tvent(i + 1),
                                          Temp,
                                          Tav2,
                                          hcv2,
                                          qv2,
                                          nperr,
                                          ErrorMessage);
                    } else {
                        shadingin(state,
                                  state.dataTarcogShading->iprop1,
                                  state.dataTarcogShading->frct1,
                                  press1,
                                  nmix1,
                                  state.dataTarcogShading->iprop2,
                                  state.dataTarcogShading->frct2,
                                  press2,
                                  nmix2,
                                  xwght,
                                  xgcon,
                                  xgvis,
                                  xgcp,
                                  Atops,
                                  Abots,
                                  Als,
                                  Ars,
                                  Ahs,
                                  s1,
                                  s2,
                                  height,
                                  width,
                                  angle,
                                  hc1,
                                  hc2,
                                  speed1,
                                  speed2,
                                  Tgap1,
                                  Tgap2,
                                  Tav1,
                                  Tav2,
                                  hcv1,
                                  hcv2,
                                  qv1,
                                  qv2,
                                  nperr,
                                  ErrorMessage);
                    }

                    // exit on error
                    if ((nperr > 0) && (nperr < 1000)) {
                        return;
                    }

                    // if (vvent(i).gt.0) then !not implemented for inside shading yet
                    //  nperr = 1006
                    //  ErrorMessage = 'Forced ventilation not implemented for internal SD layers.'
                    //  return
                    // end if

                    hcgas(i) = hcv1 / 2.0;
                    hcgas(i + 1) = hcv2 / 2.0;
                    hgas(i) = hcgas(i) + hrgas(i);
                    hgas(i + 1) = hcgas(i + 1) + hrgas(i + 1);
                    hcv(i) = hcv1;
                    hcv(i + 1) = hcv2;
                    qv(i) = qv1;
                    qv(i + 1) = qv2;
                    Tgaps(i) = Tgap1;
                    Tgaps(i + 1) = Tgap2;
                    // bi.........Add free ventilation velocity
                    vfreevent(i) = speed1;
                    vfreevent(i + 1) = speed2;
                } // if ((i.gt.1).and.(i.lt.nlayer)) then
            } // if (LayerType(i).eq.SHADING) then
        }
    }

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
                           std::string &ErrorMessage)
    {
        //**************************************************************************************************************
        //  Input:
        //  iprop      Vector of gas identifiers
        //  frct      Fraction of gasses in a mixture
        //  nmix      Number of gasses in a mixture
        //  press      Pressure in mixture
        //  s1        Gap width [m]
        //  H          IGU cavity height [m]
        //  L          IGU cavity width [m]
        //  hc        Convective/conductive coefficient for non-vented gap
        //  Tav        Average temperature of gap surfaces
        // Tinlet    Temperature of inlet air
        //  Output:
        //  hcv    Convective/conductive coefficient for vented gap
        //  qv    Heat transfer to the gap by ventilation [W/m^2]
        //  nperr      Error flag
        // ErrorMessage string containing error message
        //**************************************************************************************************************

        // Argument array dimensioning
        EP_SIZE_CHECK(iprop, maxgas);
        EP_SIZE_CHECK(frct, maxgas);
        EP_SIZE_CHECK(xwght, maxgas);
        xgcon.dim(3, maxgas);
        xgvis.dim(3, maxgas);
        xgcp.dim(3, maxgas);

        // Locals
        Real64 H0;
        Real64 dens;
        Real64 cp;
        Real64 pr;
        Real64 con;
        Real64 visc;

        GASSES90(state,
                 Tav,
                 iprop,
                 frct,
                 press,
                 nmix,
                 xwght,
                 xgcon,
                 xgvis,
                 xgcp,
                 con,
                 visc,
                 dens,
                 cp,
                 pr,
                 TARCOGGassesParams::Stdrd::ISO15099,
                 nperr,
                 ErrorMessage);

        H0 = (dens * cp * s * forcedspeed) / (4.0 * hc + 8.0 * forcedspeed);

        Toutlet = Tav - (Tav - Tinlet) * std::pow(e, -H / H0);

        qv = -dens * cp * forcedspeed * s * (Toutlet - Tinlet) / H;

        // Need to calculate surface-to-air convection heat transfer coefficient.  This is needed later to calculate layer
        // to gap thermal resistance
        hcv = 2.0 * hc + 4.0 * forcedspeed;
    }

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
                   std::string &ErrorMessage)
    {
        //**************************************************************************************************************
        //  Input:
        //  iprop1      Vector of gas identifiers
        //  frct1      Fraction of gasses in a mixture
        //  nmix1      Number of gasses in a mixture
        //  press1      Pressure in mixture
        //  iprop2      Vector of gas identifiers
        //  frct2      Fraction of gasses in a mixture
        //  nmix2      Number of gasses in a mixture
        //  press2      Pressure in mixture
        //  Atop      Opening between top of shading device and top of glazing cavity [m^2]
        //  Abot      Opening between bottom of shading device and bottom of glazing cavity [m^2]
        //  Al        Opening between left of shading device and left end of glazing cavity [m^2]
        //  Ar        Opening between right of shading device and right end of glazing cavity [m^2]
        //  Ah        Total area holes in the shading device [m^2]
        //  s1, s2      Gap width [m]
        //  H        IGU cavity height [m]
        //  L        IGU cavity width [m]
        //  angle      Window angle [degrees]
        //  hc1, hc2    Convective/conductive coefficient for non-vented gap
        //  Tav1, Tav2    Average temperature of gap surfaces
        //  Output:
        //  Tgap1, Tgap2  Temperature of vented gap
        //  hcv1, hcv2    Convective/conductive coefficient for vented gap
        //  qv1, qv2    Heat transfer to the gap by ventilation [W/m^2]
        //  speed1, speed2  Air/gas velocities in gaps around SD layer
        //  nperr      Error flag
        //**************************************************************************************************************

        // Using/Aliasing
        // Argument array dimensioning
        EP_SIZE_CHECK(iprop1, maxgas);
        EP_SIZE_CHECK(frct1, maxgas);
        EP_SIZE_CHECK(iprop2, maxgas);
        EP_SIZE_CHECK(frct2, maxgas);
        EP_SIZE_CHECK(xwght, maxgas);
        xgcon.dim(3, maxgas);
        xgvis.dim(3, maxgas);
        xgcp.dim(3, maxgas);

        // Locals
        Real64 A;
        Real64 A1;
        Real64 A2;
        Real64 B1;
        Real64 B2;
        Real64 C1;
        Real64 C2;
        Real64 D1;
        Real64 D2;
        Real64 Zin1;
        Real64 Zin2;
        Real64 Zout1;
        Real64 Zout2;
        Real64 A1eqin;
        Real64 A1eqout;
        Real64 A2eqin;
        Real64 A2eqout;
        Real64 T0;
        Real64 tilt;
        Real64 dens0;
        Real64 visc0;
        Real64 con0;
        Real64 pr0;
        Real64 cp0;
        Real64 dens1;
        Real64 visc1;
        Real64 con1;
        Real64 pr1;
        Real64 cp1;
        Real64 dens2;
        Real64 visc2;
        Real64 con2;
        Real64 pr2;
        Real64 cp2;
        Real64 Tup;
        Real64 Tdown;
        Real64 H01;
        Real64 H02;
        Real64 beta1;
        Real64 beta2;
        Real64 alpha1;
        Real64 alpha2;
        Real64 P1;
        Real64 P2;
        Real64 qsmooth;

        // iteration parameters
        int iter;
        Real64 TGapOld1;
        Real64 TGapOld2;
        Real64 Temp1;
        Real64 Temp2;
        bool converged;

        TGapOld1 = 0.0;
        TGapOld2 = 0.0;
        tilt = Constant::Pi / 180 * (angle - 90);
        T0 = 0.0 + Constant::Kelvin;
        A1eqin = 0.0;
        A2eqout = 0.0;
        A1eqout = 0.0;
        A2eqin = 0.0;
        P1 = 0.0;
        P2 = 0.0;

        GASSES90(state,
                 T0,
                 iprop1,
                 frct1,
                 press1,
                 nmix1,
                 xwght,
                 xgcon,
                 xgvis,
                 xgcp,
                 con0,
                 visc0,
                 dens0,
                 cp0,
                 pr0,
                 TARCOGGassesParams::Stdrd::ISO15099,
                 nperr,
                 ErrorMessage);

        // exit on error:
        if ((nperr > 0) && (nperr < 1000)) {
            return;
        }

        // dr...check for error messages
        if ((Tgap1 * Tgap2) == 0) {
            nperr = 15;
            ErrorMessage = "Temperature of vented gap must be greater than 0 [K].";
            return;
        }

        if ((Atop + Abot) == 0) {
            //    nperr = 16
            //    return
            Atop = 0.000001;
            Abot = 0.000001;
        }

        converged = false;
        iter = 0;
        Real64 const s1_2 = pow_2(s1);
        Real64 const s2_2 = pow_2(s2);
        Real64 const s1_s2_2 = pow_2(s1 / s2);
        Real64 const cos_Tilt = std::cos(tilt);
        while (!converged) {
            ++iter;
            GASSES90(state,
                     Tgap1,
                     iprop1,
                     frct1,
                     press1,
                     nmix1,
                     xwght,
                     xgcon,
                     xgvis,
                     xgcp,
                     con1,
                     visc1,
                     dens1,
                     cp1,
                     pr1,
                     TARCOGGassesParams::Stdrd::ISO15099,
                     nperr,
                     ErrorMessage);
            GASSES90(state,
                     Tgap2,
                     iprop2,
                     frct2,
                     press2,
                     nmix2,
                     xwght,
                     xgcon,
                     xgvis,
                     xgcp,
                     con2,
                     visc2,
                     dens2,
                     cp2,
                     pr2,
                     TARCOGGassesParams::Stdrd::ISO15099,
                     nperr,
                     ErrorMessage);

            //  A = dens0 * T0 * GravityConstant * ABS(cos(tilt)) * ABS(Tgap1 - Tgap2) / (Tgap1 * Tgap2)

            // bi...Bug fix #00005:
            A = dens0 * T0 * Constant::Gravity * H * std::abs(cos_Tilt) * std::abs(Tgap1 - Tgap2) / (Tgap1 * Tgap2);

            if (A == 0.0) {
                qv1 = 0.0;
                qv2 = 0.0;
                speed1 = 0.0;
                speed2 = 0.0;
                hcv1 = 2.0 * hc1;
                hcv2 = 2.0 * hc2;
                return;
            }

            B1 = dens1 / 2;
            B2 = (dens2 / 2) * s1_s2_2;

            C1 = 12 * visc1 * H / s1_2;
            C2 = 12 * visc2 * (H / s2_2) * (s1 / s2);

            if (Tgap1 >= Tgap2) {
                A1eqin = Abot + 0.5 * Atop * (Al + Ar + Ah) / (Abot + Atop);
                A2eqout = Abot + 0.5 * Atop * (Al + Ar + Ah) / (Abot + Atop);
                A1eqout = Atop + 0.5 * Abot * (Al + Ar + Ah) / (Abot + Atop);
                A2eqin = Atop + 0.5 * Abot * (Al + Ar + Ah) / (Abot + Atop);
            } else if (Tgap1 < Tgap2) {
                A1eqout = Abot + 0.5 * Atop * (Al + Ar + Ah) / (Abot + Atop);
                A2eqin = Abot + 0.5 * Atop * (Al + Ar + Ah) / (Abot + Atop);
                A1eqin = Atop + 0.5 * Abot * (Al + Ar + Ah) / (Abot + Atop);
                A2eqout = Atop + 0.5 * Abot * (Al + Ar + Ah) / (Abot + Atop);
            }

            Zin1 = pow_2((s1 * L / (0.6 * A1eqin)) - 1.0);
            Zin2 = pow_2((s2 * L / (0.6 * A2eqin)) - 1.0);
            Zout1 = pow_2((s1 * L / (0.6 * A1eqout)) - 1.0);
            Zout2 = pow_2((s2 * L / (0.6 * A2eqout)) - 1.0);

            D1 = (dens1 / 2.0) * (Zin1 + Zout1);
            D2 = (dens2 / 2.0) * s1_s2_2 * (Zin2 + Zout2);

            A1 = B1 + D1 + B2 + D2;
            A2 = C1 + C2;

            speed1 = (std::sqrt(pow_2(A2) + std::abs(4.0 * A * A1)) - A2) / (2.0 * A1);
            speed2 = speed1 * s1 / s2;

            H01 = (dens1 * cp1 * s1 * speed1) / (4.0 * hc1 + 8.0 * speed1);
            H02 = (dens2 * cp2 * s2 * speed2) / (4.0 * hc2 + 8.0 * speed2);

            if ((H01 != 0.0) && (H02 != 0.0)) {
                P1 = -H / H01;
                P2 = -H / H02;
            }

            beta1 = std::pow(e, P1);
            beta2 = std::pow(e, P2);

            alpha1 = 1.0 - beta1;
            alpha2 = 1.0 - beta2;

            if (Tgap1 > Tgap2) {
                Tup = (alpha1 * Tav1 + beta1 * alpha2 * Tav2) / (1.0 - beta1 * beta2);
                Tdown = alpha2 * Tav2 + beta2 * Tup;
            } else if (Tgap2 >= Tgap1) {
                Tdown = (alpha1 * Tav1 + beta1 * alpha2 * Tav2) / (1.0 - beta1 * beta2);
                Tup = alpha2 * Tav2 + beta2 * Tdown;
            }

            TGapOld1 = Tgap1;
            TGapOld2 = Tgap2;

            if (Tgap1 > Tgap2) {
                Temp1 = Tav1 - (H01 / H) * (Tup - Tdown);
                Temp2 = Tav2 - (H02 / H) * (Tdown - Tup);
            } else if (Tgap2 >= Tgap1) {
                Temp1 = Tav1 - (H01 / H) * (Tdown - Tup);
                Temp2 = Tav2 - (H02 / H) * (Tup - Tdown);
            }

            Tgap1 = AirflowRelaxationParameter * Temp1 + (1.0 - AirflowRelaxationParameter) * TGapOld1;
            Tgap2 = AirflowRelaxationParameter * Temp2 + (1.0 - AirflowRelaxationParameter) * TGapOld2;

            converged = false;
            if ((std::abs(Tgap1 - TGapOld1) < AirflowConvergenceTolerance) || (iter >= NumOfIterations)) {
                if (std::abs(Tgap2 - TGapOld2) < AirflowConvergenceTolerance) {
                    converged = true;
                }
            }
        }

        hcv1 = 2.0 * hc1 + 4.0 * speed1;
        hcv2 = 2.0 * hc2 + 4.0 * speed2;

        if (Tgap2 >= Tgap1) {
            qv1 = -dens1 * cp1 * speed1 * s1 * L * (Tdown - Tup) / (H * L);
            qv2 = -dens2 * cp2 * speed2 * s2 * L * (Tup - Tdown) / (H * L);
        } else if (Tgap2 < Tgap1) {
            qv1 = dens1 * cp1 * speed1 * s1 * L * (Tdown - Tup) / (H * L);
            qv2 = dens2 * cp2 * speed2 * s2 * L * (Tup - Tdown) / (H * L);
        }

        //  write(*, *) Tup-Tdown
        //  write(*, 998) Tup - Tdown, qv1, qv2

        // 998  format(f15.9, f15.9, f15.9)

        // bi..  testing - velocities output file
        // bi      open(unit = 33, file = 'velocities.out', status='unknown', form='formatted', iostat = er)

        // bi  write(33, 987) speed1

        qsmooth = (std::abs(qv1) + std::abs(qv2)) / 2.0;

        if (qv1 > 0.0) {
            qv1 = qsmooth;
            qv2 = -qsmooth;
        } else {
            qv1 = -qsmooth;
            qv2 = qsmooth;
        }
    }

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
                     Real64 &speed)
    {
        //**************************************************************************************************************
        //  Input:
        //  iprop1      Vector of gas identifiers
        //  frct1      Fraction of gasses in a mixture
        //  nmix1      Number of gasses in a mixture
        //  press1      Pressure in mixture
        //  iprop2      Vector of gas identifiers
        //  frct2      Fraction of gasses in a mixture
        //  nmix2      Number of gasses in a mixture
        //  press2      Pressure in mixture
        //  Atop      Opening between top of shading device and top of glazing cavity [m^2]
        //  Abot      Opening between bottom of shading device and bottom of glazing cavity [m^2]
        //  Al        Opening between left of shading device and left end of glazing cavity [m^2]
        //  Ar        Opening between right of shading device and right end of glazing cavity [m^2]
        //  Ah        Total area holes in the shading device [m^2]
        //  s        Gap width [m]
        //  H        IGU cavity height [m]
        //  L        IGU cavity width [m]
        //  angle      Window angle [degrees]
        //  forcedspeed    Speed of forced ventilation [m/s]
        //  hc        Convective/conductive coefficient for non-vented gap
        //  Tenv      Environmental temperature
        //  Tav        Average temperature of gap surfaces
        //  Output:
        //  Tgap      Temperature of vented gap
        //  hcv        Convective/conductive coefficient for vented gap
        //  qv        Heat transfer to the gap by ventilation [W/m^2]
        //  nperr      Error flag
        //  speed      Air velocity
        //**************************************************************************************************************

        // Using/Aliasing
        // Argument array dimensioning
        EP_SIZE_CHECK(iprop1, maxgas);
        EP_SIZE_CHECK(frct1, maxgas);
        EP_SIZE_CHECK(iprop2, maxgas);
        EP_SIZE_CHECK(frct2, maxgas);
        EP_SIZE_CHECK(xwght, maxgas);
        xgcon.dim(3, maxgas);
        xgvis.dim(3, maxgas);
        xgcp.dim(3, maxgas);

        // Locals
        Real64 A;
        Real64 A1;
        Real64 A2;
        Real64 B1;
        Real64 C1;
        Real64 D1;
        Real64 Zin1;
        Real64 Zout1;
        Real64 A1eqin;
        Real64 A1eqout;
        Real64 T0;
        Real64 tilt;
        Real64 dens0;
        Real64 visc0;
        Real64 con0;
        Real64 pr0;
        Real64 cp0;
        // REAL(r64) :: dens1, visc1, con1, pr1, cp1
        Real64 dens2;
        Real64 visc2;
        Real64 con2;
        Real64 pr2;
        Real64 cp2;
        Real64 Tgapout;
        Real64 H0;
        Real64 P;
        Real64 beta;

        // iteration parameters
        int iter;
        Real64 TGapOld;
        bool converged;

        tilt = Constant::Pi / 180.0 * (angle - 90.0);
        T0 = 0.0 + Constant::Kelvin;

        GASSES90(state,
                 T0,
                 iprop1,
                 frct1,
                 press1,
                 nmix1,
                 xwght,
                 xgcon,
                 xgvis,
                 xgcp,
                 con0,
                 visc0,
                 dens0,
                 cp0,
                 pr0,
                 TARCOGGassesParams::Stdrd::ISO15099,
                 nperr,
                 ErrorMessage);
        // call gasses90(Tenv, iprop1, frct1, press1, nmix1, xwght, xgcon, xgvis, xgcp, con1, visc1, dens1, cp1, pr1, 1, &
        //                nperr, ErrorMessage)

        // exit on error:
        if ((nperr > 0) && (nperr < 1000)) {
            return;
        }

        // dr...check for error messages
        if ((Tgap * Tenv) == 0.0) {
            nperr = 15;
            ErrorMessage = "Temperature of vented air must be greater then 0 [K].";
            return;
        }

        if ((Atop + Abot) == 0) {
            //    nperr = 16
            //    return
            Atop = 0.000001;
            Abot = 0.000001;
        }
        if ((Ah + Al + Ar) == 0.0) {
            Ah = 0.000001;
        }

        converged = false;
        iter = 0;
        Real64 const s_2 = pow_2(s);
        Real64 const abs_cos_tilt = std::abs(std::cos(tilt));

        while (!converged) {
            ++iter;
            GASSES90(state,
                     Tgap,
                     iprop2,
                     frct2,
                     press2,
                     nmix2,
                     xwght,
                     xgcon,
                     xgvis,
                     xgcp,
                     con2,
                     visc2,
                     dens2,
                     cp2,
                     pr2,
                     TARCOGGassesParams::Stdrd::ISO15099,
                     nperr,
                     ErrorMessage);

            if ((nperr > 0) && (nperr < 1000)) {
                return;
            }

            //  A = dens0 * T0 * gravity * ABS(cos(tilt)) * ABS(Tgap - Tenv) / (Tgap * Tenv)

            // bi...Bug fix #00005:
            A = dens0 * T0 * Constant::Gravity * H * abs_cos_tilt * std::abs(Tgap - Tenv) / (Tgap * Tenv);
            //  A = dens0 * T0 * GravityConstant * H * ABS(cos(tilt)) * (Tgap - Tenv) / (Tgap * Tenv)

            B1 = dens2 / 2;
            C1 = 12.0 * visc2 * H / s_2;

            if (Tgap > Tenv) {
                A1eqin = Abot + 0.5 * Atop * (Al + Ar + Ah) / (Abot + Atop);
                A1eqout = Atop + 0.5 * Abot * (Al + Ar + Ah) / (Abot + Atop);
            } else if (Tgap <= Tenv) {
                A1eqout = Abot + 0.5 * Atop * (Al + Ar + Ah) / (Abot + Atop);
                A1eqin = Atop + 0.5 * Abot * (Al + Ar + Ah) / (Abot + Atop);
            }

            Zin1 = pow_2((s * L / (0.6 * A1eqin)) - 1);
            Zout1 = pow_2((s * L / (0.6 * A1eqout)) - 1);

            D1 = (dens2 / 2.0) * (Zin1 + Zout1);

            A1 = B1 + D1;
            A2 = C1;

            // dr...recalculate speed if forced speed exist
            // bi...skip forced vent for now
            //  if (forcedspeed.ne.0) then
            if ((forcedspeed != 0.0) && (static_cast<int>(CalcForcedVentilation::Allow))) {
                speed = forcedspeed;
            } else {
                speed = (std::sqrt(pow_2(A2) + std::abs(4.0 * A * A1)) - A2) / (2.0 * A1);
                //  speed = ABS((SQRT((A2 ** 2) + (4 * A * A1)) - A2) / (2 * A1))
            }

            TGapOld = Tgap;

            // Speed is zero when environment temperature is equal to average layer temperatures
            // For example, this can happen when inside and outside temperatures are equal
            if (speed != 0.0) {
                H0 = (dens2 * cp2 * s * speed) / (4.0 * hc + 8.0 * speed);

                P = -H / H0;
                if (P < -700.0) {
                    beta = 0.0;
                } else {
                    beta = std::pow(e, P);
                }
                Tgapout = Tav - (Tav - Tenv) * beta;
                Tgap = Tav - (H0 / H) * (Tgapout - Tenv);
            } else {
                Tgapout = Tav;
                Tgap = Tav;
            }

            converged = false;
            if ((std::abs(Tgap - TGapOld) < AirflowConvergenceTolerance) || (iter >= NumOfIterations)) {
                converged = true;
            }

            // if (iter > NumOfIterations) then
            //  converged = .TRUE.
            // end if
        }

        // bi...Test output:
        //  write(*,101) tenv, tgap, tgapout
        // 101  format(f15.9, f15.9, f15.9)

        hcv = 2.0 * hc + 4.0 * speed;

        qv = dens2 * cp2 * speed * s * L * (Tenv - Tgapout) / (H * L);
    }

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
    )
    {
        for (int i = 1; i <= nlayer; ++i) {
            if (LayerType(i) == TARCOGLayerType::VENETBLIND_HORIZ || LayerType(i) == TARCOGLayerType::VENETBLIND_VERT) {
                const Real64 slatAngRad = SlatAngle(i) * 2 * Constant::Pi / 360;
                Real64 C1_VENET(0);
                Real64 C2_VENET(0);
                Real64 C3_VENET(0);

                if (LayerType(i) == TARCOGLayerType::VENETBLIND_HORIZ) {
                    C1_VENET = C1_VENET_HORIZONTAL;
                    C2_VENET = C2_VENET_HORIZONTAL;
                    C3_VENET = C3_VENET_HORIZONTAL;
                }
                if (LayerType(i) == TARCOGLayerType::VENETBLIND_VERT) {
                    C1_VENET = C1_VENET_VERTICAL;
                    C2_VENET = C2_VENET_VERTICAL;
                    C3_VENET = C3_VENET_VERTICAL;
                }
                Ah_eff(i) = width * height * C1_VENET * pow((Ah(i) / (width * height)) * pow(cos(slatAngRad), C2_VENET), C3_VENET);
                Al_eff(i) = 0.0;
                Ar_eff(i) = 0.0;
                Atop_eff(i) = Atop(i);
                Abot_eff(i) = Abot(i);
            } else if ((LayerType(i) == TARCOGLayerType::PERFORATED) || (LayerType(i) == TARCOGLayerType::DIFFSHADE) ||
                       (LayerType(i) == TARCOGLayerType::BSDF) || (LayerType(i) == TARCOGLayerType::WOVSHADE)) {
                Ah_eff(i) = width * height * C1_SHADE * pow((Ah(i) / (width * height)), C2_SHADE);
                Al_eff(i) = Al(i) * C3_SHADE;
                Ar_eff(i) = Ar(i) * C3_SHADE;
                Atop_eff(i) = Atop(i) * C4_SHADE;
                Abot_eff(i) = Abot(i) * C4_SHADE;
            } else {
                Ah_eff(i) = Ah(i);
                Al_eff(i) = Al(i);
                Ar_eff(i) = Ar(i);
                Atop_eff(i) = Atop(i);
                Abot_eff(i) = Abot(i);
            }
        }
    }

} // namespace TarcogShading

} // namespace EnergyPlus
