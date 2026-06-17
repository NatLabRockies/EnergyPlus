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

#ifndef EnergyPlusAPIFunctional_h_INCLUDED
#define EnergyPlusAPIFunctional_h_INCLUDED

#include <EnergyPlus/api/EnergyPlusAPI.h>
#include <EnergyPlus/api/TypeDefs.h>
#include <EnergyPlus/api/state.h>

#ifdef __cplusplus

#    include <functional>
#    include <string>

ENERGYPLUSLIB_API void registerErrorCallback(EnergyPlusState state, std::function<void(EnergyPlus::Error, const std::string &)> f);

extern "C" {
#endif

/// \file func.h
/// \brief This "functional" API category provides access to things which do not require a simulation to be running.
/// \details Currently this includes accessing API version information, and looking up glycol, refrigerant, and psychrometric
///          properties.  Each category has some limitations as this is the first version of the API, but more functionality
///          will be introduced throughout development cycles.  The functionality of this API category is available
///          without making a call to run EnergyPlus, however an `EnergyPlusState` object is still required for the
///          API calls.  The state object can be instantiated with a call to `stateNew` and managed with methods in the
///          state API category.
/// \remark Make sure to call `initializeFunctionalAPI` prior to calling any functional API methods to ensure the internal
///         structures are in place in the code.
/// \remark Glycol routines currently only operate on pure water; a future version will enable further property lookups.
/// \remark Refrigerant routines currently only operate on steam; a future version will enable further property lookups.
/// \see EnergyPlusState
/// \see stateNew
/// \see initializeFunctionalAPI

/// \brief An initialization routine that sets up all functional API structures
/// \details The functional API exposes classes and methods that can be used for outside calculations.
///          Some of these calculations rely on internal structures being allocated and set up.
///          Whenever any script is going to use a method from this functional API, this method must be called.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark When using the functional API from a Python Plugin workflow, this function should _not_ be called.
ENERGYPLUSLIB_API void initializeFunctionalAPI(EnergyPlusState state);
/// \brief Returns the version of this API, in Major.Minor form.
/// \details Over time, the API will evolve.  In most cases, there won't be breaking changes, just additional
///          functionality.  When improvements are made, but they don't break functionality, the minor number will be
///          incremented.  If a breaking change ever occurs, the major number will be incremented.
///          This function can be used by users as a way to target a specific major version of the API and avoid
///          problems if the API ever changes.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
ENERGYPLUSLIB_API const char *apiVersionFromEPlus(EnergyPlusState state);
/// \brief Returns the version of EnergyPlus currently running in string form
ENERGYPLUSLIB_API const char *energyPlusVersion();
/// \brief Allows a user to register an error callback function.
/// \details If a user script registers a callback function here, then when EnergyPlus is sending an error message to
///          the error file, it will also send it here.  The user function will then have the ability to act on the
///          error message if needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f A function that accepts an integer error level and an error string, and will be called by EnergyPlus when an error is emitted.
/// \remark A future version of this method will enable additional functionality including an argument indicating the
///         error type, and allowing the return value from this callback to determine how EnergyPlus should behave.
ENERGYPLUSLIB_API void registerErrorCallback(EnergyPlusState state, void (*f)(int, const char *errorMessage));

/// \brief This typedef is a convenient pointer to an internal glycol property class inside EnergyPlus.
/// \details In a default EnergyPlus simulation, pure water properties are available directly.  To access properties of
///          ethylene glycol or propylene glycol, user-input is required.  For the current time, the only glycol available
///          through the API is pure water.  Accessing the glycol API is initiated using the `glycolNew` function.
/// \see glycolNew
ENERGYPLUSLIB_API typedef void *Glycol;
/// \brief Returns a new reference to a Glycol class
/// \details The glycol class allows access to fluid properties.  Eventually ethylene and propylene glycol properties will
///          be made available but for now the only fluid is pure water.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] glycolName The name of the glycol.  Currently the only valid option is "water".  In future versions of the
///                       API, the function will accept other fluid types and glycol mixtures.
/// \returns This functions allocates a new glycol class and returns a Glycol, which is a pointer to that new instance.
/// \remark In API applications, when the calling script is done with the glycol instance, call `glycolDelete` to clean up the instance.
/// \see Glycol
/// \see glycolDelete
ENERGYPLUSLIB_API Glycol glycolNew(EnergyPlusState state, const char *glycolName);
/// \brief Deletes an instance of a Glycol class
/// \details When an instance of a Glycol class is created using `glycolNew`, it should be cleaned up when totally done with it.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] glycol An instance of a Glycol class to be deleted.  The Glycol class is initially created by calling `glycolNew`.
/// \see Glycol
/// \see glycolNew
ENERGYPLUSLIB_API void glycolDelete(EnergyPlusState state, Glycol glycol);
/// \brief Returns the fluid specific heat for the given Glycol instance at the specified temperature.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] glycol An instance of a Glycol class, which can be created by calling `glycolNew`.
/// \param[in] temperature The fluid temperature for property evaluation, in degrees Celsius.
/// \returns Fluid specific heat, in J/kgK
/// \see Glycol
/// \see glycolNew
ENERGYPLUSLIB_API Real64 glycolSpecificHeat(EnergyPlusState state, Glycol glycol, Real64 temperature);
/// \brief Returns the fluid density for the given Glycol instance at the specified temperature.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] glycol An instance of a Glycol class, which can be created by calling `glycolNew`.
/// \param[in] temperature The fluid temperature for property evaluation, in degrees Celsius.
/// \returns Fluid density, in kg/m3
/// \see Glycol
/// \see glycolNew
ENERGYPLUSLIB_API Real64 glycolDensity(EnergyPlusState state, Glycol glycol, Real64 temperature);
/// \brief Returns the fluid thermal conductivity for the given Glycol instance at the specified temperature.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] glycol An instance of a Glycol class, which can be created by calling `glycolNew`.
/// \param[in] temperature The fluid temperature for property evaluation, in degrees Celsius.
/// \returns Fluid thermal conductivity, in W/mK
/// \see Glycol
/// \see glycolNew
ENERGYPLUSLIB_API Real64 glycolConductivity(EnergyPlusState state, Glycol glycol, Real64 temperature);
/// \brief Returns the fluid dynamic viscosity for the given Glycol instance at the specified temperature.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] glycol An instance of a Glycol class, which can be created by calling `glycolNew`.
/// \param[in] temperature The fluid temperature for property evaluation, in degrees Celsius.
/// \returns Fluid dynamic viscosity, in Pa-s (or kg/m-s)
/// \see Glycol
/// \see glycolNew
ENERGYPLUSLIB_API Real64 glycolViscosity(EnergyPlusState state, Glycol glycol, Real64 temperature);

/// \brief This typedef is a convenient pointer to an internal refrigerant property class inside EnergyPlus.
/// \details In a default EnergyPlus simulation, refrigerant properties for steam are available directly.  To access properties of
///          other refrigerants, user-input is required.  For the current time, the only refrigerant available
///          through the API is pure steam.  Accessing the refrigerant API is initiated using the `refrigerantNew` function.
/// \see refrigerantNew
ENERGYPLUSLIB_API typedef void *Refrigerant;
/// \brief Returns a new reference to a Refrigerant class
/// \details The refrigerant class allows access to refrigerant properties.  Eventually more refrigerant properties will
///          be made available but for now the only refrigerant is pure steam.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] refrigerantName The name of the refrigerant.  Currently the only valid option is "steam".  In future versions of the
///                            API, the function will accept other refrigerant types.
/// \returns This functions allocates a new refrigerant class and returns a Refrigerant, which is a pointer to that new instance.
/// \remark In API applications, when the calling script is done with the refrigerant instance, call `refrigerantDelete` to clean up the instance.
/// \see Refrigerant
/// \see refrigerantDelete
ENERGYPLUSLIB_API Refrigerant refrigerantNew(EnergyPlusState state, const char *refrigerantName);
/// \brief Deletes an instance of a Refrigerant class
/// \details When an instance of a Refrigerant class is created using `refrigerantNew`, it should be cleaned up when totally done with it.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] refrigerant An instance of a Refrigerant class to be deleted.  The Refrigerant class is initially created by calling `refrigerantNew`.
/// \see Refrigerant
/// \see refrigerantNew
ENERGYPLUSLIB_API void refrigerantDelete(EnergyPlusState state, Refrigerant refrigerant);
/// \brief Returns the refrigerant saturation pressure for the given Refrigerant instance at the specified temperature.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] refrigerant An instance of a Refrigerant class, which can be created by calling `refrigerantNew`.
/// \param[in] temperature The refrigerant temperature for property evaluation, in degrees Celsius.
/// \returns Refrigerant saturation pressure, in Pa
/// \see Refrigerant
/// \see refrigerantNew
ENERGYPLUSLIB_API Real64 refrigerantSaturationPressure(EnergyPlusState state, Refrigerant refrigerant, Real64 temperature);
/// \brief Returns the refrigerant saturation temperature for the given Refrigerant instance at the specified pressure.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] refrigerant An instance of a Refrigerant class, which can be created by calling `refrigerantNew`.
/// \param[in] pressure The refrigerant pressure for property evaluation, in Pa.
/// \returns Refrigerant saturation temperature, in C
/// \see Refrigerant
/// \see refrigerantNew
ENERGYPLUSLIB_API Real64 refrigerantSaturationTemperature(EnergyPlusState state, Refrigerant refrigerant, Real64 pressure);
/// \brief Returns the refrigerant saturated enthalpy for the given Refrigerant instance at the specified temperature and quality.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] refrigerant An instance of a Refrigerant class, which can be created by calling `refrigerantNew`.
/// \param[in] temperature The refrigerant temperature for property evaluation, in C.
/// \param[in] quality The refrigerant quality for property evaluation, in fractional form from 0.0 to 1.0.
/// \returns Refrigerant saturated enthalpy in J/kg
/// \see Refrigerant
/// \see refrigerantNew
ENERGYPLUSLIB_API Real64 refrigerantSaturatedEnthalpy(EnergyPlusState state, Refrigerant refrigerant, Real64 temperature, Real64 quality);
/// \brief Returns the refrigerant saturated density for the given Refrigerant instance at the specified temperature and quality.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] refrigerant An instance of a Refrigerant class, which can be created by calling `refrigerantNew`.
/// \param[in] temperature The refrigerant temperature for property evaluation, in C.
/// \param[in] quality The refrigerant quality for property evaluation, in fractional form from 0.0 to 1.0.
/// \returns Refrigerant saturated density in kg/m3
/// \see Refrigerant
/// \see refrigerantNew
ENERGYPLUSLIB_API Real64 refrigerantSaturatedDensity(EnergyPlusState state, Refrigerant refrigerant, Real64 temperature, Real64 quality);
/// \brief Returns the refrigerant saturated specific heat for the given Refrigerant instance at the specified temperature and quality.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] refrigerant An instance of a Refrigerant class, which can be created by calling `refrigerantNew`.
/// \param[in] temperature The refrigerant temperature for property evaluation, in C.
/// \param[in] quality The refrigerant quality for property evaluation, in fractional form from 0.0 to 1.0.
/// \returns Refrigerant saturated enthalpy in J/kg-K
/// \see Refrigerant
/// \see refrigerantNew
ENERGYPLUSLIB_API Real64 refrigerantSaturatedSpecificHeat(EnergyPlusState state, Refrigerant refrigerant, Real64 temperature, Real64 quality);
// ENERGYPLUSLIB_API Real64 refrigerantSuperHeatedEnthalpy(Refrigerant, Real64 temperature, Real64 pressure);
// ENERGYPLUSLIB_API Real64 refrigerantSuperHeatedPressure(Refrigerant, Real64 temperature, Real64 enthalpy);
// ENERGYPLUSLIB_API Real64 refrigerantSuperHeatedDensity(Refrigerant, Real64 temperature, Real64 pressure);

/// \brief Returns the psychrometric density at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] pb Barometric pressure, in Pa
/// \param[in] tdb Dry bulb temperature, in Celsius
/// \param[in] dw Humidity ratio, in kgWater/kgDryAir
/// \returns Psychrometric density, in kg/m3
ENERGYPLUSLIB_API Real64 psyRhoFnPbTdbW(EnergyPlusState state, Real64 pb, Real64 tdb, Real64 dw);
/// \brief Returns the psychrometric latent energy of air at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] T Dry bulb temperature, in Celsius
/// \returns Psychrometric latent energy of air, in J/kg
ENERGYPLUSLIB_API Real64 psyHfgAirFnWTdb(EnergyPlusState state, Real64 T);
/// \brief Returns the psychrometric latent energy of the moisture as a gas in the air at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] T Dry bulb temperature, in Celsius
/// \returns Psychrometric latent energy of the moisture as a gas in the air, in J/kg
ENERGYPLUSLIB_API Real64 psyHgAirFnWTdb(EnergyPlusState state, Real64 T);
/// \brief Returns the psychrometric enthalpy at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] TDB Dry bulb temperature, in Celsius
/// \param[in] W Humidity ratio, in kgWater/kgDryAir
/// \returns Psychrometric enthalpy, in J/kg
ENERGYPLUSLIB_API Real64 psyHFnTdbW(EnergyPlusState state, Real64 TDB, Real64 W);
/// \brief Returns the psychrometric specific heat at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] W Humidity ratio, in kgWater/kgDryAir
/// \returns Psychrometric specific heat, in J/kg-K
ENERGYPLUSLIB_API Real64 psyCpAirFnW(EnergyPlusState state, Real64 W);
/// \brief Returns the psychrometric dry bulb temperature at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] H Enthalpy, in J/kg
/// \param[in] W Humidity ratio, in kgWater/kgDryAir
/// \returns Psychrometric dry bulb temperature, in Celsius
ENERGYPLUSLIB_API Real64 psyTdbFnHW(EnergyPlusState state, Real64 H, Real64 W);
/// \brief Returns the psychrometric vapor density at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] Tdb Dry bulb temperature, in Celsius
/// \param[in] W Humidity ratio, in kgWater/kgDryAir
/// \param[in] PB Barometric pressure, in Pascals
/// \returns Psychrometric vapor density, in kg/m3
ENERGYPLUSLIB_API Real64 psyRhovFnTdbWPb(EnergyPlusState state, Real64 Tdb, Real64 W, Real64 PB);
/// \brief Returns the psychrometric wet bulb temperature at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] Tdb Dry bulb temperature, in Celsius
/// \param[in] W Humidity ratio, in kgWater/kgDryAir
/// \param[in] PB Barometric pressure, in Pascals
/// \returns Psychrometric wet bulb temperature, in Celsius
ENERGYPLUSLIB_API Real64 psyTwbFnTdbWPb(EnergyPlusState state, Real64 Tdb, Real64 W, Real64 PB);
/// \brief Returns the psychrometric specific volume at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] TDB Dry bulb temperature, in Celsius
/// \param[in] W Humidity ratio, in kgWater/kgDryAir
/// \param[in] PB Barometric pressure, in Pascals
/// \returns Psychrometric specific volume, in m3/kg
ENERGYPLUSLIB_API Real64 psyVFnTdbWPb(EnergyPlusState state, Real64 TDB, Real64 W, Real64 PB);
/// \brief Returns the psychrometric humidity ratio at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] TDB Dry bulb temperature, in Celsius
/// \param[in] H Enthalpy, in J/kg
/// \returns Psychrometric humidity ratio, in kgWater/kgDryAir
ENERGYPLUSLIB_API Real64 psyWFnTdbH(EnergyPlusState state, Real64 TDB, Real64 H);
/// \brief Returns the psychrometric saturation pressure at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] T Dry bulb temperature, in Celsius
/// \returns Psychrometric saturation pressure, in Pascals
ENERGYPLUSLIB_API Real64 psyPsatFnTemp(EnergyPlusState state, Real64 T);
/// \brief Returns the psychrometric saturation temperature at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] H Enthalpy, in J/kg
/// \param[in] PB Barometric pressure, in Pascals
/// \returns Psychrometric saturation temperature, in Celsius
ENERGYPLUSLIB_API Real64 psyTsatFnHPb(EnergyPlusState state, Real64 H, Real64 PB);
/// \brief Returns the psychrometric vapor density at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] Tdb Dry bulb temperature, in Celsius
/// \param[in] RH Relative humidity, as a fraction from 0.0 to 1.0
/// \returns Psychrometric vapor density, in kg/m3
ENERGYPLUSLIB_API Real64 psyRhovFnTdbRh(EnergyPlusState state, Real64 Tdb, Real64 RH);
/// \brief Returns the psychrometric relative humidity at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] Tdb Dry bulb temperature, in Celsius
/// \param[in] Rhovapor Vapor density, in kg/m3
/// \returns Psychrometric relative humidity, as a fraction from 0.0 to 1.0
ENERGYPLUSLIB_API Real64 psyRhFnTdbRhov(EnergyPlusState state, Real64 Tdb, Real64 Rhovapor);
/// \brief Returns the psychrometric relative humidity at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] TDB Dry bulb temperature, in Celsius
/// \param[in] W Humidity ratio, in kgWater/kgDryAir
/// \param[in] PB Barometric pressure, in Pascals
/// \returns Psychrometric relative humidity, as a fraction from 0.0 to 1.0
ENERGYPLUSLIB_API Real64 psyRhFnTdbWPb(EnergyPlusState state, Real64 TDB, Real64 W, Real64 PB);
/// \brief Returns the psychrometric humidity ratio at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] TDP Dew point temperature, in Celsius
/// \param[in] PB Barometric pressure, in Pascals
/// \returns Psychrometric humidity ratio, in kgWater/kgDryAir
ENERGYPLUSLIB_API Real64 psyWFnTdpPb(EnergyPlusState state, Real64 TDP, Real64 PB);
/// \brief Returns the psychrometric humidity ratio at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] TDB Dry bulb temperature, in Celsius
/// \param[in] RH Relative humidity ratio, as a fraction from 0.0 to 1.0
/// \param[in] PB Barometric pressure, in Pascals
/// \returns Psychrometric humidity ratio, in kgWater/kgDryAir
ENERGYPLUSLIB_API Real64 psyWFnTdbRhPb(EnergyPlusState state, Real64 TDB, Real64 RH, Real64 PB);
/// \brief Returns the psychrometric humidity ratio at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] TDB Dry bulb temperature, in Celsius
/// \param[in] TWB Wet bulb temperature, in Celsius
/// \param[in] PB Barometric pressure, in Pascals
/// \returns Psychrometric humidity ratio, in kgWater/kgDryAir
ENERGYPLUSLIB_API Real64 psyWFnTdbTwbPb(EnergyPlusState state, Real64 TDB, Real64 TWB, Real64 PB);
/// \brief Returns the psychrometric enthalpy at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] TDB Dry bulb temperature, in Celsius
/// \param[in] RH Relative humidity ratio, as a fraction from 0.0 to 1.0
/// \param[in] PB Barometric pressure, in Pascals
/// \returns Psychrometric enthalpy, in J/kg
ENERGYPLUSLIB_API Real64 psyHFnTdbRhPb(EnergyPlusState state, Real64 TDB, Real64 RH, Real64 PB);
/// \brief Returns the psychrometric dew point temperature at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] W Humidity ratio, in kgWater/kgDryAir
/// \param[in] PB Barometric pressure, in Pascals
/// \returns Psychrometric dew point temperature, in Celsius
ENERGYPLUSLIB_API Real64 psyTdpFnWPb(EnergyPlusState state, Real64 W, Real64 PB);
/// \brief Returns the psychrometric dew point temperature at given conditions.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] TDB Dry bulb temperature, in Celsius
/// \param[in] TWB Wet bulb temperature, in Celsius
/// \param[in] PB Barometric pressure, in Pascals
/// \returns Psychrometric dew point temperature, in Celsius
ENERGYPLUSLIB_API Real64 psyTdpFnTdbTwbPb(EnergyPlusState state, Real64 TDB, Real64 TWB, Real64 PB);

#ifdef __cplusplus
}
#endif

#endif // EnergyPlusAPIFunctional_h_INCLUDED
