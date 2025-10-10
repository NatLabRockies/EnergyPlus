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

// Adapted from the code Version 1.1 released by Yi-Chuan Lu on February 23, 2023.
//
// @article{20heatindex,
//   Title   = {Extending the Heat Index},
//   Author  = {Yi-Chuan Lu and David M. Romps},
//   Journal = {Journal of Applied Meteorology and Climatology},
//   Year    = {2022},
//   Volume  = {61},
//   Number  = {10},
//   Pages   = {1367--1383},
//   Year    = {2022},
// }
//

#include <cmath>

#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/ExtendedHeatIndex.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HVACSystemRootFindingAlgorithm.hh>

namespace EnergyPlus {

namespace ExtendedHI {

    // The saturation vapor pressure
    Real64 pvstar(Real64 const T)
    {
        // Thermodynamic parameters
        constexpr Real64 Ttrip = 273.16; // K
        constexpr Real64 ptrip = 611.65; // Pa
        constexpr Real64 E0v = 2.3740e6; // J/kg
        constexpr Real64 E0s = 0.3337e6; // J/kg
        constexpr Real64 rgasa = 287.04; // J/kg/K
        constexpr Real64 rgasv = 461.;   // J/kg/K
        constexpr Real64 cva = 719.;     // J/kg/K
        constexpr Real64 cvv = 1418.;    // J/kg/K
        constexpr Real64 cvl = 4119.;    // J/kg/K
        constexpr Real64 cvs = 1861.;    // J/kg/K
        constexpr Real64 cpa = cva + rgasa;
        constexpr Real64 cpv = cvv + rgasv;

        if (T == 0.0) {
            return 0.0;
        } else if (T < Ttrip) {
            return ptrip * pow((T / Ttrip), ((cpv - cvs) / rgasv)) * exp((E0v + E0s - (cvv - cvs) * Ttrip) / rgasv * (1. / Ttrip - 1. / T));
        } else {
            return ptrip * pow((T / Ttrip), ((cpv - cvl) / rgasv)) * exp((E0v - (cvv - cvl) * Ttrip) / rgasv * (1. / Ttrip - 1. / T));
        }
        return 0.0;
    }

    // The latent heat of vaporization of water
    Real64 Le(Real64 const T)
    {
        // Thermodynamic parameters
        constexpr Real64 Ttrip = 273.16; // K
        constexpr Real64 E0v = 2.3740e6; // J/kg
        constexpr Real64 rgasv = 461.;   // J/kg/K
        constexpr Real64 cvv = 1418.;    // J/kg/K
        constexpr Real64 cvl = 4119.;    // J/kg/K
        return (E0v + (cvv - cvl) * (T - Ttrip) + rgasv * T);
    }

    // Function to calculate respiratory heat loss, W/m^2
    Real64 Qv(Real64 const Ta, Real64 const Pa)
    {
        constexpr Real64 Q = 180.;         // W/m^2     , metabolic rate per skin area, steadman1979
        constexpr Real64 phi_salt = 0.9;   //           , vapor saturation pressure level of saline solution, steadman1979
        constexpr Real64 Tc = 310.;        // K         , core temperature, steadman1979
        Real64 Pc = phi_salt * pvstar(Tc); //           , core vapor pressure
        constexpr Real64 r = 124.;         // Pa/K      , Zf/Rf, steadman1979
        constexpr Real64 eta = 1.43e-6;    // kg/J      , "inhaled mass" / "metabolic rate", steadman1979
        Real64 L = Le(310.);               //           , latent heat of vaporization at 310 K
        constexpr Real64 p = 1.013e5;      // Pa        , atmospheric pressure
        constexpr Real64 rgasa = 287.04;   // J/kg/K
        constexpr Real64 rgasv = 461.;     // J/kg/K
        constexpr Real64 cva = 719.;       // J/kg/K
        constexpr Real64 cvv = 1418.;      // J/kg/K
        constexpr Real64 cpa = cva + rgasa;
        constexpr Real64 cpv = cvv + rgasv;

        return eta * Q * (cpa * (Tc - Ta) + L * rgasa / (p * rgasv) * (Pc - Pa));
    }

    // Function to calculate mass transfer resistance through skin, Pa m^2/W
    Real64 Zs(Real64 const Rs)
    {
        return (Rs == 0.0387) ? 52.1 : 6.0e8 * std::pow(Rs, 5);
    }

    // Function to calculate heat transfer resistance through air, exposed part of skin, K m^2/W
    Real64 Ra(Real64 const Ts, Real64 const Ta)
    {
        constexpr Real64 sigma = 5.67e-8; // W/m^2/K^4 , Stefan-Boltzmann constant
        constexpr Real64 epsilon = 0.97;  //           , emissivity of surface, steadman1979
        constexpr Real64 hc = 17.4;
        constexpr Real64 phi_rad = 0.85;
        Real64 const hr = epsilon * phi_rad * sigma * (std::pow(Ts, 2) + std::pow(Ta, 2)) * (Ts + Ta);
        return 1.0 / (hc + hr);
    }

    // Function to calculate heat transfer resistance through air, clothed part of skin, K m^2/W
    Real64 Ra_bar(Real64 const Tf, Real64 const Ta)
    {
        constexpr Real64 sigma = 5.67e-8; // W/m^2/K^4 , Stefan-Boltzmann constant
        constexpr Real64 epsilon = 0.97;  //           , emissivity of surface, steadman1979
        constexpr Real64 hc = 11.6;
        constexpr Real64 phi_rad = 0.79;
        Real64 const hr = epsilon * phi_rad * sigma * (std::pow(Tf, 2) + std::pow(Ta, 2)) * (Tf + Ta);
        return 1.0 / (hc + hr);
    }

    // Function to calculate heat transfer resistance through air, when being naked, K m^2/W
    Real64 Ra_un(Real64 const Ts, Real64 const Ta)
    {
        constexpr Real64 sigma = 5.67e-8; // W/m^2/K^4 , Stefan-Boltzmann constant
        constexpr Real64 epsilon = 0.97;  //           , emissivity of surface, steadman1979
        constexpr Real64 hc = 12.3;
        constexpr Real64 phi_rad = 0.80;
        Real64 const hr = epsilon * phi_rad * sigma * (std::pow(Ts, 2) + std::pow(Ta, 2)) * (Ts + Ta);
        return 1.0 / (hc + hr);
    }

    constexpr Real64 tol = 1e-5;
    constexpr Real64 maxIter = 100;
    Real64 find_eqvar_phi(EnergyPlusData &state, Real64 const Ta, Real64 const RH)
    {
        constexpr Real64 Q = 180.;         // W/m^2     , metabolic rate per skin area, steadman1979
        constexpr Real64 phi_salt = 0.9;   //           , vapor saturation pressure level of saline solution, steadman1979
        constexpr Real64 Tc = 310.;        // K         , core temperature, steadman1979
        Real64 Pc = phi_salt * pvstar(Tc); //           , core vapor pressure
        constexpr Real64 Za = 60.6 / 17.4; // Pa m^2/W, mass transfer resistance through air, exposed part of skin

        Real64 phi = 0.84;
        Real64 const Pa = RH * pvstar(Ta);
        constexpr Real64 Rs = 0.0387;
        Real64 const ZsRs = Zs(Rs);
        Real64 const m = (Pc - Pa) / (ZsRs + Za);
        Real64 Ts;
        int SolFla;
        General::SolveRoot(
            state,
            tol,
            maxIter,
            SolFla,
            Ts,
            [&](Real64 Ts) { return (Ts - Ta) / Ra(Ts, Ta) + (Pc - Pa) / (ZsRs + Za) - (Tc - Ts) / Rs; },
            std::max(0.0, std::min(Tc, Ta) - Rs * std::abs(m)),
            std::max(Tc, Ta) + Rs * std::abs(m));
        Real64 const flux1 = Q - Qv(Ta, Pa) - (1.0 - phi) * (Tc - Ts) / Rs;
        if (flux1 <= 0.0) {
            phi = 1.0 - (Q - Qv(Ta, Pa)) * Rs / (Tc - Ts);
        }
        return phi;
    }

    Real64 find_eqvar_Rf(EnergyPlusData &state, Real64 const Ta, Real64 const RH)
    {
        constexpr Real64 Q = 180.;             // W/m^2     , metabolic rate per skin area, steadman1979
        constexpr Real64 phi_salt = 0.9;       //           , vapor saturation pressure level of saline solution, steadman1979
        constexpr Real64 Tc = 310.;            // K         , core temperature, steadman1979
        Real64 Pc = phi_salt * pvstar(Tc);     //           , core vapor pressure
        constexpr Real64 r = 124.;             // Pa/K      , Zf/Rf, steadman1979
        constexpr Real64 Za = 60.6 / 17.4;     // Pa m^2/W, mass transfer resistance through air, exposed part of skin
        constexpr Real64 Za_bar = 60.6 / 11.6; // Pa m^2/W, mass transfer resistance through air, clothed part of skin

        Real64 Pa = RH * pvstar(Ta);
        constexpr Real64 Rs = 0.0387;
        constexpr Real64 phi = 0.84;
        Real64 const ZsRs = Zs(Rs);
        Real64 const m_bar = (Pc - Pa) / (ZsRs + Za_bar);
        Real64 const m = (Pc - Pa) / (ZsRs + Za);
        Real64 Ts;
        int SolFla;
        General::SolveRoot(
            state,
            tol,
            maxIter,
            SolFla,
            Ts,
            [&](Real64 Ts) { return (Ts - Ta) / Ra(Ts, Ta) + (Pc - Pa) / (ZsRs + Za) - (Tc - Ts) / Rs; },
            std::max(0.0, std::min(Tc, Ta) - Rs * std::abs(m)),
            std::max(Tc, Ta) + Rs * std::abs(m));
        Real64 Tf;
        General::SolveRoot(
            state,
            tol,
            maxIter,
            SolFla,
            Tf,
            [&](Real64 Tf) { return (Tf - Ta) / Ra_bar(Tf, Ta) + (Pc - Pa) / (ZsRs + Za_bar) - (Tc - Tf) / Rs; },
            std::max(0.0, std::min(Tc, Ta) - Rs * std::abs(m_bar)),
            std::max(Tc, Ta) + Rs * std::abs(m_bar));
        Real64 const flux1 = Q - Qv(Ta, Pa) - (1.0 - phi) * (Tc - Ts) / Rs;
        Real64 const flux2 = Q - Qv(Ta, Pa) - (1.0 - phi) * (Tc - Ts) / Rs - phi * (Tc - Tf) / Rs;
        Real64 Rf;
        if (flux1 <= 0.0) {
            Rf = std::numeric_limits<Real64>::infinity();
        } else if (flux2 <= 0.0) {
            Real64 const Ts_bar = Tc - (Q - Qv(Ta, Pa)) * Rs / phi + (1.0 / phi - 1.0) * (Tc - Ts);
            General::SolveRoot(
                state,
                tol,
                maxIter,
                SolFla,
                Tf,
                [&](Real64 Tf) {
                    return (Tf - Ta) / Ra_bar(Tf, Ta) + (Pc - Pa) * (Tf - Ta) / ((ZsRs + Za_bar) * (Tf - Ta) + r * Ra_bar(Tf, Ta) * (Ts_bar - Tf)) -
                           (Tc - Ts_bar) / Rs;
                },
                Ta,
                Ts_bar);
            Rf = Ra_bar(Tf, Ta) * (Ts_bar - Tf) / (Tf - Ta);
        } else {
            Rf = 0.0;
        }
        return Rf;
    }

    Real64 find_eqvar_rs(EnergyPlusData &state, Real64 const Ta, Real64 const RH)
    {
        constexpr Real64 M = 83.6;                        // kg        , mass of average US adults, fryar2018
        constexpr Real64 H = 1.69;                        // m         , height of average US adults, fryar2018
        Real64 A = 0.202 * pow(M, 0.425) * pow(H, 0.725); // m^2       , DuBois formula, parson2014
        constexpr Real64 cpc = 3492.;                     // J/kg/K    , specific heat capacity of core, gagge1972
        Real64 C = M * cpc / A;                           //           , heat capacity of core
        constexpr Real64 Q = 180.;                        // W/m^2     , metabolic rate per skin area, steadman1979
        constexpr Real64 phi_salt = 0.9;                  //           , vapor saturation pressure level of saline solution, steadman1979
        constexpr Real64 Tc = 310.;                       // K         , core temperature, steadman1979
        Real64 Pc = phi_salt * pvstar(Tc);                //           , core vapor pressure
        constexpr Real64 Za = 60.6 / 17.4;                // Pa m^2/W, mass transfer resistance through air, exposed part of skin
        constexpr Real64 Za_bar = 60.6 / 11.6;            // Pa m^2/W, mass transfer resistance through air, clothed part of skin
        constexpr Real64 Za_un = 60.6 / 12.3;             // Pa m^2/W, mass transfer resistance through air, when being naked

        Real64 Pa = RH * pvstar(Ta);
        constexpr Real64 phi = 0.84;
        Real64 Rs = 0.0387;
        Real64 ZsRs = Zs(Rs);
        Real64 const m = (Pc - Pa) / (ZsRs + Za);
        Real64 const m_bar = (Pc - Pa) / (ZsRs + Za_bar);
        Real64 Ts;
        int SolFla;
        General::SolveRoot(
            state,
            tol,
            maxIter,
            SolFla,
            Ts,
            [&](Real64 Ts) { return (Ts - Ta) / Ra(Ts, Ta) + (Pc - Pa) / (ZsRs + Za) - (Tc - Ts) / Rs; },
            std::max(0.0, std::min(Tc, Ta) - Rs * std::abs(m)),
            std::max(Tc, Ta) + Rs * std::abs(m));
        Real64 Tf;
        General::SolveRoot(
            state,
            tol,
            maxIter,
            SolFla,
            Tf,
            [&](Real64 Tf) { return (Tf - Ta) / Ra_bar(Tf, Ta) + (Pc - Pa) / (ZsRs + Za_bar) - (Tc - Tf) / Rs; },
            std::max(0.0, std::min(Tc, Ta) - Rs * std::abs(m_bar)),
            std::max(Tc, Ta) + Rs * std::abs(m_bar));
        Real64 const flux1 = Q - Qv(Ta, Pa) - (1.0 - phi) * (Tc - Ts) / Rs;
        Real64 const flux2 = Q - Qv(Ta, Pa) - (1.0 - phi) * (Tc - Ts) / Rs - phi * (Tc - Tf) / Rs;
        Real64 const flux3 = Q - Qv(Ta, Pa) - (Tc - Ta) / Ra_un(Tc, Ta) - (phi_salt * pvstar(Tc) - Pa) / Za_un;
        if (flux1 > 0 && flux2 > 0) {
            if (flux3 < 0.0) {
                General::SolveRoot(
                    state,
                    tol,
                    maxIter,
                    SolFla,
                    Ts,
                    [&](Real64 Ts) { return (Ts - Ta) / Ra_un(Ts, Ta) + (Pc - Pa) / (Zs((Tc - Ts) / (Q - Qv(Ta, Pa))) + Za_un) - (Q - Qv(Ta, Pa)); },
                    0.0,
                    Tc);
                Rs = (Tc - Ts) / (Q - Qv(Ta, Pa));
                ZsRs = Zs(Rs);
                Real64 Ps = Pc - (Pc - Pa) * ZsRs / (ZsRs + Za_un);
                if (Ps > phi_salt * pvstar(Ts)) {
                    General::SolveRoot(
                        state,
                        tol,
                        maxIter,
                        SolFla,
                        Ts,
                        [&](Real64 Ts) { return (Ts - Ta) / Ra_un(Ts, Ta) + (phi_salt * pvstar(Ts) - Pa) / Za_un - (Q - Qv(Ta, Pa)); },
                        0.0,
                        Tc);
                    Rs = (Tc - Ts) / (Q - Qv(Ta, Pa));
                }
            } else {
                Rs = 0.0;
            }
        }
        return Rs;
    }

    Real64 find_eqvar_dTcdt(EnergyPlusData &state, Real64 const Ta, Real64 const RH)
    {
        constexpr Real64 M = 83.6;                        // kg        , mass of average US adults, fryar2018
        constexpr Real64 H = 1.69;                        // m         , height of average US adults, fryar2018
        Real64 A = 0.202 * pow(M, 0.425) * pow(H, 0.725); // m^2       , DuBois formula, parson2014
        constexpr Real64 cpc = 3492.;                     // J/kg/K    , specific heat capacity of core, gagge1972
        Real64 C = M * cpc / A;                           //           , heat capacity of core
        constexpr Real64 Q = 180.;                        // W/m^2     , metabolic rate per skin area, steadman1979
        constexpr Real64 phi_salt = 0.9;                  //           , vapor saturation pressure level of saline solution, steadman1979
        constexpr Real64 Tc = 310.;                       // K         , core temperature, steadman1979
        Real64 Pc = phi_salt * pvstar(Tc);                //           , core vapor pressure
        constexpr Real64 Za = 60.6 / 17.4;                // Pa m^2/W, mass transfer resistance through air, exposed part of skin
        constexpr Real64 Za_bar = 60.6 / 11.6;            // Pa m^2/W, mass transfer resistance through air, clothed part of skin
        constexpr Real64 Za_un = 60.6 / 12.3;             // Pa m^2/W, mass transfer resistance through air, when being naked

        Real64 dTcdt = 0.0;
        Real64 const Pa = RH * pvstar(Ta);
        constexpr Real64 Rs = 0.0387;
        Real64 const ZsRs = Zs(Rs);
        constexpr Real64 phi = 0.84;
        Real64 const m = (Pc - Pa) / (ZsRs + Za);
        Real64 const m_bar = (Pc - Pa) / (ZsRs + Za_bar);
        Real64 Ts;
        int SolFla;
        General::SolveRoot(
            state,
            tol,
            maxIter,
            SolFla,
            Ts,
            [&](Real64 Ts) { return (Ts - Ta) / Ra(Ts, Ta) + (Pc - Pa) / (ZsRs + Za) - (Tc - Ts) / Rs; },
            std::max(0.0, std::min(Tc, Ta) - Rs * std::abs(m)),
            std::max(Tc, Ta) + Rs * std::abs(m));
        Real64 Tf;
        General::SolveRoot(
            state,
            tol,
            maxIter,
            SolFla,
            Tf,
            [&](Real64 Tf) { return (Tf - Ta) / Ra_bar(Tf, Ta) + (Pc - Pa) / (ZsRs + Za_bar) - (Tc - Tf) / Rs; },
            std::max(0.0, std::min(Tc, Ta) - Rs * std::abs(m_bar)),
            std::max(Tc, Ta) + Rs * std::abs(m_bar));
        Real64 const flux1 = Q - Qv(Ta, Pa) - (1.0 - phi) * (Tc - Ts) / Rs;
        Real64 const flux2 = Q - Qv(Ta, Pa) - (1.0 - phi) * (Tc - Ts) / Rs - phi * (Tc - Tf) / Rs;
        Real64 const flux3 = Q - Qv(Ta, Pa) - (Tc - Ta) / Ra_un(Tc, Ta) - (phi_salt * pvstar(Tc) - Pa) / Za_un;
        if (flux1 > 0.0 && flux2 > 0.0 && flux3 >= 0.0) {
            dTcdt = (1.0 / C) * flux3;
        }
        return dTcdt;
    }

    //    given T and RH, returns a key and value pair
    Real64 find_eqvar_name_and_value(EnergyPlusData &state, Real64 const Ta, Real64 const RH, EqvarName &varname)
    {
        constexpr Real64 M = 83.6;                        // kg        , mass of average US adults, fryar2018
        constexpr Real64 H = 1.69;                        // m         , height of average US adults, fryar2018
        Real64 A = 0.202 * pow(M, 0.425) * pow(H, 0.725); // m^2       , DuBois formula, parson2014
        constexpr Real64 cpc = 3492.;                     // J/kg/K    , specific heat capacity of core, gagge1972
        Real64 C = M * cpc / A;                           //           , heat capacity of core
        constexpr Real64 Q = 180.;                        // W/m^2     , metabolic rate per skin area, steadman1979
        constexpr Real64 phi_salt = 0.9;                  //           , vapor saturation pressure level of saline solution, steadman1979
        constexpr Real64 Tc = 310.;                       // K         , core temperature, steadman1979
        Real64 Pc = phi_salt * pvstar(Tc);                //           , core vapor pressure
        constexpr Real64 r = 124.;                        // Pa/K      , Zf/Rf, steadman1979
        constexpr Real64 Za = 60.6 / 17.4;                // Pa m^2/W, mass transfer resistance through air, exposed part of skin
        constexpr Real64 Za_bar = 60.6 / 11.6;            // Pa m^2/W, mass transfer resistance through air, clothed part of skin
        constexpr Real64 Za_un = 60.6 / 12.3;             // Pa m^2/W, mass transfer resistance through air, when being naked

        Real64 Pa = RH * pvstar(Ta);
        Real64 Rs = 0.0387;
        Real64 phi = 0.84;
        Real64 dTcdt = 0.0;
        Real64 ZsRs = Zs(Rs);
        Real64 const m = (Pc - Pa) / (ZsRs + Za);
        Real64 const m_bar = (Pc - Pa) / (ZsRs + Za_bar);

        int SolFla;
        Real64 Ts;
        General::SolveRoot(
            state,
            tol,
            maxIter,
            SolFla,
            Ts,
            [&](Real64 Ts) { return (Ts - Ta) / Ra(Ts, Ta) + (Pc - Pa) / (ZsRs + Za) - (Tc - Ts) / Rs; },
            std::max(0.0, std::min(Tc, Ta) - Rs * std::abs(m)),
            std::max(Tc, Ta) + Rs * std::abs(m));

        Real64 Tf;
        General::SolveRoot(
            state,
            tol,
            maxIter,
            SolFla,
            Tf,
            [&](Real64 Tf) { return (Tf - Ta) / Ra_bar(Tf, Ta) + (Pc - Pa) / (ZsRs + Za_bar) - (Tc - Tf) / Rs; },
            std::max(0.0, std::min(Tc, Ta) - Rs * std::abs(m_bar)),
            std::max(Tc, Ta) + Rs * std::abs(m_bar));
        Real64 const flux1 = Q - Qv(Ta, Pa) - (1.0 - phi) * (Tc - Ts) / Rs;
        Real64 const flux2 = Q - Qv(Ta, Pa) - (1.0 - phi) * (Tc - Ts) / Rs - phi * (Tc - Tf) / Rs;
        Real64 Rf;

        if (flux1 <= 0.0) {
            varname = EqvarName::Phi;
            phi = 1.0 - (Q - Qv(Ta, Pa)) * Rs / (Tc - Ts);
            return phi;
        } else if (flux2 <= 0.0) {
            varname = EqvarName::Rf;
            Real64 const Ts_bar = Tc - (Q - Qv(Ta, Pa)) * Rs / phi + (1.0 / phi - 1.0) * (Tc - Ts);
            General::SolveRoot(
                state,
                tol,
                maxIter,
                SolFla,
                Tf,
                [&](Real64 Tf) {
                    return (Tf - Ta) / Ra_bar(Tf, Ta) + (Pc - Pa) * (Tf - Ta) / ((ZsRs + Za_bar) * (Tf - Ta) + r * Ra_bar(Tf, Ta) * (Ts_bar - Tf)) -
                           (Tc - Ts_bar) / Rs;
                },
                Ta,
                Ts_bar);
            Rf = Ra_bar(Tf, Ta) * (Ts_bar - Tf) / (Tf - Ta);
            return Rf;
        } else {
            Real64 const flux3 = Q - Qv(Ta, Pa) - (Tc - Ta) / Ra_un(Tc, Ta) - (phi_salt * pvstar(Tc) - Pa) / Za_un;
            if (flux3 < 0.0) {
                varname = EqvarName::Rs;
                General::SolveRoot(
                    state,
                    tol,
                    maxIter,
                    SolFla,
                    Ts,
                    [&](Real64 Ts) { return (Ts - Ta) / Ra_un(Ts, Ta) + (Pc - Pa) / (Zs((Tc - Ts) / (Q - Qv(Ta, Pa))) + Za_un) - (Q - Qv(Ta, Pa)); },
                    0.0,
                    Tc);
                Rs = (Tc - Ts) / (Q - Qv(Ta, Pa));
                ZsRs = Zs(Rs);
                Real64 const Ps = Pc - (Pc - Pa) * ZsRs / (ZsRs + Za_un);
                if (Ps > phi_salt * pvstar(Ts)) {
                    General::SolveRoot(
                        state,
                        tol,
                        maxIter,
                        SolFla,
                        Ts,
                        [&](Real64 Ts) { return (Ts - Ta) / Ra_un(Ts, Ta) + (phi_salt * pvstar(Ts) - Pa) / Za_un - (Q - Qv(Ta, Pa)); },
                        0.0,
                        Tc);
                    Rs = (Tc - Ts) / (Q - Qv(Ta, Pa));
                }
                return Rs;
            } else {
                varname = EqvarName::DTcdt;
                Rs = 0.0;
                dTcdt = (1.0 / C) * flux3;
                return dTcdt;
            }
        }
    }

    // Convert the find_T function
    Real64 find_T(EnergyPlusData &state, EqvarName const eqvar_name, Real64 const eqvar)
    {
        Real64 T;
        int SolFla;
        constexpr Real64 Pa0 = 1.6e3; // Pa        , reference air vapor pressure in regions III, IV, V, VI, steadman1979

        if (eqvar_name == EqvarName::Phi) {
            General::SolveRoot(state, tol, maxIter, SolFla, T, [&](Real64 T) { return find_eqvar_phi(state, T, 1.0) - eqvar; }, 0.0, 240.0);
        } else if (eqvar_name == EqvarName::Rf) {
            General::SolveRoot(
                state,
                tol,
                maxIter,
                SolFla,
                T,
                [&](Real64 T) { return (find_eqvar_Rf(state, T, std::min(1.0, Pa0 / pvstar(T)))) - eqvar; },
                230.0,
                300.0);
        } else if (eqvar_name == EqvarName::Rs) {
            General::SolveRoot(
                state, tol, maxIter, SolFla, T, [&](Real64 T) { return find_eqvar_rs(state, T, Pa0 / pvstar(T)) - eqvar; }, 295.0, 350.0);
        } else {
            General::SolveRoot(
                state, tol, maxIter, SolFla, T, [&](Real64 T) { return find_eqvar_dTcdt(state, T, Pa0 / pvstar(T)) - eqvar; }, 340.0, 1000.0);
        }

        return T;
    }

    Real64 heatindex(EnergyPlusData &state, Real64 const Ta, Real64 const RH)
    {

        // Ta: temperature in Kelvin
        // RH: relative humidity in range of 0.0 to 1.0
        // The function computes the extended heat index, in Kelvinn

        auto const HVACSystemRootSolverMethodBackup = state.dataRootFinder->HVACSystemRootFinding.HVACSystemRootSolverMethod;
        state.dataRootFinder->HVACSystemRootFinding.HVACSystemRootSolverMethod = HVACSystemRootSolverAlgorithm::ShortBisectionThenRegulaFalsi;
        EqvarName eqvar_name = EqvarName::Invalid;
        Real64 const eqvar_value = find_eqvar_name_and_value(state, Ta, RH, eqvar_name);

        Real64 T = find_T(state, eqvar_name, eqvar_value);

        if (Ta == 0.0) {
            T = 0.0;
        }

        state.dataRootFinder->HVACSystemRootFinding.HVACSystemRootSolverMethod = HVACSystemRootSolverMethodBackup;
        return T;
    }

} // namespace ExtendedHI
} // namespace EnergyPlus
