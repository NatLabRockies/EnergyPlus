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

#ifndef EnergyPlusAPIDataTransfer_h_INCLUDED
#define EnergyPlusAPIDataTransfer_h_INCLUDED

#include <EnergyPlus/api/EnergyPlusAPI.h>
#include <EnergyPlus/api/TypeDefs.h>
#include <EnergyPlus/api/state.h>

#ifdef __cplusplus
extern "C" {
#endif

/// \file datatransfer.h
/// \brief This "data exchange" API category provides access to read and write real-time data within a running simulation.
/// \details While an EnergyPlus simulation is running, simulation state is constantly being calculated and updated.  EMS was implemented in
///          EnergyPlus to allow user scripts to read variable data and write actuator data in custom ways.  This builds on top of that implementation
///          by allowing the user to write C code that can read/write simulation data.  This file includes methods that are specific to
///          PythonPlugin workflows, including PythonPlugin "Trend" and "Global" variables.  These mechanisms are only available in Plugin workflows
///          and will cause a program error if they are accessed in an "EnergyPlus-as-a-library" workflow.  However, there are many functions in this
///          file that are available for all API usages, including getting access to variables and meters, writing actuator values, and accessing
///          simulation state such as the day of week, hour of day, etc.

// ----- GENERIC QUERY FUNCTIONS

/// \brief A structure representing a data exchange entry for API calls
/// \details Can represent an output variable (sensor), and actuator, or several other things.
///          The type of thing should be identified by the "what" member, which can be used as a filter for a specific type.
struct APIDataEntry
{
    ///  \brief This variable will hold the basic type of API data point, in string form.
    ///  \details This can be one of the following:
    ///           "Actuator", "InternalVariable", "PluginGlobalVariable", "PluginTrendVariable", "OutputMeter", or "OutputVariable"
    ///           Once the full list of data exchange points are returned from a call to getAPIData, this parameter can be
    ///           used to quickly filter down to a specific type.
    char *what;
    /// \brief This represents the name of the entry point, not the name of the specific instance of the entry point.
    /// \details Some examples of this name could be "Chiller Heat Transfer Rate" -- which could be available for multiple chillers.
    char *name;
    /// \brief This represents the unique ID for this exchange point.
    /// \details In the example of the chiller output variable, this could be "Chiller 1". This is not used for meters
    char *key; // not used for meters
    /// \brief This represents the "type" of exchange for this exchange point.
    /// \details This is only used for actuators, and represents the control actuation.
    ///          For a node setpoint actuation, this could be either temperature or humidity, for example.
    char *type; // only used for actuators
    /// \brief This represents the unit of measurement for this exchange point.
    /// \details This is only used for non-plugin variables.
    char *unit; // NOT used for plugin variables
};

/// \brief Gets available API data for the current simulation
/// \details This function returns a char * which points to API data in CSV form for the current simulation
///          The data can be easily parsed and then used in subsequent API code.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark A future version of this will more intelligently return available data in a structured format in memory.
/// \return A char * pointing to a CSV formatted string.  This allocates a new char *, and calling clients must free this when done with it!
ENERGYPLUSLIB_API char *listAllAPIDataCSV(EnergyPlusState state);
/// \brief Provides a user-facing check for whether API data is ready to be accessed
/// \details Many parts of a simulation need to be set up to complete a run.
///          At the early stages of a simulation, most data has not been allocated and set up.
///          Calling to retrieve variable, meter, or actuator values prior to this point can cause problems.
///          This function allows a user to call the simulation to check whether data is ready for access.
///          Do not call for variable, meter, actuator, or any other internal exchange data prior to this returning true.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \return Returns 1 (true) once the data is ready, otherwise returns 0 (false).
/// \remark  Note that in the case of Surface actuators for "Construction State" and for a call to \link getConstructionHandle \endlink,
///          this is not required to be true, as construction data is generally available earlier in the simulation process.
///          Bypassing this check will allow affecting the Sizing calculations.
ENERGYPLUSLIB_API int apiDataFullyReady(EnergyPlusState state);
/// \brief Provides a user-facing check on the API error flag
/// \details Some API functions return a value of 0, which could potentially indicate an error, or an actual 0 value.
///          This function provides a way to disambiguate the response value.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \return Returns 0 (success) if the error flag is not activated, or 1 if the error flag is true.
ENERGYPLUSLIB_API int apiErrorFlag(EnergyPlusState state);
/// \brief Reset the API Error flag
/// \details In most cases, upon an error, the client will want to just abort the calculation.  However, in cases where an invalid condition allows
///          a calculation to continue, this function can reset the flag.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
ENERGYPLUSLIB_API void resetErrorFlag(EnergyPlusState state);
/// \brief Provides the input file path back to the user
/// \details In most circumstances the client will know the path to the input file, but there are some cases where code
///          is generalized in unexpected workflows.  Users have requested a way to get the input file path back from the running instance.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \return A char * of the input file path.  This allocates a new char *, and calling clients must free this when done with it!
ENERGYPLUSLIB_API char *inputFilePath(EnergyPlusState state);
/// \brief Provides the weather file path back to the user
/// \details In most circumstances the client will know the path to the weather file, but there are some cases where code
///          is generalized in unexpected workflows.  Users have requested a way to get the weather file path back from the running instance.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \return A char * of the weather file path.  This allocates a new char *, and calling clients must free this when done with it!
ENERGYPLUSLIB_API char *epwFilePath(EnergyPlusState state);

// ----- DATA TRANSFER HELPER FUNCTIONS

/// \brief Gets available API data for the current simulation
/// \details This function returns a APIDataEntry * (array) which points to an API data array.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[out] resultingSize An integer which will be set to the size of the array upon return
/// \return An APIDataEntry * pointing to an array with the size passed in the by-ref argument.  When done, pass to freeAPIData to clear the memory.
ENERGYPLUSLIB_API struct APIDataEntry *getAPIData(EnergyPlusState state, unsigned int *resultingSize);
/// \brief Clears an APIData memory allocation
/// \details This function frees an instance of the API data
/// \param[in] data An array (pointer) of API data exchange that points as returned from the getAPIData function
/// \param[in] arraySize The size of the API data exchange array, which is known after the call to getAPIData.
/// \return Nothing, this simply frees the memory
ENERGYPLUSLIB_API void freeAPIData(const struct APIDataEntry *data, unsigned int arraySize);
/// \brief Gets the names of the object instances in the current input file
/// \details Although many workflows should be aware of the input file already, there are certain cases where asking EnergyPlus
///          to report back the current input file objects has value.  The primary application is when a user is still utilizing an IDF based
///          workflow, and wants to be able to write general purpose API callbacks or Python Plugins.  In this case, they may want to loop over all
///          zones or chillers, and get handles to actuators or sensors accordingly.  Normally, the user would have to hardcode the object names, but
///          with this API function, they can simply loop over all the objects of that type.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] objectType The object type from the input schema, such as "Chiller:Electric", or "Zone" -- casing should match input schema!
/// \param[out] resultingSize An integer which will be set to the size of the array upon return
/// \return Const char * pointing to an array of const char * with the size set in the by-ref argument.  When done, pass to freeObjectNames to clear.
ENERGYPLUSLIB_API char **getObjectNames(EnergyPlusState state, const char *objectType, unsigned int *resultingSize);
/// \brief Clears an object names array allocation
/// \details This function frees an instance of the object names array, which is returned from getObjectNames
/// \param[in] objectNames An array (pointer) of char * as returned from the getObjectNames function
/// \param[in] arraySize The size of the object name array, which is known after the call to getObjectNames.
/// \return Nothing, this simply frees the memory
ENERGYPLUSLIB_API void freeObjectNames(char **objectNames, unsigned int arraySize);
// ----- FUNCTIONS RELATED TO VARIABLES

/// \brief Gets the number of nodes for a particular CondFD surface layer
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] surfName Surface name as defined in the input file, such as "ZN001:Surf001"
/// \param[in] matName Material name as defined in the input file, such as "GypsumBoardLayer"
/// \remark This function should only be called after condFDReady returns successfully.
ENERGYPLUSLIB_API int getNumNodesInCondFDSurfaceLayer(EnergyPlusState state, const char *surfName, const char *matName);
/// \brief Marks a variable as requested in a simulation
/// \details To optimize memory and cpu usage, the simulation ignores variables not requested in the IDF.
///          This function allows marking variables as used even if they are not in the input file.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] type Variable Type, such as "System Node Mass Flow Rate", or "Site Outdoor Air DryBulb Temperature"
/// \param[in] key Variable Key, such as "Node 32", or "Environment"
/// \remark This function should be called prior to executing each simulation, as the internal array is cleared when clearing the state of each run.
ENERGYPLUSLIB_API void requestVariable(EnergyPlusState state, const char *type, const char *key);
/// \brief Gets a handle to a variable
/// \details Looks up a handle to a variable within a running simulation.
///          Variables are identified by a key and type.
///          Variables are only available if they are explicitly listed as Output:Variable objects in the input file,
///          or if API calls to `requestVariable` to mark this variable as requested prior to a simulation run.
///          This function will return a handle of -1 if a match is not found for this type/key combination.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] type Variable Type, such as "System Node Mass Flow Rate", or "Site Outdoor Air DryBulb Temperature"
/// \param[in] key Variable Key, such as "Node 32", or "Environment"
/// \return The integer handle to a variable, or -1 if the variable was not found
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true
/// \see apiDataFullyReady
/// \see requestVariable
ENERGYPLUSLIB_API int getVariableHandle(EnergyPlusState state, const char *type, const char *key);
/// \brief Gets the current value of a variable
/// \details This function uses the integer handle of a variable and retrieves the current value of the variable.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] handle The handle id to a variable which can be retrieved using the `getVariableHandle` function.
/// \return The current value of the variable, in floating point form, or zero if a handle problem is encountered.  If a zero is returned, use the
///         `apiErrorFlag` function to disambiguate the return value.
/// \see getVariableHandle
ENERGYPLUSLIB_API Real64 getVariableValue(EnergyPlusState state, int handle);

// ----- FUNCTIONS RELATED TO METERS

/// \brief Gets a handle to a meter
/// \details Looks up a handle to a meter within a running simulation.
///          Meters are identified by a single name string.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] meterName The name of the meter to which a reference is retrieved
/// \return The integer handle to a meter, or -1 if the meter was not found
/// \remark Note the behavior of this function is not well-defined until the `apiDataFullyReady` function returns true
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int getMeterHandle(EnergyPlusState state, const char *meterName);
/// \brief Gets the current value of a meter
/// \details Looks up the value of an existing meter within a running simulation.  Caution: This function currently returns the instantaneous value
///          of a meter, not the cumulative value.  This will change in a future version of the API.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] handle The handle id of the meter, which can be retrieved using the `getMeterHandle` function
/// \return The floating point value of a meter at the current time, or zero if a handle problem is encountered.  If a zero is returned, use the
///         `apiErrorFlag` function to disambiguate the return value.
/// \remark Note the behavior of this function is not well-defined until the `apiDataFullyReady` function returns true
/// \see apiDataFullyReady
/// \see getMeterHandle
ENERGYPLUSLIB_API Real64 getMeterValue(EnergyPlusState state, int handle);

// ----- FUNCTIONS RELATED TO ACTUATORS

/// \brief Gets a handle to an actuator
/// \details Looks up a handle to an actuator within a running simulation.
///          Actuators are identified by three parameters: key, type, and control type
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] componentType The component type for the actuator, such as "Weather Data"
/// \param[in] controlType The specific control type for the actuator, such as "Outdoor Dew Point"
/// \param[in] uniqueKey The unique key for this actuator, such as "Environment"
/// \return The integer handle to an actuator, or -1 if the actuator was not found
/// \remark Note the behavior of this function is not well-defined until the `apiDataFullyReady` function returns true
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int getActuatorHandle(EnergyPlusState state, const char *componentType, const char *controlType, const char *uniqueKey);
/// \brief Resets an actuator so EnergyPlus will calculate the value as normal
/// \details When an actuator value is set using `setActuatorValue`, a flag is set so the actuated value will not be
///          overridden.  If the user desires to revert back and let EnergyPlus calculate the actuated value, this
///          function will reset the flag.  The user can always set the actuated value again anytime.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] handle The integer handle to the actuator, which can be retrieved using the `getActuatorHandle` function
/// \remark When calling this function, a handle index or other problem will cause the function to return 0 but set an error flag.  In Plugin
///         work flows, EnergyPlus will abort after returning, in other applications use the `apiErrorFlag` function to check the error status.
/// \remark Note the behavior of this function is not well-defined until the `apiDataFullyReady` function returns true
/// \see apiDataFullyReady
/// \see getActuatorHandle
/// \see setActuatorValue
ENERGYPLUSLIB_API void resetActuator(EnergyPlusState state, int handle);
/// \brief Sets the value of an actuator in EnergyPlus
/// \details Actuators are variables in the simulation which can be overridden.
///          Calculations made outside of EnergyPlus are performed and used to update values inside EnergyPlus via actuators.
///          Internally, actuators can alter floating point, integer, and boolean operational values.  The API only exposes
///          this set function with a floating point argument.  For floating point types, the value is assigned directly.  For
///          integer types, the value is rounded using std::lround, which will round to the nearest integer, with the halfway point
///          rounded away from zero (2.5 becomes 3), then cast to a plain integer.  For logical values, the original EMS convention
///          is kept, where a value of 1.0 means TRUE, and a value of 0.0 means FALSE -- and any other value defaults to FALSE.
///          A small tolerance is applied internally to allow for small floating point roundoff.  A value *very close* to 1.0 will
///          still evaluate to TRUE.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] handle The integer handle to the actuator, which can be retrieved using the `getActuatorHandle` function
/// \param[in] value The floating point value to be assigned to the actuator in the simulation
/// \remark When calling this function, a handle index or other problem will cause the function to return 0 but set an error flag.  In Plugin
///         work flows, EnergyPlus will abort after returning, in other applications use the `apiErrorFlag` function to check the error status.
/// \remark Note the behavior of this function is not well-defined until the `apiDataFullyReady` function returns true
/// \see apiDataFullyReady
/// \see getActuatorHandle
/// \see setActuatorValue
ENERGYPLUSLIB_API void setActuatorValue(EnergyPlusState state, int handle, Real64 value);
/// \brief Gets the value of an actuator in EnergyPlus
/// \details Actuators are variables in the simulation which can be overridden. This function allows a client to get the last
///          value assigned to the actuator.  Although in most applications, the client can just track the last value it assigned
///          to the actuator, there are some occasions (Python Plugins) where multiple scripts could read/write from/to the actuator.
///          In this case, being able to grab the last value that was assigned with `setActuatorValue` is reasonable.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] handle The integer handle to the actuator, which can be retrieved using the `getActuatorHandle` function
/// \return The floating point value last assigned to the actuator in the program, or zero if a handle problem is encountered.  If a zero
///         is returned, use the `apiErrorFlag` function to disambiguate the return value.
/// \remark Note the behavior of this function is not well-defined until the `apiDataFullyReady` function returns true
/// \see apiDataFullyReady
/// \see getActuatorHandle
/// \see setActuatorValue
ENERGYPLUSLIB_API Real64 getActuatorValue(EnergyPlusState state, int handle);

// ----- FUNCTIONS RELATED TO STATIC "INTERNAL VARIABLES"

/// \brief Gets a handle to an internal variable
/// \details Internal variables are essentially "static" data -- think zone volume or component sizing.
///          Internal variables are identified by a key and type.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] type Internal Variable Type, such as "Zone Floor Area"
/// \param[in] key Internal Variable Key, such as "Zone 1"
/// \return The integer handle to an independent variable, or -1 if the variable was not found
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int getInternalVariableHandle(EnergyPlusState state, const char *type, const char *key);
/// \brief Gets the current value of an internal variable
/// \details This function uses the integer handle of a variable and retrieves the static value of the variable.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] handle The handle id to a variable which can be retrieved using the `getInternalVariableHandle` function.
/// \return The current value of the variable, in floating point form, or zero if a handle problem is encountered.  If a zero
///         is returned, use the `apiErrorFlag` function to disambiguate the return value.
/// \see getInternalVariableHandle
ENERGYPLUSLIB_API Real64 getInternalVariableValue(EnergyPlusState state, int handle);

// ----- FUNCTIONS RELATED TO EMS GLOBAL VARIABLES (FOR CORNER CASES WHERE PLUGIN/API BLENDS WITH EMS PROGRAMS)
/// \brief Gets a handle to an EMS "Global" variable
/// \details When using EMS, it is sometimes necessary to share data between programs.
///          EMS global variables are declared in the input file and used in EMS programs.
///          EMS global variables are identified by name only.  This function returns -1 if a match is not found.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] name The name of the EMS global variable, which is declared in the EnergyPlus input file
/// \return The integer handle to an EMS global variable, or -1 if a match is not found.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int getEMSGlobalVariableHandle(EnergyPlusState state, const char *name);
/// \brief Gets the current value of an EMS "Global" variable
/// \details When using EMS, the value of the shared "global" variables can change at any time.
///          This function returns the current value of the variable.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] handle The handle id to an EMS "Global" variable, which can be retrieved using the `getEMSGlobalVariableHandle` function.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \return The current value of the variable, in floating point form, or zero if a handle problem is encountered.  If a zero
///         is returned, use the `apiErrorFlag` function to disambiguate the return value.
/// \see apiDataFullyReady
/// \see getEMSGlobalVariableHandle
ENERGYPLUSLIB_API Real64 getEMSGlobalVariableValue(EnergyPlusState state, int handle);
/// \brief Sets the value of an EMS "Global" variable
/// \details When using EMS, the value of the shared "global" variables can change at any time.
///          This function sets the variable to a new value.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] handle The handle id to an EMS "Global" variable, which can be retrieved using the `getEMSGlobalVariableHandle` function.
/// \param[in] value The floating point value to be assigned to the global variable
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark A handle index or other problem will return 0 and set a flag to cause EnergyPlus to terminate once Python completes.
/// \see apiDataFullyReady
/// \see getEMSGlobalVariableHandle
ENERGYPLUSLIB_API void setEMSGlobalVariableValue(EnergyPlusState state, int handle, Real64 value);

// ----- FUNCTIONS RELATED TO PYTHON PLUGIN GLOBAL VARIABLES (ONLY USED FOR PYTHON PLUGIN SYSTEM)

/// \brief Gets a handle to a Python Plugin "Global" variable
/// \details When using Python Plugins, it is sometimes necessary to share data between plugins.
///          These global variables are declared in the input file first, and then can be read/write by any number of plugins.
///          Plugin global variables are identified by name only.  This function returns -1 if a match is not found.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] name The name of the plugin global variable, which is declared in the EnergyPlus input file
/// \return The integer handle to a plugin global variable, or -1 if a match is not found.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark This function only has meaning in the context of Python Plugin workflows, not in regular API workflows.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int getPluginGlobalVariableHandle(EnergyPlusState state, const char *name);
/// \brief Gets the current value of a Python Plugin "Global" variable
/// \details When using Python Plugins, the value of the shared "global" variables can change at any time.
///          This function returns the current value of the variable.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] handle The handle id to a Python Plugin "Global" variable, which can be retrieved using the `getPluginGlobalVariableHandle` function.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark This function only has meaning in the context of Python Plugin workflows, not in regular API workflows.
/// \return The current value of the variable, in floating point form, or zero if a handle problem is encountered.  If a zero
///         is returned, use the `apiErrorFlag` function to disambiguate the return value.
/// \see apiDataFullyReady
/// \see getPluginGlobalVariableHandle
ENERGYPLUSLIB_API Real64 getPluginGlobalVariableValue(EnergyPlusState state, int handle);
/// \brief Sets the value of a Python Plugin "Global" variable
/// \details When using Python Plugins, the value of the shared "global" variables can change at any time.
///          This function sets the variable to a new value.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] handle The handle id to a Python Plugin "Global" variable, which can be retrieved using the `getPluginGlobalVariableHandle` function.
/// \param[in] value The floating point value to be assigned to the global variable
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark This function only has meaning in the context of Python Plugin workflows, not in regular API workflows.
/// \remark A handle index or other problem will return 0 and set a flag to cause EnergyPlus to terminate once Python completes.
/// \see apiDataFullyReady
/// \see getPluginGlobalVariableHandle
ENERGYPLUSLIB_API void setPluginGlobalVariableValue(EnergyPlusState state, int handle, Real64 value);

// ----- FUNCTIONS RELATED TO PYTHON PLUGIN TREND VARIABLES (ONLY USED FOR PYTHON PLUGIN SYSTEM)

/// \brief Gets a handle to a Python Plugin "Trend" variable
/// \details When using Python Plugins, variable history can be stored/accessed using "trend" variables.
///          These trend variables are declared in the input file first, and then can be read/write by any number of plugins.
///          Plugin trend variables are identified by name only.  If a trend is not found, this function will return -1.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] name The name of the plugin trend variable, which is declared in the EnergyPlus input file
/// \return The integer handle to a plugin trend variable, or -1 if a match is not found.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark This function only has meaning in the context of Python Plugin workflows, not in regular API workflows.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int getPluginTrendVariableHandle(EnergyPlusState state, const char *name);
/// \brief Gets the current value of a Python Plugin "Trend" variable at a specific history point
/// \details When using Python Plugins, the value of the "trend" variable can be retrieved from previous timesteps, up
///          to the number of history terms defined in the input file.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] handle The handle id to a Python Plugin "Trend" variable, which can be retrieved using the `getPluginTrendVariableHandle` function.
/// \param[in] timeIndex The number of timesteps backward to traverse the trend when returning this value.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark This function only has meaning in the context of Python Plugin workflows, not in regular API workflows.
/// \remark A handle index or other problem will return 0 and set a flag to cause EnergyPlus to terminate once Python completes.
/// \see apiDataFullyReady
/// \see getPluginTrendVariableHandle
ENERGYPLUSLIB_API Real64 getPluginTrendVariableValue(EnergyPlusState state, int handle, int timeIndex);
/// \brief Gets the average value of a Python Plugin "Trend" variable over a given number of history points
/// \details When using Python Plugins, the average value of the "trend" variable over a number of previous timesteps can be retrieved, up
///          to the number of history terms defined in the input file.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] handle The handle id to a Python Plugin "Trend" variable, which can be retrieved using the `getPluginTrendVariableHandle` function.
/// \param[in] count The number of timesteps backward to traverse the trend when returning this value.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark This function only has meaning in the context of Python Plugin workflows, not in regular API workflows.
/// \remark A handle index or other problem will return 0 and set a flag to cause EnergyPlus to terminate once Python completes.
/// \see apiDataFullyReady
/// \see getPluginTrendVariableHandle
ENERGYPLUSLIB_API Real64 getPluginTrendVariableAverage(EnergyPlusState state, int handle, int count);
/// \brief Gets the minimum value of a Python Plugin "Trend" variable over a given number of history points
/// \details When using Python Plugins, the minimum value of the "trend" variable over a number of previous timesteps can be retrieved, up
///          to the number of history terms defined in the input file.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] handle The handle id to a Python Plugin "Trend" variable, which can be retrieved using the `getPluginTrendVariableHandle` function.
/// \param[in] count The number of timesteps backward to traverse the trend when returning this value.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark This function only has meaning in the context of Python Plugin workflows, not in regular API workflows.
/// \remark A handle index or other problem will return 0 and set a flag to cause EnergyPlus to terminate once Python completes.
/// \see apiDataFullyReady
/// \see getPluginTrendVariableHandle
ENERGYPLUSLIB_API Real64 getPluginTrendVariableMin(EnergyPlusState state, int handle, int count);
/// \brief Gets the maximum value of a Python Plugin "Trend" variable over a given number of history points
/// \details When using Python Plugins, the maximum value of the "trend" variable over a number of previous timesteps can be retrieved, up
///          to the number of history terms defined in the input file.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] handle The handle id to a Python Plugin "Trend" variable, which can be retrieved using the `getPluginTrendVariableHandle` function.
/// \param[in] count The number of timesteps backward to traverse the trend when returning this value.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark This function only has meaning in the context of Python Plugin workflows, not in regular API workflows.
/// \remark A handle index or other problem will return 0 and set a flag to cause EnergyPlus to terminate once Python completes.
/// \see apiDataFullyReady
/// \see getPluginTrendVariableHandle
ENERGYPLUSLIB_API Real64 getPluginTrendVariableMax(EnergyPlusState state, int handle, int count);
/// \brief Gets the summation of a Python Plugin "Trend" variable over a given number of history points
/// \details When using Python Plugins, the summation of the "trend" variable over a number of previous timesteps can be retrieved, up
///          to the number of history terms defined in the input file.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] handle The handle id to a Python Plugin "Trend" variable, which can be retrieved using the `getPluginTrendVariableHandle` function.
/// \param[in] count The number of timesteps backward to traverse the trend when returning this value.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark This function only has meaning in the context of Python Plugin workflows, not in regular API workflows.
/// \remark A handle index or other problem will return 0 and set a flag to cause EnergyPlus to terminate once Python completes.
/// \see apiDataFullyReady
/// \see getPluginTrendVariableHandle
ENERGYPLUSLIB_API Real64 getPluginTrendVariableSum(EnergyPlusState state, int handle, int count);
/// \brief Gets the average trajectory of a Python Plugin "Trend" variable over a given number of history points
/// \details For many control applications, it is useful to know the average trajectory of a trend variable over time.
///          To calculate this, the program will sample the history of the trend over the user-specified number of time history terms,
///          perform a regression, and return the slope of this regression line.  If positive, the trend is, on average, increasing,
///          and decreasing if negative.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] handle The handle id to a Python Plugin "Trend" variable, which can be retrieved using the `getPluginTrendVariableHandle` function.
/// \param[in] count The number of timesteps backward to traverse the trend when calculate this average direction.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark This function only has meaning in the context of Python Plugin workflows, not in regular API workflows.
/// \remark A handle index or other problem will return 0 and set a flag to cause EnergyPlus to terminate once Python completes.
/// \see apiDataFullyReady
/// \see getPluginTrendVariableHandle
ENERGYPLUSLIB_API Real64 getPluginTrendVariableDirection(EnergyPlusState state, int handle, int count);

// ----- FUNCTIONS RELATED TO MISC CURRENT SIMULATION STATE

/// \brief Returns the current year of the simulation, taken from the EPW.
/// \details This is directly read from the EPW, and as such, if the EPW is for example a TMY3 file
///          the year could be set to an abritrary number and change from one timestep to the next. See calendarYear for an alternative
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int year(EnergyPlusState state);

/// \brief Returns the Calendar Year of the simulation (based on the RunPeriod object). Only valid for weather file run periods.
/// \details A simulation can span multiple years and will always have a "meaningful" year that is either user-defined explicitly,
///          determined based on other inputs in the input file, or chosen as the current year.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int calendarYear(EnergyPlusState state);

/// \brief Returns the current month of the simulation, from 1 for January to 12 for December.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int month(EnergyPlusState state);

/// \brief Returns the current day of month of the simulation, from 1 to 28, 29, 30, or 31, based on the month.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int dayOfMonth(EnergyPlusState state);

/// \brief Returns the current day of week of the simulation, from 1 for Sunday to 7 on Saturday
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int dayOfWeek(EnergyPlusState state);

/// \brief Returns the current day of year of the simulation, from 1 for January 1 to 365 (or 366 for a leap year) for December 31.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int dayOfYear(EnergyPlusState state);

/// \brief Returns the current daylight savings time status, which is 0 (zero) if DST is off, or 1 (one) if DST is on.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int daylightSavingsTimeIndicator(EnergyPlusState state);

/// \brief Returns the current hour of the day in 0-23 form
/// \details The hour within the simulation ranges from 0 for timesteps from 12:00am to 12:59am up to 23 for timesteps from 11:00pm to 11:59pm
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int hour(EnergyPlusState state);

/// \brief Returns the current time of the simulation
/// \details For this function, the current time represents the end time of the current time step.
///          The time is returned as floating point fractional hours of the day, and since it represents the end of the
///          current time step, the value will go from just over zero at the beginning of a day to 24.0 at the last time step of the day.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API Real64 currentTime(EnergyPlusState state);

/// \brief Returns the minutes into the hour from 1 to 60
/// \details The minutes into the simulation will return the value for the end of the current system time step.
///          This function returns an integer value, but because the HVAC system time step is constantly adjusted, the end of
///          the system time step may actually occur at a fractional part of a minute.  This function truncates the seconds
///          portion during integerization.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int minutes(EnergyPlusState state);

/// \brief Returns the duration of the current zone simulation time step, in fractional hours
/// \details The zone time step will vary throughout the simulation as needed to maintain convergence while being cautious about program runtime.
///          This function returns the current value of the time step
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API Real64 zoneTimeStep(EnergyPlusState state);

/// \brief Returns the number of zone time steps in an hour, which is currently a constant value throughout a simulation.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int numTimeStepsInHour(EnergyPlusState state);

/// \brief The current zone time step index, from 1 to the number of zone time steps per hour
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int zoneTimeStepNum(EnergyPlusState state);

/// \brief Returns the duration of the current HVAC system simulation time step, in fractional hours
/// \details The HVAC time step will vary throughout the simulation as needed to maintain convergence while being cautious about program runtime.
///          This function returns the current value of the time step
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API Real64 systemTimeStep(EnergyPlusState state);

/// \brief Returns the holiday index for the current day of the simulation
/// \details Days in the simulation year can be regular days, in which case this returns zero, or integers 1-N based on holiday definitions in the
/// input. \param[in] state An active EnergyPlusState instance created with `stateNew`. \remark The behavior of this function is not well-defined
/// until the `apiDataFullyReady` function returns true. \see apiDataFullyReady
ENERGYPLUSLIB_API int holidayIndex(EnergyPlusState state);

/// \brief Returns 0 if the sun is down, or 1 if the sun is up
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int sunIsUp(EnergyPlusState state);

/// \brief Returns 0 if it is not currently raining, or 1 if it is currently raining
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int isRaining(EnergyPlusState state);

/// \brief Returns the current warmup flag status, 0 if not warming up, 1 if warming up
/// \details During an EnergyPlus simulation, at the beginning of each run period or design day, the first day is
///          repeated until the simulation achieves convergence.  This eliminates the dependence on guesses to initial
///          values used when initializing the entire simulation state.  It can be useful to ignore operations during
///          warmup, and this flag allows checking that status.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int warmupFlag(EnergyPlusState state);

/// \brief Returns an integer indicator for the current simulation environment type
/// \details During an EnergyPlus simulation, there are often multiple stages of the simulation run.  These include
///          design days followed by a run period most commonly, but there are also sizing simulation environments.
///          The full list of values is:
///          - Design (Sizing) Day = 1
///          - Design (Sizing) RunPeriod = 2
///          - Weather File Run Period = 3
///          - HVAC-Sizing Design Day = 4
///          - HVAC-Sizing Run Period = 5
///          - Weather Data Processing Environment = 6
///
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int kindOfSim(EnergyPlusState state);

/// \brief Returns the current environment number
/// \details As EnergyPlus simulates, it runs through multiple phases that often include multiple design days followed
///          by a run period.  This function returns the current index which is just incremented for each new phase.
///          This has limited value in a general sense, but for some very tightly defined workflows, this could give
///          some information.  It is more advised to check the result of the `kindOfSim` function to get a reliable environment type.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
/// \see kindOfSim
ENERGYPLUSLIB_API int currentEnvironmentNum(EnergyPlusState state);

/// \brief Returns an index to a construction in the simulation
/// \details Some actuators allow specifying different constructions to allow switchable construction control.
///          This function returns an index that can be used in those functions.  The construction is specified by name.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] constructionName The name of the construction to be looked up.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int getConstructionHandle(EnergyPlusState state, const char *constructionName);

/// \brief Returns a simple sum of the time part of the date/time function
/// \details Could be used in random seeding.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int actualTime(EnergyPlusState state);

/// \brief Returns a simple sum of the date/time function
/// \details Could be used in random seeding.
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \see apiDataFullyReady
ENERGYPLUSLIB_API int actualDateTime(EnergyPlusState state);

/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \returns 1 if it is raining, 0 if not
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API int todayWeatherIsRainAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \returns 1 if it is snowing, 0 if not
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API int todayWeatherIsSnowAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 todayWeatherOutDryBulbAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 todayWeatherOutDewPointAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 todayWeatherOutBarometricPressureAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 todayWeatherOutRelativeHumidityAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 todayWeatherWindSpeedAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 todayWeatherWindDirectionAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 todayWeatherSkyTemperatureAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 todayWeatherHorizontalIRSkyAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 todayWeatherBeamSolarRadiationAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 todayWeatherDiffuseSolarRadiationAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 todayWeatherAlbedoAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 todayWeatherLiquidPrecipitationAtTime(EnergyPlusState state, int hour, int timeStepNum);

/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \returns 1 if it is raining, 0 if not
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API int tomorrowWeatherIsRainAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \returns 1 if it is snowing, 0 if not
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API int tomorrowWeatherIsSnowAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 tomorrowWeatherOutDryBulbAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 tomorrowWeatherOutDewPointAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 tomorrowWeatherOutBarometricPressureAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 tomorrowWeatherOutRelativeHumidityAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 tomorrowWeatherWindSpeedAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 tomorrowWeatherWindDirectionAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 tomorrowWeatherSkyTemperatureAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 tomorrowWeatherHorizontalIRSkyAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 tomorrowWeatherBeamSolarRadiationAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 tomorrowWeatherDiffuseSolarRadiationAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 tomorrowWeatherAlbedoAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the requested weather data at the specified time
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \param[in] hour The hour of the simulation day, from 0 to 23
/// \param[in] timeStepNum The time step index, from 1 to the number of zone time steps per hour
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 tomorrowWeatherLiquidPrecipitationAtTime(EnergyPlusState state, int hour, int timeStepNum);
/// \brief Returns the cumulative simulation time from the start of the environment, in hours
/// \param[in] state An active EnergyPlusState instance created with `stateNew`.
/// \remark The behavior of this function is not well-defined until the `apiDataFullyReady` function returns true.
/// \remark The API error flag will be set if an issue occurs in the lookup, use `apiErrorFlag` to check
/// \see apiDataFullyReady
/// \see apiErrorFlag
ENERGYPLUSLIB_API Real64 currentSimTime(EnergyPlusState state);

#ifdef __cplusplus
}
#endif

#endif // EnergyPlusAPIDataTransfer_h_INCLUDED
