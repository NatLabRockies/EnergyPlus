// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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

#ifndef EnergyPlusAPIRuntime_h_INCLUDED
#define EnergyPlusAPIRuntime_h_INCLUDED

#include <EnergyPlus/api/EnergyPlusAPI.h>
#include <EnergyPlus/api/TypeDefs.h>
#include <EnergyPlus/api/state.h>

#ifdef __cplusplus

// if we are building this with C++ in mind, we can make use of these std::function based callback registration functions
#    include <functional>
#    include <string>
ENERGYPLUSLIB_API void callbackBeginNewEnvironment(EnergyPlusState state, std::function<void(EnergyPlusState)> f);
ENERGYPLUSLIB_API void callbackAfterNewEnvironmentWarmupComplete(EnergyPlusState state, std::function<void(EnergyPlusState)> f);
ENERGYPLUSLIB_API void callbackBeginZoneTimeStepBeforeInitHeatBalance(EnergyPlusState state, std::function<void(EnergyPlusState)> f);
ENERGYPLUSLIB_API void callbackBeginZoneTimeStepAfterInitHeatBalance(EnergyPlusState state, std::function<void(EnergyPlusState)> f);
ENERGYPLUSLIB_API void callbackBeginZoneTimestepBeforeSetCurrentWeather(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f);
ENERGYPLUSLIB_API void callbackBeginTimeStepBeforePredictor(EnergyPlusState state, std::function<void(EnergyPlusState)> f);
ENERGYPLUSLIB_API void callbackAfterPredictorBeforeHVACManagers(EnergyPlusState state, std::function<void(EnergyPlusState)> f);
ENERGYPLUSLIB_API void callbackAfterPredictorAfterHVACManagers(EnergyPlusState state, std::function<void(EnergyPlusState)> f);
ENERGYPLUSLIB_API void callbackInsideSystemIterationLoop(EnergyPlusState state, std::function<void(EnergyPlusState)> f);
ENERGYPLUSLIB_API void callbackEndOfZoneTimeStepBeforeZoneReporting(EnergyPlusState state, std::function<void(EnergyPlusState)> f);
ENERGYPLUSLIB_API void callbackEndOfZoneTimeStepAfterZoneReporting(EnergyPlusState state, std::function<void(EnergyPlusState)> f);
ENERGYPLUSLIB_API void callbackEndOfSystemTimeStepBeforeHVACReporting(EnergyPlusState state, std::function<void(EnergyPlusState)> f);
ENERGYPLUSLIB_API void callbackEndOfSystemTimeStepAfterHVACReporting(EnergyPlusState state, std::function<void(EnergyPlusState)> f);
ENERGYPLUSLIB_API void callbackEndOfZoneSizing(EnergyPlusState state, std::function<void(EnergyPlusState)> f);
ENERGYPLUSLIB_API void callbackEndOfSystemSizing(EnergyPlusState state, std::function<void(EnergyPlusState)> f);
ENERGYPLUSLIB_API void callbackEndOfAfterComponentGetInput(EnergyPlusState state, std::function<void(EnergyPlusState)> f);
ENERGYPLUSLIB_API void callbackUnitarySystemSizing(EnergyPlusState state, const std::function<void(EnergyPlusState)> &f);
ENERGYPLUSLIB_API void registerStdOutCallback(EnergyPlusState state, std::function<void(const std::string &)>);
ENERGYPLUSLIB_API void registerProgressCallback(EnergyPlusState state, std::function<void(int const)>);
ENERGYPLUSLIB_API void registerExternalHVACManager(EnergyPlusState state, std::function<void(EnergyPlusState)> f);
ENERGYPLUSLIB_API void callbackUserDefinedComponentModel(EnergyPlusState state, std::function<void(EnergyPlusState)> f, const char *programName);

extern "C" {

#endif // __cplusplus

/// \file runtime.h
/// \brief This "runtime" API category provides access to link into a running simulation by providing callback functions.
/// \details While an EnergyPlus simulation is running, registered callback functions are "called back" at user-specified
///          points in the simulation.  When a function is called back, it can then leverage other APIs to do program
///          manipulation.  For example, it could leverage the functional API to look up a fluid property value, then
///          use the data exchange API to assign a new control property on a component using an actuator.
///          At the heart of the API functionality is the use of an `EnergyPlusState` object, which is required to be
///          passed into nearly every API function.  The client should get a new state instance by calling `newState`,
///          then use this new state to make API calls, including registering client functions as callbacks.
///          Once the callback functions have been registered, the client should then call the main `energyplus` function
///          to initiate a new run.  If multiple runs are to be performed in series, the `stateReset` function can be
///          used to "reset" the state.  There are also methods to allow issuing warnings through internal EnergyPlus
///          methods and to callback with updates on simulation progress and output messages.
/// \see energyplus
/// \see EnergyPlusState
/// \see stateNew
/// \see stateReset

/// \brief Runs an EnergyPlus simulation
/// \details This function launches an EnergyPlus simulation using the given arguments.  The first argument is an
///          EnergyPlus "state", which is used for storing a running simulation state.  The rest of the arguments are
///          identical to the command line for EnergyPlus(.exe).  Prior to calling this function, it is expected that
///          most workflows will register callback functions to be called back from EnergyPlus at user-specified points
///          in the simulation.  If no callbacks are registered, EnergyPlus will proceed as in a regular command line
///          fashion and return when complete.  An example usage of this API endpoint is as follows:
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] argc Size of argument list
/// \param[in] argv Argument list to be passed to EnergyPlus
/// \code{.c}
///   void progressCallback(EnergyPlusState state, int const i) {
///     printf("Updated progress: %d", i);
///   }
///   int main(int argc, const char * argv[]) {
///     EnergyPlusState state = stateNew();
///     registerProgressCallback(state, progressCallback);
///     energyplus(state, argc, argv);
///     stateDelete(state);
///   }
/// \endcode
/// \remark If this function is called multiple times in the same thread, the client must call `stateReset` in
///         between each run, or unexpected errors will occur.
/// \remark Prior to calling this function, it is also expected that most workflows will want to use the data exchange API
///         function `requestVariable` to pre-request Output:Variables to make them accessible in the data exchange functions.
/// \see stateReset
/// \see requestVariable
/// \see stateNew
ENERGYPLUSLIB_API int energyplus(EnergyPlusState state, int argc, const char *argv[]);

/// \brief Stop an EnergyPlus simulation early
/// \details This function can be used by an API client to end a simulation before the end of the scheduled run periods.
///          The simulation will end gracefully and all output files will be finalized and closed before the
///          EnergyPlus process is complete.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
ENERGYPLUSLIB_API void stopSimulation(EnergyPlusState state);

/// \brief Allows disabling and enabling console output (stdout and stderr) during a run.
/// \details Normally EnergyPlus will print lots of status update messages while running.  These can get in the way
///          when calling EnergyPlus as a library.  This function allows the client to mute (and unmute) the standard output messages.
///          Note that output messages can still be retrieved using callback functionality, even with the console output is muted.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] outputStatus An integer describing whether the console output should be printed: nonzero integer for yes, 0 for no.
ENERGYPLUSLIB_API void setConsoleOutputState(EnergyPlusState state, int outputStatus);

/// \brief Allows setting the EnergyPlus install root folder when calling EnergyPlus as a library (call this prior to running EnergyPlus)
/// \details When EnergyPlus is run as a standalone executable, it is able to locate auxiliary tools relative to the running binary exe file.
///          However, when calling as an API, the running binary will not be in the EnergyPlus install, and so EnergyPlus will fail to find those
///          auxiliary tools.  To call these tools when running as a library, the install root must be set using this function.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] path Path to the EnergyPlus install root, for example "C:\\EnergyPlus-9-5-0", or "/usr/local/EnergyPlus-9-5-0"
/// \remark It should be noted that in many workflows, it would be better to just call those auxiliary tools directly from an outside caller, rather
///         than relying on EnergyPlus to do it via command line arguments.
ENERGYPLUSLIB_API void setEnergyPlusRootDirectory(EnergyPlusState state, const char *path);

/// \brief Asks EnergyPlus to issue a warning message to the error file.
/// \details During an EnergyPlus simulation, if certain conditions arise, it may be useful to alert the user using
///          this function, which will issue a warning note in the standard error file and continue the simulation.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] message The warning message to be issued during the simulation.
/// \remark The usefulness of this function during API applications is questionable, however it is highly valuable
///         during Python Plugin applications.  In these applications, the user is still interfacing with EnergyPlus
///         as a black-box program and will rely on output messages primarily through EnergyPlus output files.  Using
///         this function to issue a message using standard EnergyPlus techniques will make the process familiar.
ENERGYPLUSLIB_API void issueWarning(EnergyPlusState state, const char *message); // Issue the warning text to the err file
/// \brief Asks EnergyPlus to issue a severe message to the error file.
/// \details During an EnergyPlus simulation, if certain conditions arise, it may be useful to alert the user using
///          this function, which will issue a severe error note in the standard error file and continue the simulation.
///          Severe errors should lead to program termination, so this should be followed with aborting the simulation in
///          one of these ways:
///          - If running as a Python Plugin, the function should return 1 to inform EnergyPlus to throw a fatal.
///          - If running as an API, the client should abort however is appropriate.  If needed, `stateReset` can be
///            called to reinitialize the program.
///
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] message The severe error message to be issued during the simulation.
/// \remark The usefulness of this function during API applications is questionable, however it is highly valuable
///         during Python Plugin applications.  In these applications, the user is still interfacing with EnergyPlus
///         as a black-box program and will rely on output messages primarily through EnergyPlus output files.  Using
///         this function to issue a message using standard EnergyPlus techniques will make the process familiar.
ENERGYPLUSLIB_API void issueSevere(EnergyPlusState state, const char *message); // Issue the severe text to the err file
/// \brief Asks EnergyPlus to issue a plain text message to the error file.
/// \details During an EnergyPlus simulation, if certain conditions arise, it may be useful to send information to the
///          user with this function, either to provide standard information, or supplemental information to a previously
///          sent warning or error.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] message The text message to be issued during the simulation.
/// \remark The usefulness of this function during API applications is questionable, however it is highly valuable
///         during Python Plugin applications.  In these applications, the user is still interfacing with EnergyPlus
///         as a black-box program and will rely on output messages primarily through EnergyPlus output files.  Using
///         this function to issue a message using standard EnergyPlus techniques will make the process familiar.
ENERGYPLUSLIB_API void issueText(EnergyPlusState state, const char *message); // Issue additional supporting text to the err file

/// \brief Register a callback function to receive updates on simulation progress
/// \details During an EnergyPlus simulation, the progress of the simulation will move from zero to one hundred percent.
///          This function can be used to get updates during the simulation that could be used to better provide
///          feedback to the user, perhaps in the form of a progress bar.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back during the simulation.  The function should expect one constant integer
///              argument, which will be the percent complete for the current simulation run.
/// \remark This function has limited usefulness in Python Plugin applications, but is highly valuable for being able
///         to report progress during an API workflow simulation.
/// \deprecated Note that in a future release, the callback function will be changed to also take a state argument.
ENERGYPLUSLIB_API void registerProgressCallback(EnergyPlusState state, void (*f)(int const));
/// \brief Register a callback function to receive standard output messages being sent from the simulation.
/// \details During an EnergyPlus simulation, there are a number of output messages being sent to standard output on the
///          command line.  This function can be used to get copies of those messages during the simulation which could
///          be captured and better presented to the user during an API workflow.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back during the simulation.  The function should expect one constant char *
///              argument, which will be the message being sent to standard output.
/// \remark This function has limited usefulness in Python Plugin applications, but is highly valuable for being able
///         to report messages during an API workflow simulation.
/// \deprecated Note that in a future release, the callback function will be changed to also take a state argument.
ENERGYPLUSLIB_API void registerStdOutCallback(EnergyPlusState state, void (*f)(const char *message));

/// \brief Register a callback function to be called at the beginning of each new environment.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
ENERGYPLUSLIB_API void callbackBeginNewEnvironment(EnergyPlusState state, void (*f)(EnergyPlusState));
/// \brief Register a callback function to be called at each new environment once warmup is complete.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
ENERGYPLUSLIB_API void callbackAfterNewEnvironmentWarmupComplete(EnergyPlusState state, void (*f)(EnergyPlusState));
/// \brief Register a callback function to be called at the beginning of each zone time step before the heat balance is initialized.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
ENERGYPLUSLIB_API void callbackBeginZoneTimeStepBeforeInitHeatBalance(EnergyPlusState state, void (*f)(EnergyPlusState));
/// \brief Register a callback function to be called at the beginning of each zone time step after the heat balance is initialized.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
ENERGYPLUSLIB_API void callbackBeginZoneTimeStepAfterInitHeatBalance(EnergyPlusState state, void (*f)(EnergyPlusState));
/// \brief Register a callback function to be called at each zone time step just before the predictor step.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
ENERGYPLUSLIB_API void callbackBeginTimeStepBeforePredictor(EnergyPlusState state, void (*f)(EnergyPlusState));
/// \brief Register a callback function to be called at each zone time step just before the weather is updated.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
ENERGYPLUSLIB_API void callbackBeginZoneTimestepBeforeSetCurrentWeather(EnergyPlusState state, void (*f)(EnergyPlusState));
/// \brief Register a callback function to be called at each zone time step before HVAC managers are called.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
ENERGYPLUSLIB_API void callbackAfterPredictorBeforeHVACManagers(EnergyPlusState state, void (*f)(EnergyPlusState));
/// \brief Register a callback function to be called at each zone time step after HVAC managers are called.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
ENERGYPLUSLIB_API void callbackAfterPredictorAfterHVACManagers(EnergyPlusState state, void (*f)(EnergyPlusState));
/// \brief Register a callback function to be called inside the HVAC system iteration loop.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
ENERGYPLUSLIB_API void callbackInsideSystemIterationLoop(EnergyPlusState state, void (*f)(EnergyPlusState));
/// \brief Register a callback function to be called at the end of each zone time step but before zones have reported data.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
ENERGYPLUSLIB_API void callbackEndOfZoneTimeStepBeforeZoneReporting(EnergyPlusState state, void (*f)(EnergyPlusState));
/// \brief Register a callback function to be called at the end of each zone time step after zones have reported data.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
ENERGYPLUSLIB_API void callbackEndOfZoneTimeStepAfterZoneReporting(EnergyPlusState state, void (*f)(EnergyPlusState));
/// \brief Register a callback function to be called at the end of each HVAC system time step but before HVAC data has been reported.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
ENERGYPLUSLIB_API void callbackEndOfSystemTimeStepBeforeHVACReporting(EnergyPlusState state, void (*f)(EnergyPlusState));
/// \brief Register a callback function to be called at the end of each HVAC system time step after HVAC data has been reported.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
ENERGYPLUSLIB_API void callbackEndOfSystemTimeStepAfterHVACReporting(EnergyPlusState state, void (*f)(EnergyPlusState));
/// \brief Register a callback function to be called at the end of the zone sizing process.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
ENERGYPLUSLIB_API void callbackEndOfZoneSizing(EnergyPlusState state, void (*f)(EnergyPlusState));
/// \brief Register a callback function to be called at the end of the HVAC system sizing process.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
ENERGYPLUSLIB_API void callbackEndOfSystemSizing(EnergyPlusState state, void (*f)(EnergyPlusState));
/// \brief Register a callback function to be called after some specific components have processed their input.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
/// \remark Currently this calling point is called after each instance of a DX coil, fan, and unitary system object.
ENERGYPLUSLIB_API void callbackEndOfAfterComponentGetInput(EnergyPlusState state, void (*f)(EnergyPlusState));
/// \brief Register a callback function to be called at unitary system sizing.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
ENERGYPLUSLIB_API void callbackUnitarySystemSizing(EnergyPlusState state, void (*f)(EnergyPlusState));
/// \brief Register a callback function to be called by a specific user-defined model.
/// \details During an EnergyPlus simulation, a number of predetermined calling points have been established at which
///          any registered callback functions are "called back".  This API function allows a client to register a function
///          with no arguments to be called at this specific calling point.  From inside this function, the client can
///          leverage other API categories to look up property values or exchange data with the simulation as needed.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
/// \param[in] programName The name of the program to match up with the user defined equipment object in the input file.
/// \remark This function is only allowed during API simulations.  For Python Plugin applications, the client will
///         create a custom Python class and override specific functions to be called at equivalent points in the simulation.
ENERGYPLUSLIB_API void callbackUserDefinedComponentModel(EnergyPlusState state, void (*f)(EnergyPlusState), const char *programName);
/// \brief Register a callback function to be used in place of the EnergyPlus ManageHVAC function.
/// \details This callback is a placeholder for advanced use cases, and will be supported in a future release.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] f The function to be called back at this specific calling point in the simulation.  The function expects one EnergyPlusState argument.
ENERGYPLUSLIB_API void registerExternalHVACManager(EnergyPlusState state, void (*f)(EnergyPlusState));

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // EnergyPlusAPIRuntime_h_INCLUDED
