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

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::DataEnvironment {

// MODULE INFORMATION:
//       AUTHOR         Rick Strand, Dan Fisher, Linda Lawrie
//       DATE WRITTEN   December 1997
//       MODIFIED       November 1998, Fred Winkelmann
//       MODIFIED       June 1999,June 2000, Linda Lawrie
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// This data-only module is a repository for the variables that relate specifically
// to the "environment" (i.e. current date data, tomorrow's date data, and
// current weather variables)

Real64 OutDryBulbTempAt(EnergyPlusData &state, Real64 const Z) // Height above ground (m)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   January 2006
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Calculates outdoor dry bulb temperature at a given altitude.

    // METHODOLOGY EMPLOYED:
    // 1976 U.S. Standard Atmosphere.

    // REFERENCES:
    // 1976 U.S. Standard Atmosphere. 1976. U.S. Government Printing Office, Washington, D.C.

    // Using/Aliasing

    // Return value
    Real64 LocalOutDryBulbTemp; // Return result for function (C)

    // Locals
    // FUNCTION ARGUMENT DEFINITIONS:

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 BaseTemp; // Base temperature at Z = 0 (C)

    BaseTemp = state.dataEnvrn->OutDryBulbTemp + state.dataEnvrn->WeatherFileTempModCoeff;

    if (state.dataEnvrn->SiteTempGradient == 0.0) {
        LocalOutDryBulbTemp = state.dataEnvrn->OutDryBulbTemp;
    } else if (Z <= 0.0) {
        LocalOutDryBulbTemp = BaseTemp;
    } else {
        LocalOutDryBulbTemp = BaseTemp - state.dataEnvrn->SiteTempGradient * DataEnvironment::EarthRadius * Z / (DataEnvironment::EarthRadius + Z);
    }

    if (LocalOutDryBulbTemp < -100.0) {
        ShowSevereError(state, "OutDryBulbTempAt: outdoor drybulb temperature < -100 C");
        ShowContinueError(state, format("...check heights, this height=[{:.0R}].", Z));
        ShowFatalError(state, "Program terminates due to preceding condition(s).");
    }

    return LocalOutDryBulbTemp;
}

Real64 OutWetBulbTempAt(EnergyPlusData &state, Real64 const Z) // Height above ground (m)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   January 2006
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Calculates outdoor wet bulb temperature at a given altitude.

    // METHODOLOGY EMPLOYED:
    // 1976 U.S. Standard Atmosphere.

    // REFERENCES:
    // 1976 U.S. Standard Atmosphere. 1976. U.S. Government Printing Office, Washington, D.C.

    // Return value
    Real64 LocalOutWetBulbTemp; // Return result for function (C)

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 BaseTemp; // Base temperature at Z = 0 (C)

    BaseTemp = state.dataEnvrn->OutWetBulbTemp + state.dataEnvrn->WeatherFileTempModCoeff;

    if (state.dataEnvrn->SiteTempGradient == 0.0) {
        LocalOutWetBulbTemp = state.dataEnvrn->OutWetBulbTemp;
    } else if (Z <= 0.0) {
        LocalOutWetBulbTemp = BaseTemp;
    } else {
        LocalOutWetBulbTemp = BaseTemp - state.dataEnvrn->SiteTempGradient * DataEnvironment::EarthRadius * Z / (DataEnvironment::EarthRadius + Z);
    }

    if (LocalOutWetBulbTemp < -100.0) {
        ShowSevereError(state, "OutWetBulbTempAt: outdoor wetbulb temperature < -100 C");
        ShowContinueError(state, format("...check heights, this height=[{:.0R}].", Z));
        ShowFatalError(state, "Program terminates due to preceding condition(s).");
    }

    return LocalOutWetBulbTemp;
}

Real64 WindSpeedAt(const EnergyPlusData &state, Real64 const Z) // Height above ground (m)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   January 2006

    // PURPOSE OF THIS FUNCTION:
    // Calculates local wind speed at a given altitude.

    // METHODOLOGY EMPLOYED:
    // 2005 ASHRAE Fundamentals, Chapter 16, Equation 4.  (Different depending on terrain).

    // REFERENCES:
    // 2005 ASHRAE Fundamentals, Chapter 16, Equation 4.  (Different depending on terrain).
    // Terrain variables are set in HeatBalanceManager or entered by the user.

    if (Z <= 0.0) {
        return 0.0;
    }
    if (state.dataEnvrn->SiteWindExp == 0.0) {
        return state.dataEnvrn->WindSpeed;
    } //  [Met] - at meterological Station, Height of measurement is usually 10m above ground
    //  LocalWindSpeed = Windspeed [Met] * (Wind Boundary LayerThickness [Met]/Height [Met])**Wind Exponent[Met] &
    //                     * (Height above ground / Site Wind Boundary Layer Thickness) ** Site Wind Exponent
    return state.dataEnvrn->WindSpeed * state.dataEnvrn->WeatherFileWindModCoeff *
           std::pow(Z / state.dataEnvrn->SiteWindBLHeight, state.dataEnvrn->SiteWindExp);
}

Real64 OutBaroPressAt(EnergyPlusData &state, Real64 const Z) // Height above ground (m)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Daeho Kang
    //       DATE WRITTEN   August 2009
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Calculates local air barometric pressure at a given altitude.

    // METHODOLOGY EMPLOYED:
    // U.S. Standard Atmosphere1976, Part 1, Chapter 1.3, Equation 33b.

    // REFERENCES:
    // U.S. Standard Atmosphere1976, Part 1, Chapter 1.3, Equation 33b.

    // Return value
    Real64 LocalAirPressure; // Return result for function (Pa)

    // FUNCTION PARAMETER DEFINITIONS:
    Real64 constexpr StdGravity(9.80665);    // The acceleration of gravity at the sea level (m/s2)
    Real64 constexpr AirMolarMass(0.028964); // Molar mass of Earth's air (kg/mol)
    Real64 constexpr GasConstant(8.31432);   // Molar gas constant (J/Mol-K)
    Real64 constexpr TempGradient(-0.0065);  // Molecular-scale temperature gradient (K/m)
    Real64 constexpr GeopotentialH(0.0);     // Geopotential height (zero within 11km from the sea level) (m)

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 BaseTemp; // Base temperature at Z

    BaseTemp = OutDryBulbTempAt(state, Z) + Constant::Kelvin;

    if (Z <= 0.0) {
        LocalAirPressure = 0.0;
    } else if (state.dataEnvrn->SiteTempGradient == 0.0) {
        LocalAirPressure = state.dataEnvrn->OutBaroPress;
    } else {
        LocalAirPressure = state.dataEnvrn->StdBaroPress * std::pow(BaseTemp / (BaseTemp + TempGradient * (Z - GeopotentialH)),
                                                                    (StdGravity * AirMolarMass) / (GasConstant * TempGradient));
    }

    return LocalAirPressure;
}

void SetOutBulbTempAt_error(EnergyPlusData &state, std::string const &Settings, Real64 const max_height, std::string const &SettingsName)
{
    // Using/Aliasing

    ShowSevereError(state, format("SetOutBulbTempAt: {} Outdoor Temperatures < -100 C", Settings));
    ShowContinueError(state, format("...check {} Heights - Maximum {} Height=[{:.0R}].", Settings, Settings, max_height));
    if (max_height >= 20000.0) {
        ShowContinueError(state, "...according to your maximum Z height, your building is somewhere in the Stratosphere.");
        ShowContinueError(state, format("...look at {} Name= {}", Settings, SettingsName));
    }
    ShowFatalError(state, "Program terminates due to preceding condition(s).");
}

void SetWindSpeedAt(EnergyPlusData const &state,
                    int const NumItems,
                    const Array1D<Real64> &Heights,
                    Array1D<Real64> &LocalWindSpeed,
                    [[maybe_unused]] std::string const &Settings)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   June 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Routine provides facility for doing bulk Set Windspeed at Height.

    if (state.dataEnvrn->SiteWindExp == 0.0) {
        LocalWindSpeed = state.dataEnvrn->WindSpeed;
    } else {
        Real64 const fac(state.dataEnvrn->WindSpeed * state.dataEnvrn->WeatherFileWindModCoeff *
                         std::pow(state.dataEnvrn->SiteWindBLHeight, -state.dataEnvrn->SiteWindExp));
        Real64 Z; // Centroid value
        for (int i = 1; i <= NumItems; ++i) {
            Z = Heights(i);
            if (Z <= 0.0) {
                LocalWindSpeed(i) = 0.0;
            } else {
                //  [Met] - at meterological Station, Height of measurement is usually 10m above ground
                //  LocalWindSpeed = Windspeed [Met] * (Wind Boundary LayerThickness [Met]/Height [Met])**Wind Exponent[Met] &
                //                     * (Height above ground / Site Wind Boundary Layer Thickness) ** Site Wind Exponent
                LocalWindSpeed(i) = fac * std::pow(Z, state.dataEnvrn->SiteWindExp);
            }
        }
    }
}

} // namespace EnergyPlus::DataEnvironment
