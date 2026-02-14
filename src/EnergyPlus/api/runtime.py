# EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University
# of Illinois, The Regents of the University of California, through Lawrence
# Berkeley National Laboratory (subject to receipt of any required approvals
# from the U.S. Dept. of Energy), Oak Ridge National Laboratory, managed by UT-
# Battelle, Alliance for Energy Innovation, LLC, and other contributors. All
# rights reserved.
#
# NOTICE: This Software was developed under funding from the U.S. Department of
# Energy and the U.S. Government consequently retains certain rights. As such,
# the U.S. Government has been granted for itself and others acting on its
# behalf a paid-up, nonexclusive, irrevocable, worldwide license in the
# Software to reproduce, distribute copies to the public, prepare derivative
# works, and perform publicly and display publicly, and to permit others to do
# so.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# (1) Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#
# (2) Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#
# (3) Neither the name of the University of California, Lawrence Berkeley
#     National Laboratory, the University of Illinois, U.S. Dept. of Energy nor
#     the names of its contributors may be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
# (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in
#     stand-alone form without changes from the version obtained under this
#     License, or (ii) Licensee makes a reference solely to the software
#     portion of its product, Licensee must refer to the software as
#     "EnergyPlus version X" software, where "X" is the version number Licensee
#     obtained under this License and may not use a different name for the
#     software. Except as specifically required in this Section (4), Licensee
#     shall not use in a company name, a product name, in advertising,
#     publicity, or other promotional activities any name, trade name,
#     trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or
#     confusingly similar designation, without the U.S. Department of Energy's
#     prior written consent.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import os
from ctypes import CFUNCTYPE, c_char_p, c_int, c_void_p, cdll
from inspect import signature
from types import FunctionType
from typing import List, Union

# CFUNCTYPE wrapped Python callbacks need to be kept in memory explicitly, otherwise GC takes it
# This causes undefined behavior but generally segfaults and illegal access violations
# Keeping them referenced in a global array here suffices to keep GC away from it
# And while we _could_ clean up after ourselves, I would imagine users aren't making *so* many callbacks that
# We have anything to worry about here
all_callbacks = []


class Runtime:
    """
    This API class enables a client to hook into EnergyPlus at runtime and sense/actuate data in a running simulation.
    The pattern is quite simple: create a callback function in Python, and register it with one of the registration
    methods on this class to allow the callback to be called at a specific point in the simulation.  Inside the callback
    function, the client can get sensor values and set actuator values using the DataTransfer API methods, and also
    look up values and perform calculations using EnergyPlus internal methods via the Functional API methods.
    """

    def __init__(self, api: cdll):
        """
        Create a new Runtime class instance.

        :param api: An active CTYPES CDLL instance.
        """
        self.api = api
        # self.api.energyplus.argtypes = [c_int, POINTER(c_char_p)]  # DEFERRED UNTIL run_energyplus call
        self.api.energyplus.restype = c_int
        self.api.issueWarning.argtypes = [c_void_p, c_char_p]
        self.api.issueWarning.restype = c_void_p
        self.api.issueSevere.argtypes = [c_void_p, c_char_p]
        self.api.issueSevere.restype = c_void_p
        self.api.issueText.argtypes = [c_void_p, c_char_p]
        self.api.issueText.restype = c_void_p
        self.api.stopSimulation.argtypes = [c_void_p]
        self.api.stopSimulation.restype = c_void_p
        self.api.setConsoleOutputState.argtypes = [c_void_p, c_int]
        self.api.setConsoleOutputState.restype = c_void_p
        self.api.setEnergyPlusRootDirectory.argtypes = [c_void_p, c_char_p]
        self.api.setEnergyPlusRootDirectory.restype = c_void_p
        self.py_progress_callback_type = CFUNCTYPE(c_void_p, c_int)
        self.api.registerProgressCallback.argtypes = [c_void_p, self.py_progress_callback_type]
        self.api.registerProgressCallback.restype = c_void_p
        self.py_message_callback_type = CFUNCTYPE(c_void_p, c_char_p)
        self.api.registerStdOutCallback.argtypes = [c_void_p, self.py_message_callback_type]
        self.api.registerStdOutCallback.restype = c_void_p
        self.py_state_callback_type = CFUNCTYPE(c_void_p, c_void_p)
        self.api.callbackBeginNewEnvironment.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.callbackBeginNewEnvironment.restype = c_void_p
        self.api.callbackAfterNewEnvironmentWarmupComplete.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.callbackAfterNewEnvironmentWarmupComplete.restype = c_void_p
        self.api.callbackBeginZoneTimeStepBeforeInitHeatBalance.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.callbackBeginZoneTimeStepBeforeInitHeatBalance.restype = c_void_p
        self.api.callbackBeginZoneTimeStepAfterInitHeatBalance.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.callbackBeginZoneTimeStepAfterInitHeatBalance.restype = c_void_p
        self.api.callbackBeginTimeStepBeforePredictor.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.callbackBeginTimeStepBeforePredictor.restype = c_void_p
        self.api.callbackBeginZoneTimestepBeforeSetCurrentWeather.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.callbackBeginZoneTimestepBeforeSetCurrentWeather.restype = c_void_p
        self.api.callbackAfterPredictorBeforeHVACManagers.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.callbackAfterPredictorBeforeHVACManagers.restype = c_void_p
        self.api.callbackAfterPredictorAfterHVACManagers.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.callbackAfterPredictorAfterHVACManagers.restype = c_void_p
        self.api.callbackInsideSystemIterationLoop.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.callbackInsideSystemIterationLoop.restype = c_void_p
        self.api.callbackEndOfZoneTimeStepBeforeZoneReporting.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.callbackEndOfZoneTimeStepBeforeZoneReporting.restype = c_void_p
        self.api.callbackEndOfZoneTimeStepAfterZoneReporting.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.callbackEndOfZoneTimeStepAfterZoneReporting.restype = c_void_p
        self.api.callbackEndOfSystemTimeStepBeforeHVACReporting.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.callbackEndOfSystemTimeStepBeforeHVACReporting.restype = c_void_p
        self.api.callbackEndOfSystemTimeStepAfterHVACReporting.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.callbackEndOfSystemTimeStepAfterHVACReporting.restype = c_void_p
        self.api.callbackEndOfZoneSizing.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.callbackEndOfZoneSizing.restype = c_void_p
        self.api.callbackEndOfSystemSizing.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.callbackEndOfSystemSizing.restype = c_void_p
        self.api.callbackEndOfAfterComponentGetInput.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.callbackEndOfAfterComponentGetInput.restype = c_void_p
        self.api.callbackUserDefinedComponentModel.argtypes = [c_void_p, self.py_state_callback_type, c_char_p]
        self.api.callbackUserDefinedComponentModel.restype = c_void_p
        self.api.callbackUnitarySystemSizing.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.callbackUnitarySystemSizing.restype = c_void_p
        self.api.registerExternalHVACManager.argtypes = [c_void_p, self.py_state_callback_type]
        self.api.registerExternalHVACManager.restype = c_void_p

    @staticmethod
    def _check_callback_args(function_to_check: FunctionType, expected_num_args: int, calling_point_name: str):
        sig = signature(function_to_check)
        num_args = len(sig.parameters)
        if num_args != expected_num_args:
            raise TypeError(
                f"Registering function with incorrect arguments, calling point = {calling_point_name} needs {expected_num_args} arguments"
            )

    def _set_energyplus_root_directory(self, state, path: str):
        """
        Sets the EnergyPlus install root folder when calling EnergyPlus as a library.
        When calling the API from Python, this can be done automagically.  Python can find the currently running
        script, which is how this script finds the E+ DLL.  Because of this, this function is hidden in Python and the
        client does not need to call this.  When calling directly through the C layer, the client should call the
        underlying C function directly because C does not provide that same automagic.
        For a developer perspective, this should be called prior to running EnergyPlus.  When EnergyPlus is run as EXE,
        it is able to locate auxiliary tools relative to the running binary exe file.  However, when calling as an API,
        the running binary will not be in the EnergyPlus install, and so EnergyPlus will fail to find those auxiliary
        tools.  To call these tools when running as a library, the install root must be set using this function.
        It should be noted that in many workflows, it would be better to just call those auxiliary tools directly from
        an outside caller, rather than relying on EnergyPlus to do it via command line arguments.
        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param path: Path to the EnergyPlus install root, for example "C:\\EnergyPlus-9-5-0".  Because this function is
                     only called internally, a Python str is required for this argument, and it is encoded into utf-8
                     bytes before calling the C function to set to the install root inside the running EnergyPlus.
        :return: Nothing
        """
        path = path.encode("utf-8")
        self.api.setEnergyPlusRootDirectory(state, path)

    def run_energyplus(self, state: c_void_p, command_line_args: List[Union[str, bytes]]) -> int:
        """
        This function calls EnergyPlus to run a simulation.  The C API expects to find arguments matching the command
        line string when executing EnergyPlus.  When calling the C API directly, the client must create a list of char
        arguments starting with the program name, followed by all the command line options.  For this Python API, the
        program name is not passed in as an argument, rather only the command line options.

        An example call:
        state = api.state_manager.new_state()
        run_energyplus(state, ['-d', '/path/to/output/directory', '-w', '/path/to/weather.epw', '/path/to/input.idf'])

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param command_line_args: The command line arguments that would be passed into EnergyPlus if executing directly
                                  from the EnergyPlus executable.
        :return: An integer exit code from the simulation, zero is success, non-zero is failure
        """
        # note that we need to set the install root when we call E+ so that E+ can find the auxiliary tools
        # in Python, we can do this automatically with runtime introspection, the same way we find the E+ DLL itself
        this_file = os.path.realpath(__file__)  # returns C:\EnergyPlus\pyenergyplus\runtime.py
        this_script_dir = os.path.dirname(this_file)  # returns C:\EnergyPlus\pyenergyplus
        eplus_root_dir = os.path.dirname(os.path.normpath(this_script_dir))  # returns C:\EnergyPlus
        self._set_energyplus_root_directory(state, eplus_root_dir)

        args_with_program_name = [b"energyplus"]  # don't require the program name argument, just add it here
        for cla in command_line_args:
            if isinstance(cla, str):
                prepped_cla = cla.encode("utf-8")
            else:
                prepped_cla = cla
            args_with_program_name.append(prepped_cla)
        # Passing an array arg is a smidge tricky, as you need to cast to a fixed size array.
        # Thus we must dynamically create the argument "type" based on the current number of arguments.
        # The Python type checker may be complaining that c_char_p is actually needing to be an integer;
        # this is something to do with the dynamic nature of the ctypes "type" objects.
        # For now I'm adding this noinspection marker to get it to hush
        # noinspection PyTypeChecker
        cli_arg_type = c_char_p * len(args_with_program_name)
        # OK, so now that we have the argument "type" set as a fixed length array of char*, we assign it to the
        # DLL function call.  This must be done here because the arg count may change with each call.
        self.api.energyplus.argtypes = [c_void_p, c_int, cli_arg_type]
        # Now cast the actual Python list instance into that dynamically created ctype "type".  This is another
        # line where the type checker is complaining because it things cli_arg_type is actually an "int", and as such,
        # it wouldn't be a callable item.  cli_arg_type is actually a callable type, so I'm muting this warning.
        # noinspection PyCallingNonCallable
        cli_args = cli_arg_type(*args_with_program_name)
        return self.api.energyplus(state, len(args_with_program_name), cli_args)

    def stop_simulation(self, state: c_void_p) -> None:
        self.api.stopSimulation(state)

    def set_console_output_status(self, state, print_output: bool) -> None:
        """
        Allows disabling (and enabling) of console output (stdout and stderr) when calling EnergyPlus as a library.
        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param print_output: A boolean flag for whether we should mute console output
        :return: Nothing
        """
        if print_output:
            self.api.setConsoleOutputState(state, 1)
        else:
            self.api.setConsoleOutputState(state, 0)

    def issue_warning(self, state: c_void_p, message: Union[str, bytes]) -> None:
        """
        This function allows a script to issue a warning through normal EnergyPlus methods.  The message will be listed
        in the standard EnergyPlus error file once the simulation is complete.  This function has limited usefulness
        when calling EnergyPlus as a library, as errors can be handled by the calling client, however, in a
        PythonPlugin workflow, this can be an important tool to alert the user of issues once EnergyPlus(.exe) has
        finished running.

        Note that the argument passed in here can be either a string or a bytes object, as this wrapper handles
        conversion as needed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param message: The warning message to be listed in the error file.
        :return: Nothing
        """
        if isinstance(message, str):
            message = message.encode("utf-8")
        self.api.issueWarning(state, message)

    def issue_severe(self, state: c_void_p, message: Union[str, bytes]) -> None:
        """
        This function allows a script to issue an error through normal EnergyPlus methods.  The message will be listed
        in the standard EnergyPlus error file once the simulation is complete.  This function has limited usefulness
        when calling EnergyPlus as a library, as errors can be handled by the calling client, however, in a
        PythonPlugin workflow, this can be an important tool to alert the user of issues once EnergyPlus(.exe) has
        finished running.

        Note the severe errors tend to lead to EnergyPlus terminating with a Fatal Error.  This can be accomplished
        in PythonPlugin workflows by issuing a severe error, followed by returning 1 from the plugin function.
        EnergyPlus will interpret this return value as a signal to terminate with a fatal error.

        Note that the argument passed in here can be either a string or a bytes object, as this wrapper handles
        conversion as needed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param message: The error message to be listed in the error file.
        :return: Nothing
        """
        if isinstance(message, str):
            message = message.encode("utf-8")
        self.api.issueSevere(state, message)

    def issue_text(self, state: c_void_p, message: Union[str, bytes]) -> None:
        """
        This function allows a script to issue a message through normal EnergyPlus methods.  The message will be listed
        in the standard EnergyPlus error file once the simulation is complete.  This function can be used alongside the
        warning and error issuance functions to provide further context to the situation. This function has limited
        usefulness when calling EnergyPlus as a library, as errors can be handled by the calling client, however, in a
        PythonPlugin workflow, this can be an important tool to alert the user of issues once EnergyPlus(.exe) has
        finished running.

        Note that the argument passed in here can be either a string or a bytes object, as this wrapper handles
        conversion as needed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param message: The message to be listed in the error file.
        :return: Nothing
        """
        if isinstance(message, str):
            message = message.encode("utf-8")
        self.api.issueText(state, message)

    def callback_progress(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus at the end of each
        day with a progress (percentage) indicator

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes an integer argument and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_progress")
        cb_ptr = self.py_progress_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.registerProgressCallback(state, cb_ptr)

    def callback_message(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus when printing anything
        to standard output.  This can allow a GUI to easily show the output of EnergyPlus streaming by.  When used in
        conjunction with the progress callback, a progress bar and status text label can provide a nice EnergyPlus
        experience on a GUI.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes a string (bytes) argument and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_message")
        cb_ptr = self.py_message_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.registerStdOutCallback(state, cb_ptr)

    def callback_begin_new_environment(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus at the beginning of
        each environment.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_begin_new_environment")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.callbackBeginNewEnvironment(state, cb_ptr)

    def callback_after_new_environment_warmup_complete(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus at the warmup of
        each environment.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_after_new_environment_warmup_complete")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.callbackAfterNewEnvironmentWarmupComplete(state, cb_ptr)

    def callback_begin_zone_timestep_before_init_heat_balance(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus at the beginning of the
        zone time step before init heat balance.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_begin_zone_timestep_before_init_heat_balance")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.callbackBeginZoneTimeStepBeforeInitHeatBalance(state, cb_ptr)

    def callback_begin_zone_timestep_after_init_heat_balance(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus at the beginning of the
        zone time step after init heat balance.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_begin_zone_timestep_after_init_heat_balance")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.callbackBeginZoneTimeStepAfterInitHeatBalance(state, cb_ptr)

    def callback_begin_system_timestep_before_predictor(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus at the beginning of
        system time step .

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_begin_system_timestep_before_predictor")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.callbackBeginTimeStepBeforePredictor(state, cb_ptr)

    def callback_begin_zone_timestep_before_set_current_weather(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus at the beginning of
        zone time step, before weather is updated.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_begin_zone_timestep_before_set_current_weather")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.callbackBeginZoneTimestepBeforeSetCurrentWeather(state, cb_ptr)

    def callback_after_predictor_before_hvac_managers(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus at the end of the
        predictor step but before HVAC managers.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_after_predictor_before_hvac_managers")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.callbackAfterPredictorBeforeHVACManagers(state, cb_ptr)

    def callback_after_predictor_after_hvac_managers(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus at the end of the
        predictor step after HVAC managers.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_after_predictor_after_hvac_managers")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.callbackAfterPredictorAfterHVACManagers(state, cb_ptr)

    def callback_inside_system_iteration_loop(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus inside the system
        iteration loop.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_inside_system_iteration_loop")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.callbackInsideSystemIterationLoop(state, cb_ptr)

    def callback_end_zone_timestep_before_zone_reporting(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus at the end of a zone
        time step but before zone reporting has been completed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_end_zone_timestep_before_zone_reporting")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.callbackEndOfZoneTimeStepBeforeZoneReporting(state, cb_ptr)

    def callback_end_zone_timestep_after_zone_reporting(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus at the end of a zone
        time step and after zone reporting.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_end_zone_timestep_after_zone_reporting")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.callbackEndOfZoneTimeStepAfterZoneReporting(state, cb_ptr)

    def callback_end_system_timestep_before_hvac_reporting(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus at the end of a system
        time step, but before HVAC reporting.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_end_system_timestep_before_hvac_reporting")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.callbackEndOfSystemTimeStepBeforeHVACReporting(state, cb_ptr)

    def callback_end_system_timestep_after_hvac_reporting(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus at the end of a system
        time step and after HVAC reporting.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_end_system_timestep_after_hvac_reporting")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.callbackEndOfSystemTimeStepAfterHVACReporting(state, cb_ptr)

    def callback_end_zone_sizing(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus at the end of the zone
        sizing process.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_end_zone_sizing")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.callbackEndOfZoneSizing(state, cb_ptr)

    def callback_end_system_sizing(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus at the end of the system
        sizing process.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_end_system_sizing")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.callbackEndOfSystemSizing(state, cb_ptr)

    def callback_after_component_get_input(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus at the end of
        component get input processes.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_after_component_get_input")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.callbackEndOfAfterComponentGetInput(state, cb_ptr)

    def callback_user_defined_component_model(self, state: c_void_p, f: FunctionType, program_name: str) -> None:
        """
        This function allows a client to register a function to be called by a specific user-defined equipment object
        inside EnergyPlus.  This registration function takes a string for the program "name" which should match the
        name given in the IDF.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing.
        :param program_name: The program name which is listed in the IDF on the user-defined object, either as an
                             initialization program name or a simulation program name.
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_user_defined_component_model")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        if isinstance(program_name, str):
            program_name = program_name.encode("utf-8")
        self.api.callbackUserDefinedComponentModel(state, cb_ptr, program_name)

    def callback_unitary_system_sizing(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus in unitary system sizing.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_unitary_system_sizing")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.callbackUnitarySystemSizing(state, cb_ptr)

    def callback_register_external_hvac_manager(self, state: c_void_p, f: FunctionType) -> None:
        """
        This function allows a client to register an external HVAC manager function to be called back in EnergyPlus.
        By registering this function, EnergyPlus will bypass all HVAC calculations and expect that this function will
        manage all HVAC through sensors and actuators. Right now this function is not well-supported, and this callback
        should be considered purely as a placeholder until a future release refines the use case.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes one argument, the current state instance, and returns nothing
        :return: Nothing
        """
        self._check_callback_args(f, 1, "callback_register_external_hvac_manager")
        cb_ptr = self.py_state_callback_type(f)
        all_callbacks.append(cb_ptr)
        self.api.registerExternalHVACManager(state, cb_ptr)

    @staticmethod
    def clear_callbacks() -> None:
        """
        This function is used if you are running this script continually making multiple calls into the E+ library in
        one thread.  EnergyPlus should be cleaned up between runs.

        Note this will clean all registered callbacks, so functions must be registered again prior to the next run.

        :return: Nothing
        """
        all_callbacks.clear()
