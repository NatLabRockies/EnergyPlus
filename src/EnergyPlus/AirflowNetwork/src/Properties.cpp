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

#include "AirflowNetwork/Properties.hpp"
#include "../../Data/EnergyPlusData.hh"
#include "../../General.hh"

namespace EnergyPlus {

namespace AirflowNetwork {

    AirState::AirState(Real64 const density)
        : temperature(20.0), humidity_ratio(0.0), density(density), sqrt_density(sqrt(density)), viscosity(AIRDYNAMICVISCOSITY_CONSTEXPR(20.0))
    {
    }

    AirState::AirState()
        : temperature(20.0), humidity_ratio(0.0), density(AIRDENSITY_CONSTEXPR(101325.0, 20.0, 0.0)),
          sqrt_density(std::sqrt(AIRDENSITY_CONSTEXPR(101325.0, 20.0, 0.0))), viscosity(AIRDYNAMICVISCOSITY_CONSTEXPR(20.0))
    {
    }

    Real64 AirProperties::density(Real64 P, // Barometric pressure
                                  Real64 T, // Temperature in Celsius
                                  Real64 W  // Humidity ratio

    )
    {
        return Psychrometrics::PsyRhoAirFnPbTdbW(m_state, P, T, W);
    }

    Real64 AirProperties::thermal_conductivity(Real64 T // Temperature in Celsius
    )
    {
        // Dry air thermal conductivity {W/m-K}
        // Correlated over the range -20C to 70C
        // Reference Cengel & Ghajar, Heat and Mass Transfer. 5th ed.

        Real64 const LowerLimit = -20;
        Real64 const UpperLimit = 70;

        Real64 const a = 0.02364;
        Real64 const b = 0.0000754772569209165;
        Real64 const c = -2.40977632412045e-8;

        if (T < LowerLimit) {
            if (lowerLimitErrIdx == 0) {
                ShowWarningMessage(m_state, "Air temperature below lower limit of -20C for conductivity calculation");
            }
            ShowRecurringWarningErrorAtEnd(m_state,
                                           format("Air temperature below lower limit of -20C for conductivity calculation. Air temperature of {:.1R} "
                                                  "used for conductivity calculation.",
                                                  LowerLimit),
                                           lowerLimitErrIdx);
            T = LowerLimit;
        } else if (T > UpperLimit) {
            if (upperLimitErrIdx == 0) {
                ShowWarningMessage(m_state, "Air temperature above upper limit of 70C for conductivity calculation");
            }
            ShowRecurringWarningErrorAtEnd(m_state,
                                           format("Air temperature above upper limit of 70C for conductivity calculation. Air temperature of {:.1R} "
                                                  "used for conductivity calculation.",
                                                  UpperLimit),
                                           upperLimitErrIdx);
            T = UpperLimit;
        }

        return a + b * T + c * pow_2(T);
    }

    Real64 AirProperties::dynamic_viscosity(Real64 T // Temperature in Celsius
    )
    {
        return 1.71432e-5 + 4.828e-8 * T;
    }

    Real64 AirProperties::kinematic_viscosity(Real64 P, // Barometric pressure
                                              Real64 T, // Temperature in Celsius
                                              Real64 W  // Humidity ratio

    )
    {
        // Dry air kinematic viscosity {m2/s}
        // Correlated over the range -20C to 70C
        // Reference Cengel & Ghajar, Heat and Mass Transfer. 5th ed.

        Real64 const LowerLimit = -20;
        Real64 const UpperLimit = 70;

        if (T < LowerLimit) {
            T = LowerLimit;
        } else if (T > UpperLimit) {
            T = UpperLimit;
        }

        return dynamic_viscosity(T) / Psychrometrics::PsyRhoAirFnPbTdbW(m_state, P, T, W);
    }

    Real64 AirProperties::thermal_diffusivity(Real64 P, // Barometric pressure
                                              Real64 T, // Temperature in Celsius
                                              Real64 W  // Humidity ratio
    )
    {
        // Dry air thermal diffusivity {-}
        // Correlated over the range -20C to 70C
        // Reference Cengel & Ghajar, Heat and Mass Transfer. 5th ed.

        Real64 const LowerLimit = -20;
        Real64 const UpperLimit = 70;

        if (T < LowerLimit) {
            T = LowerLimit;
        } else if (T > UpperLimit) {
            T = UpperLimit;
        }

        return thermal_conductivity(T) / (AIRCP(W) * Psychrometrics::PsyRhoAirFnPbTdbW(m_state, P, T, W));
    }

    Real64 AirProperties::prandtl_number(Real64 P, // Barometric pressure
                                         Real64 T, // Temperature in Celsius
                                         Real64 W  // Humidity ratio
    )
    {
        // Dry air Prandtl number {-}
        // Correlated over the range -20C to 70C
        // Reference Cengel & Ghajar, Heat and Mass Transfer. 5th ed.

        Real64 const LowerLimit = -20;
        Real64 const UpperLimit = 70;

        if (T < LowerLimit) {
            T = LowerLimit;
        } else if (T > UpperLimit) {
            T = UpperLimit;
        }

        return kinematic_viscosity(P, T, W) / thermal_diffusivity(P, T, W);
    }

} // namespace AirflowNetwork

} // namespace EnergyPlus
