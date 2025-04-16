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

#include "AirflowNetwork/Elements.hpp"
#include "AirflowNetwork/Properties.hpp"
#include "AirflowNetwork/Solver.hpp"

#include "../../Data/EnergyPlusData.hh"
#include "../../DataAirLoop.hh"
#include "../../DataEnvironment.hh"
#include "../../DataHVACGlobals.hh"
#include "../../DataLoopNode.hh"
#include "../../DataSurfaces.hh"

namespace EnergyPlus {

namespace AirflowNetwork {

    static Real64 square(Real64 x)
    {
        return x * x;
    }

    int Duct::calculate([[maybe_unused]] EnergyPlusData &state,
                        bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                        Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                        [[maybe_unused]] int const i,             // Linkage number
                        [[maybe_unused]] const Real64 multiplier, // Element multiplier
                        [[maybe_unused]] const Real64 control,    // Element control signal
                        const AirState &propN,                    // Node 1 properties
                        const AirState &propM,                    // Node 2 properties
                        std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                        std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a duct/pipe component using Colebrook equation for the
        // turbulent friction factor

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr C(0.868589);
        Real64 constexpr EPS(0.001);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 A0;
        Real64 A1;
        Real64 A2;
        Real64 B;
        Real64 D;
        Real64 S2;
        Real64 CDM;
        Real64 FL; // friction factor for laminar flow.
        Real64 FT; // friction factor for turbulent flow.
        Real64 FTT;
        Real64 RE; // Reynolds number.
        Real64 ed;
        Real64 ld;
        Real64 g;
        Real64 AA1;

        // Formats
        // static gio::Fmt Format_901("(A5,I3,6X,4E16.7)");

        // CompNum = state.afn->AirflowNetworkCompData(j).TypeNum;
        ed = roughness / hydraulicDiameter;
        ld = L / hydraulicDiameter;
        g = 1.14 - 0.868589 * std::log(ed);
        AA1 = g;

        if (LFLAG) {
            // Initialization by linear relation.
            if (PDROP >= 0.0) {
                DF[0] = (2.0 * propN.density * A * hydraulicDiameter) / (propN.viscosity * InitLamCoef * ld);
            } else {
                DF[0] = (2.0 * propM.density * A * hydraulicDiameter) / (propM.viscosity * InitLamCoef * ld);
            }
            F[0] = -DF[0] * PDROP;
            // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwi:" << i << InitLamCoef << F[0] << DF[0];
        } else {
            // Standard calculation.
            if (PDROP >= 0.0) {
                // Flow in positive direction.
                // Laminar flow coefficient !=0
                if (LamFriCoef >= 0.001) {
                    A2 = LamFriCoef / (2.0 * propN.density * A * A);
                    A1 = (propN.viscosity * LamDynCoef * ld) / (2.0 * propN.density * A * hydraulicDiameter);
                    A0 = -PDROP;
                    CDM = std::sqrt(A1 * A1 - 4.0 * A2 * A0);
                    FL = (CDM - A1) / (2.0 * A2);
                    CDM = 1.0 / CDM;
                } else {
                    CDM = (2.0 * propN.density * A * hydraulicDiameter) / (propN.viscosity * LamDynCoef * ld);
                    FL = CDM * PDROP;
                }
                RE = FL * hydraulicDiameter / (propN.viscosity * A);
                // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwl:" << i << PDROP << FL << CDM << RE;
                // Turbulent flow; test when Re>10.
                if (RE >= 10.0) {
                    S2 = std::sqrt(2.0 * propN.density * PDROP) * A;
                    FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                    // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << S2 << FTT << g;
                    while (true) {
                        FT = FTT;
                        B = (9.3 * propN.viscosity * A) / (FT * roughness);
                        D = 1.0 + g * B;
                        g -= (g - AA1 + C * std::log(D)) / (1.0 + C * B / D);
                        FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                        // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << B << FTT << g;
                        if (std::abs(FTT - FT) / FTT < EPS) break;
                    }
                    FT = FTT;
                } else {
                    FT = FL;
                }
            } else {
                // Flow in negative direction.
                // Laminar flow coefficient !=0
                if (LamFriCoef >= 0.001) {
                    A2 = LamFriCoef / (2.0 * propM.density * A * A);
                    A1 = (propM.viscosity * LamDynCoef * ld) / (2.0 * propM.density * A * hydraulicDiameter);
                    A0 = PDROP;
                    CDM = std::sqrt(A1 * A1 - 4.0 * A2 * A0);
                    FL = -(CDM - A1) / (2.0 * A2);
                    CDM = 1.0 / CDM;
                } else {
                    CDM = (2.0 * propM.density * A * hydraulicDiameter) / (propM.viscosity * LamDynCoef * ld);
                    FL = CDM * PDROP;
                }
                RE = -FL * hydraulicDiameter / (propM.viscosity * A);
                // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwl:" << i << PDROP << FL << CDM << RE;
                // Turbulent flow; test when Re>10.
                if (RE >= 10.0) {
                    S2 = std::sqrt(-2.0 * propM.density * PDROP) * A;
                    FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                    // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << S2 << FTT << g;
                    while (true) {
                        FT = FTT;
                        B = (9.3 * propM.viscosity * A) / (FT * roughness);
                        D = 1.0 + g * B;
                        g -= (g - AA1 + C * std::log(D)) / (1.0 + C * B / D);
                        FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                        // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << B << FTT << g;
                        if (std::abs(FTT - FT) / FTT < EPS) break;
                    }
                    FT = -FTT;
                } else {
                    FT = FL;
                }
            }
            // Select laminar or turbulent flow.
            if (std::abs(FL) <= std::abs(FT)) {
                F[0] = FL;
                DF[0] = CDM;
            } else {
                F[0] = FT;
                DF[0] = 0.5 * FT / PDROP;
            }
        }
        return 1;
    }

    int Duct::calculate([[maybe_unused]] EnergyPlusData &state,
                        Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                        [[maybe_unused]] const Real64 multiplier, // Element multiplier
                        [[maybe_unused]] const Real64 control,    // Element control signal
                        const AirState &propN,                    // Node 1 properties
                        const AirState &propM,                    // Node 2 properties
                        std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                        std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a duct/pipe component using Colebrook equation for the
        // turbulent friction factor

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr C(0.868589);
        Real64 constexpr EPS(0.001);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 A0;
        Real64 A1;
        Real64 A2;
        Real64 B;
        Real64 D;
        Real64 S2;
        Real64 CDM;
        Real64 FL; // friction factor for laminar flow.
        Real64 FT; // friction factor for turbulent flow.
        Real64 FTT;
        Real64 RE; // Reynolds number.
        Real64 ed;
        Real64 ld;
        Real64 g;
        Real64 AA1;

        // Formats
        // static gio::Fmt Format_901("(A5,I3,6X,4E16.7)");

        // CompNum = state.afn->AirflowNetworkCompData(j).TypeNum;
        ed = roughness / hydraulicDiameter;
        ld = L / hydraulicDiameter;
        g = 1.14 - 0.868589 * std::log(ed);
        AA1 = g;

        // Standard calculation.
        if (PDROP >= 0.0) {
            // Flow in positive direction.
            // Laminar flow coefficient !=0
            if (LamFriCoef >= 0.001) {
                A2 = LamFriCoef / (2.0 * propN.density * A * A);
                A1 = (propN.viscosity * LamDynCoef * ld) / (2.0 * propN.density * A * hydraulicDiameter);
                A0 = -PDROP;
                CDM = std::sqrt(A1 * A1 - 4.0 * A2 * A0);
                FL = (CDM - A1) / (2.0 * A2);
                CDM = 1.0 / CDM;
            } else {
                CDM = (2.0 * propN.density * A * hydraulicDiameter) / (propN.viscosity * LamDynCoef * ld);
                FL = CDM * PDROP;
            }
            RE = FL * hydraulicDiameter / (propN.viscosity * A);
            // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwl:" << i << PDROP << FL << CDM << RE;
            // Turbulent flow; test when Re>10.
            if (RE >= 10.0) {
                S2 = std::sqrt(2.0 * propN.density * PDROP) * A;
                FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << S2 << FTT << g;
                while (true) {
                    FT = FTT;
                    B = (9.3 * propN.viscosity * A) / (FT * roughness);
                    D = 1.0 + g * B;
                    g -= (g - AA1 + C * std::log(D)) / (1.0 + C * B / D);
                    FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                    // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << B << FTT << g;
                    if (std::abs(FTT - FT) / FTT < EPS) break;
                }
                FT = FTT;
            } else {
                FT = FL;
            }
        } else {
            // Flow in negative direction.
            // Laminar flow coefficient !=0
            if (LamFriCoef >= 0.001) {
                A2 = LamFriCoef / (2.0 * propM.density * A * A);
                A1 = (propM.viscosity * LamDynCoef * ld) / (2.0 * propM.density * A * hydraulicDiameter);
                A0 = PDROP;
                CDM = std::sqrt(A1 * A1 - 4.0 * A2 * A0);
                FL = -(CDM - A1) / (2.0 * A2);
                CDM = 1.0 / CDM;
            } else {
                CDM = (2.0 * propM.density * A * hydraulicDiameter) / (propM.viscosity * LamDynCoef * ld);
                FL = CDM * PDROP;
            }
            RE = -FL * hydraulicDiameter / (propM.viscosity * A);
            // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwl:" << i << PDROP << FL << CDM << RE;
            // Turbulent flow; test when Re>10.
            if (RE >= 10.0) {
                S2 = std::sqrt(-2.0 * propM.density * PDROP) * A;
                FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << S2 << FTT << g;
                while (true) {
                    FT = FTT;
                    B = (9.3 * propM.viscosity * A) / (FT * roughness);
                    D = 1.0 + g * B;
                    g -= (g - AA1 + C * std::log(D)) / (1.0 + C * B / D);
                    FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                    // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << B << FTT << g;
                    if (std::abs(FTT - FT) / FTT < EPS) break;
                }
                FT = -FTT;
            } else {
                FT = FL;
            }
        }
        // Select laminar or turbulent flow.
        if (std::abs(FL) <= std::abs(FT)) {
            F[0] = FL;
            DF[0] = CDM;
        } else {
            F[0] = FT;
            DF[0] = 0.5 * FT / PDROP;
        }
        return 1;
    }

    int SurfaceCrack::calculate([[maybe_unused]] EnergyPlusData &state,
                                bool const linear,            // Initialization flag. If true, use linear relationship
                                Real64 const pdrop,           // Total pressure drop across a component (P1 - P2) [Pa]
                                [[maybe_unused]] int const i, // Linkage number
                                const Real64 multiplier,      // Element multiplier
                                const Real64 control,         // Element control signal
                                const AirState &propN,        // Node 1 properties
                                const AirState &propM,        // Node 2 properties
                                std::array<Real64, 2> &F,     // Airflow through the component [kg/s]
                                std::array<Real64, 2> &DF     // Partial derivative:  DF/DP
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  Jason DeGraw

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a surface crack component

        // Real64 rhoz_norm = AIRDENSITY(StandardP, StandardT, StandardW);
        // Real64 viscz_norm = 1.71432e-5 + 4.828e-8 * StandardT;

        Real64 VisAve{0.5 * (propN.viscosity + propM.viscosity)};
        Real64 Tave{0.5 * (propN.temperature + propM.temperature)};

        Real64 sign{1.0};
        Real64 upwind_temperature{propN.temperature};
        Real64 upwind_density{propN.density};
        Real64 upwind_viscosity{propN.viscosity};
        Real64 upwind_sqrt_density{propN.sqrt_density};
        Real64 abs_pdrop = pdrop;

        if (pdrop < 0.0) {
            sign = -1.0;
            upwind_temperature = propM.temperature;
            upwind_density = propM.density;
            upwind_viscosity = propM.viscosity;
            upwind_sqrt_density = propM.sqrt_density;
            abs_pdrop = -pdrop;
        }

        Real64 coef = coefficient * control * multiplier / upwind_sqrt_density;

        // Laminar calculation
        Real64 RhoCor{TOKELVIN(upwind_temperature) / TOKELVIN(Tave)};
        Real64 Ctl{std::pow(reference_density / upwind_density / RhoCor, exponent - 1.0) *
                   std::pow(reference_viscosity / VisAve, 2.0 * exponent - 1.0)};
        Real64 CDM{coef * upwind_density / upwind_viscosity * Ctl};
        Real64 FL{CDM * pdrop};
        Real64 abs_FT;

        if (linear) {
            DF[0] = CDM;
            F[0] = FL;
        } else {
            // Turbulent flow.
            if (exponent == 0.5) {
                abs_FT = coef * upwind_sqrt_density * std::sqrt(abs_pdrop) * Ctl;
            } else {
                abs_FT = coef * upwind_sqrt_density * std::pow(abs_pdrop, exponent) * Ctl;
            }
            // Select linear or turbulent flow.
            if (std::abs(FL) <= abs_FT) {
                F[0] = FL;
                DF[0] = CDM;
            } else {
                F[0] = sign * abs_FT;
                DF[0] = F[0] * exponent / pdrop;
            }
        }
        return 1;
    }

    int SurfaceCrack::calculate(EnergyPlusData &state,
                                Real64 const pdrop,       // Total pressure drop across a component (P1 - P2) [Pa]
                                const Real64 multiplier,  // Element multiplier
                                const Real64 control,     // Element control signal
                                const AirState &propN,    // Node 1 properties
                                const AirState &propM,    // Node 2 properties
                                std::array<Real64, 2> &F, // Airflow through the component [kg/s]
                                std::array<Real64, 2> &DF // Partial derivative:  DF/DP
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  Jason DeGraw

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a surface crack component

        // Real64 rhoz_norm = AIRDENSITY(StandardP, StandardT, StandardW);
        // Real64 viscz_norm = 1.71432e-5 + 4.828e-8 * StandardT;

        Real64 VisAve{0.5 * (propN.viscosity + propM.viscosity)};
        Real64 Tave{0.5 * (propN.temperature + propM.temperature)};

        Real64 sign{1.0};
        Real64 upwind_temperature{propN.temperature};
        Real64 upwind_density{propN.density};
        Real64 upwind_viscosity{propN.viscosity};
        Real64 upwind_sqrt_density{propN.sqrt_density};
        Real64 abs_pdrop = pdrop;

        if (pdrop < 0.0) {
            sign = -1.0;
            upwind_temperature = propM.temperature;
            upwind_density = propM.density;
            upwind_viscosity = propM.viscosity;
            upwind_sqrt_density = propM.sqrt_density;
            abs_pdrop = -pdrop;
        }

        Real64 coef = coefficient * control * multiplier / upwind_sqrt_density;

        // Laminar calculation
        Real64 RhoCor{TOKELVIN(upwind_temperature) / TOKELVIN(Tave)};
        Real64 Ctl{std::pow(reference_density / upwind_density / RhoCor, exponent - 1.0) *
                   std::pow(reference_viscosity / VisAve, 2.0 * exponent - 1.0)};
        Real64 CDM{coef * upwind_density / upwind_viscosity * Ctl};
        Real64 FL{CDM * pdrop};
        Real64 abs_FT;

        // Turbulent flow.
        if (exponent == 0.5) {
            abs_FT = coef * upwind_sqrt_density * std::sqrt(abs_pdrop) * Ctl;
        } else {
            abs_FT = coef * upwind_sqrt_density * std::pow(abs_pdrop, exponent) * Ctl;
        }
        // Select linear or turbulent flow.
        if (std::abs(FL) <= abs_FT) {
            F[0] = FL;
            DF[0] = CDM;
        } else {
            F[0] = sign * abs_FT;
            DF[0] = F[0] * exponent / pdrop;
        }

        return 1;
    }

    int DuctLeak::calculate(EnergyPlusData &state,
                            bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                            Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                            [[maybe_unused]] int const i,             // Linkage number
                            [[maybe_unused]] const Real64 multiplier, // Element multiplier
                            [[maybe_unused]] const Real64 control,    // Element control signal
                            const AirState &propN,                    // Node 1 properties
                            const AirState &propM,                    // Node 2 properties
                            std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                            std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a power law resistance airflow component

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CDM;
        Real64 FL;
        Real64 FT;
        Real64 Ctl;

        // Formats
        // static gio::Fmt Format_901("(A5,I3,6X,4E16.7)");

        // Crack standard condition: T=20C, p=101325 Pa and 0 g/kg
        Real64 RhozNorm = state.afn->properties.density(101325.0, 20.0, 0.0);
        Real64 VisczNorm = 1.71432e-5 + 4.828e-8 * 20.0;
        Real64 coef = FlowCoef;

        if (PDROP >= 0.0) {
            coef /= propN.sqrt_density;
        } else {
            coef /= propM.sqrt_density;
        }

        if (LFLAG) {
            // Initialization by linear relation.
            if (PDROP >= 0.0) {
                Ctl = std::pow(RhozNorm / propN.density, FlowExpo - 1.0) * std::pow(VisczNorm / propN.viscosity, 2.0 * FlowExpo - 1.0);
                DF[0] = coef * propN.density / propN.viscosity * Ctl;
            } else {
                Ctl = std::pow(RhozNorm / propM.density, FlowExpo - 1.0) * std::pow(VisczNorm / propM.viscosity, 2.0 * FlowExpo - 1.0);
                DF[0] = coef * propM.density / propM.viscosity * Ctl;
            }
            F[0] = -DF[0] * PDROP;
        } else {
            // Standard calculation.
            if (PDROP >= 0.0) {
                // Flow in positive direction for laminar flow.
                Ctl = std::pow(RhozNorm / propN.density, FlowExpo - 1.0) * std::pow(VisczNorm / propN.viscosity, 2.0 * FlowExpo - 1.0);
                CDM = coef * propN.density / propN.viscosity * Ctl;
                FL = CDM * PDROP;
                // Flow in positive direction for turbulent flow.
                if (FlowExpo == 0.5) {
                    FT = coef * propN.sqrt_density * std::sqrt(PDROP);
                } else {
                    FT = coef * propN.sqrt_density * std::pow(PDROP, FlowExpo);
                }
            } else {
                // Flow in negative direction for laminar flow
                CDM = coef * propM.density / propM.viscosity;
                FL = CDM * PDROP;
                // Flow in negative direction for turbulent flow
                if (FlowExpo == 0.5) {
                    FT = -coef * propM.sqrt_density * std::sqrt(-PDROP);
                } else {
                    FT = -coef * propM.sqrt_density * std::pow(-PDROP, FlowExpo);
                }
            }
            // Select laminar or turbulent flow.
            // if (LIST >= 4) gio::write(Unit21, Format_901) << " plr: " << i << PDROP << FL << FT;
            if (std::abs(FL) <= std::abs(FT)) {
                F[0] = FL;
                DF[0] = CDM;
            } else {
                F[0] = FT;
                DF[0] = FT * FlowExpo / PDROP;
            }
        }
        return 1;
    }

    int DuctLeak::calculate(EnergyPlusData &state,
                            Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                            [[maybe_unused]] const Real64 multiplier, // Element multiplier
                            [[maybe_unused]] const Real64 control,    // Element control signal
                            const AirState &propN,                    // Node 1 properties
                            const AirState &propM,                    // Node 2 properties
                            std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                            std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a power law resistance airflow component

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CDM;
        Real64 FL;
        Real64 FT;
        Real64 Ctl;

        // Formats
        // static gio::Fmt Format_901("(A5,I3,6X,4E16.7)");

        // Crack standard condition: T=20C, p=101325 Pa and 0 g/kg
        Real64 RhozNorm = state.afn->properties.density(101325.0, 20.0, 0.0);
        Real64 VisczNorm = 1.71432e-5 + 4.828e-8 * 20.0;
        Real64 coef = FlowCoef;

        if (PDROP >= 0.0) {
            coef /= propN.sqrt_density;
        } else {
            coef /= propM.sqrt_density;
        }

        // Standard calculation.
        if (PDROP >= 0.0) {
            // Flow in positive direction for laminar flow.
            Ctl = std::pow(RhozNorm / propN.density, FlowExpo - 1.0) * std::pow(VisczNorm / propN.viscosity, 2.0 * FlowExpo - 1.0);
            CDM = coef * propN.density / propN.viscosity * Ctl;
            FL = CDM * PDROP;
            // Flow in positive direction for turbulent flow.
            if (FlowExpo == 0.5) {
                FT = coef * propN.sqrt_density * std::sqrt(PDROP);
            } else {
                FT = coef * propN.sqrt_density * std::pow(PDROP, FlowExpo);
            }
        } else {
            // Flow in negative direction for laminar flow
            CDM = coef * propM.density / propM.viscosity;
            FL = CDM * PDROP;
            // Flow in negative direction for turbulent flow
            if (FlowExpo == 0.5) {
                FT = -coef * propM.sqrt_density * std::sqrt(-PDROP);
            } else {
                FT = -coef * propM.sqrt_density * std::pow(-PDROP, FlowExpo);
            }
        }
        // Select laminar or turbulent flow.
        // if (LIST >= 4) gio::write(Unit21, Format_901) << " plr: " << i << PDROP << FL << FT;
        if (std::abs(FL) <= std::abs(FT)) {
            F[0] = FL;
            DF[0] = CDM;
        } else {
            F[0] = FT;
            DF[0] = FT * FlowExpo / PDROP;
        }
        return 1;
    }

    int ConstantVolumeFan::calculate(EnergyPlusData &state,
                                     bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                                     Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                                     int const i,                              // Linkage number
                                     [[maybe_unused]] const Real64 multiplier, // Element multiplier
                                     [[maybe_unused]] const Real64 control,    // Element control signal
                                     const AirState &propN,                    // Node 1 properties
                                     const AirState &propM,                    // Node 2 properties
                                     std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                                     std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a constant flow rate airflow component -- using standard interface.

        // Using/Aliasing
        auto &NumPrimaryAirSys = state.dataHVACGlobal->NumPrimaryAirSys;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int k;
        int k1;
        Real64 SumTermFlow;     // Sum of all Terminal flows [kg/s]
        Real64 SumFracSuppLeak; // Sum of all supply leaks as a fraction of supply fan flow rate
        int Node1;
        int Node2;

        int NF(1);

        int AirLoopNum = state.afn->AirflowNetworkLinkageData(i).AirLoopNum;

        if (fanType == HVAC::FanType::OnOff) {
            if (state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopFanOperationMode == HVAC::FanOp::Cycling &&
                state.dataLoopNodes->Node(InletNode).MassFlowRate == 0.0) {
                NF = GenericDuct(0.1, 0.001, LFLAG, PDROP, propN, propM, F, DF);
            } else if (state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopFanOperationMode == HVAC::FanOp::Cycling &&
                       state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOnMassFlowrate > 0.0) {
                F[0] = state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOnMassFlowrate;
            } else {
                F[0] = state.dataLoopNodes->Node(InletNode).MassFlowRate * Ctrl;
                if (state.afn->MultiSpeedHPIndicator == 2) {
                    F[0] = state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOnMassFlowrate *
                               state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopCompCycRatio +
                           state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOffMassFlowrate *
                               (1.0 - state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopCompCycRatio);
                }
            }
        } else if (fanType == HVAC::FanType::Constant) {
            if (state.dataLoopNodes->Node(InletNode).MassFlowRate > 0.0) {
                F[0] = FlowRate * Ctrl;
            } else if (NumPrimaryAirSys > 1 && state.dataLoopNodes->Node(InletNode).MassFlowRate <= 0.0) {
                NF = GenericDuct(0.1, 0.001, LFLAG, PDROP, propN, propM, F, DF);
            }

            if (state.afn->MultiSpeedHPIndicator == 2) {
                F[0] = state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOnMassFlowrate;
            }
        } else if (fanType == HVAC::FanType::VAV) {
            // Check VAV termals with a damper
            SumTermFlow = 0.0;
            SumFracSuppLeak = 0.0;
            for (k = 1; k <= state.afn->ActualNumOfLinks; ++k) {
                if (state.afn->AirflowNetworkLinkageData(k).VAVTermDamper && state.afn->AirflowNetworkLinkageData(k).AirLoopNum == AirLoopNum) {
                    k1 = state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(k).NodeNums[0]).EPlusNodeNum;
                    if (state.dataLoopNodes->Node(k1).MassFlowRate > 0.0) {
                        SumTermFlow += state.dataLoopNodes->Node(k1).MassFlowRate;
                    }
                }
                if (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(k).CompNum).CompTypeNum == AirflowElementType::ELR) {
                    // Calculate supply leak sensible losses
                    Node1 = state.afn->AirflowNetworkLinkageData(k).NodeNums[0];
                    Node2 = state.afn->AirflowNetworkLinkageData(k).NodeNums[1];
                    if ((state.afn->AirflowNetworkNodeData(Node2).EPlusZoneNum > 0) && (state.afn->AirflowNetworkNodeData(Node1).EPlusNodeNum == 0) &&
                        (state.afn->AirflowNetworkNodeData(Node1).AirLoopNum == AirLoopNum)) {
                        SumFracSuppLeak +=
                            state.afn->DisSysCompELRData(state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(k).CompNum).TypeNum)
                                .ELR;
                    }
                }
            }
            F[0] = SumTermFlow / (1.0 - SumFracSuppLeak);
            state.afn->VAVTerminalRatio = 0.0;
            if (F[0] > MaxAirMassFlowRate) {
                state.afn->VAVTerminalRatio = MaxAirMassFlowRate / F[0];
                F[0] = MaxAirMassFlowRate;
            }
        }
        DF[0] = 0.0;
        return NF;
    }

    int DetailedFan::calculate(EnergyPlusData &state,
                               bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                               Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                               int const i,                              // Linkage number
                               [[maybe_unused]] const Real64 multiplier, // Element multiplier
                               const Real64 control,                     // Element control signal
                               const AirState &propN,                    // Node 1 properties
                               const AirState &propM,                    // Node 2 properties
                               std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                               std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a detailed fan component -- using standard interface.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr TOL(0.00001);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int j;
        int k;
        int L;
        Real64 DPDF;
        Real64 PRISE; // pressure rise (negative of pressure drop) (Pa).
        Real64 BX;
        Real64 BY;
        Real64 CX;
        Real64 CY;
        Real64 CCY;
        Real64 DX;
        Real64 DY;

        // Formats
        // static gio::Fmt Format_901("(A5,I3,5E14.6)");

        int NumCur = n;
        if (control <= 0.0) {
            // Speed = 0; treat fan as resistance.
            generic_crack(FlowCoef, FlowExpo, LFLAG, PDROP, propN, propM, F, DF);
            return 1;
        }
        // Pressure rise at reference fan speed.
        if (control >= TranRat) {
            PRISE = -PDROP * (RhoAir / propN.density) / (control * control);
        } else {
            PRISE = -PDROP * (RhoAir / propN.density) / (TranRat * control);
        }
        // if (LIST >= 4) gio::write(Unit21, Format_901) << " fan:" << i << PDROP << PRISE << AFECTL(i) << DisSysCompDetFanData(CompNum).TranRat;
        if (LFLAG) {
            // Initialization by linear approximation.
            F[0] = -Qfree * control * (1.0 - PRISE / Pshut);
            DPDF = -Pshut / Qfree;
            // if (LIST >= 4)
            //    gio::write(Unit21, Format_901) << " fni:" << JA << DisSysCompDetFanData(CompNum).Qfree << DisSysCompDetFanData(CompNum).Pshut;
        } else {
            // Solution of the fan performance curve.
            // Determine curve fit range.
            j = 1;
            k = 5 * (j - 1) + 1;
            BX = Coeff(k);
            BY = Coeff(k + 1) + BX * (Coeff(k + 2) + BX * (Coeff(k + 3) + BX * Coeff(k + 4))) - PRISE;
            if (BY < 0.0) ShowFatalError(state, "Out of range, too low in an AirflowNetwork detailed Fan");

            while (true) {
                DX = Coeff(k + 5);
                DY = Coeff(k + 1) + DX * (Coeff(k + 2) + DX * (Coeff(k + 3) + DX * Coeff(k + 5))) - PRISE;
                // if (LIST >= 4) gio::write(Unit21, Format_901) << " fp0:" << j << BX << BY << DX << DY;
                if (BY * DY <= 0.0) break;
                ++j;
                if (j > NumCur) ShowFatalError(state, "Out of range, too high (FAN) in ADS simulation");
                k += 5;
                BX = DX;
                BY = DY;
            }
            // Determine reference mass flow rate by false position method.
            L = 0;
            CY = 0.0;
        Label40:;
            ++L;
            if (L > 100) ShowFatalError(state, "Too many iterations (FAN) in AirflowNtework simulation");
            CCY = CY;
            CX = BX - BY * ((DX - BX) / (DY - BY));
            CY = Coeff(k + 1) + CX * (Coeff(k + 2) + CX * (Coeff(k + 3) + CX * Coeff(k + 4))) - PRISE;
            if (BY * CY == 0.0) goto Label90;
            if (BY * CY > 0.0) goto Label60;
            DX = CX;
            DY = CY;
            if (CY * CCY > 0.0) BY *= 0.5;
            goto Label70;
        Label60:;
            BX = CX;
            BY = CY;
            if (CY * CCY > 0.0) DY *= 0.5;
        Label70:;
            // if (LIST >= 4) gio::write(Unit21, Format_901) << " fpi:" << j << BX << CX << DX << BY << DY;
            if (DX - BX < TOL * CX) goto Label80;
            if (DX - BX < TOL) goto Label80;
            goto Label40;
        Label80:;
            CX = 0.5 * (BX + DX);
        Label90:;
            F[0] = CX;
            DPDF = Coeff(k + 2) + CX * (2.0 * Coeff(k + 3) + CX * 3.0 * Coeff(k + 4));
        }
        // Convert to flow at given speed.
        F[0] *= (propN.density / RhoAir) * control;
        // Set derivative w/r pressure drop (-).
        if (control >= TranRat) {
            DF[0] = -control / DPDF;
        } else {
            DF[0] = -1.0 / DPDF;
        }
        return 1;
    }

    int DetailedFan::calculate(EnergyPlusData &state,
                               Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                               [[maybe_unused]] const Real64 multiplier, // Element multiplier
                               const Real64 control,                     // Element control signal
                               const AirState &propN,                    // Node 1 properties
                               const AirState &propM,                    // Node 2 properties
                               std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                               std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a detailed fan component -- using standard interface.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr TOL(0.00001);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int j;
        int k;
        int L;
        Real64 DPDF;
        Real64 PRISE; // pressure rise (negative of pressure drop) (Pa).
        Real64 BX;
        Real64 BY;
        Real64 CX;
        Real64 CY;
        Real64 CCY;
        Real64 DX;
        Real64 DY;

        // Formats
        // static gio::Fmt Format_901("(A5,I3,5E14.6)");

        int NumCur = n;

        if (control <= 0.0) {
            // Speed = 0; treat fan as resistance.
            generic_crack(FlowCoef, FlowExpo, false, PDROP, propN, propM, F, DF);
            return 1;
        }
        // Pressure rise at reference fan speed.
        if (control >= TranRat) {
            PRISE = -PDROP * (RhoAir / propN.density) / pow_2(control);
        } else {
            PRISE = -PDROP * (RhoAir / propN.density) / (TranRat * control);
        }
        // if (LIST >= 4) gio::write(Unit21, Format_901) << " fan:" << i << PDROP << PRISE << AFECTL(i) << DisSysCompDetFanData(CompNum).TranRat;
        // if (LFLAG) {
        //    // Initialization by linear approximation.
        //    F[0] = -Qfree * control * (1.0 - PRISE / Pshut);
        //    DPDF = -Pshut / Qfree;
        //    // if (LIST >= 4)
        //    //    gio::write(Unit21, Format_901) << " fni:" << JA << DisSysCompDetFanData(CompNum).Qfree << DisSysCompDetFanData(CompNum).Pshut;
        //} else {
        // Solution of the fan performance curve.
        // Determine curve fit range.
        j = 1;
        k = 5 * (j - 1) + 1;
        BX = Coeff(k);
        BY = Coeff(k + 1) + BX * (Coeff(k + 2) + BX * (Coeff(k + 3) + BX * Coeff(k + 4))) - PRISE;
        if (BY < 0.0) ShowFatalError(state, "Out of range, too low in an AirflowNetwork detailed Fan");

        while (true) {
            DX = Coeff(k + 5);
            DY = Coeff(k + 1) + DX * (Coeff(k + 2) + DX * (Coeff(k + 3) + DX * Coeff(k + 5))) - PRISE;
            // if (LIST >= 4) gio::write(Unit21, Format_901) << " fp0:" << j << BX << BY << DX << DY;
            if (BY * DY <= 0.0) break;
            ++j;
            if (j > NumCur) ShowFatalError(state, "Out of range, too high (FAN) in ADS simulation");
            k += 5;
            BX = DX;
            BY = DY;
        }
        // Determine reference mass flow rate by false position method.
        L = 0;
        CY = 0.0;
    Label40:;
        ++L;
        if (L > 100) ShowFatalError(state, "Too many iterations (FAN) in AirflowNtework simulation");
        CCY = CY;
        CX = BX - BY * ((DX - BX) / (DY - BY));
        CY = Coeff(k + 1) + CX * (Coeff(k + 2) + CX * (Coeff(k + 3) + CX * Coeff(k + 4))) - PRISE;
        if (BY * CY == 0.0) goto Label90;
        if (BY * CY > 0.0) goto Label60;
        DX = CX;
        DY = CY;
        if (CY * CCY > 0.0) BY *= 0.5;
        goto Label70;
    Label60:;
        BX = CX;
        BY = CY;
        if (CY * CCY > 0.0) DY *= 0.5;
    Label70:;
        // if (LIST >= 4) gio::write(Unit21, Format_901) << " fpi:" << j << BX << CX << DX << BY << DY;
        if (DX - BX < TOL * CX) goto Label80;
        if (DX - BX < TOL) goto Label80;
        goto Label40;
    Label80:;
        CX = 0.5 * (BX + DX);
    Label90:;
        F[0] = CX;
        DPDF = Coeff(k + 2) + CX * (2.0 * Coeff(k + 3) + CX * 3.0 * Coeff(k + 4));

        // Convert to flow at given speed.
        F[0] *= (propN.density / RhoAir) * control;
        // Set derivative w/r pressure drop (-).
        if (control >= TranRat) {
            DF[0] = -control / DPDF;
        } else {
            DF[0] = -1.0 / DPDF;
        }
        return 1;
    }

    int Damper::calculate([[maybe_unused]] EnergyPlusData &state,
                          bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                          Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                          int const i,                              // Linkage number
                          [[maybe_unused]] const Real64 multiplier, // Element multiplier
                          [[maybe_unused]] const Real64 control,    // Element control signal
                          const AirState &propN,                    // Node 1 properties
                          const AirState &propM,                    // Node 2 properties
                          std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                          std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a Controlled power law resistance airflow component (damper)

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 C;

        // Formats
        // static gio::Fmt Format_901("(A5,I3,6X,4E16.7)");

        C = control;
        if (C < FlowMin) C = FlowMin;
        if (C > FlowMax) C = FlowMax;
        C = A0 + C * (A1 + C * (A2 + C * A3));
        // if (LIST >= 4)
        //    gio::write(Unit21, Format_901) << " Dmp:" << i << AFECTL(i) << DisSysCompDamperData(CompNum).FlowMin
        //                                   << DisSysCompDamperData(CompNum).FlowMax << C;
        if (LFLAG || std::abs(PDROP) <= LTP) {
            //                              Laminar flow.
            if (PDROP >= 0.0) {
                DF[0] = C * LamFlow * propN.density / propN.viscosity;
            } else {
                DF[0] = C * LamFlow * propM.density / propM.viscosity;
            }
            F[0] = DF[0] * PDROP;
        } else {
            //                              Turbulent flow.
            if (PDROP >= 0.0) {
                F[0] = C * TurFlow * propN.sqrt_density * std::pow(PDROP, FlowExpo);
            } else {
                F[0] = -C * TurFlow * propM.sqrt_density * std::pow(-PDROP, FlowExpo);
            }
            DF[0] = F[0] * FlowExpo / PDROP;
        }
        return 1;
    }

    int Damper::calculate([[maybe_unused]] EnergyPlusData &state,
                          const Real64 PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                          [[maybe_unused]] const Real64 multiplier, // Element multiplier
                          const Real64 control,                     // Element control signal
                          const AirState &propN,                    // Node 1 properties
                          const AirState &propM,                    // Node 2 properties
                          std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                          std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a Controlled power law resistance airflow component (damper)

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 C;

        // Formats
        // static gio::Fmt Format_901("(A5,I3,6X,4E16.7)");

        C = control;
        if (C < FlowMin) C = FlowMin;
        if (C > FlowMax) C = FlowMax;
        C = A0 + C * (A1 + C * (A2 + C * A3));
        // if (LIST >= 4)
        //    gio::write(Unit21, Format_901) << " Dmp:" << i << AFECTL(i) << DisSysCompDamperData(CompNum).FlowMin
        //                                   << DisSysCompDamperData(CompNum).FlowMax << C;
        if (std::abs(PDROP) <= LTP) {
            //                              Laminar flow.
            if (PDROP >= 0.0) {
                DF[0] = C * LamFlow * propN.density / propN.viscosity;
            } else {
                DF[0] = C * LamFlow * propM.density / propM.viscosity;
            }
            F[0] = DF[0] * PDROP;
        } else {
            //                              Turbulent flow.
            if (PDROP >= 0.0) {
                F[0] = C * TurFlow * propN.sqrt_density * std::pow(PDROP, FlowExpo);
            } else {
                F[0] = -C * TurFlow * propM.sqrt_density * std::pow(-PDROP, FlowExpo);
            }
            DF[0] = F[0] * FlowExpo / PDROP;
        }
        return 1;
    }

    int EffectiveLeakageRatio::calculate([[maybe_unused]] EnergyPlusData &state,
                                         bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                                         Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                                         [[maybe_unused]] int const i,             // Linkage number
                                         [[maybe_unused]] const Real64 multiplier, // Element multiplier
                                         [[maybe_unused]] const Real64 control,    // Element control signal
                                         const AirState &propN,                    // Node 1 properties
                                         const AirState &propM,                    // Node 2 properties
                                         std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                                         std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a Effective leakage ratio component

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CDM;
        Real64 FL;
        Real64 FT;
        Real64 FlowCoef;

        // Formats
        // static gio::Fmt Format_901("(A5,I3,6X,4E16.7)");

        // Get component properties
        FlowCoef = ELR * FlowRate / propN.density * std::pow(RefPres, -FlowExpo);

        if (LFLAG) {
            // Initialization by linear relation.
            if (PDROP >= 0.0) {
                DF[0] = FlowCoef * propN.density / propN.viscosity;
            } else {
                DF[0] = FlowCoef * propM.density / propM.viscosity;
            }
            F[0] = -DF[0] * PDROP;
        } else {
            // Standard calculation.
            if (PDROP >= 0.0) {
                // Flow in positive direction.
                // Laminar flow.
                CDM = FlowCoef * propN.density / propN.viscosity;
                FL = CDM * PDROP;
                // Turbulent flow.
                if (FlowExpo == 0.5) {
                    FT = FlowCoef * propN.sqrt_density * std::sqrt(PDROP);
                } else {
                    FT = FlowCoef * propN.sqrt_density * std::pow(PDROP, FlowExpo);
                }
            } else {
                // Flow in negative direction.
                // Laminar flow.
                CDM = FlowCoef * propM.density / propM.viscosity;
                FL = CDM * PDROP;
                // Turbulent flow.
                if (FlowExpo == 0.5) {
                    FT = -FlowCoef * propM.sqrt_density * std::sqrt(-PDROP);
                } else {
                    FT = -FlowCoef * propM.sqrt_density * std::pow(-PDROP, FlowExpo);
                }
            }
            // Select laminar or turbulent flow.
            // if (LIST >= 4) gio::write(Unit21, Format_901) << " plr: " << i << PDROP << FL << FT;
            if (std::abs(FL) <= std::abs(FT)) {
                F[0] = FL;
                DF[0] = CDM;
            } else {
                F[0] = FT;
                DF[0] = FT * FlowExpo / PDROP;
            }
        }
        return 1;
    }

    int EffectiveLeakageRatio::calculate([[maybe_unused]] EnergyPlusData &state,
                                         Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                                         [[maybe_unused]] const Real64 multiplier, // Element multiplier
                                         [[maybe_unused]] const Real64 control,    // Element control signal
                                         const AirState &propN,                    // Node 1 properties
                                         const AirState &propM,                    // Node 2 properties
                                         std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                                         std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a Effective leakage ratio component

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CDM;
        Real64 FL;
        Real64 FT;
        Real64 FlowCoef;

        // Formats
        // static gio::Fmt Format_901("(A5,I3,6X,4E16.7)");

        // Get component properties
        FlowCoef = ELR * FlowRate / propN.density * std::pow(RefPres, -FlowExpo);

        // Standard calculation.
        if (PDROP >= 0.0) {
            // Flow in positive direction.
            // Laminar flow.
            CDM = FlowCoef * propN.density / propN.viscosity;
            FL = CDM * PDROP;
            // Turbulent flow.
            if (FlowExpo == 0.5) {
                FT = FlowCoef * propN.sqrt_density * std::sqrt(PDROP);
            } else {
                FT = FlowCoef * propN.sqrt_density * std::pow(PDROP, FlowExpo);
            }
        } else {
            // Flow in negative direction.
            // Laminar flow.
            CDM = FlowCoef * propM.density / propM.viscosity;
            FL = CDM * PDROP;
            // Turbulent flow.
            if (FlowExpo == 0.5) {
                FT = -FlowCoef * propM.sqrt_density * std::sqrt(-PDROP);
            } else {
                FT = -FlowCoef * propM.sqrt_density * std::pow(-PDROP, FlowExpo);
            }
        }
        // Select laminar or turbulent flow.
        // if (LIST >= 4) gio::write(Unit21, Format_901) << " plr: " << i << PDROP << FL << FT;
        if (std::abs(FL) <= std::abs(FT)) {
            F[0] = FL;
            DF[0] = CDM;
        } else {
            F[0] = FT;
            DF[0] = FT * FlowExpo / PDROP;
        }
        return 1;
    }

    int DetailedOpening::calculate(EnergyPlusData &state,
                                   [[maybe_unused]] bool const LFLAG,        // Initialization flag.If = 1, use laminar relationship
                                   Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                                   int const IL,                             // Linkage number
                                   [[maybe_unused]] const Real64 multiplier, // Element multiplier
                                   [[maybe_unused]] const Real64 control,    // Element control signal
                                   [[maybe_unused]] const AirState &propN,   // Node 1 properties
                                   [[maybe_unused]] const AirState &propM,   // Node 2 properties
                                   std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                                   std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Oct. 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  This subroutine is revised based on a vertical large opening subroutine from COMIS

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simulates airflow and pressure of a detailed large opening component.

        // METHODOLOGY EMPLOYED:
        // Purpose:  This routine calculates the massflow and its derivative
        //       through a large opening in both flow directions. As input
        //       the density profiles RhoProfF/T are required aswell as the
        //       effective pressure difference profile DpProfNew, which is the
        //       sum of the stack pressure difference profile DpProf and the
        //       difference of the actual pressures at reference height. The
        //       profiles are calculated in the routine PresProfile.
        //       The massflow and its derivative are calculated for each
        //       interval representing a step of the pressure difference
        //       profile. The total flow and derivative are obtained by
        //       summation over the whole opening.
        //       The calculation is split into different cases representing
        //       different situations of the opening:
        //       - closed opening (opening factor = 0): summation of top and
        //         bottom crack (crack length = lwmax) plus "integration" over
        //         a vertically distributed crack of length (2*lhmax+lextra).
        //       - type 1: normal rectangular opening: "integration" over NrInt
        //         openings with width actlw and height actlh/NrInt
        //       - type 2: horizontally pivoted window: flow direction assumed
        //         strictly perpendicular to the plane of the opening
        //         -> "integration" over normal rectangular openings at top
        //         and bottom of LO plus a rectangular opening in series with two
        //         triangular openings in the middle of the LO (most general
        //         situation). The geometry is defined by the input parameters
        //         actlw(=lwmax), actlh, axisheight, opening angle.
        //       Assuming the massflow perpendicular to the opening plane in all
        //       cases the ownheightfactor has no influence on the massflow.

        // REFERENCES:
        // Helmut E. Feustel and Alison Rayner-Hooson, "COMIS Fundamentals," LBL-28560,
        // Lawrence Berkeley National Laboratory, Berkeley, CA, May 1990

        // USE STATEMENTS:
        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr RealMin(1e-37);
        static Real64 const sqrt_1_2(std::sqrt(1.2));

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        static Real64 const sqrt_2(std::sqrt(2.0));

        Real64 Width;
        Real64 Height;

        Real64 fma12;                         // massflow in direction "from-to" [kg/s]
        Real64 fma21;                         // massflow in direction "to-from" [kg/s]
        Real64 dp1fma12;                      // derivative d fma12 / d Dp [kg/s/Pa]
        Real64 dp1fma21;                      // derivative d fma21 / d Dp [kg/s/Pa]
        Array1D<Real64> DpProfNew(NrInt + 2); // Differential pressure profile for Large Openings, taking into account fixed
        // pressures and actual zone pressures at reference height
        Real64 Fact;   // Actual opening factor
        Real64 DifLim; // Limit for the pressure difference where laminarization takes place [Pa]
        Real64 Cfact;
        Real64 FvVeloc;

        Real64 ActLh;
        Real64 ActLw;
        Real64 Lextra;
        Real64 Axishght;
        Real64 ActCD;
        Real64 Cs;
        Real64 expn;
        Real64 Type;
        Real64 Interval;
        Real64 fmasum;
        Real64 dfmasum;
        Real64 Prefact;
        Array1D<Real64> EvalHghts(NrInt + 2);
        Real64 h2;
        Real64 h4;
        Real64 alpha;
        Real64 rholink;
        Real64 c1;
        Real64 c2;
        Real64 DpZeroOffset;
        Real64 area;
        Real64 WFact;
        Real64 HFact;
        int i;
        int Loc;
        int iNum;

        // Get component properties
        DifLim = 1.0e-4;
        Width = state.afn->MultizoneSurfaceData(IL).width;
        Height = state.afn->MultizoneSurfaceData(IL).height;
        Fact = state.afn->MultizoneSurfaceData(IL).OpenFactor;
        Loc = (state.afn->AirflowNetworkLinkageData(IL).DetOpenNum - 1) * (NrInt + 2);
        iNum = NumFac;
        ActCD = 0.0;

        if (iNum == 2) {
            if (Fact <= OpenFac2) {
                WFact = WidthFac1 + (Fact - OpenFac1) / (OpenFac2 - OpenFac1) * (WidthFac2 - WidthFac1);
                HFact = HeightFac1 + (Fact - OpenFac1) / (OpenFac2 - OpenFac1) * (HeightFac2 - HeightFac1);
                Cfact = DischCoeff1 + (Fact - OpenFac1) / (OpenFac2 - OpenFac1) * (DischCoeff2 - DischCoeff1);
            } else {
                ShowFatalError(
                    state,
                    "Open Factor is above the maximum input range for opening factors in AirflowNetwork:MultiZone:Component:DetailedOpening = " +
                        name);
            }
        }

        if (iNum == 3) {
            if (Fact <= OpenFac2) {
                WFact = WidthFac1 + (Fact - OpenFac1) / (OpenFac2 - OpenFac1) * (WidthFac2 - WidthFac1);
                HFact = HeightFac1 + (Fact - OpenFac1) / (OpenFac2 - OpenFac1) * (HeightFac2 - HeightFac1);
                Cfact = DischCoeff1 + (Fact - OpenFac1) / (OpenFac2 - OpenFac1) * (DischCoeff2 - DischCoeff1);
            } else if (Fact <= OpenFac3) {
                WFact = WidthFac2 + (Fact - OpenFac2) / (OpenFac3 - OpenFac2) * (WidthFac3 - WidthFac2);
                HFact = HeightFac2 + (Fact - OpenFac2) / (OpenFac3 - OpenFac2) * (HeightFac3 - HeightFac2);
                Cfact = DischCoeff2 + (Fact - OpenFac2) / (OpenFac3 - OpenFac2) * (DischCoeff3 - DischCoeff2);
            } else {
                ShowFatalError(
                    state,
                    "Open Factor is above the maximum input range for opening factors in AirflowNetwork:MultiZone:Component:DetailedOpening = " +
                        name);
            }
        }

        if (iNum == 4) {
            if (Fact <= OpenFac2) {
                WFact = WidthFac1 + (Fact - OpenFac1) / (OpenFac2 - OpenFac1) * (WidthFac2 - WidthFac1);
                HFact = HeightFac1 + (Fact - OpenFac1) / (OpenFac2 - OpenFac1) * (HeightFac2 - HeightFac1);
                Cfact = DischCoeff1 + (Fact - OpenFac1) / (OpenFac2 - OpenFac1) * (DischCoeff2 - DischCoeff1);
            } else if (Fact <= OpenFac3) {
                WFact = WidthFac2 + (Fact - OpenFac2) / (OpenFac3 - OpenFac2) * (WidthFac3 - WidthFac2);
                HFact = HeightFac2 + (Fact - OpenFac2) / (OpenFac3 - OpenFac2) * (HeightFac3 - HeightFac2);
                Cfact = DischCoeff2 + (Fact - OpenFac2) / (OpenFac3 - OpenFac2) * (DischCoeff3 - DischCoeff2);
            } else if (Fact <= OpenFac4) {
                WFact = WidthFac3 + (Fact - OpenFac3) / (OpenFac4 - OpenFac3) * (WidthFac4 - WidthFac3);
                HFact = HeightFac3 + (Fact - OpenFac3) / (OpenFac4 - OpenFac3) * (HeightFac4 - HeightFac3);
                Cfact = DischCoeff3 + (Fact - OpenFac3) / (OpenFac4 - OpenFac3) * (DischCoeff4 - DischCoeff3);
            } else {
                ShowFatalError(
                    state,
                    "Open Factor is above the maximum input range for opening factors in AirflowNetwork:MultiZone:Component:DetailedOpening = " +
                        name);
            }
        }

        // calculate DpProfNew
        for (i = 1; i <= NrInt + 2; ++i) {
            DpProfNew(i) = PDROP + state.afn->dos.DpProf(Loc + i) - state.afn->dos.DpL(IL, 1);
        }

        // Get opening data based on the opening factor
        if (Fact == 0) {
            ActLw = state.afn->MultizoneSurfaceData(IL).width;
            ActLh = state.afn->MultizoneSurfaceData(IL).height;
            Cfact = 0.0;
        } else {
            ActLw = state.afn->MultizoneSurfaceData(IL).width * WFact;
            ActLh = state.afn->MultizoneSurfaceData(IL).height * HFact;
            ActCD = Cfact;
        }

        Cs = FlowCoef;
        expn = FlowExpo;
        Type = LVOType;
        if (Type == 1) {
            Lextra = LVOValue;
            Axishght = 0.0;
        } else if (Type == 2) {
            Lextra = 0.0;
            Axishght = LVOValue;
            ActLw = state.afn->MultizoneSurfaceData(IL).width;
            ActLh = state.afn->MultizoneSurfaceData(IL).height;
        }

        // Add window multiplier with window close
        if (state.afn->MultizoneSurfaceData(IL).Multiplier > 1.0) Cs *= state.afn->MultizoneSurfaceData(IL).Multiplier;
        // Add window multiplier with window open
        if (Fact > 0.0) {
            if (state.afn->MultizoneSurfaceData(IL).Multiplier > 1.0) ActLw *= state.afn->MultizoneSurfaceData(IL).Multiplier;
            // Add recurring warnings
            if (ActLw == 0.0) {
                ++WidthErrCount;
                if (WidthErrCount < 2) {
                    ShowWarningError(state, "The actual width of the AirflowNetwork:MultiZone:Component:DetailedOpening of " + name + " is 0.");
                    ShowContinueError(state, "The actual width is set to 1.0E-6 m.");
                    ShowContinueErrorTimeStamp(state, "Occurrence info:");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "The actual width of the AirflowNetwork:MultiZone:Component:DetailedOpening of " + name +
                                                       " is 0 error continues.",
                                                   WidthErrIndex,
                                                   ActLw,
                                                   ActLw);
                }
                ActLw = 1.0e-6;
            }
            if (ActLh == 0.0) {
                ++HeightErrCount;
                if (HeightErrCount < 2) {
                    ShowWarningError(state, "The actual height of the AirflowNetwork:MultiZone:Component:DetailedOpening of " + name + " is 0.");
                    ShowContinueError(state, "The actual height is set to 1.0E-6 m.");
                    ShowContinueErrorTimeStamp(state, "Occurrence info:");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "The actual width of the AirflowNetwork:MultiZone:Component:DetailedOpening of " + name +
                                                       " is 0 error continues.",
                                                   HeightErrIndex,
                                                   ActLh,
                                                   ActLh);
                }
                ActLh = 1.0e-6;
            }
        }
        // Initialization:
        int NF(1);
        Interval = ActLh / NrInt;
        fma12 = 0.0;
        fma21 = 0.0;
        dp1fma12 = 0.0;
        dp1fma21 = 0.0;

        // Closed LO
        if (Cfact == 0) {
            DpZeroOffset = DifLim;
            // bottom crack
            if (DpProfNew(1) > 0) {
                if (std::abs(DpProfNew(1)) <= DpZeroOffset) {
                    dfmasum = Cs * ActLw * std::pow(DpZeroOffset, expn) / DpZeroOffset;
                    fmasum = DpProfNew(1) * dfmasum;
                } else {
                    fmasum = Cs * ActLw * std::pow(DpProfNew(1), expn);
                    dfmasum = fmasum * expn / DpProfNew(1);
                }
                fma12 += fmasum;
                dp1fma12 += dfmasum;
            } else {
                if (std::abs(DpProfNew(1)) <= DpZeroOffset) {
                    dfmasum = -Cs * ActLw * std::pow(DpZeroOffset, expn) / DpZeroOffset;
                    fmasum = DpProfNew(1) * dfmasum;
                } else {
                    fmasum = Cs * ActLw * std::pow(-DpProfNew(1), expn);
                    dfmasum = fmasum * expn / DpProfNew(1);
                }
                fma21 += fmasum;
                dp1fma21 += dfmasum;
            }
            // top crack
            if (DpProfNew(NrInt + 2) > 0) {
                if (std::abs(DpProfNew(NrInt + 2)) <= DpZeroOffset) {
                    dfmasum = Cs * ActLw * std::pow(DpZeroOffset, expn) / DpZeroOffset;
                    fmasum = DpProfNew(NrInt + 2) * dfmasum;
                } else {
                    fmasum = Cs * ActLw * std::pow(DpProfNew(NrInt + 2), expn);
                    dfmasum = fmasum * expn / DpProfNew(NrInt + 2);
                }
                fma12 += fmasum;
                dp1fma12 += dfmasum;
            } else {
                if (std::abs(DpProfNew(NrInt + 2)) <= DpZeroOffset) {
                    dfmasum = -Cs * ActLw * std::pow(DpZeroOffset, expn) / DpZeroOffset;
                    fmasum = DpProfNew(NrInt + 2) * dfmasum;
                } else {
                    fmasum = Cs * ActLw * std::pow(-DpProfNew(NrInt + 2), expn);
                    dfmasum = fmasum * expn / DpProfNew(NrInt + 2);
                }
                fma21 += fmasum;
                dp1fma21 += dfmasum;
            }
            // side and extra cracks
            Prefact = Interval * (2 + Lextra / ActLh) * Cs;
            for (i = 2; i <= NrInt + 1; ++i) {
                if (DpProfNew(i) > 0) {
                    if (std::abs(DpProfNew(i)) <= DpZeroOffset) {
                        dfmasum = Prefact * std::pow(DpZeroOffset, expn) / DpZeroOffset;
                        fmasum = DpProfNew(i) * dfmasum;
                    } else {
                        fmasum = Prefact * std::pow(DpProfNew(i), expn);
                        dfmasum = fmasum * expn / DpProfNew(i);
                    }
                    fma12 += fmasum;
                    dp1fma12 += dfmasum;
                } else {
                    if (std::abs(DpProfNew(i)) <= DpZeroOffset) {
                        dfmasum = -Prefact * std::pow(DpZeroOffset, expn) / DpZeroOffset;
                        fmasum = DpProfNew(i) * dfmasum;
                    } else {
                        fmasum = Prefact * std::pow(-DpProfNew(i), expn);
                        dfmasum = fmasum * expn / DpProfNew(i);
                    }
                    fma21 += fmasum;
                    dp1fma21 += dfmasum;
                }
            }
        }

        // Open LO, type 1
        if ((Cfact != 0) && (Type == 1)) {
            DpZeroOffset = DifLim * 1e-3;
            Prefact = ActLw * ActCD * Interval * sqrt_2;
            for (i = 2; i <= NrInt + 1; ++i) {
                if (DpProfNew(i) > 0) {
                    if (std::abs(DpProfNew(i)) <= DpZeroOffset) {
                        dfmasum = std::sqrt(state.afn->dos.RhoProfF(Loc + i) * DpZeroOffset) / DpZeroOffset;
                        fmasum = DpProfNew(i) * dfmasum;
                    } else {
                        fmasum = std::sqrt(state.afn->dos.RhoProfF(Loc + i) * DpProfNew(i));
                        dfmasum = 0.5 * fmasum / DpProfNew(i);
                    }
                    fma12 += fmasum;
                    dp1fma12 += dfmasum;
                } else {
                    if (std::abs(DpProfNew(i)) <= DpZeroOffset) {
                        dfmasum = -std::sqrt(state.afn->dos.RhoProfT(Loc + i) * DpZeroOffset) / DpZeroOffset;
                        fmasum = DpProfNew(i) * dfmasum;
                    } else {
                        fmasum = std::sqrt(-state.afn->dos.RhoProfT(Loc + i) * DpProfNew(i));
                        dfmasum = 0.5 * fmasum / DpProfNew(i);
                    }
                    fma21 += fmasum;
                    dp1fma21 += dfmasum;
                }
            }

            fma12 *= Prefact;
            fma21 *= Prefact;
            dp1fma12 *= Prefact;
            dp1fma21 *= Prefact;
        }

        // Open LO, type 2
        if ((Cfact != 0) && (Type == 2)) {
            // Initialization
            DpZeroOffset = DifLim * 1e-3;
            // New definition for opening factors for LVO type 2: opening angle = 90 degrees --> opening factor = 1.0
            // should be PIOvr2 in below?
            alpha = Fact * Constant::PiOvr2;
            Real64 const cos_alpha(std::cos(alpha));
            Real64 const tan_alpha(std::tan(alpha));
            h2 = Axishght * (1.0 - cos_alpha);
            h4 = Axishght + (ActLh - Axishght) * cos_alpha;
            EvalHghts(1) = 0.0;
            EvalHghts(NrInt + 2) = ActLh;
            // New definition for opening factors for LVO type 2: pening angle = 90 degrees --> opening factor = 1.0
            if (Fact == 1.0) {
                h2 = Axishght;
                h4 = Axishght;
            }

            for (i = 2; i <= NrInt + 1; ++i) {
                EvalHghts(i) = Interval * (i - 1.5);
            }

            // Calculation of massflow and its derivative
            for (i = 2; i <= NrInt + 1; ++i) {
                if (DpProfNew(i) > 0) {
                    rholink = state.afn->dos.RhoProfF(Loc + i);
                } else {
                    rholink = state.afn->dos.RhoProfT(Loc + i);
                }

                if ((EvalHghts(i) <= h2) || (EvalHghts(i) >= h4)) {
                    if (std::abs(DpProfNew(i)) <= DpZeroOffset) {
                        dfmasum = ActCD * ActLw * Interval * std::sqrt(2.0 * rholink * DpZeroOffset) / DpZeroOffset * sign(1, DpProfNew(i));
                        fmasum = DpProfNew(i) * dfmasum;
                    } else {
                        fmasum = ActCD * ActLw * Interval * std::sqrt(2.0 * rholink * std::abs(DpProfNew(i)));
                        dfmasum = 0.5 * fmasum / DpProfNew(i);
                    }
                } else {
                    // triangular opening at the side of LO
                    c1 = ActCD * ActLw * Interval * std::sqrt(2.0 * rholink);
                    c2 = 2 * ActCD * std::abs(Axishght - EvalHghts(i)) * tan_alpha * Interval * std::sqrt(2.0 * rholink);
                    if ((c1 != 0) && (c2 != 0)) {
                        if (std::abs(DpProfNew(i)) <= DpZeroOffset) {
                            dfmasum = std::sqrt(DpZeroOffset / (1 / c1 / c1 + 1 / c2 / c2)) / DpZeroOffset * sign(1, DpProfNew(i));
                            fmasum = DpProfNew(i) * dfmasum;
                        } else {
                            fmasum = std::sqrt(std::abs(DpProfNew(i)) / (1 / c1 / c1 + 1 / c2 / c2));
                            dfmasum = 0.5 * fmasum / DpProfNew(i);
                        }
                    } else {
                        fmasum = 0.0;
                        dfmasum = 0.0;
                    }
                }

                if (DpProfNew(i) > 0) {
                    fma12 += fmasum;
                    dp1fma12 += dfmasum;
                } else {
                    fma21 += fmasum;
                    dp1fma21 += dfmasum;
                }
            }
        }

        // Calculate some velocity in the large opening
        area = ActLh * ActLw * ActCD;
        if (area > (Cs + RealMin)) {
            if (area > RealMin) {
                FvVeloc = (fma21 + fma12) / area;
            } else {
                FvVeloc = 0.0;
            }
        } else {
            // here the average velocity over the full area, may blow half in half out.
            // velocity= Fva/Nett area=Fma/Rho/(Cm/( (2**N)* SQRT(1.2) ) )
            if (Cs > 0.0) {
                // get the average Rho for this closed window
                for (i = 2; i <= NrInt + 1; ++i) {
                    // rholink = 0.0;
                    // if (DpProfNew(i) > 0) {
                    //    rholink = state.afn->dos.RhoProfF(Loc + i);
                    //} else {
                    //    rholink = state.afn->dos.RhoProfT(Loc + i);
                    //}
                    // rholink /= NrInt;
                    rholink = 1.2;
                }
                FvVeloc = (fma21 + fma12) * std::pow(2.0, expn) * sqrt_1_2 / (rholink * Cs);
            } else {
                FvVeloc = 0.0;
            }
        }

        // Output mass flow rates and associated derivatives
        F[0] = fma12 - fma21;
        DF[0] = dp1fma12 - dp1fma21;
        F[1] = 0.0;
        if (fma12 != 0.0 && fma21 != 0.0) {
            F[1] = fma21;
        }
        DF[1] = 0.0;
        return NF;
    }

    int SimpleOpening::calculate(EnergyPlusData &state,
                                 bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                                 Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                                 int const i,                              // Linkage number
                                 [[maybe_unused]] const Real64 multiplier, // Element multiplier
                                 [[maybe_unused]] const Real64 control,    // Element control signal
                                 const AirState &propN,                    // Node 1 properties
                                 const AirState &propM,                    // Node 2 properties
                                 std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                                 std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a Doorway airflow component using standard interface.
        // A doorway may have two-way airflows. Heights measured relative to the bottom of the door.

        // SUBROUTINE PARAMETER DEFINITIONS:
        // Replace this with the value from numbers when we have C++20
        Real64 constexpr SQRT2(1.414213562373095);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 DPMID; // pressure drop at mid-height of doorway.
        Real64 C;
        Real64 DF0;  // derivative factor at the bottom of the door.
        Real64 DFH;  // derivative factor at the top of the door.
        Real64 DRHO; // difference in air densities between rooms.
        Real64 GDRHO;
        Real64 F0; // flow factor at the bottom of the door.
        Real64 FH; // flow factor at the top of the door.
        Real64 Y;  // height of neutral plane rel. to bottom of door (m).
        Real64 coeff;
        Real64 Width;
        Real64 Height;
        Real64 OpenFactor;

        int NF(1);

        // Formats
        // static gio::Fmt Format_900("(A5,9X,4E16.7)");
        // static gio::Fmt Format_903("(A5,3I3,4E16.7)");

        Width = state.afn->MultizoneSurfaceData(i).width;
        Height = state.afn->MultizoneSurfaceData(i).height;
        coeff = FlowCoef * 2.0 * (Width + Height);
        OpenFactor = state.afn->MultizoneSurfaceData(i).OpenFactor;
        if (OpenFactor > 0.0) {
            Width *= OpenFactor;
            if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).surface_number).Tilt < 90.0) {
                Height *= state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).surface_number).SinTilt;
            }
        }

        if (PDROP >= 0.0) {
            coeff /= propN.sqrt_density;
        } else {
            coeff /= propM.sqrt_density;
        }

        // Add window multiplier with window close
        if (state.afn->MultizoneSurfaceData(i).Multiplier > 1.0) coeff *= state.afn->MultizoneSurfaceData(i).Multiplier;
        // Add window multiplier with window open
        if (OpenFactor > 0.0) {
            if (state.afn->MultizoneSurfaceData(i).Multiplier > 1.0) Width *= state.afn->MultizoneSurfaceData(i).Multiplier;
        }

        DRHO = propN.density - propM.density;
        GDRHO = 9.8 * DRHO;
        // if (LIST >= 4) gio::write(Unit21, Format_903) << " DOR:" << i << n << m << PDROP << std::abs(DRHO) << MinRhoDiff;
        if (OpenFactor == 0.0) {
            generic_crack(coeff, FlowExpo, LFLAG, PDROP, propN, propM, F, DF);
            return 1;
        }
        if (std::abs(DRHO) < MinRhoDiff || LFLAG) {
            DPMID = PDROP - 0.5 * Height * GDRHO;
            // Initialization or identical temps: treat as one-way flow.
            NF = 1;
            generic_crack(coeff, FlowExpo, LFLAG, DPMID, propN, propM, F, DF);
            // if (LIST >= 4) gio::write(Unit21, Format_900) << " Drs:" << DPMID << F[0] << DF[0];
        } else {
            // Possible two-way flow:
            Y = PDROP / GDRHO;
            // if (LIST >= 4) gio::write(Unit21, Format_900) << " DrY:" << PDROP << GDRHO << Y;
            // F0 = lower flow, FH = upper flow.
            C = SQRT2 * Width * DischCoeff;
            DF0 = C * std::sqrt(std::abs(PDROP)) / std::abs(GDRHO);
            //        F0 = 0.666667d0*C*SQRT(ABS(GDRHO*Y))*ABS(Y)
            F0 = (2.0 / 3.0) * C * std::sqrt(std::abs(GDRHO * Y)) * std::abs(Y);
            DFH = C * std::sqrt(std::abs((Height - Y) / GDRHO));
            //        FH = 0.666667d0*DFH*ABS(GDRHO*(Height-Y))
            FH = (2.0 / 3.0) * DFH * std::abs(GDRHO * (Height - Y));
            // if (LIST >= 4) gio::write(Unit21, Format_900) << " DrF:" << F0 << DF0 << FH << DFH;
            if (Y <= 0.0) {
                // One-way flow (negative).
                if (DRHO >= 0.0) {
                    F[0] = -propM.sqrt_density * std::abs(FH - F0);
                    DF[0] = propM.sqrt_density * std::abs(DFH - DF0);
                } else {
                    F[0] = propN.sqrt_density * std::abs(FH - F0);
                    DF[0] = propN.sqrt_density * std::abs(DFH - DF0);
                }
                // if (LIST >= 4) gio::write(Unit21, Format_900) << " Dr1:" << C << F[0] << DF[0];
            } else if (Y >= Height) {
                // One-way flow (positive).
                if (DRHO >= 0.0) {
                    F[0] = propN.sqrt_density * std::abs(FH - F0);
                    DF[0] = propN.sqrt_density * std::abs(DFH - DF0);
                } else {
                    F[0] = -propM.sqrt_density * std::abs(FH - F0);
                    DF[0] = propM.sqrt_density * std::abs(DFH - DF0);
                }
                // if (LIST >= 4) gio::write(Unit21, Format_900) << " Dr2:" << C << F[0] << DF[0];
            } else {
                // Two-way flow.
                NF = 2;
                if (DRHO >= 0.0) {
                    F[0] = -propM.sqrt_density * FH;
                    DF[0] = propM.sqrt_density * DFH;
                    F[1] = propN.sqrt_density * F0;
                    DF[1] = propN.sqrt_density * DF0;
                } else {
                    F[0] = propN.sqrt_density * FH;
                    DF[0] = propN.sqrt_density * DFH;
                    F[1] = -propM.sqrt_density * F0;
                    DF[1] = propM.sqrt_density * DF0;
                }
                // if (LIST >= 4) gio::write(Unit21, Format_900) << " Dr3:" << C << F[0] << DF[0];
                // if (LIST >= 4) gio::write(Unit21, Format_900) << " Dr4:" << C << F[1] << DF[1];
            }
        }
        return NF;
    }

    int ConstantPressureDrop::calculate([[maybe_unused]] EnergyPlusData &state,
                                        [[maybe_unused]] bool const LFLAG,        // Initialization flag.If = 1, use laminar relationship
                                        const Real64 PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                                        int const i,                              // Linkage number
                                        [[maybe_unused]] const Real64 multiplier, // Element multiplier
                                        [[maybe_unused]] const Real64 control,    // Element control signal
                                        const AirState &propN,                    // Node 1 properties
                                        [[maybe_unused]] const AirState &propM,   // Node 2 properties
                                        std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                                        std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a Constant pressure drop component

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // Real64 Co;

        int n = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
        int m = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];

        // Get component properties
        // A  = Cross section area [m2]
        // DP = Pressure difference across the element [Pa]

        if (PDROP == 0.0) {
            F[0] = std::sqrt(2.0 * propN.density) * A * std::sqrt(DP);
            DF[0] = 0.5 * F[0] / DP;
        } else {
            for (int k = 1; k <= state.afn->ActualNumOfLinks; ++k) {
                if (state.afn->AirflowNetworkLinkageData(k).NodeNums[1] == n) {
                    F[0] = state.afn->AFLOW(k);
                    break;
                }
            }
            state.afn->PZ(m) = state.afn->PZ(n) - DP;
            // Co = F[0] / DP; // can be deleted since it is not used
            DF[0] = 10.e10;
        }
        return 1;
    }

    int EffectiveLeakageArea::calculate([[maybe_unused]] EnergyPlusData &state,
                                        bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                                        Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                                        [[maybe_unused]] int const i,             // Linkage number
                                        [[maybe_unused]] const Real64 multiplier, // Element multiplier
                                        [[maybe_unused]] const Real64 control,    // Element control signal
                                        const AirState &propN,                    // Node 1 properties
                                        const AirState &propM,                    // Node 2 properties
                                        std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                                        std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a Surface effective leakage area component

        // SUBROUTINE PARAMETER DEFINITIONS:
        static Real64 const sqrt_2(std::sqrt(2.0));

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CDM;
        Real64 FL;
        Real64 FT;
        Real64 FlowCoef;

        // Formats
        // static gio::Fmt Format_901("(A5,I3,6X,4E16.7)");

        // Get component properties
        FlowCoef = ELA * DischCoeff * sqrt_2 * std::pow(RefDeltaP, 0.5 - FlowExpo);

        if (LFLAG) {
            // Initialization by linear relation.
            if (PDROP >= 0.0) {
                DF[0] = FlowCoef * propN.density / propN.viscosity;
            } else {
                DF[0] = FlowCoef * propM.density / propM.viscosity;
            }
            F[0] = -DF[0] * PDROP;
        } else {
            // Standard calculation.
            if (PDROP >= 0.0) {
                // Flow in positive direction.
                // Laminar flow.
                CDM = FlowCoef * propN.density / propN.viscosity;
                FL = CDM * PDROP;
                // Turbulent flow.
                if (FlowExpo == 0.5) {
                    FT = FlowCoef * propN.sqrt_density * std::sqrt(PDROP);
                } else {
                    FT = FlowCoef * propN.sqrt_density * std::pow(PDROP, FlowExpo);
                }
            } else {
                // Flow in negative direction.
                // Laminar flow.
                CDM = FlowCoef * propM.density / propM.viscosity;
                FL = CDM * PDROP;
                // Turbulent flow.
                if (FlowExpo == 0.5) {
                    FT = -FlowCoef * propM.sqrt_density * std::sqrt(-PDROP);
                } else {
                    FT = -FlowCoef * propM.sqrt_density * std::pow(-PDROP, FlowExpo);
                }
            }
            // Select laminar or turbulent flow.
            // if (LIST >= 4) gio::write(Unit21, Format_901) << " plr: " << i << PDROP << FL << FT;
            if (std::abs(FL) <= std::abs(FT)) {
                F[0] = FL;
                DF[0] = CDM;
            } else {
                F[0] = FT;
                DF[0] = FT * FlowExpo / PDROP;
            }
        }
        return 1;
    }

    int EffectiveLeakageArea::calculate([[maybe_unused]] EnergyPlusData &state,
                                        Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                                        [[maybe_unused]] const Real64 multiplier, // Element multiplier
                                        [[maybe_unused]] const Real64 control,    // Element control signal
                                        const AirState &propN,                    // Node 1 properties
                                        const AirState &propM,                    // Node 2 properties
                                        std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                                        std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a Surface effective leakage area component

        // SUBROUTINE PARAMETER DEFINITIONS:
        static Real64 const sqrt_2(std::sqrt(2.0));

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CDM;
        Real64 FL;
        Real64 FT;
        Real64 FlowCoef;

        // Formats
        // static gio::Fmt Format_901("(A5,I3,6X,4E16.7)");

        // Get component properties
        FlowCoef = ELA * DischCoeff * sqrt_2 * std::pow(RefDeltaP, 0.5 - FlowExpo);

        // Standard calculation.
        if (PDROP >= 0.0) {
            // Flow in positive direction.
            // Laminar flow.
            CDM = FlowCoef * propN.density / propN.viscosity;
            FL = CDM * PDROP;
            // Turbulent flow.
            if (FlowExpo == 0.5) {
                FT = FlowCoef * propN.sqrt_density * std::sqrt(PDROP);
            } else {
                FT = FlowCoef * propN.sqrt_density * std::pow(PDROP, FlowExpo);
            }
        } else {
            // Flow in negative direction.
            // Laminar flow.
            CDM = FlowCoef * propM.density / propM.viscosity;
            FL = CDM * PDROP;
            // Turbulent flow.
            if (FlowExpo == 0.5) {
                FT = -FlowCoef * propM.sqrt_density * std::sqrt(-PDROP);
            } else {
                FT = -FlowCoef * propM.sqrt_density * std::pow(-PDROP, FlowExpo);
            }
        }
        // Select laminar or turbulent flow.
        // if (LIST >= 4) gio::write(Unit21, Format_901) << " plr: " << i << PDROP << FL << FT;
        if (std::abs(FL) <= std::abs(FT)) {
            F[0] = FL;
            DF[0] = CDM;
        } else {
            F[0] = FT;
            DF[0] = FT * FlowExpo / PDROP;
        }

        return 1;
    }

    int DisSysCompCoilProp::calculate([[maybe_unused]] EnergyPlusData &state,
                                      bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                                      [[maybe_unused]] int const i,             // Linkage number
                                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                                      [[maybe_unused]] const Real64 control,    // Element control signal
                                      const AirState &propN,                    // Node 1 properties
                                      const AirState &propM,                    // Node 2 properties
                                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a coil component

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr C(0.868589);
        Real64 constexpr EPS(0.001);
        Real64 constexpr Rough(0.0001);
        Real64 constexpr InitLamCoef(128.0);
        Real64 constexpr LamDynCoef(64.0);
        Real64 constexpr LamFriCoef(0.0001);
        Real64 constexpr TurDynCoef(0.0001);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 A0;
        Real64 A1;
        Real64 A2;
        Real64 B;
        Real64 D;
        Real64 S2;
        Real64 CDM;
        Real64 FL;
        Real64 FT;
        Real64 FTT;
        Real64 RE;
        Real64 ed;
        Real64 ld;
        Real64 g;
        Real64 AA1;
        Real64 area;

        // Formats
        // static gio::Fmt Format_901("(A5,I3,6X,4E16.7)");

        // Get component properties
        // ed = Rough / DisSysCompCoilData(CompNum).hydraulicDiameter;
        ed = Rough / hydraulicDiameter;

        area = square(hydraulicDiameter) * Constant::Pi;
        ld = L / hydraulicDiameter;
        g = 1.14 - 0.868589 * std::log(ed);
        AA1 = g;

        if (LFLAG) {
            // Initialization by linear relation.
            if (PDROP >= 0.0) {
                DF[0] = (2.0 * propN.density * area * hydraulicDiameter) / (propN.viscosity * InitLamCoef * ld);
            } else {
                DF[0] = (2.0 * propM.density * area * hydraulicDiameter) / (propM.viscosity * InitLamCoef * ld);
            }
            F[0] = -DF[0] * PDROP;
            // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwi:" << i << InitLamCoef << F[0] << DF[0];
        } else {
            // Standard calculation.
            if (PDROP >= 0.0) {
                // Flow in positive direction.
                // Laminar flow coefficient !=0
                if (LamFriCoef >= 0.001) {
                    A2 = LamFriCoef / (2.0 * propN.density * area * area);
                    A1 = (propN.viscosity * LamDynCoef * ld) / (2.0 * propN.density * area * hydraulicDiameter);
                    A0 = -PDROP;
                    CDM = std::sqrt(A1 * A1 - 4.0 * A2 * A0);
                    FL = (CDM - A1) / (2.0 * A2);
                    CDM = 1.0 / CDM;
                } else {
                    CDM = (2.0 * propN.density * area * hydraulicDiameter) / (propN.viscosity * LamDynCoef * ld);
                    FL = CDM * PDROP;
                }
                RE = FL * hydraulicDiameter / (propN.viscosity * area);
                // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwl:" << i << PDROP << FL << CDM << RE;
                // Turbulent flow; test when Re>10.
                if (RE >= 10.0) {
                    S2 = std::sqrt(2.0 * propN.density * PDROP) * area;
                    FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                    // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << S2 << FTT << g;
                    while (true) {
                        FT = FTT;
                        B = (9.3 * propN.viscosity * area) / (FT * Rough);
                        D = 1.0 + g * B;
                        g -= (g - AA1 + C * std::log(D)) / (1.0 + C * B / D);
                        FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                        // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << B << FTT << g;
                        if (std::abs(FTT - FT) / FTT < EPS) break;
                    }
                    FT = FTT;
                } else {
                    FT = FL;
                }
            } else {
                // Flow in negative direction.
                // Laminar flow coefficient !=0
                if (LamFriCoef >= 0.001) {
                    A2 = LamFriCoef / (2.0 * propM.density * area * area);
                    A1 = (propM.viscosity * LamDynCoef * ld) / (2.0 * propM.density * area * hydraulicDiameter);
                    A0 = PDROP;
                    CDM = std::sqrt(A1 * A1 - 4.0 * A2 * A0);
                    FL = -(CDM - A1) / (2.0 * A2);
                    CDM = 1.0 / CDM;
                } else {
                    CDM = (2.0 * propM.density * area * hydraulicDiameter) / (propM.viscosity * LamDynCoef * ld);
                    FL = CDM * PDROP;
                }
                RE = -FL * hydraulicDiameter / (propM.viscosity * area);
                // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwl:" << i << PDROP << FL << CDM << RE;
                // Turbulent flow; test when Re>10.
                if (RE >= 10.0) {
                    S2 = std::sqrt(-2.0 * propM.density * PDROP) * area;
                    FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                    // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << S2 << FTT << g;
                    while (true) {
                        FT = FTT;
                        B = (9.3 * propM.viscosity * area) / (FT * Rough);
                        D = 1.0 + g * B;
                        g -= (g - AA1 + C * std::log(D)) / (1.0 + C * B / D);
                        FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                        // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << B << FTT << g;
                        if (std::abs(FTT - FT) / FTT < EPS) break;
                    }
                    FT = -FTT;
                } else {
                    FT = FL;
                }
            }
            // Select laminar or turbulent flow.
            if (std::abs(FL) <= std::abs(FT)) {
                F[0] = FL;
                DF[0] = CDM;
            } else {
                F[0] = FT;
                DF[0] = 0.5 * FT / PDROP;
            }
        }
        return 1;
    }

    int DisSysCompCoilProp::calculate([[maybe_unused]] EnergyPlusData &state,
                                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                                      [[maybe_unused]] const Real64 control,    // Element control signal
                                      const AirState &propN,                    // Node 1 properties
                                      const AirState &propM,                    // Node 2 properties
                                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a coil component

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr C(0.868589);
        Real64 constexpr EPS(0.001);
        Real64 constexpr Rough(0.0001);
        Real64 constexpr LamDynCoef(64.0);
        Real64 constexpr LamFriCoef(0.0001);
        Real64 constexpr TurDynCoef(0.0001);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 A0;
        Real64 A1;
        Real64 A2;
        Real64 B;
        Real64 D;
        Real64 S2;
        Real64 CDM;
        Real64 FL;
        Real64 FT;
        Real64 FTT;
        Real64 RE;
        Real64 ed;
        Real64 ld;
        Real64 g;
        Real64 AA1;
        Real64 area;

        // Formats
        // static gio::Fmt Format_901("(A5,I3,6X,4E16.7)");

        // Get component properties
        // ed = Rough / DisSysCompCoilData(CompNum).hydraulicDiameter;
        ed = Rough / hydraulicDiameter;

        area = square(hydraulicDiameter) * Constant::Pi;
        ld = L / hydraulicDiameter;
        g = 1.14 - 0.868589 * std::log(ed);
        AA1 = g;

        // Standard calculation.
        if (PDROP >= 0.0) {
            // Flow in positive direction.
            // Laminar flow coefficient !=0
            if (LamFriCoef >= 0.001) {
                A2 = LamFriCoef / (2.0 * propN.density * area * area);
                A1 = (propN.viscosity * LamDynCoef * ld) / (2.0 * propN.density * area * hydraulicDiameter);
                A0 = -PDROP;
                CDM = std::sqrt(A1 * A1 - 4.0 * A2 * A0);
                FL = (CDM - A1) / (2.0 * A2);
                CDM = 1.0 / CDM;
            } else {
                CDM = (2.0 * propN.density * area * hydraulicDiameter) / (propN.viscosity * LamDynCoef * ld);
                FL = CDM * PDROP;
            }
            RE = FL * hydraulicDiameter / (propN.viscosity * area);
            // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwl:" << i << PDROP << FL << CDM << RE;
            // Turbulent flow; test when Re>10.
            if (RE >= 10.0) {
                S2 = std::sqrt(2.0 * propN.density * PDROP) * area;
                FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << S2 << FTT << g;
                while (true) {
                    FT = FTT;
                    B = (9.3 * propN.viscosity * area) / (FT * Rough);
                    D = 1.0 + g * B;
                    g -= (g - AA1 + C * std::log(D)) / (1.0 + C * B / D);
                    FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                    // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << B << FTT << g;
                    if (std::abs(FTT - FT) / FTT < EPS) break;
                }
                FT = FTT;
            } else {
                FT = FL;
            }
        } else {
            // Flow in negative direction.
            // Laminar flow coefficient !=0
            if (LamFriCoef >= 0.001) {
                A2 = LamFriCoef / (2.0 * propM.density * area * area);
                A1 = (propM.viscosity * LamDynCoef * ld) / (2.0 * propM.density * area * hydraulicDiameter);
                A0 = PDROP;
                CDM = std::sqrt(A1 * A1 - 4.0 * A2 * A0);
                FL = -(CDM - A1) / (2.0 * A2);
                CDM = 1.0 / CDM;
            } else {
                CDM = (2.0 * propM.density * area * hydraulicDiameter) / (propM.viscosity * LamDynCoef * ld);
                FL = CDM * PDROP;
            }
            RE = -FL * hydraulicDiameter / (propM.viscosity * area);
            // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwl:" << i << PDROP << FL << CDM << RE;
            // Turbulent flow; test when Re>10.
            if (RE >= 10.0) {
                S2 = std::sqrt(-2.0 * propM.density * PDROP) * area;
                FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << S2 << FTT << g;
                while (true) {
                    FT = FTT;
                    B = (9.3 * propM.viscosity * area) / (FT * Rough);
                    D = 1.0 + g * B;
                    g -= (g - AA1 + C * std::log(D)) / (1.0 + C * B / D);
                    FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                    // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << B << FTT << g;
                    if (std::abs(FTT - FT) / FTT < EPS) break;
                }
                FT = -FTT;
            } else {
                FT = FL;
            }
        }
        // Select laminar or turbulent flow.
        if (std::abs(FL) <= std::abs(FT)) {
            F[0] = FL;
            DF[0] = CDM;
        } else {
            F[0] = FT;
            DF[0] = 0.5 * FT / PDROP;
        }
        return 1;
    }

    int DisSysCompTermUnitProp::calculate([[maybe_unused]] EnergyPlusData &state,
                                          bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                                          Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                                          int const i,                              // Linkage number
                                          [[maybe_unused]] const Real64 multiplier, // Element multiplier
                                          [[maybe_unused]] const Real64 control,    // Element control signal
                                          const AirState &propN,                    // Node 1 properties
                                          const AirState &propM,                    // Node 2 properties
                                          std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                                          std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a terminal unit component

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr C(0.868589);
        Real64 constexpr EPS(0.001);
        Real64 constexpr Rough(0.0001);
        Real64 constexpr InitLamCoef(128.0);
        Real64 constexpr LamDynCoef(64.0);
        Real64 constexpr LamFriCoef(0.0001);
        Real64 constexpr TurDynCoef(0.0001);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 A0;
        Real64 A1;
        Real64 A2;
        Real64 B;
        Real64 D;
        Real64 S2;
        Real64 CDM;
        Real64 FL;
        Real64 FT;
        Real64 FTT;
        Real64 RE;
        Real64 ed;
        Real64 ld;
        Real64 g;
        Real64 AA1;
        Real64 area;

        // Formats
        // static gio::Fmt Format_901("(A5,I3,6X,4E16.7)");

        // Get component properties
        ed = Rough / hydraulicDiameter;
        area = pow_2(hydraulicDiameter) * Constant::Pi;
        ld = L / hydraulicDiameter;
        g = 1.14 - 0.868589 * std::log(ed);
        AA1 = g;

        if (LFLAG) {
            // Initialization by linear relation.
            if (PDROP >= 0.0) {
                DF[0] = (2.0 * propN.density * area * hydraulicDiameter) / (propN.viscosity * InitLamCoef * ld);
            } else {
                DF[0] = (2.0 * propM.density * area * hydraulicDiameter) / (propM.viscosity * InitLamCoef * ld);
            }
            F[0] = -DF[0] * PDROP;
            // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwi:" << i << InitLamCoef << F[0] << DF[0];
        } else {
            // Standard calculation.
            if (PDROP >= 0.0) {
                // Flow in positive direction.
                // Laminar flow coefficient !=0
                if (LamFriCoef >= 0.001) {
                    A2 = LamFriCoef / (2.0 * propN.density * area * area);
                    A1 = (propN.viscosity * LamDynCoef * ld) / (2.0 * propN.density * area * hydraulicDiameter);
                    A0 = -PDROP;
                    CDM = std::sqrt(A1 * A1 - 4.0 * A2 * A0);
                    FL = (CDM - A1) / (2.0 * A2);
                    CDM = 1.0 / CDM;
                } else {
                    CDM = (2.0 * propN.density * area * hydraulicDiameter) / (propN.viscosity * LamDynCoef * ld);
                    FL = CDM * PDROP;
                }
                RE = FL * hydraulicDiameter / (propN.viscosity * area);
                // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwl:" << i << PDROP << FL << CDM << RE;
                // Turbulent flow; test when Re>10.
                if (RE >= 10.0) {
                    S2 = std::sqrt(2.0 * propN.density * PDROP) * area;
                    FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                    // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << S2 << FTT << g;
                    while (true) {
                        FT = FTT;
                        B = (9.3 * propN.viscosity * area) / (FT * Rough);
                        D = 1.0 + g * B;
                        g -= (g - AA1 + C * std::log(D)) / (1.0 + C * B / D);
                        FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                        // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << B << FTT << g;
                        if (std::abs(FTT - FT) / FTT < EPS) break;
                    }
                    FT = FTT;
                } else {
                    FT = FL;
                }
            } else {
                // Flow in negative direction.
                // Laminar flow coefficient !=0
                if (LamFriCoef >= 0.001) {
                    A2 = LamFriCoef / (2.0 * propM.density * area * area);
                    A1 = (propM.viscosity * LamDynCoef * ld) / (2.0 * propM.density * area * hydraulicDiameter);
                    A0 = PDROP;
                    CDM = std::sqrt(A1 * A1 - 4.0 * A2 * A0);
                    FL = -(CDM - A1) / (2.0 * A2);
                    CDM = 1.0 / CDM;
                } else {
                    CDM = (2.0 * propM.density * area * hydraulicDiameter) / (propM.viscosity * LamDynCoef * ld);
                    FL = CDM * PDROP;
                }
                RE = -FL * hydraulicDiameter / (propM.viscosity * area);
                // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwl:" << i << PDROP << FL << CDM << RE;
                // Turbulent flow; test when Re>10.
                if (RE >= 10.0) {
                    S2 = std::sqrt(-2.0 * propM.density * PDROP) * area;
                    FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                    // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << S2 << FTT << g;
                    while (true) {
                        FT = FTT;
                        B = (9.3 * propM.viscosity * area) / (FT * Rough);
                        D = 1.0 + g * B;
                        g -= (g - AA1 + C * std::log(D)) / (1.0 + C * B / D);
                        FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                        // if (LIST >= 4) gio::write(Unit21, Format_901) << " dwt:" << i << B << FTT << g;
                        if (std::abs(FTT - FT) / FTT < EPS) break;
                    }
                    FT = -FTT;
                } else {
                    FT = FL;
                }
            }
            // Select laminar or turbulent flow.
            if (std::abs(FL) <= std::abs(FT)) {
                F[0] = FL;
                DF[0] = CDM;
            } else {
                F[0] = FT;
                DF[0] = 0.5 * FT / PDROP;
            }
        }
        // If damper, setup the airflows from nodal values calculated from terminal
        if (state.afn->AirflowNetworkLinkageData(i).VAVTermDamper) {
            F[0] = state.dataLoopNodes->Node(DamperInletNode).MassFlowRate;
            if (state.afn->VAVTerminalRatio > 0.0) {
                F[0] *= state.afn->VAVTerminalRatio;
            }
            DF[0] = 0.0;
        }
        return 1;
    }

    int DisSysCompHXProp::calculate([[maybe_unused]] EnergyPlusData &state,
                                    bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                                    Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                                    [[maybe_unused]] int const i,             // Linkage number
                                    [[maybe_unused]] const Real64 multiplier, // Element multiplier
                                    [[maybe_unused]] const Real64 control,    // Element control signal
                                    const AirState &propN,                    // Node 1 properties
                                    const AirState &propM,                    // Node 2 properties
                                    std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                                    std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 1/18/09
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a heat exchanger component

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr C(0.868589);
        Real64 constexpr EPS(0.001);
        Real64 constexpr Rough(0.0001);
        Real64 constexpr InitLamCoef(128.0);
        Real64 constexpr LamDynCoef(64.0);
        Real64 constexpr LamFriCoef(0.0001);
        Real64 constexpr TurDynCoef(0.0001);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 A0;
        Real64 A1;
        Real64 A2;
        Real64 B;
        Real64 D;
        Real64 S2;
        Real64 CDM;
        Real64 FL;
        Real64 FT;
        Real64 FTT;
        Real64 RE;
        Real64 ed;
        Real64 ld;
        Real64 g;
        Real64 AA1;
        Real64 area;

        // Get component properties
        ed = Rough / hydraulicDiameter;
        area = pow_2(hydraulicDiameter) * Constant::Pi;
        ld = L / hydraulicDiameter;
        g = 1.14 - 0.868589 * std::log(ed);
        AA1 = g;

        if (LFLAG) {
            // Initialization by linear relation.
            if (PDROP >= 0.0) {
                DF[0] = (2.0 * propN.density * area * hydraulicDiameter) / (propN.viscosity * InitLamCoef * ld);
            } else {
                DF[0] = (2.0 * propM.density * area * hydraulicDiameter) / (propM.viscosity * InitLamCoef * ld);
            }
            F[0] = -DF[0] * PDROP;
        } else {
            // Standard calculation.
            if (PDROP >= 0.0) {
                // Flow in positive direction.
                // Laminar flow coefficient !=0
                if (LamFriCoef >= 0.001) {
                    A2 = LamFriCoef / (2.0 * propN.density * area * area);
                    A1 = (propN.viscosity * LamDynCoef * ld) / (2.0 * propN.density * area * hydraulicDiameter);
                    A0 = -PDROP;
                    CDM = std::sqrt(A1 * A1 - 4.0 * A2 * A0);
                    FL = (CDM - A1) / (2.0 * A2);
                    CDM = 1.0 / CDM;
                } else {
                    CDM = (2.0 * propN.density * area * hydraulicDiameter) / (propN.viscosity * LamDynCoef * ld);
                    FL = CDM * PDROP;
                }
                RE = FL * hydraulicDiameter / (propN.viscosity * area);
                // Turbulent flow; test when Re>10.
                if (RE >= 10.0) {
                    S2 = std::sqrt(2.0 * propN.density * PDROP) * area;
                    FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                    while (true) {
                        FT = FTT;
                        B = (9.3 * propN.viscosity * area) / (FT * Rough);
                        D = 1.0 + g * B;
                        g -= (g - AA1 + C * std::log(D)) / (1.0 + C * B / D);
                        FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                        if (std::abs(FTT - FT) / FTT < EPS) break;
                    }
                    FT = FTT;
                } else {
                    FT = FL;
                }
            } else {
                // Flow in negative direction.
                // Laminar flow coefficient !=0
                if (LamFriCoef >= 0.001) {
                    A2 = LamFriCoef / (2.0 * propM.density * area * area);
                    A1 = (propM.viscosity * LamDynCoef * ld) / (2.0 * propM.density * area * hydraulicDiameter);
                    A0 = PDROP;
                    CDM = std::sqrt(A1 * A1 - 4.0 * A2 * A0);
                    FL = -(CDM - A1) / (2.0 * A2);
                    CDM = 1.0 / CDM;
                } else {
                    CDM = (2.0 * propM.density * area * hydraulicDiameter) / (propM.viscosity * LamDynCoef * ld);
                    FL = CDM * PDROP;
                }
                RE = -FL * hydraulicDiameter / (propM.viscosity * area);
                // Turbulent flow; test when Re>10.
                if (RE >= 10.0) {
                    S2 = std::sqrt(-2.0 * propM.density * PDROP) * area;
                    FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                    while (true) {
                        FT = FTT;
                        B = (9.3 * propM.viscosity * area) / (FT * Rough);
                        D = 1.0 + g * B;
                        g -= (g - AA1 + C * std::log(D)) / (1.0 + C * B / D);
                        FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                        if (std::abs(FTT - FT) / FTT < EPS) break;
                    }
                    FT = -FTT;
                } else {
                    FT = FL;
                }
            }
            // Select laminar or turbulent flow.
            if (std::abs(FL) <= std::abs(FT)) {
                F[0] = FL;
                DF[0] = CDM;
            } else {
                F[0] = FT;
                DF[0] = 0.5 * FT / PDROP;
            }
        }
        return 1;
    }

    int DisSysCompHXProp::calculate([[maybe_unused]] EnergyPlusData &state,
                                    Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                                    [[maybe_unused]] const Real64 multiplier, // Element multiplier
                                    [[maybe_unused]] const Real64 control,    // Element control signal
                                    const AirState &propN,                    // Node 1 properties
                                    const AirState &propM,                    // Node 2 properties
                                    std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                                    std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 1/18/09
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a heat exchanger component

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr C(0.868589);
        Real64 constexpr EPS(0.001);
        Real64 constexpr Rough(0.0001);
        Real64 constexpr LamDynCoef(64.0);
        Real64 constexpr LamFriCoef(0.0001);
        Real64 constexpr TurDynCoef(0.0001);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 A0;
        Real64 A1;
        Real64 A2;
        Real64 B;
        Real64 D;
        Real64 S2;
        Real64 CDM;
        Real64 FL;
        Real64 FT;
        Real64 FTT;
        Real64 RE;
        Real64 ed;
        Real64 ld;
        Real64 g;
        Real64 AA1;
        Real64 area;

        // Get component properties
        ed = Rough / hydraulicDiameter;
        area = pow_2(hydraulicDiameter) * Constant::Pi;
        ld = L / hydraulicDiameter;
        g = 1.14 - 0.868589 * std::log(ed);
        AA1 = g;

        // Standard calculation.
        if (PDROP >= 0.0) {
            // Flow in positive direction.
            // Laminar flow coefficient !=0
            if (LamFriCoef >= 0.001) {
                A2 = LamFriCoef / (2.0 * propN.density * area * area);
                A1 = (propN.viscosity * LamDynCoef * ld) / (2.0 * propN.density * area * hydraulicDiameter);
                A0 = -PDROP;
                CDM = std::sqrt(A1 * A1 - 4.0 * A2 * A0);
                FL = (CDM - A1) / (2.0 * A2);
                CDM = 1.0 / CDM;
            } else {
                CDM = (2.0 * propN.density * area * hydraulicDiameter) / (propN.viscosity * LamDynCoef * ld);
                FL = CDM * PDROP;
            }
            RE = FL * hydraulicDiameter / (propN.viscosity * area);
            // Turbulent flow; test when Re>10.
            if (RE >= 10.0) {
                S2 = std::sqrt(2.0 * propN.density * PDROP) * area;
                FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                while (true) {
                    FT = FTT;
                    B = (9.3 * propN.viscosity * area) / (FT * Rough);
                    D = 1.0 + g * B;
                    g -= (g - AA1 + C * std::log(D)) / (1.0 + C * B / D);
                    FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                    if (std::abs(FTT - FT) / FTT < EPS) break;
                }
                FT = FTT;
            } else {
                FT = FL;
            }
        } else {
            // Flow in negative direction.
            // Laminar flow coefficient !=0
            if (LamFriCoef >= 0.001) {
                A2 = LamFriCoef / (2.0 * propM.density * area * area);
                A1 = (propM.viscosity * LamDynCoef * ld) / (2.0 * propM.density * area * hydraulicDiameter);
                A0 = PDROP;
                CDM = std::sqrt(A1 * A1 - 4.0 * A2 * A0);
                FL = -(CDM - A1) / (2.0 * A2);
                CDM = 1.0 / CDM;
            } else {
                CDM = (2.0 * propM.density * area * hydraulicDiameter) / (propM.viscosity * LamDynCoef * ld);
                FL = CDM * PDROP;
            }
            RE = -FL * hydraulicDiameter / (propM.viscosity * area);
            // Turbulent flow; test when Re>10.
            if (RE >= 10.0) {
                S2 = std::sqrt(-2.0 * propM.density * PDROP) * area;
                FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                while (true) {
                    FT = FTT;
                    B = (9.3 * propM.viscosity * area) / (FT * Rough);
                    D = 1.0 + g * B;
                    g -= (g - AA1 + C * std::log(D)) / (1.0 + C * B / D);
                    FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                    if (std::abs(FTT - FT) / FTT < EPS) break;
                }
                FT = -FTT;
            } else {
                FT = FL;
            }
        }
        // Select laminar or turbulent flow.
        if (std::abs(FL) <= std::abs(FT)) {
            F[0] = FL;
            DF[0] = CDM;
        } else {
            F[0] = FT;
            DF[0] = 0.5 * FT / PDROP;
        }
        return 1;
    }

    int ZoneExhaustFan::calculate(EnergyPlusData &state,
                                  bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                                  Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                                  int const i,                              // Linkage number
                                  [[maybe_unused]] const Real64 multiplier, // Element multiplier
                                  [[maybe_unused]] const Real64 control,    // Element control signal
                                  const AirState &propN,                    // Node 1 properties
                                  const AirState &propM,                    // Node 2 properties
                                  std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                                  std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 12/17/06
        //                      Revised for zone exhaust fan
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a surface crack component

        // Using/Aliasing
        using HVAC::VerySmallMassFlow;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CDM;
        Real64 FL;
        Real64 FT;
        Real64 RhozNorm;
        Real64 VisczNorm;
        Real64 expn;
        Real64 Ctl;
        Real64 coef;
        Real64 Corr;
        Real64 VisAve;
        Real64 Tave;
        Real64 RhoCor;
        // int InletNode;

        // Formats
        // static gio::Fmt Format_901("(A5,I3,6X,4E16.7)");

        if (state.dataLoopNodes->Node(InletNode).MassFlowRate > VerySmallMassFlow) {
            // Treat the component as an exhaust fan
            if (state.afn->PressureSetFlag == PressureCtrlExhaust) {
                F[0] = state.afn->ExhaustFanMassFlowRate;
            } else {
                F[0] = state.dataLoopNodes->Node(InletNode).MassFlowRate;
            }
            DF[0] = 0.0;
            return 1;
        } else {
            // Treat the component as a surface crack
            // Crack standard condition from given inputs
            Corr = state.afn->MultizoneSurfaceData(i).factor;
            RhozNorm = state.afn->properties.density(StandardP, StandardT, StandardW);
            VisczNorm = 1.71432e-5 + 4.828e-8 * StandardT;

            expn = FlowExpo;
            VisAve = (propN.viscosity + propM.viscosity) / 2.0;
            Tave = (propN.temperature + propM.temperature) / 2.0;
            if (PDROP >= 0.0) {
                coef = FlowCoef / propN.sqrt_density * Corr;
            } else {
                coef = FlowCoef / propM.sqrt_density * Corr;
            }

            if (LFLAG) {
                // Initialization by linear relation.
                if (PDROP >= 0.0) {
                    RhoCor = TOKELVIN(propN.temperature) / TOKELVIN(Tave);
                    Ctl = std::pow(RhozNorm / propN.density / RhoCor, expn - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * expn - 1.0);
                    DF[0] = coef * propN.density / propN.viscosity * Ctl;
                } else {
                    RhoCor = TOKELVIN(propM.temperature) / TOKELVIN(Tave);
                    Ctl = std::pow(RhozNorm / propM.density / RhoCor, expn - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * expn - 1.0);
                    DF[0] = coef * propM.density / propM.viscosity * Ctl;
                }
                F[0] = -DF[0] * PDROP;
            } else {
                // Standard calculation.
                if (PDROP >= 0.0) {
                    // Flow in positive direction.
                    // Laminar flow.
                    RhoCor = TOKELVIN(propN.temperature) / TOKELVIN(Tave);
                    Ctl = std::pow(RhozNorm / propN.density / RhoCor, expn - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * expn - 1.0);
                    CDM = coef * propN.density / propN.viscosity * Ctl;
                    FL = CDM * PDROP;
                    // Turbulent flow.
                    if (expn == 0.5) {
                        FT = coef * propN.sqrt_density * std::sqrt(PDROP) * Ctl;
                    } else {
                        FT = coef * propN.sqrt_density * std::pow(PDROP, expn) * Ctl;
                    }
                } else {
                    // Flow in negative direction.
                    // Laminar flow.
                    RhoCor = TOKELVIN(propM.temperature) / TOKELVIN(Tave);
                    Ctl = std::pow(RhozNorm / propM.density / RhoCor, expn - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * expn - 1.0);
                    CDM = coef * propM.density / propM.viscosity * Ctl;
                    FL = CDM * PDROP;
                    // Turbulent flow.
                    if (expn == 0.5) {
                        FT = -coef * propM.sqrt_density * std::sqrt(-PDROP) * Ctl;
                    } else {
                        FT = -coef * propM.sqrt_density * std::pow(-PDROP, expn) * Ctl;
                    }
                }
                // Select laminar or turbulent flow.
                // if (LIST >= 4) gio::write(Unit21, Format_901) << " scr: " << i << PDROP << FL << FT;
                if (std::abs(FL) <= std::abs(FT)) {
                    F[0] = FL;
                    DF[0] = CDM;
                } else {
                    F[0] = FT;
                    DF[0] = FT * expn / PDROP;
                }
            }
        }
        return 1;
    }

    int ZoneExhaustFan::calculate(EnergyPlusData &state,
                                  Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                                  [[maybe_unused]] const Real64 multiplier, // Element multiplier
                                  const Real64 control,                     // Element control signal
                                  const AirState &propN,                    // Node 1 properties
                                  const AirState &propM,                    // Node 2 properties
                                  std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                                  std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 12/17/06
        //                      Revised for zone exhaust fan
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a surface crack component

        // Using/Aliasing
        using HVAC::VerySmallMassFlow;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CDM;
        Real64 FL;
        Real64 FT;
        Real64 RhozNorm;
        Real64 VisczNorm;
        Real64 expn;
        Real64 Ctl;
        Real64 coef;
        Real64 VisAve;
        Real64 Tave;
        Real64 RhoCor;
        // int InletNode;

        // Formats
        // static gio::Fmt Format_901("(A5,I3,6X,4E16.7)");

        if (state.dataLoopNodes->Node(InletNode).MassFlowRate > VerySmallMassFlow) {
            // Treat the component as an exhaust fan
            if (state.afn->PressureSetFlag == PressureCtrlExhaust) {
                F[0] = state.afn->ExhaustFanMassFlowRate;
            } else {
                F[0] = state.dataLoopNodes->Node(InletNode).MassFlowRate;
            }
            DF[0] = 0.0;
            return 1;
        } else {
            // Treat the component as a surface crack
            // Crack standard condition from given inputs
            RhozNorm = state.afn->properties.density(StandardP, StandardT, StandardW);
            VisczNorm = 1.71432e-5 + 4.828e-8 * StandardT;

            expn = FlowExpo;
            VisAve = (propN.viscosity + propM.viscosity) / 2.0;
            Tave = (propN.temperature + propM.temperature) / 2.0;
            if (PDROP >= 0.0) {
                coef = control * FlowCoef / propN.sqrt_density;
            } else {
                coef = control * FlowCoef / propM.sqrt_density;
            }

            // Standard calculation.
            if (PDROP >= 0.0) {
                // Flow in positive direction.
                // Laminar flow.
                RhoCor = TOKELVIN(propN.temperature) / TOKELVIN(Tave);
                Ctl = std::pow(RhozNorm / propN.density / RhoCor, expn - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * expn - 1.0);
                CDM = coef * propN.density / propN.viscosity * Ctl;
                FL = CDM * PDROP;
                // Turbulent flow.
                if (expn == 0.5) {
                    FT = coef * propN.sqrt_density * std::sqrt(PDROP) * Ctl;
                } else {
                    FT = coef * propN.sqrt_density * std::pow(PDROP, expn) * Ctl;
                }
            } else {
                // Flow in negative direction.
                // Laminar flow.
                RhoCor = TOKELVIN(propM.temperature) / TOKELVIN(Tave);
                Ctl = std::pow(RhozNorm / propM.density / RhoCor, expn - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * expn - 1.0);
                CDM = coef * propM.density / propM.viscosity * Ctl;
                FL = CDM * PDROP;
                // Turbulent flow.
                if (expn == 0.5) {
                    FT = -coef * propM.sqrt_density * std::sqrt(-PDROP) * Ctl;
                } else {
                    FT = -coef * propM.sqrt_density * std::pow(-PDROP, expn) * Ctl;
                }
            }
            // Select laminar or turbulent flow.
            // if (LIST >= 4) gio::write(Unit21, Format_901) << " scr: " << i << PDROP << FL << FT;
            if (std::abs(FL) <= std::abs(FT)) {
                F[0] = FL;
                DF[0] = CDM;
            } else {
                F[0] = FT;
                DF[0] = FT * expn / PDROP;
            }
        }
        return 1;
    }

    int HorizontalOpening::calculate(EnergyPlusData &state,
                                     bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                                     Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                                     int const i,                              // Linkage number
                                     [[maybe_unused]] const Real64 multiplier, // Element multiplier
                                     [[maybe_unused]] const Real64 control,    // Element control signal
                                     const AirState &propN,                    // Node 1 properties
                                     const AirState &propM,                    // Node 2 properties
                                     std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                                     std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Apr. 2009
        //       MODIFIED       na
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a horizontal opening component. The subroutine was
        // developed based on the subroutine AFEPLR of AIRNET.

        // METHODOLOGY EMPLOYED:
        // Combine forced and buyancy airflows together with a cap

        // REFERENCES:
        // Cooper, L., 1989, "Calculation of the Flow Through a Horizontal Ceiling/Floor Vent,"
        // NISTIR 89-4052, National Institute of Standards and Technology, Gaithersburg, MD

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 RhozAver;
        Real64 expn;
        Real64 coef;
        Real64 Width;  // Opening width
        Real64 Height; // Opening height
        Real64 Fact;   // Opening factor
        // Real64 Slope;      // Opening slope
        // Real64 DischCoeff; // Discharge coefficient
        Real64 fma12;      // massflow in direction "from-to" [kg/s]
        Real64 fma21;      // massflow in direction "to-from" [kg/s]
        Real64 dp1fma12;   // derivative d fma12 / d Dp [kg/s/Pa]
        Real64 dp1fma21;   // derivative d fma21 / d Dp [kg/s/Pa]
        Real64 PurgedP;    // Purge pressure [Pa]
        Real64 BuoFlow;    // Buoyancy flow rate [Pa]
        Real64 BuoFlowMax; // Maximum buoyancy flow rate [Pa]
        Real64 dPBuoFlow;  // Derivative of buoyancy flow rate [kg/s/Pa]
        Real64 DH;         // Hydraulic diameter [m]
        Real64 Cshape;     // Shape factor [dimensionless]
        Real64 OpenArea;   // Opening area [m2]

        // Get information on the horizontal opening
        RhozAver = (propN.density + propM.density) / 2.0;
        Width = state.afn->MultizoneSurfaceData(i).width;
        Height = state.afn->MultizoneSurfaceData(i).height;
        Fact = state.afn->MultizoneSurfaceData(i).OpenFactor;
        expn = FlowExpo;
        coef = FlowCoef;
        // Slope = MultizoneCompHorOpeningData(CompNum).Slope;
        // DischCoeff = MultizoneCompHorOpeningData(CompNum).DischCoeff;
        Cshape = 0.942 * Width / Height;
        OpenArea = Width * Height * Fact * std::sin(Slope * Constant::Pi / 180.0) * (1.0 + std::cos(Slope * Constant::Pi / 180.0));
        DH = 4.0 * (Width * Height) / 2.0 / (Width + Height) * Fact;

        // Check which zone is higher
        if (Fact == 0.0) {
            generic_crack(coef, expn, LFLAG, PDROP, propN, propM, F, DF);
            return 1;
        }

        fma12 = 0.0;
        fma21 = 0.0;
        dp1fma12 = 0.0;
        dp1fma21 = 0.0;
        BuoFlow = 0.0;
        dPBuoFlow = 0.0;

        if (state.afn->AirflowNetworkLinkageData(i).NodeHeights[0] > state.afn->AirflowNetworkLinkageData(i).NodeHeights[1]) {
            // Node N is upper zone
            if (propN.density > propM.density) {
                BuoFlowMax = RhozAver * 0.055 * std::sqrt(9.81 * std::abs(propN.density - propM.density) * pow_5(DH) / RhozAver);
                PurgedP = Cshape * Cshape * 9.81 * std::abs(propN.density - propM.density) * pow_5(DH) / (2.0 * pow_2(OpenArea));
                if (std::abs(PDROP) <= PurgedP) {
                    BuoFlow = BuoFlowMax * (1.0 - std::abs(PDROP) / PurgedP);
                    dPBuoFlow = BuoFlowMax / PurgedP;
                }
            }
        } else {
            // Node M is upper zone
            if (propN.density < propM.density) {
                BuoFlowMax = RhozAver * 0.055 * std::sqrt(9.81 * std::abs(propN.density - propM.density) * pow_5(DH) / RhozAver);
                PurgedP = Cshape * Cshape * 9.81 * std::abs(propN.density - propM.density) * pow_5(DH) / (2.0 * pow_2(OpenArea));
                if (std::abs(PDROP) <= PurgedP) {
                    BuoFlow = BuoFlowMax * (1.0 - std::abs(PDROP) / PurgedP);
                    dPBuoFlow = BuoFlowMax / PurgedP;
                }
            }
        }

        if (PDROP == 0.0) {
            fma12 = BuoFlow;
            fma21 = BuoFlow;
            dp1fma12 = 0.0;
            dp1fma21 = 0.0;
        } else if (PDROP > 0.0) {
            fma12 = propN.density * OpenArea * Fact * DischCoeff * std::sqrt(2.0 * PDROP / RhozAver) + BuoFlow;
            dp1fma12 = propN.density * OpenArea * DischCoeff / std::sqrt(2.0 * PDROP * RhozAver) + dPBuoFlow;
            if (BuoFlow > 0.0) {
                fma21 = BuoFlow;
                dp1fma21 = dPBuoFlow;
            }
        } else { // PDROP.LT.0.0
            fma21 = propM.density * OpenArea * Fact * DischCoeff * std::sqrt(2.0 * std::abs(PDROP) / RhozAver) + BuoFlow;
            dp1fma21 = -propM.density * OpenArea * DischCoeff / std::sqrt(2.0 * std::abs(PDROP) * RhozAver) + dPBuoFlow;
            if (BuoFlow > 0.0) {
                fma12 = BuoFlow;
                dp1fma12 = dPBuoFlow;
            }
        }

        F[0] = fma12 - fma21;
        DF[0] = dp1fma12 - dp1fma21;
        F[1] = 0.0;
        if (fma12 != 0.0 && fma21 != 0.0) {
            F[1] = BuoFlow;
        }
        DF[1] = 0.0;

        return 1;
    }

    int SpecifiedMassFlow::calculate([[maybe_unused]] EnergyPlusData &state,
                                     [[maybe_unused]] bool const LFLAG,      // Initialization flag.If = 1, use laminar relationship
                                     [[maybe_unused]] Real64 const PDROP,    // Total pressure drop across a component (P1 - P2) [Pa]
                                     [[maybe_unused]] int const i,           // Linkage number
                                     const Real64 multiplier,                // Element multiplier
                                     const Real64 control,                   // Element control signal
                                     [[maybe_unused]] const AirState &propN, // Node 1 properties
                                     [[maybe_unused]] const AirState &propM, // Node 2 properties
                                     std::array<Real64, 2> &F,               // Airflow through the component [kg/s]
                                     std::array<Real64, 2> &DF               // Partial derivative:  DF/DP
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason DeGraw and Prateek Shrestha
        //       DATE WRITTEN   June 2021
        //       MODIFIED       na
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine computes airflow for a specified mass flow element.

        F[0] = mass_flow * control * multiplier;
        DF[0] = 0.0;
        F[1] = 0.0;
        DF[1] = 0.0;

        return 1;
    }

    int SpecifiedVolumeFlow::calculate([[maybe_unused]] EnergyPlusData &state,
                                       [[maybe_unused]] bool const LFLAG,   // Initialization flag.If = 1, use laminar relationship
                                       [[maybe_unused]] Real64 const PDROP, // Total pressure drop across a component (P1 - P2) [Pa]
                                       [[maybe_unused]] int const i,        // Linkage number
                                       const Real64 multiplier,             // Element multiplier
                                       const Real64 control,                // Element control signal
                                       const AirState &propN,               // Node 1 properties
                                       const AirState &propM,               // Node 2 properties
                                       std::array<Real64, 2> &F,            // Airflow through the component [kg/s]
                                       std::array<Real64, 2> &DF            // Partial derivative:  DF/DP
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason DeGraw and Prateek Shrestha
        //       DATE WRITTEN   June 2021
        //       MODIFIED       na
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine computes airflow for a specified volume flow element.

        Real64 flow = volume_flow * control * multiplier;

        Real64 upwind_density{propN.density};

        if (flow < 0.0) {
            upwind_density = propM.density;
        }

        F[0] = flow * upwind_density;
        DF[0] = 0.0;
        F[1] = 0.0;
        DF[1] = 0.0;

        return 1;
    }

    int OutdoorAirFan::calculate(EnergyPlusData &state,
                                 bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                                 Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                                 int const i,                              // Linkage number
                                 [[maybe_unused]] const Real64 multiplier, // Element multiplier
                                 [[maybe_unused]] const Real64 control,    // Element control signal
                                 const AirState &propN,                    // Node 1 properties
                                 const AirState &propM,                    // Node 2 properties
                                 std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                                 std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a constant flow rate airflow component -- using standard interface.

        // Using/Aliasing
        using HVAC::VerySmallMassFlow;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 expn;
        Real64 Ctl;
        Real64 coef;
        Real64 Corr;
        Real64 VisAve;
        Real64 Tave;
        Real64 RhoCor;
        // int InletNode;
        Real64 RhozNorm;
        Real64 VisczNorm;
        Real64 CDM;
        Real64 FL;
        Real64 FT;

        int AirLoopNum = state.afn->AirflowNetworkLinkageData(i).AirLoopNum;

        if (state.dataLoopNodes->Node(InletNode).MassFlowRate > VerySmallMassFlow) {
            // Treat the component as an exhaust fan
            F[0] = state.dataLoopNodes->Node(InletNode).MassFlowRate;
            DF[0] = 0.0;
            if (state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopFanOperationMode == HVAC::FanOp::Cycling &&
                state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio > 0.0) {
                F[0] = F[0] / state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio;
            }
            return 1;
        } else {
            // Treat the component as a surface crack
            // Crack standard condition from given inputs
            Corr = 1.0;
            RhozNorm = state.afn->properties.density(StandardP, StandardT, StandardW);
            VisczNorm = 1.71432e-5 + 4.828e-8 * StandardT;

            expn = FlowExpo;
            VisAve = (propN.viscosity + propM.viscosity) / 2.0;
            Tave = (propN.temperature + propM.temperature) / 2.0;
            if (PDROP >= 0.0) {
                coef = FlowCoef / propN.sqrt_density * Corr;
            } else {
                coef = FlowCoef / propM.sqrt_density * Corr;
            }

            if (LFLAG) {
                // Initialization by linear relation.
                if (PDROP >= 0.0) {
                    RhoCor = TOKELVIN(propN.temperature) / TOKELVIN(Tave);
                    Ctl = std::pow(RhozNorm / propN.density / RhoCor, expn - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * expn - 1.0);
                    DF[0] = coef * propN.density / propN.viscosity * Ctl;
                } else {
                    RhoCor = TOKELVIN(propM.temperature) / TOKELVIN(Tave);
                    Ctl = std::pow(RhozNorm / propM.density / RhoCor, expn - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * expn - 1.0);
                    DF[0] = coef * propM.density / propM.viscosity * Ctl;
                }
                F[0] = -DF[0] * PDROP;
            } else {
                // Standard calculation.
                if (PDROP >= 0.0) {
                    // Flow in positive direction.
                    // Laminar flow.
                    RhoCor = TOKELVIN(propN.temperature) / TOKELVIN(Tave);
                    Ctl = std::pow(RhozNorm / propN.density / RhoCor, expn - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * expn - 1.0);
                    CDM = coef * propN.density / propN.viscosity * Ctl;
                    FL = CDM * PDROP;
                    // Turbulent flow.
                    if (expn == 0.5) {
                        FT = coef * propN.sqrt_density * std::sqrt(PDROP) * Ctl;
                    } else {
                        FT = coef * propN.sqrt_density * std::pow(PDROP, expn) * Ctl;
                    }
                } else {
                    // Flow in negative direction.
                    // Laminar flow.
                    RhoCor = TOKELVIN(propM.temperature) / TOKELVIN(Tave);
                    Ctl = std::pow(RhozNorm / propM.density / RhoCor, expn - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * expn - 1.0);
                    CDM = coef * propM.density / propM.viscosity * Ctl;
                    FL = CDM * PDROP;
                    // Turbulent flow.
                    if (expn == 0.5) {
                        FT = -coef * propM.sqrt_density * std::sqrt(-PDROP) * Ctl;
                    } else {
                        FT = -coef * propM.sqrt_density * std::pow(-PDROP, expn) * Ctl;
                    }
                }
                // Select laminar or turbulent flow.
                if (std::abs(FL) <= std::abs(FT)) {
                    F[0] = FL;
                    DF[0] = CDM;
                } else {
                    F[0] = FT;
                    DF[0] = FT * expn / PDROP;
                }
            }
        }
        return 1;
    }

    int ReliefFlow::calculate(EnergyPlusData &state,
                              bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                              Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                              int const i,                              // Linkage number
                              [[maybe_unused]] const Real64 multiplier, // Element multiplier
                              [[maybe_unused]] const Real64 control,    // Element control signal
                              const AirState &propN,                    // Node 1 properties
                              const AirState &propM,                    // Node 2 properties
                              std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                              std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
    )
    {

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a constant flow rate airflow component -- using standard interface.

        // Using/Aliasing
        using HVAC::VerySmallMassFlow;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 expn;
        Real64 Ctl;
        Real64 coef;
        Real64 Corr;
        Real64 VisAve;
        Real64 Tave;
        Real64 RhoCor;
        Real64 RhozNorm;
        Real64 VisczNorm;
        Real64 CDM;
        Real64 FL;
        Real64 FT;

        int AirLoopNum = state.afn->AirflowNetworkLinkageData(i).AirLoopNum;

        if (state.dataLoopNodes->Node(OutletNode).MassFlowRate > VerySmallMassFlow) {
            // Treat the component as an exhaust fan
            DF[0] = 0.0;
            if (state.afn->PressureSetFlag == PressureCtrlRelief) {
                F[0] = state.afn->ReliefMassFlowRate;
            } else {
                F[0] = state.dataLoopNodes->Node(OutletNode).MassFlowRate;
                if (state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopFanOperationMode == HVAC::FanOp::Cycling &&
                    state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio > 0.0) {
                    F[0] = F[0] / state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio;
                }
            }
            return 1;
        } else {
            // Treat the component as a surface crack
            // Crack standard condition from given inputs
            Corr = 1.0;
            RhozNorm = state.afn->properties.density(StandardP, StandardT, StandardW);
            VisczNorm = 1.71432e-5 + 4.828e-8 * StandardT;

            expn = FlowExpo;
            VisAve = (propN.viscosity + propM.viscosity) / 2.0;
            Tave = (propN.temperature + propM.temperature) / 2.0;
            if (PDROP >= 0.0) {
                coef = FlowCoef / propN.sqrt_density * Corr;
            } else {
                coef = FlowCoef / propM.sqrt_density * Corr;
            }

            if (LFLAG) {
                // Initialization by linear relation.
                if (PDROP >= 0.0) {
                    RhoCor = TOKELVIN(propN.temperature) / TOKELVIN(Tave);
                    Ctl = std::pow(RhozNorm / propN.density / RhoCor, expn - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * expn - 1.0);
                    DF[0] = coef * propN.density / propN.viscosity * Ctl;
                } else {
                    RhoCor = TOKELVIN(propM.temperature) / TOKELVIN(Tave);
                    Ctl = std::pow(RhozNorm / propM.density / RhoCor, expn - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * expn - 1.0);
                    DF[0] = coef * propM.density / propM.viscosity * Ctl;
                }
                F[0] = -DF[0] * PDROP;
            } else {
                // Standard calculation.
                if (PDROP >= 0.0) {
                    // Flow in positive direction.
                    // Laminar flow.
                    RhoCor = TOKELVIN(propN.temperature) / TOKELVIN(Tave);
                    Ctl = std::pow(RhozNorm / propN.density / RhoCor, expn - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * expn - 1.0);
                    CDM = coef * propN.density / propN.viscosity * Ctl;
                    FL = CDM * PDROP;
                    // Turbulent flow.
                    if (expn == 0.5) {
                        FT = coef * propN.sqrt_density * std::sqrt(PDROP) * Ctl;
                    } else {
                        FT = coef * propN.sqrt_density * std::pow(PDROP, expn) * Ctl;
                    }
                } else {
                    // Flow in negative direction.
                    // Laminar flow.
                    RhoCor = TOKELVIN(propM.temperature) / TOKELVIN(Tave);
                    Ctl = std::pow(RhozNorm / propM.density / RhoCor, expn - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * expn - 1.0);
                    CDM = coef * propM.density / propM.viscosity * Ctl;
                    FL = CDM * PDROP;
                    // Turbulent flow.
                    if (expn == 0.5) {
                        FT = -coef * propM.sqrt_density * std::sqrt(-PDROP) * Ctl;
                    } else {
                        FT = -coef * propM.sqrt_density * std::pow(-PDROP, expn) * Ctl;
                    }
                }
                // Select laminar or turbulent flow.
                if (std::abs(FL) <= std::abs(FT)) {
                    F[0] = FL;
                    DF[0] = CDM;
                } else {
                    F[0] = FT;
                    DF[0] = FT * expn / PDROP;
                }
            }
        }
        return 1;
    }

    void generic_crack(Real64 const coefficient, // Flow coefficient
                       Real64 const exponent,    // Flow exponent
                       bool const linear,        // Initialization flag. If true, use linear relationship
                       Real64 const pdrop,       // Total pressure drop across a component (P1 - P2) [Pa]
                       const AirState &propN,    // Node 1 properties
                       const AirState &propM,    // Node 2 properties
                       std::array<Real64, 2> &F, // Airflow through the component [kg/s]
                       std::array<Real64, 2> &DF // Partial derivative:  DF/DP
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  This subroutine is revised from AFEPLR developed by George Walton, NIST
        //                      Jason DeGraw

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves airflow for a power law component

        // METHODOLOGY EMPLOYED:
        // Using Q=C(dP)^n

        // FLOW:
        // Calculate normal density and viscocity at reference conditions
        constexpr Real64 reference_density = AIRDENSITY_CONSTEXPR(101325.0, 20.0, 0.0);
        constexpr Real64 reference_viscosity = 1.71432e-5 + 4.828e-8 * 20.0;

        Real64 VisAve{0.5 * (propN.viscosity + propM.viscosity)};
        Real64 Tave{0.5 * (propN.temperature + propM.temperature)};

        Real64 sign{1.0};
        Real64 upwind_temperature{propN.temperature};
        Real64 upwind_density{propN.density};
        Real64 upwind_viscosity{propN.viscosity};
        Real64 upwind_sqrt_density{propN.sqrt_density};
        Real64 abs_pdrop = pdrop;

        if (pdrop < 0.0) {
            sign = -1.0;
            upwind_temperature = propM.temperature;
            upwind_density = propM.density;
            upwind_viscosity = propM.viscosity;
            upwind_sqrt_density = propM.sqrt_density;
            abs_pdrop = -pdrop;
        }

        Real64 coef = coefficient / upwind_sqrt_density;

        // Laminar calculation
        Real64 RhoCor{TOKELVIN(upwind_temperature) / TOKELVIN(Tave)};
        Real64 Ctl{std::pow(reference_density / upwind_density / RhoCor, exponent - 1.0) *
                   std::pow(reference_viscosity / VisAve, 2.0 * exponent - 1.0)};
        Real64 CDM{coef * upwind_density / upwind_viscosity * Ctl};
        Real64 FL{CDM * pdrop};

        if (linear) {
            DF[0] = CDM;
            F[0] = FL;
        } else {
            // Turbulent flow.
            Real64 abs_FT;
            if (exponent == 0.5) {
                abs_FT = coef * upwind_sqrt_density * std::sqrt(abs_pdrop) * Ctl;
            } else {
                abs_FT = coef * upwind_sqrt_density * std::pow(abs_pdrop, exponent) * Ctl;
            }
            // Select linear or nonlinear flow.
            if (std::abs(FL) <= abs_FT) {
                F[0] = FL;
                DF[0] = CDM;
            } else {
                F[0] = sign * abs_FT;
                DF[0] = F[0] * exponent / pdrop;
            }
        }
    }

    int GenericDuct(Real64 const Length,      // Duct length
                    Real64 const Diameter,    // Duct diameter
                    bool const LFLAG,         // Initialization flag.If = 1, use laminar relationship
                    Real64 const PDROP,       // Total pressure drop across a component (P1 - P2) [Pa]
                    const AirState &propN,    // Node 1 properties
                    const AirState &propM,    // Node 2 properties
                    std::array<Real64, 2> &F, // Airflow through the component [kg/s]
                    std::array<Real64, 2> &DF // Partial derivative:  DF/DP
    )
    {

        // This subroutine solve air flow as a duct if fan has zero flow rate

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr C(0.868589);
        Real64 constexpr EPS(0.001);
        Real64 constexpr Rough(0.0001);
        Real64 constexpr InitLamCoef(128.0);
        Real64 constexpr LamDynCoef(64.0);
        Real64 constexpr LamFriCoef(0.0001);
        Real64 constexpr TurDynCoef(0.0001);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 A0;
        Real64 A1;
        Real64 A2;
        Real64 B;
        Real64 D;
        Real64 S2;
        Real64 CDM;
        Real64 FL;
        Real64 FT;
        Real64 FTT;
        Real64 RE;

        // Get component properties
        Real64 ed = Rough / Diameter;
        Real64 area = Diameter * Diameter * Constant::Pi / 4.0;
        Real64 ld = Length / Diameter;
        Real64 g = 1.14 - 0.868589 * std::log(ed);
        Real64 AA1 = g;

        if (LFLAG) {
            // Initialization by linear relation.
            if (PDROP >= 0.0) {
                DF[0] = (2.0 * propN.density * area * Diameter) / (propN.viscosity * InitLamCoef * ld);
            } else {
                DF[0] = (2.0 * propM.density * area * Diameter) / (propM.viscosity * InitLamCoef * ld);
            }
            F[0] = -DF[0] * PDROP;
        } else {
            // Standard calculation.
            if (PDROP >= 0.0) {
                // Flow in positive direction.
                // Laminar flow coefficient !=0
                if (LamFriCoef >= 0.001) {
                    A2 = LamFriCoef / (2.0 * propN.density * area * area);
                    A1 = (propN.viscosity * LamDynCoef * ld) / (2.0 * propN.density * area * Diameter);
                    A0 = -PDROP;
                    CDM = std::sqrt(A1 * A1 - 4.0 * A2 * A0);
                    FL = (CDM - A1) / (2.0 * A2);
                    CDM = 1.0 / CDM;
                } else {
                    CDM = (2.0 * propN.density * area * Diameter) / (propN.viscosity * LamDynCoef * ld);
                    FL = CDM * PDROP;
                }
                RE = FL * Diameter / (propN.viscosity * area);
                // Turbulent flow; test when Re>10.
                if (RE >= 10.0) {
                    S2 = std::sqrt(2.0 * propN.density * PDROP) * area;
                    FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                    while (true) {
                        FT = FTT;
                        B = (9.3 * propN.viscosity * area) / (FT * Rough);
                        D = 1.0 + g * B;
                        g -= (g - AA1 + C * std::log(D)) / (1.0 + C * B / D);
                        FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                        if (std::abs(FTT - FT) / FTT < EPS) break;
                    }
                    FT = FTT;
                } else {
                    FT = FL;
                }
            } else {
                // Flow in negative direction.
                // Laminar flow coefficient !=0
                if (LamFriCoef >= 0.001) {
                    A2 = LamFriCoef / (2.0 * propM.density * area * area);
                    A1 = (propM.viscosity * LamDynCoef * ld) / (2.0 * propM.density * area * Diameter);
                    A0 = PDROP;
                    CDM = std::sqrt(A1 * A1 - 4.0 * A2 * A0);
                    FL = -(CDM - A1) / (2.0 * A2);
                    CDM = 1.0 / CDM;
                } else {
                    CDM = (2.0 * propM.density * area * Diameter) / (propM.viscosity * LamDynCoef * ld);
                    FL = CDM * PDROP;
                }
                RE = -FL * Diameter / (propM.viscosity * area);
                // Turbulent flow; test when Re>10.
                if (RE >= 10.0) {
                    S2 = std::sqrt(-2.0 * propM.density * PDROP) * area;
                    FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                    while (true) {
                        FT = FTT;
                        B = (9.3 * propM.viscosity * area) / (FT * Rough);
                        D = 1.0 + g * B;
                        g -= (g - AA1 + C * std::log(D)) / (1.0 + C * B / D);
                        FTT = S2 / std::sqrt(ld / pow_2(g) + TurDynCoef);
                        if (std::abs(FTT - FT) / FTT < EPS) break;
                    }
                    FT = -FTT;
                } else {
                    FT = FL;
                }
            }
            // Select laminar or turbulent flow.
            if (std::abs(FL) <= std::abs(FT)) {
                F[0] = FL;
                DF[0] = CDM;
            } else {
                F[0] = FT;
                DF[0] = 0.5 * FT / PDROP;
            }
        }
        return 1;
    }

    void DetailedOpeningSolver::presprofile(EnergyPlusData &state,
                                            int const il,                  // Linkage number
                                            int const Pprof,               // Opening number
                                            Real64 const G,                // gravitation field strength [N/kg]
                                            const Array1D<Real64> &DpF,    // Stack pressures at start heights of Layers
                                            const Array1D<Real64> &DpT,    // Stack pressures at start heights of Layers
                                            const Array1D<Real64> &BetaF,  // Density gradients in the FROM zone (starting at linkheight) [Kg/m3/m]
                                            const Array1D<Real64> &BetaT,  // Density gradients in the TO zone (starting at linkheight) [Kg/m3/m]
                                            const Array1D<Real64> &RhoStF, // Density at the start heights of Layers in the FROM zone
                                            const Array1D<Real64> &RhoStT, // Density at the start heights of Layers in the TO zone
                                            int const From,                // Number of FROM zone
                                            int const To,                  // Number of To zone
                                            Real64 const ActLh,            // Actual height of opening [m]
                                            Real64 const OwnHeightFactor // Cosine of deviation angle of the opening plane from the vertical direction
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Oct. 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  This subroutine is revised based on PresProfile subroutine from COMIS

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates for a large opening profiles of stack pressure difference and
        // densities in the zones linked by the a detailed opening cmponent.

        // METHODOLOGY EMPLOYED:
        // The profiles are obtained in the following
        // way:    - the opening is divided into NrInt vertical intervals
        //         - the stack pressure difference and densities in From-
        //           and To-zone are calculated at the centre of each
        //           interval aswell as at the top and bottom of the LO
        //          - these values are stored in the (NrInt+2)-dimensional
        //             arrays DpProf, RhoProfF, RhoProfT.
        // The calculation of stack pressure and density in the two zones
        // is based on the arrays DpF/T, RhoStF/T, BetaF/T. These arrays
        // are calculated in the COMIS routine Lclimb. They contain the
        // values of stack pressure and density at the startheight of the
        // opening and at startheights of all layers lying inside the
        // opening, and the density gradients across the layers.
        // The effective startheight zl(1/2) in the From/To zone and the
        // effective length actLh of the LO take into account the
        // startheightfactor, heightfactor and ownheightfactor. Thus for
        // slanted windows the range of the profiles is the vertical
        // projection of the actual opening.

        // REFERENCES:
        // Helmut E. Feustel and Alison Rayner-Hooson, "COMIS Fundamentals," LBL-28560,
        // Lawrence Berkeley National Laboratory, Berkeley, CA, May 1990

        // Argument array dimensioning
        EP_SIZE_CHECK(DpF, 2);
        EP_SIZE_CHECK(DpT, 2);
        EP_SIZE_CHECK(BetaF, 2);
        EP_SIZE_CHECK(BetaT, 2);
        EP_SIZE_CHECK(RhoStF, 2);
        EP_SIZE_CHECK(RhoStT, 2);

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // in the FROM zone (starting at linkheight) [Pa]
        // (starting at linkheight) [Kg/m3]
        // (starting at linkheight) [Kg/m3]

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array1D<Real64> zF(2); // Startheights of layers in FROM-, TO-zone
        Array1D<Real64> zT(2);
        Array1D<Real64> zStF(2); // Startheights of layers within the LO, starting with the actual startheight of the LO.
        Array1D<Real64> zStT(2);
        // The values in the arrays DpF, DpT, BetaF, BetaT, RhoStF, RhoStT are calculated at these heights.
        Real64 hghtsFR;
        Real64 hghtsTR;
        Array1D<Real64> hghtsF(NrInt + 2); // Heights of evaluation points for pressure and density profiles
        Array1D<Real64> hghtsT(NrInt + 2);
        Real64 Interval; // Distance between two evaluation points
        Real64 delzF;    // Interval between actual evaluation point and startheight of actual layer in FROM-, TO-zone
        Real64 delzT;
        int AnzLayF; // Number of layers in FROM-, TO-zone
        int AnzLayT;
        int lF; // Actual index for DpF/T, BetaF/T, RhoStF/T, zStF/T
        int lT;
        int n;
        int i;
        int k;

        // Initialization
        delzF = 0.0;
        delzT = 0.0;
        Interval = ActLh * OwnHeightFactor / NrInt;

        for (n = 1; n <= NrInt; ++n) {
            hghtsF(n + 1) = state.afn->AirflowNetworkLinkageData(il).NodeHeights[0] + Interval * (n - 0.5);
            hghtsT(n + 1) = state.afn->AirflowNetworkLinkageData(il).NodeHeights[1] + Interval * (n - 0.5);
        }
        hghtsF(1) = state.afn->AirflowNetworkLinkageData(il).NodeHeights[0];
        hghtsT(1) = state.afn->AirflowNetworkLinkageData(il).NodeHeights[1];
        hghtsF(NrInt + 2) = state.afn->AirflowNetworkLinkageData(il).NodeHeights[0] + ActLh * OwnHeightFactor;
        hghtsT(NrInt + 2) = state.afn->AirflowNetworkLinkageData(il).NodeHeights[1] + ActLh * OwnHeightFactor;

        lF = 1;
        lT = 1;
        if (From == 0) {
            AnzLayF = 1;
        } else {
            AnzLayF = 0;
        }
        if (To == 0) {
            AnzLayT = 1;
        } else {
            AnzLayT = 0;
        }

        if (AnzLayF > 0) {
            for (n = 1; n <= AnzLayF; ++n) {
                zF(n) = 0.0;
                if (hghtsF(1) < 0.0) zF(n) = hghtsF(1);
            }
        }

        if (AnzLayT > 0) {
            for (n = 1; n <= AnzLayT; ++n) {
                zT(n) = 0.0;
                if (hghtsT(1) < 0.0) zT(n) = hghtsT(1);
            }
        }

        zStF(1) = state.afn->AirflowNetworkLinkageData(il).NodeHeights[0];
        i = 2;
        k = 1;

        while (k <= AnzLayF) {
            if (zF(k) > zStF(1)) break;
            ++k;
        }

        while (k <= AnzLayF) {
            if (zF(k) > hghtsF(NrInt)) break;
            zStF(i) = zF(k); // Autodesk:BoundsViolation zStF(i) @ i>2 and zF(k) @ k>2
            ++i;
            ++k;
        }

        zStF(i) = state.afn->AirflowNetworkLinkageData(il).NodeHeights[0] + ActLh * OwnHeightFactor; // Autodesk:BoundsViolation zStF(i) @ i>2
        zStT(1) = state.afn->AirflowNetworkLinkageData(il).NodeHeights[1];
        i = 2;
        k = 1;

        while (k <= AnzLayT) {
            if (zT(k) > zStT(1)) break;
            ++k;
        }

        while (k <= AnzLayT) {
            if (zT(k) > hghtsT(NrInt)) break; // Autodesk:BoundsViolation zT(k) @ k>2
            zStT(i) = zT(k);                  // Autodesk:BoundsViolation zStF(i) @ i>2 and zT(k) @ k>2
            ++i;
            ++k;
        }

        zStT(i) = state.afn->AirflowNetworkLinkageData(il).NodeHeights[1] + ActLh * OwnHeightFactor; // Autodesk:BoundsViolation zStT(i) @ i>2

        // Calculation of DpProf, RhoProfF, RhoProfT
        for (i = 1; i <= NrInt + 2; ++i) {
            hghtsFR = hghtsF(i);
            hghtsTR = hghtsT(i);

            while (true) {
                if (hghtsFR > zStF(lF + 1)) {
                    if (lF > 2) break;
                    ++lF;
                }
                if (hghtsFR <= zStF(lF + 1)) break;
            }

            while (true) {
                if (hghtsTR > zStT(lT + 1)) {
                    ++lT;
                }
                if (hghtsTR <= zStT(lT + 1)) break;
            }

            delzF = hghtsF(i) - zStF(lF);
            delzT = hghtsT(i) - zStT(lT);

            RhoProfF(i + Pprof) = RhoStF(lF) + BetaF(lF) * delzF;
            RhoProfT(i + Pprof) = RhoStT(lT) + BetaT(lT) * delzT;

            DpProf(i + Pprof) = DpF(lF) - DpT(lT) - G * (RhoStF(lF) * delzF + BetaF(lF) * pow_2(delzF) / 2.0) +
                                G * (RhoStT(lT) * delzT + BetaT(lT) * pow_2(delzT) / 2.0);
        }
    }

    void DetailedOpeningSolver::pstack(EnergyPlusData &state, std::vector<AirflowNetwork::AirState> &props, Array1D<Real64> &pz)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Oct. 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  This subroutine is revised based on PresProfile subroutine from COMIS

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates the stack pressures for a link between two zones

        // REFERENCES:
        // Helmut E. Feustel and Alison Rayner-Hooson, "COMIS Fundamentals," LBL-28560,
        // Lawrence Berkeley National Laboratory, Berkeley, CA, May 1990

        // USE STATEMENTS:
        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr PSea(101325.0);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        //      REAL(r64) RhoOut ! air density outside [kg/m3]
        Real64 G;                                                  // gravity field strength [N/kg]
        Real64 Pbz;                                                // Pbarom at entrance level [Pa]
        Array2D<Real64> RhoDrL(state.afn->NumOfLinksMultiZone, 2); // dry air density on both sides of the link [kg/m3]
        Real64 TempL1;                                             // Temp in From and To zone at link level [C]
        Real64 TempL2;
        //      REAL(r64) Tout ! outside temperature [C]
        Real64 Xhl1; // Humidity in From and To zone at link level [kg/kg]
        Real64 Xhl2;
        //      REAL(r64) Xhout ! outside humidity [kg/kg]
        Array1D<Real64> Hfl(state.afn->NumOfLinksMultiZone); // Own height factor for large (slanted) openings
        int Nl;                                              // number of links

        Array1D<Real64> DpF(2);
        Real64 DpP;
        Array1D<Real64> DpT(2);
        Real64 H;
        Array1D<Real64> RhoStF(2);
        Array1D<Real64> RhoStT(2);
        Real64 RhoDrDummi;
        Array1D<Real64> BetaStF(2);
        Array1D<Real64> BetaStT(2);
        Real64 T;
        Real64 X;
        Array1D<Real64> HSt(2);
        Real64 TzFrom;
        Real64 XhzFrom;
        Real64 TzTo;
        Real64 XhzTo;
        Real64 ActLh;
        Real64 ActLOwnh;
        Real64 Pref;
        Real64 PzFrom;
        Real64 PzTo;
        Array1D<Real64> RhoLd(2);
        Real64 RhoStd;
        int Fromz;
        int Toz;
        AirflowElementType Ltyp;
        int i;
        int ll;
        int Pprof;
        int OpenNum;

        Real64 RhoREF;
        Real64 CONV;

        RhoREF = state.afn->properties.density(PSea, state.dataEnvrn->OutDryBulbTemp, state.dataEnvrn->OutHumRat);

        CONV = state.dataEnvrn->Latitude * 2.0 * Constant::Pi / 360.0;
        G = 9.780373 * (1.0 + 0.0052891 * pow_2(std::sin(CONV)) - 0.0000059 * pow_2(std::sin(2.0 * CONV)));

        Hfl = 1.0;
        Pbz = state.dataEnvrn->OutBaroPress;
        Nl = state.afn->NumOfLinksMultiZone;
        OpenNum = 0;
        RhoLd(1) = 1.2;
        RhoLd(2) = 1.2;
        RhoStd = 1.2;

        for (i = 1; i <= Nl; ++i) {
            // Check surface tilt
            if (i <= Nl - state.afn->NumOfLinksIntraZone) { // Revised by L.Gu, on 9 / 29 / 10
                if (state.afn->AirflowNetworkLinkageData(i).DetOpenNum > 0 &&
                    state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).surface_number).Tilt < 90) {
                    Hfl(i) = state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).surface_number).SinTilt;
                }
            }
            // Initialisation
            int From = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
            int To = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
            if (state.afn->AirflowNetworkNodeData(From).EPlusZoneNum > 0 && state.afn->AirflowNetworkNodeData(To).EPlusZoneNum > 0) {
                ll = 0;
            } else if (state.afn->AirflowNetworkNodeData(From).EPlusZoneNum == 0 && state.afn->AirflowNetworkNodeData(To).EPlusZoneNum > 0) {
                ll = 1;
            } else {
                ll = 3;
            }

            Ltyp = state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).CompTypeNum;
            if (Ltyp == AirflowElementType::DOP) {
                ActLh = state.afn->MultizoneSurfaceData(i).height;
                ActLOwnh = ActLh * 1.0;
            } else {
                ActLh = 0.0;
                ActLOwnh = 0.0;
            }

            TempL1 = props[From].temperature;
            Xhl1 = props[From].humidity_ratio;
            TzFrom = props[From].temperature;
            XhzFrom = props[From].humidity_ratio;
            // RhoL1 = props[From].density;
            if (ll == 0 || ll == 3) {
                PzFrom = pz(From);
            } else {
                PzFrom = 0.0;
                From = 0;
            }

            int ilayptr = 0;
            if (From == 0) ilayptr = 1;
            if (ilayptr == 0) {
                Fromz = 0;
            } else {
                Fromz = From;
            }

            TempL2 = props[To].temperature;
            Xhl2 = props[To].humidity_ratio;
            TzTo = props[To].temperature;
            XhzTo = props[To].humidity_ratio;
            // RhoL2 = props[To].density;

            if (ll < 3) {
                PzTo = pz(To);
            } else {
                PzTo = 0.0;
                To = 0;
            }
            ilayptr = 0;
            if (To == 0) ilayptr = 1;
            if (ilayptr == 0) {
                Toz = 0;
            } else {
                Toz = To;
            }

            // RhoDrL is Rho at link level without pollutant but with humidity
            RhoDrL(i, 1) = state.afn->properties.density(state.dataEnvrn->OutBaroPress + PzFrom, TempL1, Xhl1);
            RhoDrL(i, 2) = state.afn->properties.density(state.dataEnvrn->OutBaroPress + PzTo, TempL2, Xhl2);

            // End initialisation

            // calculate DpF the difference between Pz and P at Node 1 height
            ilayptr = 0;
            if (Fromz == 0) ilayptr = 1;
            int j = ilayptr;
            int k = 1;
            lclimb(state, G, RhoLd(1), state.afn->AirflowNetworkLinkageData(i).NodeHeights[0], TempL1, Xhl1, DpF(k), Toz, PzTo, Pbz, RhoDrL(i, 1));
            Real64 RhoL1 = RhoLd(1);
            // For large openings calculate the stack pressure difference profile and the
            // density profile within the the top- and the bottom- height of the large opening
            if (ActLOwnh > 0.0) {
                HSt(k) = state.afn->AirflowNetworkLinkageData(i).NodeHeights[0];
                RhoStF(k) = RhoL1;
                ++k;
                HSt(k) = 0.0;
                if (HSt(k - 1) < 0.0) HSt(k) = HSt(k - 1);

                // Search for the first startheight of a layer which is within the top- and the
                // bottom- height of the large opening.
                while (true) {
                    ilayptr = 0;
                    if (Fromz == 0) ilayptr = 9;
                    if ((j > ilayptr) || (HSt(k) > state.afn->AirflowNetworkLinkageData(i).NodeHeights[0])) break;
                    j += 9;
                    HSt(k) = 0.0;
                    if (HSt(k - 1) < 0.0) HSt(k) = HSt(k - 1);
                }

                // Calculate Rho and stack pressure for every StartHeight of a layer which is
                // within the top- and the bottom-height of the  large opening.
                while (true) {
                    ilayptr = 0;
                    if (Fromz == 0) ilayptr = 9;
                    if ((j > ilayptr) || (HSt(k) >= (state.afn->AirflowNetworkLinkageData(i).NodeHeights[0] + ActLOwnh)))
                        break; // Autodesk:BoundsViolation HSt(k) @ k>2
                    T = TzFrom;
                    X = XhzFrom;
                    lclimb(
                        state, G, RhoStd, HSt(k), T, X, DpF(k), Fromz, PzFrom, Pbz, RhoDrDummi); // Autodesk:BoundsViolation HSt(k) and DpF(k) @ k>2
                    RhoStF(k) = RhoStd;                                                          // Autodesk:BoundsViolation RhoStF(k) @ k>2
                    j += 9;
                    ++k;                                       // Autodesk:Note k>2 now
                    HSt(k) = 0.0;                              // Autodesk:BoundsViolation @ k>2
                    if (HSt(k - 1) < 0.0) HSt(k) = HSt(k - 1); // Autodesk:BoundsViolation @ k>2
                }
                // Stack pressure difference and rho for top-height of the large opening
                HSt(k) = state.afn->AirflowNetworkLinkageData(i).NodeHeights[0] + ActLOwnh; // Autodesk:BoundsViolation k>2 poss
                T = TzFrom;
                X = XhzFrom;
                lclimb(state, G, RhoStd, HSt(k), T, X, DpF(k), Fromz, PzFrom, Pbz, RhoDrDummi); // Autodesk:BoundsViolation k>2 poss
                RhoStF(k) = RhoStd;                                                             // Autodesk:BoundsViolation k >= 3 poss

                for (j = 1; j <= (k - 1); ++j) {
                    BetaStF(j) = (RhoStF(j + 1) - RhoStF(j)) / (HSt(j + 1) - HSt(j));
                }
            }

            // repeat procedure for the "To" node, DpT
            ilayptr = 0;
            if (Toz == 0) ilayptr = 1;
            j = ilayptr;
            // Calculate Rho at link height only if we have large openings or layered zones.
            k = 1;
            lclimb(state, G, RhoLd(2), state.afn->AirflowNetworkLinkageData(i).NodeHeights[1], TempL2, Xhl2, DpT(k), Toz, PzTo, Pbz, RhoDrL(i, 2));
            Real64 RhoL2 = RhoLd(2);

            // For large openings calculate the stack pressure difference profile and the
            // density profile within the the top- and the bottom- height of the large opening
            if (ActLOwnh > 0.0) {
                HSt(k) = state.afn->AirflowNetworkLinkageData(i).NodeHeights[1];
                RhoStT(k) = RhoL2;
                ++k;
                HSt(k) = 0.0;
                if (HSt(k - 1) < 0.0) HSt(k) = HSt(k - 1);
                while (true) {
                    ilayptr = 0;
                    if (Toz == 0) ilayptr = 9;
                    if ((j > ilayptr) || (HSt(k) > state.afn->AirflowNetworkLinkageData(i).NodeHeights[1])) break;
                    j += 9;
                    HSt(k) = 0.0;
                    if (HSt(k - 1) < 0.0) HSt(k) = HSt(k - 1);
                }
                // Calculate Rho and stack pressure for every StartHeight of a layer which is
                // within the top- and the bottom-height of the  large opening.
                while (true) {
                    ilayptr = 0;
                    if (Toz == 0) ilayptr = 9;
                    if ((j > ilayptr) || (HSt(k) >= (state.afn->AirflowNetworkLinkageData(i).NodeHeights[1] + ActLOwnh)))
                        break; // Autodesk:BoundsViolation Hst(k) @ k>2
                    T = TzTo;
                    X = XhzTo;
                    lclimb(state, G, RhoStd, HSt(k), T, X, DpT(k), Toz, PzTo, Pbz, RhoDrDummi); // Autodesk:BoundsViolation HSt(k) and DpT(k) @ k>2
                    RhoStT(k) = RhoStd;                                                         // Autodesk:BoundsViolation RhoStT(k) @ k>2
                    j += 9;
                    ++k;                                       // Autodesk:Note k>2 now
                    HSt(k) = 0.0;                              // Autodesk:BoundsViolation @ k>2
                    if (HSt(k - 1) < 0.0) HSt(k) = HSt(k - 1); // Autodesk:BoundsViolation @ k>2
                }
                // Stack pressure difference and rho for top-height of the large opening
                HSt(k) = state.afn->AirflowNetworkLinkageData(i).NodeHeights[1] + ActLOwnh; // Autodesk:BoundsViolation k>2 poss
                T = TzTo;
                X = XhzTo;
                lclimb(state, G, RhoStd, HSt(k), T, X, DpT(k), Toz, PzTo, Pbz, RhoDrDummi); // Autodesk:BoundsViolation k>2 poss
                RhoStT(k) = RhoStd;                                                         // Autodesk:BoundsViolation k>2 poss

                for (j = 1; j <= (k - 1); ++j) {
                    BetaStT(j) = (RhoStT(j + 1) - RhoStT(j)) / (HSt(j + 1) - HSt(j));
                }
            }

            // CALCULATE STACK PRESSURE FOR THE PATH ITSELF for different flow directions
            H = double(state.afn->AirflowNetworkLinkageData(i).NodeHeights[1]) - double(state.afn->AirflowNetworkLinkageData(i).NodeHeights[0]);
            if (ll == 0 || ll == 3 || ll == 6) {
                H -= state.afn->AirflowNetworkNodeData(From).NodeHeight;
            }
            if (ll < 3) {
                H += state.afn->AirflowNetworkNodeData(To).NodeHeight;
            }

            // IF AIR FLOWS from "From" to "To"
            Pref = Pbz + PzFrom + DpF(1);
            DpP = psz(Pref, RhoLd(1), 0.0, 0.0, H, G);
            DpL(i, 1) = (DpF(1) - DpT(1) + DpP);

            // IF AIR FLOWS from "To" to "From"
            Pref = Pbz + PzTo + DpT(1);
            DpP = -psz(Pref, RhoLd(2), 0.0, 0.0, -H, G);
            DpL(i, 2) = (DpF(1) - DpT(1) + DpP);

            if (Ltyp == AirflowElementType::DOP) {
                Pprof = OpenNum * (NrInt + 2);
                presprofile(state, i, Pprof, G, DpF, DpT, BetaStF, BetaStT, RhoStF, RhoStT, From, To, ActLh, Hfl(i));
                ++OpenNum;
            }
        }
    }

    Real64 DetailedOpeningSolver::psz(Real64 const Pz0,  // Pressure at altitude z0 [Pa]
                                      Real64 const Rho0, // density at altitude z0 [kg/m3]
                                      Real64 const beta, // density gradient [kg/m4]
                                      Real64 const z0,   // reference altitude [m]
                                      Real64 const z,    // altitude[m]
                                      Real64 const g     // gravity field strength [N/kg]
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Oct. 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  This subroutine is revised based on psz function from COMIS

        // PURPOSE OF THIS SUBROUTINE:
        // This function determines the pressure due to buoyancy in a stratified density environment

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 dz;
        Real64 rho;

        dz = z - z0;
        rho = (Rho0 + beta * dz / 2.0);
        return -Pz0 * (1.0 - std::exp(-dz * rho * g / Pz0)); // Differential pressure from z to z0 [Pa]
    }

    void DetailedOpeningSolver::lclimb(EnergyPlusData &state,
                                       Real64 const G,   // gravity field strength [N/kg]
                                       Real64 &Rho,      // Density link level (initialized with rho zone) [kg/m3]
                                       Real64 const Z,   // Height of the link above the zone reference [m]
                                       Real64 &T,        // temperature at link level [C]
                                       Real64 &X,        // absolute humidity at link level [kg/kg]
                                       Real64 &Dp,       // Stackpressure to the linklevel [Pa]
                                       int const zone,   // Zone number
                                       Real64 const PZ,  // Zone Pressure (reflevel) [Pa]
                                       Real64 const Pbz, // Barometric pressure at entrance level [Pa]
                                       Real64 &RhoDr     // Air density of dry air on the link level used
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Oct. 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  This subroutine is revised based on subroutine IClimb from COMIS

        // PURPOSE OF THIS SUBROUTINE:
        // This function the differential pressure from the reflevel in a zone To Z, the level of a link

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 H;        // Start Height of the layer
        Real64 BetaT;    // Temperature gradient of this layer
        Real64 BetaXfct; // Humidity gradient factor of this layer
        Real64 BetaCfct; // Concentration 1 gradient factor of this layer
        Real64 X0;
        Real64 P;
        Real64 Htop;
        Real64 Hbot;
        Real64 Rho0;
        Real64 Rho1;
        Real64 BetaRho;
        static int L(0);
        static int ilayptr(0);

        Dp = 0.0;
        Rho0 = Rho;
        X0 = X;
        if (Z > 0.0) {
            // initialize start values
            H = 0.0;
            BetaT = 0.0;
            BetaXfct = 0.0;
            BetaCfct = 0.0;
            BetaRho = 0.0;
            Hbot = 0.0;

            while (H < 0.0) {
                // loop until H>0 ; The start of the layer is above 0
                BetaT = 0.0;
                BetaXfct = 0.0;
                BetaCfct = 0.0;
                L += 9;
                ilayptr = 0;
                if (zone == 0) ilayptr = 9;
                if (L >= ilayptr) {
                    H = Z + 1.0;
                } else {
                    H = 0.0;
                }
            }

            // The link is in this layer also if it is on the top of it.

            while (true) {
                if (H >= Z) {
                    // the link ends in this layer , we reached the final Dp and BetaRho
                    Htop = Z;
                    P = PZ + Dp;
                    if (Htop != Hbot) {
                        Rho0 = state.afn->properties.density(Pbz + P, T, X);
                        T += (Htop - Hbot) * BetaT;
                        X += (Htop - Hbot) * BetaXfct * X0;
                        Rho1 = state.afn->properties.density(Pbz + P, T, X);
                        BetaRho = (Rho1 - Rho0) / (Htop - Hbot);
                        Dp += psz(Pbz + P, Rho0, BetaRho, Hbot, Htop, G);
                    }
                    RhoDr = state.afn->properties.density(Pbz + PZ + Dp, T, X);
                    Rho = state.afn->properties.density(Pbz + PZ + Dp, T, X);
                    return;

                } else {
                    // bottom of the layer is below Z  (Z above ref)
                    Htop = H;
                    // P is the pressure up to the start height of the layer we just reached
                    P = PZ + Dp;
                    if (Htop != Hbot) {
                        Rho0 = state.afn->properties.density(Pbz + P, T, X);
                        T += (Htop - Hbot) * BetaT;
                        X += (Htop - Hbot) * BetaXfct * X0;
                        Rho1 = state.afn->properties.density(Pbz + P, T, X);
                        BetaRho = (Rho1 - Rho0) / (Htop - Hbot);
                        Dp += psz(Pbz + P, Rho0, BetaRho, Hbot, Htop, G);
                    }

                    RhoDr = state.afn->properties.density(Pbz + PZ + Dp, T, X);
                    Rho = state.afn->properties.density(Pbz + PZ + Dp, T, X);

                    // place current values Hbot and Beta's
                    Hbot = H;
                    BetaT = 0.0;
                    BetaXfct = 0.0;
                    BetaCfct = 0.0;
                    L += 9;
                    ilayptr = 0;
                    if (zone == 0) ilayptr = 9;
                    if (L >= ilayptr) {
                        H = Z + 1.0;
                    } else {
                        H = 0.0;
                    }
                }
            }

        } else {
            // This is the ELSE for negative linkheights Z below the refplane
            H = 0.0;
            BetaT = 0.0;
            BetaXfct = 0.0;
            BetaCfct = 0.0;
            BetaRho = 0.0;
            Htop = 0.0;
            while (H > 0.0) {
                // loop until H<0 ; The start of the layer is below the zone refplane
                L -= 9;
                ilayptr = 0;
                if (zone == 0) ilayptr = 1;
                if (L < ilayptr) {
                    // with H=Z (negative) this loop will exit, no data for interval Z-refplane
                    H = Z;
                    BetaT = 0.0;
                    BetaXfct = 0.0;
                    BetaCfct = 0.0;
                    BetaRho = 0.0;
                } else {
                    H = 0.0;
                    BetaT = 0.0;
                    BetaXfct = 0.0;
                    BetaCfct = 0.0;
                }
            }

            // The link is in this layer also if it is on the bottom of it.
            while (true) {
                if (H <= Z) {
                    Hbot = Z;
                    P = PZ + Dp;
                    if (Htop != Hbot) {
                        Rho1 = state.afn->properties.density(Pbz + P, T, X);
                        T += (Hbot - Htop) * BetaT;
                        X += (Hbot - Htop) * BetaXfct * X0;
                        Rho0 = state.afn->properties.density(Pbz + P, T, X);
                        BetaRho = (Rho1 - Rho0) / (Htop - Hbot);
                        Dp -= psz(Pbz + P, Rho0, BetaRho, Hbot, Htop, G);
                    }
                    RhoDr = state.afn->properties.density(Pbz + PZ + Dp, T, X);
                    Rho = state.afn->properties.density(Pbz + PZ + Dp, T, X);
                    return;
                } else {
                    // bottom of the layer is below Z  (Z below ref)
                    Hbot = H;
                    P = PZ + Dp;
                    if (Htop != Hbot) {
                        Rho1 = state.afn->properties.density(Pbz + P, T, X);
                        // T,X,C calculated for the lower height
                        T += (Hbot - Htop) * BetaT;
                        X += (Hbot - Htop) * BetaXfct * X0;
                        Rho0 = state.afn->properties.density(Pbz + P, T, X);
                        BetaRho = (Rho1 - Rho0) / (Htop - Hbot);
                        Dp -= psz(Pbz + P, Rho0, BetaRho, Hbot, Htop, G);
                    }
                    RhoDr = state.afn->properties.density(Pbz + PZ + Dp, T, X);
                    Rho = state.afn->properties.density(Pbz + PZ + Dp, T, X);

                    // place current values Hbot and Beta's
                    Htop = H;
                    L -= 9;
                    ilayptr = 0;
                    if (zone == 0) ilayptr = 1;
                    if (L < ilayptr) {
                        H = Z - 1.0;
                        BetaT = 0.0;
                        BetaXfct = 0.0;
                        BetaCfct = 0.0;
                    } else {
                        H = 0.0;
                        BetaT = 0.0;
                        BetaXfct = 0.0;
                        BetaCfct = 0.0;
                    }
                }
                // ENDIF H<Z
            }
        }
    }

} // namespace AirflowNetwork

} // namespace EnergyPlus
