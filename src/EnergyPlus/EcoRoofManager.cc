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
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataWater.hh>
#include <EnergyPlus/EcoRoofManager.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/SolarShading.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace EcoRoofManager {
    // Module containing the heat balance simulation routines
    // calculation (initialization) routines

    // MODULE INFORMATION:
    //       AUTHOR         David Sailor and Toan Pham, Portland State University
    //       DATE WRITTEN   Jan 2007
    //       MODIFIED       Oct 2010

    // PURPOSE OF THIS MODULE:
    // Module for implementing an ecoroof (aka Green Roof)

    // METHODOLOGY EMPLOYED:
    // Vikram Madhusudan's Portland State Univ. MS Thesis (Dec 2005) based on FASST model
    // of Frankenstein and Koenig (2004) - DRDC/CRREL Technical Report TR-04-25.
    // Precipitation schedules and irrigation schedules can be used to define hourly moisture
    // inputs (m). Moisture transport updated Oct 2010.

    void CalcEcoRoof(EnergyPlusData &state,
                     int const SurfNum,   // Indicator of Surface Number for the current surface
                     int const ConstrNum, // Indicator for construction index for the current surface
                     Real64 &TempExt      // Exterior temperature boundary condition
    )
    {
        // SUBROUTINE INFORMATION
        //     AUTHOR          David Sailor and Toan Pham
        //     DATE WRITTEN    January 2007
        //     MODIFIED        David Sailor - to fix initialization between DD runs and during warm-up

        // PURPOSE OF THIS MODULE:
        // To calculate the heat balance for surfaces with eco roof specified as outside surface
        // Note that only ONE ecoroof construction can be employed at present time. If multiple
        // surfaces have ecoroof as the outside layer the energy balance is only calculated for
        // the first such surface.

        // METHODOLOGY EMPLOYED:
        // Vikram Madhusudan's Portland State Univ. MS Thesis (Dec 2005) based on FASST model
        // of Frankenstein and Koenig (2004) - DRDC/CRREL Technical Report TR-04-25.
        // Some data used herein are from: European Centre for Medium-Range Weather Forecasts (ECMWF)
        // IFS Documentation, CY25R1 (April 2002), www.ecmwf.int/research/ifsdocs/CY24r1/Physics/
        // Physics-08-03.html.
        // The Atmospheric Boundary Layer - by J.R. Garratt (Cambridge Atmos. & Space Science Series), 316pp.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr Kv(0.4);           // Von Karmen's constant (source FASST)
        Real64 constexpr rch(0.63);         // Turbulent Schimdt Number
        Real64 constexpr rche(0.71);        // Turbulent Prandtl Number
        Real64 constexpr Rair(0.286e3);     // Gas Constant of air J/Kg K
        Real64 constexpr g1(9.81);          // Gravity. In m/sec^2.
        Real64 constexpr Sigma(5.6697e-08); // Stefan-Boltzmann constant W/m^2K^4
        Real64 constexpr Cpa(1005.6);       // Specific heat of Water Vapor. (J/Kg.K)

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Material::SurfaceRoughness RoughSurf; // Roughness index of the exterior (ecoroof) surface.
        Real64 Tgk;                           // Ground temperature in Kelvin
        Real64 Ta;                            // current air temperature
        Real64 Waf;                           // Windspeed within canopy (m/s)
        Real64 qaf;                           // mixing ratio of air near canopy
        Real64 qg;                            // mixing ratio of air at surface.
        Real64 RS;                            // shortwave radiation
        Real64 EpsilonOne;
        Real64 eair;
        Real64 Rhoa;
        Real64 Tak;
        Real64 qa; // mixing ratio of air
        Real64 Tafk;
        Real64 Taf;
        Real64 Rhof;
        Real64 Rhoaf; // Average air density
        Real64 sigmaf;
        Real64 Zd;     // zero displacement height (m)
        Real64 Zo;     // foliage roughness length (m)
        Real64 Cfhn;   // transfer coefficient at near-neutral conditions
        Real64 Cf;     // bulk Transfer coefficient, equation 10 page 6 (FASST).
        Real64 ra;     // Aerodynamic Resistance
        Real64 f2inv;  // intermediate calculation variable
        Real64 f1;     // intermediate calculation variable
        Real64 f2;     // intermediate calculation variable
        Real64 r_s;    // Minimum Stomatal Resistance, specific to each plant
        Real64 Mg;     // Surface soil moisture content m^3/m^3 (Moisture / MoistureMax)
        Real64 dOne;   // intermediate calculation variable
        Real64 esf;    // the saturation vapor pressure (Pa)
        Real64 qsf;    // Saturation specific humidity at leaf temperature (qfsat)
        Real64 Lef;    // Latent heat of vaporation at leaf surface temperature (J/kg)
        Real64 Desf;   // Derivative of Saturation vapor pressure
        Real64 dqf;    // Derivative of saturation specific humidity
        Real64 dqg;    // this is given by Clausius-Clapeyron equation
        Real64 esg;    // Saturation vapor pressure (Pa)
        Real64 qsg;    // Saturation specific humidity(mixing ratio?) at ground surface temperature
        Real64 Leg;    // Latent heat vaporization  at the ground temperature (J/kg)
        Real64 Desg;   // derivative of esg Saturation vapor pressure(?)
        Real64 F1temp; // intermediate variable in computing flux from the soil layer
        Real64 P1;     // intermediate variable in the equation for Tf
        Real64 P2;     // intermediate variable in the equation for Tf and Tg
        Real64 P3;     // intermediate variable in the equation for Tf and Tg
        Real64 Rhog;   // Density of air at the soil surface temperature
        Real64 Rhoag;  // Average density of air with respect to ground surface and air temperature
        Real64 Rib;    // Richardson Number
        Real64 Chng;   // bulk transfer coefficient near ground
        Real64 Ce;     // bulk transfer coefficient (this is in fact Ceg in equation 28 main report)
        Real64 Gammah; // latent heat exchange stability correction factor
        Real64 Chg;    // in fact it is the same as Ce (=Ceg) is transfer coefficient (but wot?)
        Real64 T3G;    // intermediate variable in the equation for Tg
        Real64 T2G;    // intermediate variable in the equation for Tg
        Real64 LeafTK; // the current leaf's temperature (Kelvin)
        Real64 SoilTK; // the current soil's temperature (Kelvin)
        Real64 Chne;   // is similar to near ground bulk transfer coefficient for latent heat flux (at neutral condition)
        Real64 Tif;    // previous leaf temperature
        Real64 rn;     // rn is the combined effect of both stomatal and aerodynamic resistances
        // in fact this is called r'' in the main report
        Real64 T1G;        // intermediate variable in the equation for Tg
        Real64 Qsoilpart1; // intermediate variable for evaluating Qsoil (part without the unknown)
        Real64 Qsoilpart2; // intermediate variable for evaluating Qsoil (part coeff of the ground temperature)

        Real64 Ws = DataEnvironment::WindSpeedAt(state, state.dataSurface->Surface(SurfNum).Centroid.z); // use windspeed at Z of roof
        if (Ws < 2.0) { // Later we need to adjust for building roof height...
            Ws = 2.0;   // Set minimum possible wind speed outside vegetation to 2.0 m/s
                        // consistent with FASST TR-04-25 p. x (W' = 2.0)
        }

        auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
        auto const *thisMaterial = state.dataMaterial->materials(thisConstruct.LayerPoint(1));
        RoughSurf = thisMaterial->Roughness;
        Real64 AbsThermSurf = thisMaterial->AbsorpThermal; // Thermal absorptance of the exterior surface
        Real64 HMovInsul = 0.0;                            // "Convection" coefficient of movable insulation

        if (state.dataSurface->Surface(SurfNum).ExtWind) {
            Convect::InitExtConvCoeff(state,
                                      SurfNum,
                                      HMovInsul,
                                      RoughSurf,
                                      AbsThermSurf,
                                      state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum),
                                      state.dataHeatBalSurf->SurfHConvExt(SurfNum),
                                      state.dataHeatBalSurf->SurfHSkyExt(SurfNum),
                                      state.dataHeatBalSurf->SurfHGrdExt(SurfNum),
                                      state.dataHeatBalSurf->SurfHAirExt(SurfNum),
                                      state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum));
        }
        // Long Wave Radiation (W/m^2) - original equation shown in comment
        // Real64 Latm = 1.0 * Sigma * 1.0 * state.dataSurface->Surface(SurfNum).ViewFactorGround * pow_4(state.dataEnvrn->GroundTempKelvin) +
        //               1.0 * Sigma * 1.0 * state.dataSurface->Surface(SurfNum).ViewFactorSky * pow_4(state.dataEnvrn->SkyTempKelvin);
        Real64 Latm = Sigma * state.dataSurface->Surface(SurfNum).ViewFactorGround * pow_4(state.dataEnvrn->GroundTempKelvin) +
                      Sigma * state.dataSurface->Surface(SurfNum).ViewFactorSky * pow_4(state.dataEnvrn->SkyTempKelvin);

        if (state.dataEcoRoofMgr->EcoRoofbeginFlag) {
            initEcoRoofFirstTime(state, SurfNum, ConstrNum); // Initialization statements for first entry into ecoroof routines
        }

        initEcoRoof(state, SurfNum, ConstrNum);

        // If current surface is = FirstEcoSurf then for this time step we need to update the soil moisture
        if (SurfNum == state.dataEcoRoofMgr->FirstEcoSurf) {
            int unit(0); // not actually used in the function it is passed into
            UpdateSoilProps(state,
                            state.dataEcoRoofMgr->Moisture,
                            state.dataEcoRoofMgr->MeanRootMoisture,
                            state.dataEcoRoofMgr->MoistureMax,
                            state.dataEcoRoofMgr->MoistureResidual,
                            state.dataEcoRoofMgr->SoilThickness,
                            state.dataEcoRoofMgr->Vfluxf,
                            state.dataEcoRoofMgr->Vfluxg,
                            ConstrNum,
                            state.dataEcoRoofMgr->Alphag,
                            unit,
                            state.dataEcoRoofMgr->Tg,
                            state.dataEcoRoofMgr->Tf,
                            state.dataEcoRoofMgr->Qsoil);

            Ta = DataEnvironment::OutDryBulbTempAt(
                state,
                state.dataSurface->Surface(SurfNum).Centroid.z); // temperature outdoor - Surface is dry, use normal correlation
            state.dataEcoRoofMgr->Tg = state.dataEcoRoofMgr->Tgold;
            state.dataEcoRoofMgr->Tf = state.dataEcoRoofMgr->Tfold;

            if (thisConstruct.CTFCross[0] > 0.01) {
                state.dataEcoRoofMgr->QuickConductionSurf = true;
                int spaceNum = state.dataSurface->Surface(SurfNum).spaceNum;
                F1temp = thisConstruct.CTFCross[0] / (thisConstruct.CTFInside[0] + state.dataHeatBalSurf->SurfHConvInt(SurfNum));
                Qsoilpart1 =
                    -state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum) +
                    F1temp * (state.dataHeatBalSurf->SurfCTFConstInPart(SurfNum) + state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(SurfNum) +
                              state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum) +
                              state.dataConstruction->Construct(ConstrNum).CTFSourceIn[0] * state.dataHeatBalSurf->SurfQsrcHist(SurfNum, 1) +
                              state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT +
                              state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(SurfNum));
            } else {
                Qsoilpart1 =
                    -state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum) + thisConstruct.CTFCross[0] * state.dataHeatBalSurf->SurfTempIn(SurfNum);
                F1temp = 0.0;
            }

            Qsoilpart2 = thisConstruct.CTFOutside[0] - F1temp * thisConstruct.CTFCross[0];

            state.dataEcoRoofMgr->Pa = state.dataEnvrn->StdBaroPress; // standard atmospheric pressure (apparently in Pascals)
            Tgk = state.dataEcoRoofMgr->Tg + Constant::Kelvin;
            Tak = Ta + Constant::Kelvin;

            sigmaf = 0.9 - 0.7 * std::exp(-0.75 * state.dataEcoRoofMgr->LAI); // Fractional veg cover based on (2) from FASST TR-04-25
            // Formula for grasses modified to incorporate limits from
            // Table 1 for sigmaf_max and min (0.20 to 0.9)

            EpsilonOne = state.dataEcoRoofMgr->epsilonf + state.dataEcoRoofMgr->epsilong -
                         state.dataEcoRoofMgr->epsilong * state.dataEcoRoofMgr->epsilonf; // Checked (eqn. 6 in FASST Veg Models)
            state.dataEcoRoofMgr->RH = state.dataEnvrn->OutRelHum;                        // Get humidity in % from the DataEnvironment.cc
            eair = (state.dataEcoRoofMgr->RH / 100.0) * 611.2 * std::exp(17.67 * Ta / (Tak - 29.65));
            qa = (0.622 * eair) / (state.dataEcoRoofMgr->Pa - 1.000 * eair); // Mixing Ratio of air
            Rhoa = state.dataEcoRoofMgr->Pa / (Rair * Tak);                  // Density of air. kg/m^3
            Tif = state.dataEcoRoofMgr->Tf;

            // Air Temperature within the canopy is given as
            // (Deardorff (1987)). Kelvin. based of the previous temperatures
            Tafk = (1.0 - sigmaf) * Tak + sigmaf * (0.3 * Tak + 0.6 * (Tif + Constant::Kelvin) + 0.1 * Tgk);

            Taf = Tafk - Constant::Kelvin;                          // Air Temperature within canopy in Celsius (C).
            Rhof = state.dataEcoRoofMgr->Pa / (Rair * Tafk);        // Density of air at the leaf temperature
            Rhoaf = (Rhoa + Rhof) / 2.0;                            // Average of air density
            Zd = 0.701 * std::pow(state.dataEcoRoofMgr->Zf, 0.979); // Zero displacement height
            Zo = 0.131 * std::pow(state.dataEcoRoofMgr->Zf, 0.997); // Foliage roughness length. (m) Source Ballick (1981)
            if (Zo < 0.02) {
                Zo = 0.02; // limit based on p.7 TR-04-25 and Table 2
            }

            // transfer coefficient at near-neutral condition Cfhn
            Cfhn = pow_2(Kv / std::log((state.dataEcoRoofMgr->Za - Zd) / Zo)); // Equation 12, page 7, FASST Model
            Waf = 0.83 * std::sqrt(Cfhn) * sigmaf * Ws + (1.0 - sigmaf) * Ws;  // Wind Speed inside foliage. Equation #6, FASST model
            Cf = 0.01 * (1.0 + 0.3 / Waf);                                     // The bulk Transfer coefficient, equation 10 page 6.
            state.dataEcoRoofMgr->sheatf = state.dataEcoRoofMgr->e0 + 1.1 * state.dataEcoRoofMgr->LAI * Rhoaf * Cpa * Cf *
                                                                          Waf; // Intermediate calculation for Sensible Heat Transfer
            state.dataEcoRoofMgr->sensiblef =
                state.dataEcoRoofMgr->sheatf *
                (Taf - state.dataEcoRoofMgr->Tf); // DJS Jan 2011 sensible flux TO foliage into air (Frankenstein 2004, eqn7)
            // sourced from Frankenstein et al (2004a). Added e0 windless correction factor.
            // an early version had (1.0-0.7)*sigmaf in calc of sensiblef... how did that get there!?! Fixed.

            // These parameters were taken from "The Atm Boundary Layer", By J.R. Garratt
            // NOTE the Garratt eqn. (A21) gives esf in units of hPA so we have multiplied
            // the constant 6.112 by a factor of 100.
            esf = 611.2 * std::exp(17.67 * Tif / (Tif + Constant::Kelvin - 29.65));

            // From Garratt - eqn. A21, p284. Note that Tif and Tif+Constant::Kelvin() usage is correct.
            // Saturation specific humidity at leaf temperature again based on previous temperatures

            qsf = 0.622 * esf / (state.dataEcoRoofMgr->Pa - 1.000 * esf); // "The Atm Boundary Layer", J.R Garrat for Saturation mixing ratio
            // Calculate stomatal resistance and atmospheric resistance Part
            ra = 1.0 / (Cf * Waf); // Aerodynamic Resistance. Resistance that is caused
            // by the boundary layer on a leaf surface to transfer water vapor. It is measured in
            // s/m and depends on wind speed, leaf's surface roughness,
            // and stability of atmosphere.

            CalculateEcoRoofSolar(state, RS, f1, SurfNum);
            if (state.dataEcoRoofMgr->MoistureMax == state.dataEcoRoofMgr->MoistureResidual) {
                f2inv = 1.0e10;
            } else {
                f2inv = (state.dataEcoRoofMgr->MeanRootMoisture - state.dataEcoRoofMgr->MoistureResidual) /
                        (state.dataEcoRoofMgr->MoistureMax - state.dataEcoRoofMgr->MoistureResidual); // Equation 19 p. 9 FASST model
            }

            // In FASST, Eq 20 is used to compute Moisture.

            f2 = 1.0 / f2inv; // In dry areas f2 --> LARGE so that r_s --> LARGE
            // and both rn and Latent flux --> 0.0
            state.dataEcoRoofMgr->f3 = 1.0 / (std::exp(-0.0 * (esf - eair))); // Note the 0.0 here is gd which has value of 0.0
            // for most plants and 0.03 for trees (see ECMWF
            // note given above.
            r_s = state.dataEcoRoofMgr->StomatalResistanceMin * f1 * f2 * state.dataEcoRoofMgr->f3 /
                  state.dataEcoRoofMgr->LAI; //  Stomatal Resistance (r_s)
            rn = ra / (ra + r_s);            // rn is foliage surface wetness ... NOT a resistance

            // This routine is to calculate ground moisture factor. This factor is from *****
            Mg = state.dataEcoRoofMgr->Moisture / state.dataEcoRoofMgr->MoistureMax; // m^3/m^3.
            dOne = 1.0 - sigmaf * (0.6 * (1.0 - rn) + 0.1 * (1.0 - Mg));

            // Latent heat of vaporation at leaf surface temperature. The source of this
            // equation is Henderson-Sellers (1984)
            Lef = 1.91846e6 * pow_2((Tif + Constant::Kelvin) / (Tif + Constant::Kelvin - 33.91));
            // Check to see if ice is sublimating or frost is forming.
            if (state.dataEcoRoofMgr->Tfold < 0.0) {
                Lef = 2.838e6; // per FASST documentation p.15 after eqn. 37.
            }

            // Derivative of Saturation vapor pressure, which is used in the calculation of
            // derivative of saturation specific humidity.

            Desf = 611.2 * std::exp(17.67 * (state.dataEcoRoofMgr->Tf / (state.dataEcoRoofMgr->Tf + Constant::Kelvin - 29.65))) *
                   (17.67 * state.dataEcoRoofMgr->Tf * (-1.0) * std::pow(state.dataEcoRoofMgr->Tf + Constant::Kelvin - 29.65, -2) +
                    17.67 / (Constant::Kelvin - 29.65 + state.dataEcoRoofMgr->Tf));
            dqf = ((0.622 * state.dataEcoRoofMgr->Pa) / pow_2(state.dataEcoRoofMgr->Pa - esf)) * Desf; // Derivative of saturation specific humidity
            esg = 611.2 * std::exp(17.67 * (state.dataEcoRoofMgr->Tg /
                                            ((state.dataEcoRoofMgr->Tg + Constant::Kelvin) - 29.65))); // Pa saturation vapor pressure
            // From Garratt - eqn. A21, p284.
            // Note that Tg and Tg+Constant::Kelvin() usage is correct.
            qsg = 0.622 * esg / (state.dataEcoRoofMgr->Pa - esg); // Saturation mixing ratio at ground surface temperature.

            // Latent heat vaporization  at the ground temperature
            Leg = 1.91846e6 * pow_2(Tgk / (Tgk - 33.91));
            // Check to see if ice is sublimating or frost is forming.
            if (state.dataEcoRoofMgr->Tgold < 0.0) {
                Leg = 2.838e6; // per FASST documentation p.15 after eqn. 37.
            }

            Desg = 611.2 * std::exp(17.67 * (state.dataEcoRoofMgr->Tg / (state.dataEcoRoofMgr->Tg + Constant::Kelvin - 29.65))) *
                   (17.67 * state.dataEcoRoofMgr->Tg * (-1.0) * std::pow(state.dataEcoRoofMgr->Tg + Constant::Kelvin - 29.65, -2) +
                    17.67 / (Constant::Kelvin - 29.65 + state.dataEcoRoofMgr->Tg));
            dqg = (0.622 * state.dataEcoRoofMgr->Pa / pow_2(state.dataEcoRoofMgr->Pa - esg)) * Desg;

            // Final Ground Atmosphere Energy Balance
            // Density of air at the soil surface temperature
            Rhog = state.dataEcoRoofMgr->Pa / (Rair * Tgk);

            // Average density of air with respect to ground surface and air temperature
            Rhoag = (Rhoa + Rhog) / 2.0;
            Rib = 2.0 * g1 * state.dataEcoRoofMgr->Za * (Taf - state.dataEcoRoofMgr->Tg) / ((Tafk + Tgk) * pow_2(Waf)); // Richardson Number

            // Compute the stability factor Gammah
            if (Rib < 0.0) {
                Gammah = std::pow(1.0 - 16.0 * Rib, -0.5);
            } else {
                if (Rib >= 0.19) {
                    Rib = 0.19;
                }
                Gammah = std::pow(1.0 - 5.0 * Rib, -0.5);
            }

            // "VeryRough, Rough, MediumRough, MediumSmooth, Smooth, VerySmooth"
            std::array<Real64, static_cast<int>(Material::SurfaceRoughness::Num)> zogLookup = {0.005, 0.0030, 0.0020, 0.0015, 0.0010, 0.0008};
            state.dataEcoRoofMgr->Zog = zogLookup[static_cast<int>(RoughSurf)];

            Chng = pow_2(Kv / std::log(state.dataEcoRoofMgr->Za / state.dataEcoRoofMgr->Zog)) / rch; // bulk transfer coefficient near ground
            Chg = Gammah * ((1.0 - sigmaf) * Chng + sigmaf * Cfhn);
            state.dataEcoRoofMgr->sheatg = state.dataEcoRoofMgr->e0 + Rhoag * Cpa * Chg * Waf; // added the e0 windless correction
            state.dataEcoRoofMgr->sensibleg =
                state.dataEcoRoofMgr->sheatg *
                (Taf - state.dataEcoRoofMgr->Tg); // sensible flux TO soil (W/m^2) DJS Jan 2011 (eqn. 32 in Frankenstein 2004)

            Chne = pow_2(Kv / std::log(state.dataEcoRoofMgr->Za / state.dataEcoRoofMgr->Zog)) / rche;
            Ce = Gammah * ((1.0 - sigmaf) * Chne + sigmaf * Cfhn); // this is in fact Ceg in eq (28)

            // we can approximate Gammae by Gammah (Supported by FASST Veg Models p. 15)
            qaf = ((1.0 - sigmaf) * qa + sigmaf * (0.3 * qa + 0.6 * qsf * rn + 0.1 * qsg * Mg)) /
                  (1.0 - sigmaf * (0.6 * (1.0 - rn) + 0.1 * (1.0 - Mg)));
            qg = Mg * qsg + (1.0 - Mg) * qaf; // eq main report (13)
            // According to FASST documentation this is correct.
            // The following also used Rhoaf and Rhoag respectively... shouldn't these be water densities??!!
            state.dataEcoRoofMgr->Lf = Lef * state.dataEcoRoofMgr->LAI * Rhoaf * Cf * Waf * rn * (qaf - qsf); // This had Leg but should be Lef...
            state.dataEcoRoofMgr->Lg = Ce * Leg * Waf * Rhoag * (qaf - qg) * Mg; // In the FASST documentation there is NO Mg. However, in looking
            // back at the Deardorff 1978 paper it appears that an alpha = Mg term is
            // used to distinguish from POTENTIAL and ACTUAL ground surface evaporation...
            // the Lf and Lg calculations are NOT used in this formulation
            // rather, the effects are included in terms of dqg and dqf !
            // These equations for Lf and Lg are based on Deardorff's paper, but there is
            // a sign difference !!! (qsf -qaf) and (qg - qaf) ?!
            // These may be useful, however for telling the UpdateSoilProps routine
            // how much evaporation has taken place...
            state.dataEcoRoofMgr->Vfluxf = -1.0 * state.dataEcoRoofMgr->Lf / Lef / 990.0; // water evapotranspire rate [m/s]
            state.dataEcoRoofMgr->Vfluxg = -1.0 * state.dataEcoRoofMgr->Lg / Leg / 990.0; // water evapotranspire rate [m/s]
            if (state.dataEcoRoofMgr->Vfluxf < 0.0) {
                state.dataEcoRoofMgr->Vfluxf = 0.0; // According to FASST Veg. Models p. 11, eqn 26-27, if Qfsat > qaf the actual
            }
            if (state.dataEcoRoofMgr->Vfluxg < 0.0) {
                state.dataEcoRoofMgr->Vfluxg = 0.0; // evaporative fluxes should be set to zero (delta_c = 1 or 0).
            }

            // P1, P2, P3 correspond to first, second and third terms of equation 37 in the main report.

            //   Note: the FASST model has a term -gamma_p*(1.0-exp...) in first line for P1 (c1_f) where gamma_p is
            //   a precipitation variable. So, if we assume no precip this term vanishes. We should
            //   revisit this issue later.
            //   Implement an iterative solution scheme to solve the simultaneous equations for Leaf and Soil temperature.
            //   Prior experience suggests that no more than 3 iterations are likely needed
            LeafTK = state.dataEcoRoofMgr->Tf + Constant::Kelvin;
            SoilTK = state.dataEcoRoofMgr->Tg + Constant::Kelvin;

            for (int EcoLoop = 1; EcoLoop <= 3; ++EcoLoop) {
                P1 = sigmaf * (RS * (1.0 - state.dataEcoRoofMgr->Alphaf) + state.dataEcoRoofMgr->epsilonf * Latm) -
                     3.0 * sigmaf * state.dataEcoRoofMgr->epsilonf * state.dataEcoRoofMgr->epsilong * Sigma * pow_4(SoilTK) / EpsilonOne -
                     3.0 *
                         (-sigmaf * state.dataEcoRoofMgr->epsilonf * Sigma -
                          sigmaf * state.dataEcoRoofMgr->epsilonf * state.dataEcoRoofMgr->epsilong * Sigma / EpsilonOne) *
                         pow_4(LeafTK) +
                     state.dataEcoRoofMgr->sheatf * (1.0 - 0.7 * sigmaf) * (Ta + Constant::Kelvin) +
                     state.dataEcoRoofMgr->LAI * Rhoaf * Cf * Lef * Waf * rn * ((1.0 - 0.7 * sigmaf) / dOne) * qa +
                     state.dataEcoRoofMgr->LAI * Rhoaf * Cf * Lef * Waf * rn * (((0.6 * sigmaf * rn) / dOne) - 1.0) * (qsf - LeafTK * dqf) +
                     state.dataEcoRoofMgr->LAI * Rhoaf * Cf * Lef * Waf * rn * ((0.1 * sigmaf * Mg) / dOne) * (qsg - SoilTK * dqg);
                P2 = 4.0 * (sigmaf * state.dataEcoRoofMgr->epsilonf * state.dataEcoRoofMgr->epsilong * Sigma) * pow_3(SoilTK) / EpsilonOne +
                     0.1 * sigmaf * state.dataEcoRoofMgr->sheatf +
                     state.dataEcoRoofMgr->LAI * Rhoaf * Cf * Lef * Waf * rn * (0.1 * sigmaf * Mg) / dOne * dqg;
                P3 = 4.0 *
                         (-sigmaf * state.dataEcoRoofMgr->epsilonf * Sigma -
                          (sigmaf * state.dataEcoRoofMgr->epsilonf * Sigma * state.dataEcoRoofMgr->epsilong) / EpsilonOne) *
                         pow_3(LeafTK) +
                     (0.6 * sigmaf - 1.0) * state.dataEcoRoofMgr->sheatf +
                     state.dataEcoRoofMgr->LAI * Rhoaf * Cf * Lef * Waf * rn * (((0.6 * sigmaf * rn) / dOne) - 1.0) * dqf;

                // T1G, T2G, & T3G corresponds to first, second & third terms of equation 38
                // in the main report.
                //  as with the equations for vegetation the first term in the ground eqn in FASST has a
                //  term starting with gamma_p --- if no precip this vanishes. Again, revisit this issue later.

                T1G = (1.0 - sigmaf) * (RS * (1.0 - state.dataEcoRoofMgr->Alphag) + state.dataEcoRoofMgr->epsilong * Latm) -
                      (3.0 * (sigmaf * state.dataEcoRoofMgr->epsilonf * state.dataEcoRoofMgr->epsilong * Sigma) / EpsilonOne) * pow_4(LeafTK) -
                      3.0 *
                          (-(1.0 - sigmaf) * state.dataEcoRoofMgr->epsilong * Sigma -
                           sigmaf * state.dataEcoRoofMgr->epsilonf * state.dataEcoRoofMgr->epsilong * Sigma / EpsilonOne) *
                          pow_4(SoilTK) +
                      state.dataEcoRoofMgr->sheatg * (1.0 - 0.7 * sigmaf) * (Ta + Constant::Kelvin) +
                      Rhoag * Ce * Leg * Waf * Mg * ((1.0 - 0.7 * sigmaf) / dOne) * qa +
                      Rhoag * Ce * Leg * Waf * Mg * (0.1 * sigmaf * Mg / dOne - Mg) * (qsg - SoilTK * dqg) +
                      Rhoag * Ce * Leg * Waf * Mg * (0.6 * sigmaf * rn / dOne) * (qsf - LeafTK * dqf) + Qsoilpart1 +
                      Qsoilpart2 * (Constant::Kelvin); // finished by T1G

                T2G = 4.0 *
                          (-(1.0 - sigmaf) * state.dataEcoRoofMgr->epsilong * Sigma -
                           sigmaf * state.dataEcoRoofMgr->epsilonf * state.dataEcoRoofMgr->epsilong * Sigma / EpsilonOne) *
                          pow_3(SoilTK) +
                      (0.1 * sigmaf - 1.0) * state.dataEcoRoofMgr->sheatg + Rhoag * Ce * Leg * Waf * Mg * (0.1 * sigmaf * Mg / dOne - Mg) * dqg -
                      Qsoilpart2;

                T3G = (4.0 * (sigmaf * state.dataEcoRoofMgr->epsilong * state.dataEcoRoofMgr->epsilonf * Sigma) / EpsilonOne) * pow_3(LeafTK) +
                      0.6 * sigmaf * state.dataEcoRoofMgr->sheatg + Rhoag * Ce * Leg * Waf * Mg * (0.6 * sigmaf * rn / dOne) * dqf;

                LeafTK = 0.5 * (LeafTK + (P1 * T2G - P2 * T1G) / (-P3 * T2G + T3G * P2)); // take avg of old and new each iteration
                SoilTK = 0.5 * (SoilTK + (P1 * T3G - P3 * T1G) / (-P2 * T3G + P3 * T2G)); // take avg of old and new each iteration
                // in earlier implementations we simply assigned new Leaf and Soil temps once (no iteration -- ecoloop =1,1, but
                // with LeafTK = ( P1*T2G - P2*T1G )/( -P3*T2G + T3G*P2 )  and
                // SoilTK =(SoilTK+( P1*T3G - P3*T1G )/( -P2*T3G + P3*T2G ). This iterative solution was initially attempted to
                // deal with stability issues associated with the CTF. It has virtually no impact on temperatures and it is not
                // clear how much it helped with stability problems. Eventually when CTF solution is replaced with a finite
                // difference scheme this loop structure should be removed.

            } // This loop does an iterative solution of the simultaneous equations
            state.dataEcoRoofMgr->Qsoil = -1.0 * (Qsoilpart1 - Qsoilpart2 * (SoilTK - Constant::Kelvin)); // This is heat flux INTO top of the soil
            state.dataEcoRoofMgr->Tfold = LeafTK - Constant::Kelvin;
            state.dataEcoRoofMgr->Tgold = SoilTK - Constant::Kelvin;

        } // if firstecosurface (if not we do NOT need to recalculate ecoroof energybalance as all ecoroof surfaces MUST be the same
        // this endif was moved here from the if statement regarding whether we are looking at the first ecoroof surface or not.

        state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) = state.dataEcoRoofMgr->Tgold; // SoilTemperature
        TempExt = state.dataEcoRoofMgr->Tgold;
    }

    void initEcoRoofFirstTime(EnergyPlusData &state, int const SurfNum, int const ConstrNum)
    {
        auto const *mat = state.dataMaterial->materials(state.dataConstruction->Construct(ConstrNum).LayerPoint(1));
        assert(mat->group == Material::Group::EcoRoof);

        auto const *matER = dynamic_cast<Material::MaterialEcoRoof const *>(mat);
        assert(matER != nullptr);
        auto &thisEcoRoof = state.dataEcoRoofMgr;

        thisEcoRoof->EcoRoofbeginFlag = false;

        if (state.dataSurface->Surface(SurfNum).HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::CTF) {
            ShowSevereError(state,
                            "initEcoRoofFirstTime: EcoRoof simulation but HeatBalanceAlgorithm is not ConductionTransferFunction(CTF). EcoRoof model "
                            "currently works only with CTF heat balance solution algorithm.");
            ShowContinueError(state, format("Occurs for surface named {}", state.dataSurface->Surface(SurfNum).Name));
            ShowContinueError(state, "Check input syntax for HeatBalanceAlgorithm, SurfaceProperty:HeatTransferAlgorithm,");
            ShowContinueError(state, "SurfaceProperty:HeatTransferAlgorithm:MultipleSurface, and SurfaceProperty:HeatTransferAlgorithm:SurfaceList ");
            ShowContinueError(state, "to verify that the solution method is set to CTF for the surface that is an EcoRoof.");
            ShowFatalError(state, "initEcoRoofFirstTime: Program terminates due to preceding conditions.");
        }

        // ONLY READ ECOROOF PROPERTIES IN THE FIRST TIME
        thisEcoRoof->Zf = matER->HeightOfPlants;               // Plant height (m)
        thisEcoRoof->LAI = matER->LAI;                         // Leaf Area Index
        thisEcoRoof->Alphag = 1.0 - matER->AbsorpSolar;        // albedo rather than absorptivity
        thisEcoRoof->Alphaf = matER->Lreflectivity;            // Leaf Reflectivity
        thisEcoRoof->epsilonf = matER->LEmissitivity;          // Leaf Emissivity
        thisEcoRoof->StomatalResistanceMin = matER->RStomata;  // Leaf min stomatal resistance
        thisEcoRoof->epsilong = matER->AbsorpThermal;          // Soil Emissivity
        thisEcoRoof->MoistureMax = matER->Porosity;            // Max moisture content in soil
        thisEcoRoof->MoistureResidual = matER->MinMoisture;    // Min moisture content in soil
        thisEcoRoof->Moisture = matER->InitMoisture;           // Initial moisture content in soil
        thisEcoRoof->MeanRootMoisture = thisEcoRoof->Moisture; // DJS Oct 2007 Release --> all soil at same initial moisture for Reverse DD fix

        thisEcoRoof->SoilThickness = matER->Thickness; // Total thickness of soil layer (m)

        thisEcoRoof->FirstEcoSurf = SurfNum; // this determines WHEN to updatesoilProps

        // DJS NOVEMBER 2010 - Make calls to SetupOutput Variable to allow for reporting of ecoroof variables
        SetupOutputVariable(state,
                            "Green Roof Soil Temperature",
                            Constant::Units::C,
                            thisEcoRoof->Tg,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            "Environment");
        SetupOutputVariable(state,
                            "Green Roof Vegetation Temperature",
                            Constant::Units::C,
                            thisEcoRoof->Tf,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            "Environment");
        SetupOutputVariable(state,
                            "Green Roof Soil Root Moisture Ratio",
                            Constant::Units::None,
                            thisEcoRoof->MeanRootMoisture,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            "Environment");
        SetupOutputVariable(state,
                            "Green Roof Soil Near Surface Moisture Ratio",
                            Constant::Units::None,
                            thisEcoRoof->Moisture,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            "Environment");
        SetupOutputVariable(state,
                            "Green Roof Soil Sensible Heat Transfer Rate per Area",
                            Constant::Units::W_m2,
                            thisEcoRoof->sensibleg,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            "Environment");
        SetupOutputVariable(state,
                            "Green Roof Vegetation Sensible Heat Transfer Rate per Area",
                            Constant::Units::W_m2,
                            thisEcoRoof->sensiblef,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            "Environment");
        SetupOutputVariable(state,
                            "Green Roof Vegetation Moisture Transfer Rate",
                            Constant::Units::m_s,
                            thisEcoRoof->Vfluxf,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            "Environment");
        SetupOutputVariable(state,
                            "Green Roof Soil Moisture Transfer Rate",
                            Constant::Units::m_s,
                            thisEcoRoof->Vfluxg,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            "Environment");
        SetupOutputVariable(state,
                            "Green Roof Vegetation Latent Heat Transfer Rate per Area",
                            Constant::Units::W_m2,
                            thisEcoRoof->Lf,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            "Environment");
        SetupOutputVariable(state,
                            "Green Roof Soil Latent Heat Transfer Rate per Area",
                            Constant::Units::W_m2,
                            thisEcoRoof->Lg,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            "Environment");

        SetupOutputVariable(state,
                            "Green Roof Cumulative Precipitation Depth",
                            Constant::Units::m,
                            thisEcoRoof->CumPrecip,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            "Environment");
        SetupOutputVariable(state,
                            "Green Roof Cumulative Irrigation Depth",
                            Constant::Units::m,
                            thisEcoRoof->CumIrrigation,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            "Environment");
        SetupOutputVariable(state,
                            "Green Roof Cumulative Runoff Depth",
                            Constant::Units::m,
                            thisEcoRoof->CumRunoff,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            "Environment");
        SetupOutputVariable(state,
                            "Green Roof Cumulative Evapotranspiration Depth",
                            Constant::Units::m,
                            thisEcoRoof->CumET,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            "Environment");
        SetupOutputVariable(state,
                            "Green Roof Current Precipitation Depth",
                            Constant::Units::m,
                            thisEcoRoof->CurrentPrecipitation,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            "Environment");
        SetupOutputVariable(state,
                            "Green Roof Current Irrigation Depth",
                            Constant::Units::m,
                            thisEcoRoof->CurrentIrrigation,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            "Environment");
        SetupOutputVariable(state,
                            "Green Roof Current Runoff Depth",
                            Constant::Units::m,
                            thisEcoRoof->CurrentRunoff,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            "Environment");
        SetupOutputVariable(state,
                            "Green Roof Current Evapotranspiration Depth",
                            Constant::Units::m,
                            thisEcoRoof->CurrentET,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            "Environment");
    }

    void initEcoRoof(EnergyPlusData &state, int const SurfNum, int const ConstrNum)
    {
        auto const *mat = state.dataMaterial->materials(state.dataConstruction->Construct(ConstrNum).LayerPoint(1));
        assert(mat->group == Material::Group::EcoRoof);
        auto const *matER = dynamic_cast<Material::MaterialEcoRoof const *>(mat);
        assert(matER != nullptr);
        auto &thisSurf = state.dataSurface->Surface(SurfNum);

        // DJS July 2007
        // Make sure the ecoroof module resets its conditions at start of EVERY warmup day and every new design day
        // for Reverse DD testing
        if (state.dataGlobal->BeginEnvrnFlag || state.dataGlobal->WarmupFlag) {
            state.dataEcoRoofMgr->Moisture = matER->InitMoisture;                    // Initial moisture content in soil
            state.dataEcoRoofMgr->MeanRootMoisture = state.dataEcoRoofMgr->Moisture; // Start the root zone moisture at the same value as the surface.
            state.dataEcoRoofMgr->Alphag = 1.0 - matER->AbsorpSolar;                 // albedo rather than absorptivity
        }

        if (state.dataGlobal->BeginEnvrnFlag && state.dataEcoRoofMgr->CalcEcoRoofMyEnvrnFlag) {
            state.dataEcoRoofMgr->Tgold = DataEnvironment::OutDryBulbTempAt(state, thisSurf.Centroid.z); // OutDryBulbTemp initial guess
            state.dataEcoRoofMgr->Tfold = DataEnvironment::OutDryBulbTempAt(state, thisSurf.Centroid.z); // OutDryBulbTemp initial guess
            state.dataEcoRoofMgr->Tg = 10.0;
            state.dataEcoRoofMgr->Tf = 10.0;
            state.dataEcoRoofMgr->Vfluxf = 0.0;
            state.dataEcoRoofMgr->Vfluxg = 0.0;
            state.dataEcoRoofMgr->CumRunoff = 0.0;
            state.dataEcoRoofMgr->CumET = 0.0;
            state.dataEcoRoofMgr->CumPrecip = 0.0;
            state.dataEcoRoofMgr->CumIrrigation = 0.0;
            state.dataEcoRoofMgr->CurrentRunoff = 0.0;
            state.dataEcoRoofMgr->CurrentET = 0.0;
            state.dataEcoRoofMgr->CurrentPrecipitation = 0.0;
            state.dataEcoRoofMgr->CurrentIrrigation = 0.0;
            state.dataEcoRoofMgr->CalcEcoRoofMyEnvrnFlag = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataEcoRoofMgr->CalcEcoRoofMyEnvrnFlag = true;
        }
    }

    void UpdateSoilProps(EnergyPlusData &state,
                         Real64 &Moisture,
                         Real64 &MeanRootMoisture,
                         Real64 const MoistureMax,
                         Real64 const MoistureResidual,
                         Real64 const SoilThickness,
                         Real64 const Vfluxf, // Water mass flux from vegetation [m/s]
                         Real64 const Vfluxg, // Water mass flux from soil surface [m/s]
                         int const ConstrNum, // Indicator for construction index for the current surface
                         Real64 &Alphag,
                         [[maybe_unused]] int const unit,    // unused1208
                         [[maybe_unused]] Real64 const Tg,   // unused1208
                         [[maybe_unused]] Real64 const Tf,   // unused1208
                         [[maybe_unused]] Real64 const Qsoil // unused1208
    )
    {
        // SUBROUTINE INFORMATION
        //     AUTHOR          David Sailor
        //     DATE WRITTEN    Jan 2007
        //     MODIFIED        Stephen Forner, Portland State University (SF); 7/15/2010

        // PURPOSE OF THIS MODULE:
        // Track moisture input/output to ecoroof soil media (precipitation, irrigation, evapotranspiration, runoff)
        // Update soil thermal properties associated with variations in soil moisture and update CTF calculations
        // for the ecoroof construction layer.

        // METHODOLOGY EMPLOYED:
        // Define 2 soil layers (top and root). Moisture redistribution between top and root layers is proportional
        // to moisture differences. Input and Output of moisture are partitioned between layers.
        // Soil thermal properties vary based on non-dimensionalization of experimental data for 8 typical soils.
        // Specifically, THERMAL PROPERTY = Dry Value + (fraction of moisture content)*Wet Value

        // SUBROUTINE PARAMETER DEFINITIONS:
        static Real64 const depth_fac((161240.0 * std::pow(2.0, -2.3)) / 60.0);

        // Soil Parameters from Reference listed in the code:
        Real64 constexpr alpha(23.0); // These parameters are empirical constants
        Real64 constexpr n(1.27);     // These parameters are empirical constants
        Real64 constexpr lambda(0.5); // These parameters are empirical constants
        // This is another parameter of the soil which describes the soil conductivity at the saturation point (m/s)
        Real64 constexpr SoilConductivitySaturation(5.157e-7);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 RatioMax;
        Real64 RatioMin;
        Real64 MoistureDiffusion; // moisture transport down from near-surface to root zone
        Real64 SoilConductivity;  // Moisture dependent conductivity to be fed back into CTF Calculator
        Real64 SoilSpecHeat;      // Moisture dependent Spec. Heat to be fed back into CTF Calculator
        Real64 SoilAbsorpSolar;   // Moisture dependent Solar absorptance (1-albedo)
        Real64 SoilDensity;       // Moisture dependent density to be fed back into CTF Calculator
        Real64 SatRatio;
        Real64 TestRatio;   // Ratio to determine if timestep change in properties is too abrupt for CTF
        Real64 AvgMoisture; // Average soil moisture over depth of ecoroof media

        // NOTE:  As Energyplus calls the energy balance manager (and hence CalcEcoroof)
        // once for each surface within each zone that has an ecoroof
        // --- the CALCECOROOF routine is called multiple times within each time step
        // So, it is important that the UpdateSoilProps subroutine is called ONLY ONCE for each time step!!!

        // Recall Moisture = near-surface moisture value (m^3/m^3)
        // Recall MeanRootMoisture = root zone moisture value (m^3/m^3)

        // DJS 2009 set the ratiomax and ratiomin values in the code now (rather than as parameters) so that
        // DJS 2009 we can link them to timesteps and make these limits apply to actual RATES...
        // DJS 2009 reasonable rates are +/- 10% change in properties per 15 minute period... Otherwise we have
        // DJS 2009 stability issues.
        // DJS 2011 FEB - Since we no longer use CTF with soil-dependent properties (Do not RECALL INITCONDUCTION...
        // DJS 2011 FEB - we may be able to get away with NO limits on rates of change when using CFD routine.
        // DJS 2011 FEB - for now we stick with 20% per quarter hour.
        RatioMax = 1.0 + 0.20 * state.dataGlobal->MinutesInTimeStep / 15.0;
        RatioMin = 1.0 - 0.20 * state.dataGlobal->MinutesInTimeStep / 15.0;

        auto *mat = state.dataMaterial->materials(state.dataConstruction->Construct(ConstrNum).LayerPoint(1));
        assert(mat->group == Material::Group::EcoRoof);
        auto *matER = dynamic_cast<Material::MaterialEcoRoof *>(mat);
        if (state.dataEcoRoofMgr->UpdatebeginFlag) {

            // SET dry values that NEVER CHANGE
            state.dataEcoRoofMgr->DryCond = matER->Conductivity;
            state.dataEcoRoofMgr->DryDens = matER->Density;
            state.dataEcoRoofMgr->DryAbsorp = matER->AbsorpSolar;
            state.dataEcoRoofMgr->DrySpecHeat = matER->SpecHeat;

            // DETERMINE RELATIVE THICKNESS OF TWO LAYERS OF SOIL (also unchanging)
            if (SoilThickness > 0.12) {
                state.dataEcoRoofMgr->TopDepth = 0.06; // For now use 6cm as top depth - everything else is root layer
            } else {
                state.dataEcoRoofMgr->TopDepth = 0.5 * SoilThickness; // In unusual case of very thin soil make topdepth half of total
            }
            // This loop outputs the minimum number of time steps needed to keep the solution stable
            // The equation is minimum timestep in seconds=161240*((number of layers)**(-2.3))*(Total thickness of the soil)**2.07
            if (matER->calcMethod == Material::EcoRoofCalcMethod::SchaapGenuchten) {
                int index1;
                Real64 const depth_limit(depth_fac * std::pow(state.dataEcoRoofMgr->TopDepth + state.dataEcoRoofMgr->RootDepth, 2.07));
                for (index1 = 1; index1 <= 20; ++index1) {
                    if (double(state.dataGlobal->MinutesInTimeStep / index1) <= depth_limit) {
                        break;
                    }
                }
                if (index1 > 1) {
                    ShowWarningError(state, "CalcEcoRoof: Too few time steps per hour for stability.");
                    if (ceil(60 * index1 / state.dataGlobal->MinutesInTimeStep) <= 60) {
                        ShowContinueError(
                            state,
                            format("...Entered Timesteps per hour=[{}], Change to some value greater than or equal to [{}] for assured stability.",
                                   state.dataGlobal->TimeStepsInHour,
                                   60 * index1 / state.dataGlobal->MinutesInTimeStep));
                        ShowContinueError(state, "...Note that EnergyPlus has a maximum of 60 timesteps per hour");
                        ShowContinueError(state,
                                          "...The program will continue, but if the simulation fails due to too low/high temperatures, instability "
                                          "here could be the reason.");
                    } else {
                        ShowContinueError(state,
                                          format("...Entered Timesteps per hour=[{}], however the required frequency for stability [{}] is over the "
                                                 "EnergyPlus maximum of 60.",
                                                 state.dataGlobal->TimeStepsInHour,
                                                 60 * index1 / state.dataGlobal->MinutesInTimeStep));
                        ShowContinueError(state, "...Consider using the simple moisture diffusion calculation method for this application");
                        ShowContinueError(state,
                                          "...The program will continue, but if the simulation fails due to too low/high temperatures, instability "
                                          "here could be the reason.");
                    }
                }
            }

            state.dataEcoRoofMgr->RootDepth = SoilThickness - state.dataEcoRoofMgr->TopDepth;
            // Next create a timestep in seconds
            state.dataEcoRoofMgr->TimeStepZoneSec = state.dataGlobal->MinutesInTimeStep * 60.0;

            state.dataEcoRoofMgr->UpdatebeginFlag = false;
        }

        state.dataEcoRoofMgr->CurrentRunoff = 0.0; // Initialize current time step runoff as it is used in several spots below...

        // FIRST Subtract water evaporated by plants and at soil surface
        Moisture -= (Vfluxg)*state.dataGlobal->MinutesInTimeStep * 60.0 / state.dataEcoRoofMgr->TopDepth;          // soil surface evaporation
        MeanRootMoisture -= (Vfluxf)*state.dataGlobal->MinutesInTimeStep * 60.0 / state.dataEcoRoofMgr->RootDepth; // plant extraction from root zone

        // NEXT Update evapotranspiration summary variable for print out
        state.dataEcoRoofMgr->CurrentET = (Vfluxg + Vfluxf) * state.dataGlobal->MinutesInTimeStep * 60.0; // units are meters
        if (!state.dataGlobal->WarmupFlag) {
            state.dataEcoRoofMgr->CumET += state.dataEcoRoofMgr->CurrentET;
        }

        // NEXT Add Precipitation to surface soil moisture variable (if a schedule exists)
        Moisture += state.dataEcoRoofMgr->CurrentPrecipitation / state.dataEcoRoofMgr->TopDepth; // x (m) evenly put into top layer

        if (!state.dataGlobal->WarmupFlag) {
            state.dataEcoRoofMgr->CumPrecip += state.dataEcoRoofMgr->CurrentPrecipitation;
        }

        // NEXT Add Irrigation to surface soil moisture variable (if a schedule exists)
        state.dataEcoRoofMgr->CurrentIrrigation = 0.0; // first initialize to zero
        state.dataWaterData->Irrigation.ActualAmount = 0.0;
        if (state.dataWaterData->Irrigation.ModeID == DataWater::IrrigationMode::SchedDesign) {
            state.dataEcoRoofMgr->CurrentIrrigation = state.dataWaterData->Irrigation.ScheduledAmount; // units of m
            state.dataWaterData->Irrigation.ActualAmount = state.dataEcoRoofMgr->CurrentIrrigation;
            //    elseif (Irrigation%ModeID ==IrrSmartSched .and. moisture .lt. 0.4d0*MoistureMax) then
        } else if (state.dataWaterData->Irrigation.ModeID == DataWater::IrrigationMode::SmartSched &&
                   Moisture < state.dataWaterData->Irrigation.IrrigationThreshold * MoistureMax) {
            // Smart schedule only irrigates when scheduled AND the soil is less than 40% saturated
            state.dataEcoRoofMgr->CurrentIrrigation = state.dataWaterData->Irrigation.ScheduledAmount; // units of m
            state.dataWaterData->Irrigation.ActualAmount = state.dataEcoRoofMgr->CurrentIrrigation;
        } else { // no schedule
            if (state.dataWaterData->RainFall.ModeID == DataWater::RainfallMode::EPWPrecipitation) {
                state.dataEcoRoofMgr->CurrentIrrigation = 0; // units of m
                state.dataWaterData->Irrigation.ActualAmount = state.dataEcoRoofMgr->CurrentIrrigation;
            }
        }

        Moisture += state.dataEcoRoofMgr->CurrentIrrigation / state.dataEcoRoofMgr->TopDepth; // irrigation in (m)/timestep put into top layer

        if ((state.dataEnvrn->RunPeriodEnvironment) && (!state.dataGlobal->WarmupFlag)) {
            state.dataEcoRoofMgr->CumIrrigation += state.dataEcoRoofMgr->CurrentIrrigation;
            // aggregate to monthly for reporting
            int month = state.dataEnvrn->Month;
            state.dataEcoRoofMgr->MonthlyIrrigation.at(month - 1) += state.dataWaterData->Irrigation.ActualAmount * 1000.0;
        }

        // Note: If soil top layer gets a massive influx of rain &/or irrigation some of
        // the water will simply run right off the top and not penetrate at all!
        // At the present time this limit is fairly small due to some minor stability issues
        // in EnergyPlus. If the moisture changes too rapidly the code cannot handle the rapid changes in
        // surface characteristics and heat fluxes. The result that I've noticed is a non-physical fluctuation
        // in ground surface temperature that oscillates up to 10 deg C from one hour to the next until the
        // code catches up. The temporary solution is to simply limit how much moisture can enter the soil
        // in any time step to 0.5"/hour. In the future this might be fixed by running with finer time steps
        // or by using a finite difference temperature solution rather than the CTF.
        // I suspect that 15 minute intervals may be needed. Another option is to have an internal moisture
        // overflow bin that will hold extra moisture and then distribute it in subsequent hours. This way the
        // soil still gets the same total moisture... it is just distributed over a longer period.
        if (state.dataEcoRoofMgr->CurrentIrrigation + state.dataEcoRoofMgr->CurrentPrecipitation >
            0.5 * 0.0254 * state.dataGlobal->MinutesInTimeStep / 60.0) {
            state.dataEcoRoofMgr->CurrentRunoff = state.dataEcoRoofMgr->CurrentIrrigation + state.dataEcoRoofMgr->CurrentPrecipitation -
                                                  (0.5 * 0.0254 * state.dataGlobal->MinutesInTimeStep / 60.0);
            // If we get here then TOO much moisture has already been added to soil (must now subtract excess)
            Moisture -= state.dataEcoRoofMgr->CurrentRunoff / state.dataEcoRoofMgr->TopDepth; // currently any incident moisture in excess of 1/4 "
                                                                                              // per hour
                                                                                              // simply runs off the top of the soil.
        }
        // Now, if top layer is beyond saturation... the excess simply runs off without penetrating into the lower
        // layers.
        if (Moisture > MoistureMax) {
            state.dataEcoRoofMgr->CurrentRunoff += (Moisture - MoistureMax) * state.dataEcoRoofMgr->TopDepth;
            Moisture = MoistureMax;
        }

        if (matER->calcMethod == Material::EcoRoofCalcMethod::Simple) {

            // THE SECTION BELOW WAS THE INITIAL MOISTURE DISTRIBUTION MODEL.
            // Any line with "!-" was code.  A line with "!" was just a comment.  This is done in case this code needs to be resurrected in the
            // future. See below this commented out code for the new moisture distribution model.
            //*********************************************************************************************************
            //*********************************************************************************************************
            // NEXT Redistribute moisture based on moisture diffusion.
            // The effective diffusivities should be revisited when better moisture transport data in ecoroof soils are
            // available.
            // Here the diffusion rate is in units of [1/s]
            // A value of 0.0001 would be ~ 36% / hour
            // A value of 0.00005 would be ~ 18%/hour change in moisture
            // A value of 0.000025 gives about a 9%/hour change in moisture
            // a value of 0.0000125 gives 5%/hour...
            // Note: This formulation allows moisture to have a directional bias - ie, it can sink into soil easier than
            // it can be brought up.
            if (Moisture > MeanRootMoisture) {
                // move moisture from top layer into root zone
                MoistureDiffusion = min((MoistureMax - MeanRootMoisture) * state.dataEcoRoofMgr->RootDepth,
                                        (Moisture - MeanRootMoisture) * state.dataEcoRoofMgr->TopDepth);
                MoistureDiffusion = max(0.0, MoistureDiffusion); // Safety net to keep positive (not needed?)
                // at this point moistureDiffusion is in units of (m)/timestep
                MoistureDiffusion *= 0.00005 * state.dataGlobal->MinutesInTimeStep * 60.0;
                Moisture -= MoistureDiffusion / state.dataEcoRoofMgr->TopDepth;
                MeanRootMoisture += MoistureDiffusion / state.dataEcoRoofMgr->RootDepth;
            } else if (MeanRootMoisture > Moisture) {
                // move moisture to top layer from root zone
                MoistureDiffusion =
                    min((MoistureMax - Moisture) * state.dataEcoRoofMgr->TopDepth, (MeanRootMoisture - Moisture) * state.dataEcoRoofMgr->RootDepth);
                MoistureDiffusion = max(0.0, MoistureDiffusion); // Safety net (not needed?)
                // at this point moistureDiffusion is in units of (m)/timestep
                MoistureDiffusion *= 0.00001 * state.dataGlobal->MinutesInTimeStep * 60.0;
                Moisture += MoistureDiffusion / state.dataEcoRoofMgr->TopDepth;
                MeanRootMoisture -= MoistureDiffusion / state.dataEcoRoofMgr->RootDepth;
            }
        } else {
            //********************************************************************************************************
            //********************************************************************************************************
            // THE SECTION ABOVE WAS THE MOISTURE DISTRIBUTION MODEL. REPLACED SF-7/21/2010

            // NEXT redistribute the moisture in the soil based on:
            // Marcel G Schaap and Martinus Th van Genuchten, 2006, 'A modified Maulem-van
            // Genuchten Formulation for Improved Description of the Hydraulic Conductivity Near Saturation'.
            // Written in MATLAB by Vishal Sharma (of Portland State) and modified for FORTRAN by Stephen Forner Summer 2010
            // This model is based on curve fit data that describes the capillary motion of the water in combination with the gravitational
            // forces on the water.
            // This set of equations is unstable if the time step is larger than given by a curve fit equation.  This first DO loop is to
            // see how many time steps are needed to be stable.
            // This method of moisture distribution relies on variables which are experimentally determined: alpha, lambda, n and the
            // hydraulic conductivity at saturation.

            // Now, solve for the soil parameters for  of the top soil layer

            state.dataEcoRoofMgr->RelativeSoilSaturationTop = (Moisture - MoistureResidual) / (MoistureMax - MoistureResidual);
            if (state.dataEcoRoofMgr->RelativeSoilSaturationTop < 0.0001) {
                if (state.dataEcoRoofMgr->ErrIndex == 0) {
                    ShowWarningMessage(state,
                                       format("EcoRoof: UpdateSoilProps: Relative Soil Saturation Top Moisture <= 0.0001, Value=[{:.5R}].",
                                              state.dataEcoRoofMgr->RelativeSoilSaturationTop));
                    ShowContinueError(state, "Value is set to 0.0001 and simulation continues.");
                    ShowContinueError(state, "You may wish to increase the number of timesteps to attempt to alleviate the problem.");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "EcoRoof: UpdateSoilProps: Relative Soil Saturation Top Moisture < 0. continues",
                                               state.dataEcoRoofMgr->ErrIndex,
                                               state.dataEcoRoofMgr->RelativeSoilSaturationTop,
                                               state.dataEcoRoofMgr->RelativeSoilSaturationTop);
                state.dataEcoRoofMgr->RelativeSoilSaturationTop = 0.0001;
            }
            state.dataEcoRoofMgr->SoilHydroConductivityTop =
                SoilConductivitySaturation * std::pow(state.dataEcoRoofMgr->RelativeSoilSaturationTop, lambda) *
                pow_2(1.0 - std::pow(1.0 - std::pow(state.dataEcoRoofMgr->RelativeSoilSaturationTop, n / (n - 1.0)), (n - 1.0) / n));
            state.dataEcoRoofMgr->CapillaryPotentialTop =
                (-1.0 / alpha) * std::pow(std::pow(1.0 / state.dataEcoRoofMgr->RelativeSoilSaturationTop, n / (n - 1.0)) - 1.0, 1.0 / n);

            // Then the soil parameters for the root soil layer
            state.dataEcoRoofMgr->RelativeSoilSaturationRoot = (MeanRootMoisture - MoistureResidual) / (MoistureMax - MoistureResidual);
            state.dataEcoRoofMgr->SoilHydroConductivityRoot =
                SoilConductivitySaturation * std::pow(state.dataEcoRoofMgr->RelativeSoilSaturationRoot, lambda) *
                pow_2(1.0 - std::pow(1.0 - std::pow(state.dataEcoRoofMgr->RelativeSoilSaturationRoot, n / (n - 1.0)), (n - 1.0) / n));
            state.dataEcoRoofMgr->CapillaryPotentialRoot =
                (-1.0 / alpha) * std::pow(std::pow(1.0 / state.dataEcoRoofMgr->RelativeSoilSaturationRoot, n / (n - 1.0)) - 1.0, 1.0 / n);

            // Next, using the soil parameters, solve for the soil moisture
            state.dataEcoRoofMgr->SoilConductivityAveTop =
                (state.dataEcoRoofMgr->SoilHydroConductivityTop + state.dataEcoRoofMgr->SoilHydroConductivityRoot) * 0.5;
            Moisture +=
                (state.dataEcoRoofMgr->TimeStepZoneSec / state.dataEcoRoofMgr->TopDepth) *
                ((state.dataEcoRoofMgr->SoilConductivityAveTop *
                  (state.dataEcoRoofMgr->CapillaryPotentialTop - state.dataEcoRoofMgr->CapillaryPotentialRoot) / state.dataEcoRoofMgr->TopDepth) -
                 state.dataEcoRoofMgr->SoilConductivityAveTop);

            // Now limit the soil from going over the moisture maximum and takes excess to create runoff
            if (Moisture >= MoistureMax) {       // This statement makes sure that the top layer is not over the moisture maximum for the soil.
                Moisture = 0.9999 * MoistureMax; // then it takes any moisture over the maximum amount and makes it runoff
                state.dataEcoRoofMgr->CurrentRunoff += (Moisture - MoistureMax * 0.9999) * state.dataEcoRoofMgr->TopDepth;
            }

            // Now make sure that the soil does not go below the moisture minimum
            if (Moisture <= (1.01 * MoistureResidual)) {
                Moisture = 1.01 * MoistureResidual;
            }

            // Next, solve the parameters for the bottom layer
            state.dataEcoRoofMgr->SoilConductivityAveRoot = state.dataEcoRoofMgr->SoilHydroConductivityRoot;

            // Now make sure the rate of liquid leaving the soil is more than one drop per hour
            if ((state.dataEcoRoofMgr->SoilConductivityAveRoot * 3600.0) <= (2.33e-7)) {
                state.dataEcoRoofMgr->SoilConductivityAveRoot = 0.0;
            }

            // Using the parameters above, distribute the Root Layer moisture
            state.dataEcoRoofMgr->TestMoisture = MeanRootMoisture;
            MeanRootMoisture +=
                (state.dataEcoRoofMgr->TimeStepZoneSec / state.dataEcoRoofMgr->RootDepth) *
                ((state.dataEcoRoofMgr->SoilConductivityAveTop *
                  (state.dataEcoRoofMgr->CapillaryPotentialTop - state.dataEcoRoofMgr->CapillaryPotentialRoot) / state.dataEcoRoofMgr->RootDepth) +
                 state.dataEcoRoofMgr->SoilConductivityAveTop - state.dataEcoRoofMgr->SoilConductivityAveRoot);

            // Limit the moisture from going over the saturation limit and create runoff:
            if (MeanRootMoisture >= MoistureMax) {
                MeanRootMoisture = 0.9999 * MoistureMax;
                state.dataEcoRoofMgr->CurrentRunoff += (Moisture - MoistureMax * 0.9999) * state.dataEcoRoofMgr->RootDepth;
            }

            // Limit the soil from going below the soil saturation limit:
            if (MeanRootMoisture <= (1.01 * MoistureResidual)) {
                MeanRootMoisture = 1.01 * MoistureResidual;
            }

            // Next, track runoff from the bottom of the soil:
            state.dataEcoRoofMgr->CurrentRunoff += state.dataEcoRoofMgr->SoilConductivityAveRoot * state.dataEcoRoofMgr->TimeStepZoneSec;

            //~~~END SF EDITS
        }

        // NEXT Limit moisture values to saturation (create RUNOFF that we can track)
        // CurrentRunoff is sum of "overwatering" in a timestep and excess moisture content
        if (!state.dataGlobal->WarmupFlag) {
            state.dataEcoRoofMgr->CumRunoff += state.dataEcoRoofMgr->CurrentRunoff;
        }

        if (MeanRootMoisture <= MoistureResidual * 1.00001) {
            Moisture -= (MoistureResidual * 1.00001 - MeanRootMoisture) * state.dataEcoRoofMgr->RootDepth / state.dataEcoRoofMgr->TopDepth;
            // If the plant has extracted more moisture than is in the root zone soil, then make it come from
            // the top layer rather than the root zone... unless top is also dry...
            if (Moisture < MoistureResidual * 1.00001) {
                Moisture = MoistureResidual * 1.00001;
            }
            MeanRootMoisture = MoistureResidual * 1.00001; // Need to make sure we do not divide by zero later.
        }

        // ------------------------------------------------------------------------------------------
        // Having updated moisture values now we modify soil thermal properties based on moisture content

        // NOTE: Variables SoilAbsorpSolar, SoilDensity, SoilSpecHeat, and SoilConductivity are the values
        // that the soil properties OUGHT to attain for the current moisture level. These values are then
        // moderated using the TestRatio variable so that from one time step to the next no variable
        // changes by more than a certain percentage (typically 5-20%).

        // Note wet soil absorptance is generally 25-50% higher than dry soil absorptance (assume linear)
        SoilAbsorpSolar = state.dataEcoRoofMgr->DryAbsorp +
                          (0.92 - state.dataEcoRoofMgr->DryAbsorp) * (Moisture - MoistureResidual) / (MoistureMax - MoistureResidual);
        // Limit solar absorptivity to 95% so soil albedo is always above 5%
        if (SoilAbsorpSolar > 0.95) {
            SoilAbsorpSolar = 0.95;
        }
        // Limit solar absorptivity to greater than 20% so that albedo is always less than 80%
        if (SoilAbsorpSolar < 0.20) {
            SoilAbsorpSolar = 0.20;
        }

        // Need to use for albedo in CalcEcoroof
        TestRatio = (1.0 - SoilAbsorpSolar) / Alphag;
        if (TestRatio > RatioMax) {
            TestRatio = RatioMax;
        }
        if (TestRatio < RatioMin) {
            TestRatio = RatioMin;
        }
        Alphag *= TestRatio; //  included 1.0 - to make it albedo rather than SW absorptivity
        // Note wet soil density is calculated by simply adding the mass of water...
        AvgMoisture = (state.dataEcoRoofMgr->RootDepth * MeanRootMoisture + state.dataEcoRoofMgr->TopDepth * Moisture) / SoilThickness;
        SoilDensity = state.dataEcoRoofMgr->DryDens + (AvgMoisture - MoistureResidual) * 990.0;
        // Note 990 kg/m^3 is water density and the moisture is depth-averaged

        // Note wet soil has specific heat that is 40% higher than dry soil (assume linear)
        // OLD ::  SoilSpecHeat = DrySpecHeat*(1.0d0+ 0.4d0*(AvgMoisture-MoistureResidual)/(MoistureMax-MoistureResidual))
        // This is now based on Melos Hagos's results for C (March 2009)
        //    SoilSpecHeat = DrySpecHeat + 3.09*(AvgMoisture) CLEARLY in ERROR BY FACTOR of 1000
        //    DJS - Melos report has Spec = Cdry + 1.9 theta (where C is in kJ/kg/K), so...
        SoilSpecHeat = state.dataEcoRoofMgr->DrySpecHeat + 1900.0 * AvgMoisture;

        // Note wet soil has thermal conductivity that is up to 3 times that of  dry soil ...
        // For now simply let it DOUBLE over the range of moisture

        // Note wet soil has thermal conductivity that is up to 3 times that of  dry soil ...
        // OLD :: SoilConductivity = DryCond* (1.0d0 + 1.0d0 * (AvgMoisture-MoistureResidual)/(MoistureMax-MoistureResidual))
        // This is now based on Melos Hagos's results for k/kdry (March 2009)
        SatRatio = (AvgMoisture - MoistureResidual) / (MoistureMax - MoistureResidual);
        SoilConductivity = (state.dataEcoRoofMgr->DryCond / 1.15) * (1.45 * std::exp(4.411 * SatRatio)) / (1.0 + 0.45 * std::exp(4.411 * SatRatio));
        // DJS 2009 - note, this allows the actual conductivity to dip a little below the dry value... corresponding to
        // DJS 2009 - "bone dry" if you will, when moisture --> residual value.

        // HERE WE RE-RUN THE CONDUCTION TRANSFER FUNCTION (CTF) CALCULATIONS

        // NOTE: CTF uses the original dataMaterial.Material( )%xxx variables, so must update these for current construction and
        //       time step...
        // TestRatio variable is available just in case there are stability issues. If so, we can limit the amount
        // by which soil properties are allowed to vary in one time step (10% in example below).

        TestRatio = SoilConductivity / matER->Conductivity;
        if (TestRatio > RatioMax) {
            TestRatio = RatioMax;
        }
        if (TestRatio < RatioMin) {
            TestRatio = RatioMin;
        }
        matER->Conductivity *= TestRatio;
        SoilConductivity = matER->Conductivity;

        TestRatio = SoilDensity / matER->Density;
        if (TestRatio > RatioMax) {
            TestRatio = RatioMax;
        }
        if (TestRatio < RatioMin) {
            TestRatio = RatioMin;
        }
        matER->Density *= TestRatio;
        SoilDensity = matER->Density;

        TestRatio = SoilSpecHeat / matER->SpecHeat;
        if (TestRatio > RatioMax) {
            TestRatio = RatioMax;
        }
        if (TestRatio < RatioMin) {
            TestRatio = RatioMin;
        }
        matER->SpecHeat *= TestRatio;
        SoilSpecHeat = matER->SpecHeat;

        // Now call InitConductionTransferFunction with the ConstrNum as the argument. As long as the argument is
        // non-zero InitConductionTransferFunction will ONLY update this construction. If the argument is 0 it will
        // rerun the ENTIRE InitConductionTransferFunction on all constructions (as in initial code start-up mode).

        // DJS The following works for most simulations, but has stability issues in some cases.
        // DJS - in the present version it seems best to NOT update soil thermal properties.
        //   Call InitConductionTransferFunctions(ConstrNum)
        // DJS In future revision we will implement these modified soil thermal properties in the finite difference
        // solution scheme.

        // DJS - Note the following write/format statements should be commented out prior to releasing within EnergyPlus
        // DJS - they are handy in doing hourly validation/evaluations, though, so leave them in for future development.

        //   write(unit,799) DayofYear, HourOfDay, Qsoil,Tg, Tf, Moisture, MeanRootMoisture,CumPrecip &
        //  ,CumET,CumRunoff, CumIrrigation, SoilDensity, SoilSpecHeat,SoilConductivity,Alphag
        // 799 format(' ',I3,' ',I3,' ',' ',f9.3,' ',f6.2,' ',f6.2,' ',f5.3,' ',f5.3,' ',f6.4, '  '  &
        //    f7.3, ' ', f7.3, ' ',f7.3, ' ',f6.1,' ',f7.1,'  ',f6.3,'  ',f6.2)
    }

    void CalculateEcoRoofSolar(EnergyPlusData &state,
                               Real64 &RS, // Solar on roof (assumed horizontal)
                               Real64 &f1, // Solar term for Stomatal Resistance
                               int const SurfNum)
    {

        // Use SOLCOS(3) here to calculate RS since as stated in comments above this is only done for one surface currently.
        // So, it is better to assume that the roof is flat until multiple surfaces can be handled.  Then, the next line can
        // use state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum) instead of
        // SOLCOS(3).
        RS = max(state.dataEnvrn->SOLCOS(3), 0.0) * state.dataEnvrn->BeamSolarRad +
             state.dataSolarShading->SurfAnisoSkyMult(SurfNum) * state.dataEnvrn->DifSolarRad;
        Real64 f1inv = min(1.0, (0.004 * RS + 0.005) / (0.81 * (0.004 * RS + 1.0)));
        f1 = 1.0 / f1inv;
    }

} // namespace EcoRoofManager

} // namespace EnergyPlus
