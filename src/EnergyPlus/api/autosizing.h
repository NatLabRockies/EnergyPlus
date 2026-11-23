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

#ifndef ENERGYPLUS_AUTOSIZING_H
#define ENERGYPLUS_AUTOSIZING_H

#include <EnergyPlus/api/EnergyPlusAPI.h>
#include <EnergyPlus/api/TypeDefs.h>
#include <EnergyPlus/api/state.h>

#ifdef __cplusplus
extern "C" {
#endif

/// \file autosizing.h
/// \brief This "Auto-sizing" API category provides access to auto-sizing calculations inside EnergyPlus.
/// \details Auto-sizing is an important aspect of an EnergyPlus simulation, allowing users to gather simulation results
///          for cases where the actual size of components and systems are not known ahead of time.

/// \brief This typedef is a convenient pointer to an internal sizer class.
/// \details Just one is declared here as any sizer object can fit in it.  An instance can be found by calling any
///          sizer***New() function
ENERGYPLUSLIB_API typedef void *Sizer;

/// \brief Gets warning and error messages from the autosizing process.
/// \details This function returns a char * which points to a string of error messages, or blank if none have been reported.
///          This function also clears the message buffer so a subsequent call will get only new messages.
/// \param[in] sizer An instance of a Sizer class, it works for any sizer type.
/// \return A char * pointing to a string of error messages, or blank if none have been reported.
ENERGYPLUSLIB_API char *sizerGetLastErrorMessages(Sizer sizer);

/// \brief Returns a new reference to a HeatingAirflowUA Sizer class
/// \details The sizer class allows access to sizer stuff.
/// \returns This functions allocates a new sizer class and returns a Sizer, which is a pointer to that new instance.
/// \remark In API applications, when the calling script is done with the sizer instance, call `sizerHeatingAirflowUADelete` to clean up the instance.
/// \see Sizer
/// \see sizerHeatingAirflowUADelete
ENERGYPLUSLIB_API Sizer sizerHeatingAirflowUANew();
/// \brief Deletes an instance of a HeatingAirflowUA Sizer class
/// \details When an instance of a Sizer class is created using `sizerHeatingAirflowUANew`, it should be cleaned up when totally done with it.
/// \param[in] sizer An instance of a Sizer class to be deleted.  The Sizer class is initially created by calling `sizerHeatingAirflowUANew`.
/// \see Sizer
/// \see sizerHeatingAirflowUANew
ENERGYPLUSLIB_API void sizerHeatingAirflowUADelete(Sizer sizer);
/// \brief Defines the different possible zone sizing configurations
enum HeatingAirflowUAZoneConfigType
{
    HeatingAirflowUAZoneTerminal = 0,
    HeatingAirflowUAZoneInductionUnit = 1,
    HeatingAirflowUAZoneFanCoil = 2
};
/// \brief Initializes the HeatingAirflowUA sizer class for zone configurations
/// \details This function allows the sizer to be used for zone sizing calculations, supporting
///          the configurations defined in the `HeatingAirflowUAZoneConfigType` enumeration.
/// \param[in] state An instance of an EnergyPlusState that must be created with a call to `stateNew`.
/// \param[in] sizer An instance of a Sizer class, which can be created by calling `sizerHeatingAirflowUANew`.
/// \param[in] zoneConfig One zone configuration type from the `HeatingAirflowUAZoneConfigType` enumeration
/// \param[in] elevation The elevation above sea level for evaluating fluid properties, in m
/// \param[in] representativeFlowRate The flow rate used for this configuration type.
///            - Terminal: this flow rate should be the base volume flow rate in the terminal unit
///            - InductionUnit: this flow rate should be the base volume flow rate in the unit before any multipliers are applied
///            - FanCoil: this flow rate should be the base heating volume flow rate in the fan coil unit
/// \param[in] reheatMultiplier Define the reheat multiplier in induction unit configs, for other configs it will be ignored
/// \remark This or another initialization function must be called prior to each call to sizerHeatingAirflowUASize
/// \see Sizer
/// \see stateNew
/// \see sizerHeatingAirflowUASize
/// \see sizerHeatingAirflowUADelete
/// \see HeatingAirflowUAZoneConfigType
ENERGYPLUSLIB_API void sizerHeatingAirflowUAInitializeForZone(EnergyPlusState state,
                                                              Sizer sizer,
                                                              enum HeatingAirflowUAZoneConfigType zoneConfig,
                                                              Real64 elevation,
                                                              Real64 representativeFlowRate,
                                                              Real64 reheatMultiplier);
/// \brief Defines the different possible system sizing configurations
enum HeatingAirflowUASystemConfigType
{
    HeatingAirflowUASystemConfigTypeOutdoorAir = 0,
    HeatingAirflowUASystemConfigTypeMainDuct = 1,
    HeatingAirflowUASystemConfigTypeCoolingDuct = 2,
    HeatingAirflowUASystemConfigTypeHeatingDuct = 3,
    HeatingAirflowUASystemConfigTypeOtherDuct = 4
};
/// \brief Initializes the HeatingAirflowUA sizer class for system configurations
/// \details This function allows the sizer to be used for system sizing calculations, supporting
///          the configurations defined in the `HeatingAirflowUASystemConfigType` enumeration.
/// \param[in] state An instance of an EnergyPlusState that must be created with a call to `stateNew`.
/// \param[in] sizer An instance of a Sizer class, which can be created by calling `sizerHeatingAirflowUANew`.
/// \param[in] sysConfig One System configuration type from the `HeatingAirflowUASystemConfigType` enumeration
/// \param[in] elevation The elevation above sea level for evaluating fluid properties, in m
/// \param[in] representativeFlowRate The flow rate used for this configuration type.
///            - OutdoorAir: this flow rate should be the base mass flow rate in the outdoor air loop
///            - MainDuct: this flow rate should be the base volume flow in the main air loop
///            - CoolingDuct: this flow rate should be the BLAHBLAHBLAH
///            - HeatingDuct: this flow rate should be the BLAHBLAHBLAH
///            - OtherDuct: this flow rate should be the BLAHBLAHBLAH
/// \param[in] minFlowRateRatio Define the ratio for BLAHBLAHBLAH configs, not used in other configs
/// \param[in] DOAS Specify if this coil is placed in a DOAS loop (1 = true, 0 = false)
/// \remark This or another initialization function must be called prior to each call to sizerHeatingAirflowUASize
/// \see Sizer
/// \see stateNew
/// \see sizerHeatingAirflowUASize
/// \see sizerHeatingAirflowUADelete
/// \see HeatingAirflowUASystemConfigType
ENERGYPLUSLIB_API void sizerHeatingAirflowUAInitializeForSystem(EnergyPlusState state,
                                                                Sizer sizer,
                                                                enum HeatingAirflowUASystemConfigType sysConfig,
                                                                Real64 elevation,
                                                                Real64 representativeFlowRate,
                                                                Real64 minFlowRateRatio,
                                                                int DOAS);
/// \brief Does calculation of the HeatingAirflowUA sizer
/// \param[in] state An instance of an EnergyPlusState that must be created with a call to `stateNew`.
/// \param[in] sizer An instance of a HeatingAirflowUA Sizer class, which can be created by calling `sizerHeatingAirflowUANew`.
/// \returns This function returns true (0) if the autosizing calculation was successful, or false (1) if not.
/// \see Sizer
/// \see stateNew
/// \see sizerHeatingAirflowUANew
ENERGYPLUSLIB_API int sizerHeatingAirflowUASize(EnergyPlusState state, Sizer sizer);
/// \brief Returns the resulting autosized value after sizerHeatingAirflowUASize() is called.
/// \param[in] sizer An instance of a HeatingAirflowUA Sizer class, which can be created by calling `sizerHeatingAirflowUANew`.
/// \returns Autosized Heating Air Mass Flow Rate, in kg/s
/// \see Sizer
/// \see sizerHeatingAirflowUANew
/// \see sizerHeatingAirflowUASize
ENERGYPLUSLIB_API Real64 sizerHeatingAirflowUAValue(Sizer sizer);

#ifdef __cplusplus
}
#endif

#endif // ENERGYPLUS_AUTOSIZING_H
