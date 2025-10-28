# EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University
# of Illinois, The Regents of the University of California, through Lawrence
# Berkeley National Laboratory (subject to receipt of any required approvals
# from the U.S. Dept. of Energy), Oak Ridge National Laboratory, managed by UT-
# Battelle, Alliance for Sustainable Energy, LLC, and other contributors. All
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

from ctypes import cdll, c_int, c_char_p, c_void_p, POINTER, Structure, byref
from pyenergyplus.common import RealEP, EnergyPlusException, is_number
from typing import List, Union
from pathlib import Path

class DataExchange:
    """
    This API class enables data transfer between EnergyPlus and a client.  Output variables and meters are treated as
    "sensor" data.  A client should get a handle (integer) using one of the worker methods then make calls to get data
    using that handle.  Some specific variables in EnergyPlus are controllable as actuators.  These work the same way,
    in that a client should get an integer handle to an actuator and then use the workers to set the value on the
    actuator.  There are also some static data members in EnergyPlus that are exposed as "internal" variables.  These
    variables hold static data such as zone floor area and volume, so they do not change during a simulation, but are
    constant once they are assigned.  Even with this difference, the variables are still handled the same way, by
    getting an integer handle and then accessing the data using that handle.

    This data transfer class is used in one of two workflows:

    - When an outside tool is already running EnergyPlus using the Runtime API, and data transfer is to be made during
      callback functions. In this case, the script should create a DataTransfer API class by calling the `data_transfer`
      method on the main API class, never trying to create this class directly.
    - When a Python script is used in the EnergyPlus Python Plugin System, and the user runs the EnergyPlus binary.  In
      this case, the plugin may need access to state data to make control decisions, and this class enables that.  The
      plugin base class automatically creates an instance of this class, so client plugins that inherit that class will
      have a `self.api.exchange` instance of this class available, and should *not* attempt to create another one.

    Client Python code may make use of the methods to get/set plugin global variables, but only in the Python Plugin
    cases.  For the outside tool API usage, plugin global variables are not available, and data should be shared in the
    outside calling code.
    """

    class _APIDataEntry(Structure):
        _fields_ = [("what", c_char_p),("name", c_char_p),("key", c_char_p),("type", c_char_p),("unit", c_char_p)]

    class APIDataExchangePoint:
        """
        A class of string members that describe a single API exchange point.  The exchange described in this
        class could represent output variables, output meters, actuators, and more.  The "type" member variable
        can be used to filter a specific type.
        """
        def __init__(self, _what: str, _name: str, _key: str, _type: str, _unit: str):
            #: This variable will hold the basic type of API data point, in string form.
            #: This can be one of the following: "Actuator", "InternalVariable", "PluginGlobalVariable",
            #: "PluginTrendVariable", "OutputMeter", or "OutputVariable".  Once the full list of data exchange points
            #: are returned from a call to get_api_data, this can be used to quickly filter down to a specific type.
            self.what: str = _what
            #: This represents the name of the entry point, not the name of the specific instance of the entry point.
            #: Some examples of this name could be "Chiller Heat Transfer Rate" -- which could be available for multiple
            #: chillers.
            self.name: str = _name
            #: This represents the unique ID for this exchange point.  In the example of the chiller output variable,
            #: this could be "Chiller 1". This is not used for meters
            self.key: str = _key
            #: This represents the "type" of exchange for this exchange point.  This is only used for actuators,
            #: and represents the control actuation.  For a node setpoint actuation, this could be "temperature" or
            #: "humidity", for example.
            self.type: str = _type
            #: This represents the unit of measure for this exchange point.
            #: This is NOT used for plugin variables such as PluginGlobalVariable and PluginTrendVariable.
            self.unit: str = _unit

    def __init__(self, api: cdll, running_as_python_plugin: bool = False):
        """
        Creates a new DataExchange API class instance

        :param api: An active CTYPES CDLL instance
        :param running_as_python_plugin: A flag for whether we are running in plugin mode
        """
        self.api = api
        self.running_as_python_plugin = running_as_python_plugin
        self.api.getAPIData.argtypes = [c_void_p, POINTER(c_int)]
        self.api.getAPIData.restype = POINTER(DataExchange._APIDataEntry)
        self.api.listAllAPIDataCSV.argtypes = [c_void_p]
        self.api.listAllAPIDataCSV.restype = c_char_p
        self.api.freeAPIData.argtypes = [POINTER(DataExchange._APIDataEntry), c_int]
        self.api.freeAPIData.restype = c_void_p
        self.api.getObjectNames.argtypes = [c_void_p, c_char_p, POINTER(c_int)]
        self.api.getObjectNames.restype = POINTER(c_char_p)
        self.api.freeObjectNames.argtypes = [POINTER(c_char_p), c_int]
        self.api.freeObjectNames.restype = c_void_p
        self.api.apiDataFullyReady.argtypes = [c_void_p]
        self.api.apiDataFullyReady.restype = c_int
        self.api.apiErrorFlag.argtypes = [c_void_p]
        self.api.apiErrorFlag.restype = c_int
        self.api.resetErrorFlag.argtypes = [c_void_p]
        self.api.resetErrorFlag.restype = c_void_p
        self.api.inputFilePath.argtypes = [c_void_p]
        self.api.inputFilePath.restype = c_char_p
        self.api.epwFilePath.argtypes = [c_void_p]
        self.api.epwFilePath.restype = c_char_p
        self.api.requestVariable.argtypes = [c_void_p, c_char_p, c_char_p]
        self.api.getNumNodesInCondFDSurfaceLayer.argtypes = [c_void_p, c_char_p, c_char_p]
        self.api.requestVariable.restype = c_void_p
        self.api.getVariableHandle.argtypes = [c_void_p, c_char_p, c_char_p]
        self.api.getVariableHandle.restype = c_int
        self.api.getMeterHandle.argtypes = [c_void_p, c_char_p]
        self.api.getMeterHandle.restype = c_int
        self.api.getActuatorHandle.argtypes = [c_void_p, c_char_p, c_char_p, c_char_p]
        self.api.getActuatorHandle.restype = c_int
        self.api.getVariableValue.argtypes = [c_void_p, c_int]
        self.api.getVariableValue.restype = RealEP
        self.api.getMeterValue.argtypes = [c_void_p, c_int]
        self.api.getMeterValue.restype = RealEP
        self.api.setActuatorValue.argtypes = [c_void_p, c_int, RealEP]
        self.api.setActuatorValue.restype = c_void_p
        self.api.resetActuator.argtypes = [c_void_p, c_int]
        self.api.resetActuator.restype = c_void_p
        self.api.getActuatorValue.argtypes = [c_void_p, c_int]
        self.api.getActuatorValue.restype = RealEP
        self.api.getInternalVariableHandle.argtypes = [c_void_p, c_char_p, c_char_p]
        self.api.getInternalVariableHandle.restype = c_int
        self.api.getInternalVariableValue.argtypes = [c_void_p, c_int]
        self.api.getInternalVariableValue.restype = RealEP
        # some simulation data values are available for plugins or regular runtime calls
        self.api.year.argtypes = [c_void_p]
        self.api.year.restype = c_int
        self.api.calendarYear.argtypes = [c_void_p]
        self.api.calendarYear.restype = c_int
        self.api.month.argtypes = [c_void_p]
        self.api.month.restype = c_int
        self.api.dayOfMonth.argtypes = [c_void_p]
        self.api.dayOfMonth.restype = c_int
        self.api.dayOfWeek.argtypes = [c_void_p]
        self.api.dayOfWeek.restype = c_int
        self.api.dayOfYear.argtypes = [c_void_p]
        self.api.dayOfYear.restype = c_int
        self.api.daylightSavingsTimeIndicator.argtypes = [c_void_p]
        self.api.daylightSavingsTimeIndicator.restype = c_int
        self.api.hour.argtypes = [c_void_p]
        self.api.hour.restype = c_int
        self.api.numTimeStepsInHour.argtypes = [c_void_p]
        self.api.numTimeStepsInHour.restype = c_int
        self.api.zoneTimeStepNum.argtypes = [c_void_p]
        self.api.zoneTimeStepNum.restype = c_int
        self.api.currentTime.argtypes = [c_void_p]
        self.api.currentTime.restype = RealEP
        self.api.minutes.argtypes = [c_void_p]
        self.api.minutes.restype = c_int
        self.api.holidayIndex.argtypes = [c_void_p]
        self.api.holidayIndex.restype = c_int
        self.api.sunIsUp.argtypes = [c_void_p]
        self.api.sunIsUp.restype = c_int
        self.api.isRaining.argtypes = [c_void_p]
        self.api.isRaining.restype = c_int
        self.api.zoneTimeStep.argtypes = [c_void_p]
        self.api.zoneTimeStep.restype = RealEP
        self.api.systemTimeStep.argtypes = [c_void_p]
        self.api.systemTimeStep.restype = RealEP
        self.api.currentEnvironmentNum.argtypes = [c_void_p]
        self.api.currentEnvironmentNum.restype = c_int
        self.api.warmupFlag.argtypes = [c_void_p]
        self.api.warmupFlag.restype = c_int
        self.api.getEMSGlobalVariableHandle.argtypes = [c_void_p, c_char_p]
        self.api.getEMSGlobalVariableHandle.restype = c_int
        self.api.getEMSGlobalVariableValue.argtypes = [c_void_p, c_int]
        self.api.getEMSGlobalVariableValue.restype = RealEP
        self.api.setEMSGlobalVariableValue.argtypes = [c_void_p, c_int, RealEP]
        self.api.setEMSGlobalVariableValue.restype = c_void_p
        self.api.getPluginGlobalVariableHandle.argtypes = [c_void_p, c_char_p]
        self.api.getPluginGlobalVariableHandle.restype = c_int
        self.api.getPluginGlobalVariableValue.argtypes = [c_void_p, c_int]
        self.api.getPluginGlobalVariableValue.restype = RealEP
        self.api.setPluginGlobalVariableValue.argtypes = [c_void_p, c_int, RealEP]
        self.api.setPluginGlobalVariableValue.restype = c_void_p
        self.api.getPluginTrendVariableHandle.argtypes = [c_void_p, c_char_p]
        self.api.getPluginTrendVariableHandle.restype = c_int
        self.api.getPluginTrendVariableValue.argtypes = [c_void_p, c_int, c_int]
        self.api.getPluginTrendVariableValue.restype = RealEP
        self.api.getPluginTrendVariableAverage.argtypes = [c_void_p, c_int, c_int]
        self.api.getPluginTrendVariableAverage.restype = RealEP
        self.api.getPluginTrendVariableMin.argtypes = [c_void_p, c_int, c_int]
        self.api.getPluginTrendVariableMin.restype = RealEP
        self.api.getPluginTrendVariableMax.argtypes = [c_void_p, c_int, c_int]
        self.api.getPluginTrendVariableMax.restype = RealEP
        self.api.getPluginTrendVariableSum.argtypes = [c_void_p, c_int, c_int]
        self.api.getPluginTrendVariableSum.restype = RealEP
        self.api.getPluginTrendVariableDirection.argtypes = [c_void_p, c_int, c_int]
        self.api.getPluginTrendVariableDirection.restype = RealEP
        self.api.getConstructionHandle.argtypes = [c_void_p, c_char_p]
        self.api.getConstructionHandle.restype = c_int
        self.api.actualTime.argtypes = [c_void_p]
        self.api.actualTime.restype = c_int
        self.api.actualDateTime.argtypes = [c_void_p]
        self.api.actualDateTime.restype = c_int
        self.api.kindOfSim.argtypes = [c_void_p]
        self.api.kindOfSim.restype = c_int
        self.api.todayWeatherIsRainAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.todayWeatherIsRainAtTime.restype = c_int
        self.api.todayWeatherIsSnowAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.todayWeatherIsSnowAtTime.restype = c_int
        self.api.todayWeatherOutDryBulbAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.todayWeatherOutDryBulbAtTime.restype = RealEP
        self.api.todayWeatherOutDewPointAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.todayWeatherOutDewPointAtTime.restype = RealEP
        self.api.todayWeatherOutBarometricPressureAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.todayWeatherOutBarometricPressureAtTime.restype = RealEP
        self.api.todayWeatherOutRelativeHumidityAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.todayWeatherOutRelativeHumidityAtTime.restype = RealEP
        self.api.todayWeatherWindSpeedAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.todayWeatherWindSpeedAtTime.restype = RealEP
        self.api.todayWeatherWindDirectionAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.todayWeatherWindDirectionAtTime.restype = RealEP
        self.api.todayWeatherSkyTemperatureAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.todayWeatherSkyTemperatureAtTime.restype = RealEP
        self.api.todayWeatherHorizontalIRSkyAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.todayWeatherHorizontalIRSkyAtTime.restype = RealEP
        self.api.todayWeatherBeamSolarRadiationAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.todayWeatherBeamSolarRadiationAtTime.restype = RealEP
        self.api.todayWeatherDiffuseSolarRadiationAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.todayWeatherDiffuseSolarRadiationAtTime.restype = RealEP
        self.api.todayWeatherAlbedoAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.todayWeatherAlbedoAtTime.restype = RealEP
        self.api.todayWeatherLiquidPrecipitationAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.todayWeatherLiquidPrecipitationAtTime.restype = RealEP
        self.api.tomorrowWeatherIsRainAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.tomorrowWeatherIsRainAtTime.restype = c_int
        self.api.tomorrowWeatherIsSnowAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.tomorrowWeatherIsSnowAtTime.restype = c_int
        self.api.tomorrowWeatherOutDryBulbAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.tomorrowWeatherOutDryBulbAtTime.restype = RealEP
        self.api.tomorrowWeatherOutDewPointAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.tomorrowWeatherOutDewPointAtTime.restype = RealEP
        self.api.tomorrowWeatherOutBarometricPressureAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.tomorrowWeatherOutBarometricPressureAtTime.restype = RealEP
        self.api.tomorrowWeatherOutRelativeHumidityAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.tomorrowWeatherOutRelativeHumidityAtTime.restype = RealEP
        self.api.tomorrowWeatherWindSpeedAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.tomorrowWeatherWindSpeedAtTime.restype = RealEP
        self.api.tomorrowWeatherWindDirectionAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.tomorrowWeatherWindDirectionAtTime.restype = RealEP
        self.api.tomorrowWeatherSkyTemperatureAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.tomorrowWeatherSkyTemperatureAtTime.restype = RealEP
        self.api.tomorrowWeatherHorizontalIRSkyAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.tomorrowWeatherHorizontalIRSkyAtTime.restype = RealEP
        self.api.tomorrowWeatherBeamSolarRadiationAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.tomorrowWeatherBeamSolarRadiationAtTime.restype = RealEP
        self.api.tomorrowWeatherDiffuseSolarRadiationAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.tomorrowWeatherDiffuseSolarRadiationAtTime.restype = RealEP
        self.api.tomorrowWeatherAlbedoAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.tomorrowWeatherAlbedoAtTime.restype = RealEP
        self.api.tomorrowWeatherLiquidPrecipitationAtTime.argtypes = [c_void_p, c_int, c_int]
        self.api.tomorrowWeatherLiquidPrecipitationAtTime.restype = RealEP
        self.api.currentSimTime.argtypes = [c_void_p]
        self.api.currentSimTime.restype = RealEP

    def get_api_data(self, state: c_void_p) -> List[APIDataExchangePoint]:
        """
        Returns a nicely formed list of API data exchange points available in the current simulation.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: Returns a Python list of APIDataExchangePoint instances, which can be filtered to inspect exchanges.
        """
        count = c_int()
        r = self.api.getAPIData(state, byref(count))
        list_response = [
            DataExchange.APIDataExchangePoint(
                r[i].what.decode('utf-8'),
                r[i].name.decode('utf-8'),
                r[i].key.decode('utf-8'),
                r[i].type.decode('utf-8'),
                r[i].unit.decode('utf-8'),
            ) for i in range(count.value)
        ]
        self.api.freeAPIData(r, count)  # free the underlying C memory now that we have a Python copy
        return list_response

    def list_available_api_data_csv(self, state: c_void_p) -> bytes:
        """
        Lists out all API data stuff in an easily parseable CSV form

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: Returns a raw bytes CSV representation of the available API data
        """
        return self.api.listAllAPIDataCSV(state)

    def api_data_fully_ready(self, state: c_void_p) -> bool:
        """
        Check whether the data exchange API is fully ready.
        Handles to variables, actuators, and other data are not guaranteed to be defined prior to this being true.
        Up until this point some output vars, meters, actuators, etc., may not have been registered yet.

        :param state: An active EnergyPlus "state" that is returned from a call to :func:`api.state_manager.new_state() <state.StateManager.new_state>`.
        :return: Returns a boolean value to indicate whether variables, actuators, and other data are ready for access.

        .. note::
            In the case of Surface actuators for "Construction State" and for a call to :func:`get_construction_handle`,
            this is not required to be true, as construction data is generally available earlier in the simulation process.
            Bypassing this check will allow affecting the Sizing calculations.
        """
        if self.api.apiDataFullyReady(state) == 1:
            return True
        return False

    def api_error_flag(self, state: c_void_p) -> bool:
        """
        Check whether the error flag has been activated.
        A number of functions will return 0 in erroneous situations, and this function allows for disambiguation
        between valid zero return values and the error condition.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: Returns true if the error flag was activated during prior calculations.
        """
        if self.api.apiErrorFlag(state) == 1:
            return True
        return False

    def reset_api_error_flag(self, state: c_void_p) -> None:
        """
        Resets the error flag for API calls.
        A number of functions will return 0 in erroneous situations, but activate an error flag.  In certain work flows,
        it may be useful to reset this error flag (unit testing, etc.).  This function allows resetting it to false.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        """
        self.api.resetErrorFlag(state)

    def get_input_file_path(self, state: c_void_p) -> Path:
        """
        Provides the input file path back to the client. In most circumstances the client will know the path to the
        input file, but there are some cases where code is generalized in unexpected workflows.  Users have requested
        a way to get the input file path back from the running instance.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: A pathlib.Path of the input file path
        """
        c_string = self.api.inputFilePath(state)
        res = Path(c_string.decode('utf-8'))
        return res

    def get_weather_file_path(self, state: c_void_p) -> Path:
        """
        Provides the weather file path back to the client. In most circumstances the client will know the path to the
        weather file, but there are some cases where code is generalized in unexpected workflows.  Users have requested
        a way to get the weather file path back from the running instance.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: A pathlib.Path of the weather file
        """
        c_string = self.api.epwFilePath(state)
        res = Path(c_string.decode('utf-8'))
        return res


    def get_object_names(self, state: c_void_p, object_type_name: Union[str, bytes]) -> List[str]:
        """
        Gets the instance names for a given object type in the current input file
        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param object_type_name: The object type name as defined in the schema, such as "Zone" or "Chiller:Electric"
        :return: A list of strings represented the names of the objects in the input file
        """

        if isinstance(object_type_name, str):
            object_type_name = object_type_name.encode('utf-8')
        elif not isinstance(object_type_name, bytes):
            raise EnergyPlusException(
                "`get_object_names` expects `object_type_name` as a `str` or UTF-8 encoded `bytes`, not "
                "'{}'".format(object_type_name))
        count = c_int()
        r = self.api.getObjectNames(state, object_type_name, byref(count))
        if not r:
            return []
        list_response = [r[i].decode('utf-8', errors='ignore') for i in range(count.value)]
        self.api.freeObjectNames(r, count)  # free the underlying C memory now that we have a Python copy
        return list_response

    def get_num_nodes_in_cond_fd_surf_layer(self, state: c_void_p, surface_name: Union[str, bytes],
                                            material_name: Union[str, bytes]) -> None:
        """
        Get the number of nodes in CondFD surface layer.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param surface_name: The name of the surface as defined in the input file,
                               such as "ZN001:Surf001".
        :param material_name: The name of the surface material layer as defined in the input file,
                               such as "GypsumBoardLayer".
        :return: Nothing
        """
        if isinstance(surface_name, str):
            surface_name = surface_name.encode('utf-8')
        elif not isinstance(surface_name, bytes):
            raise EnergyPlusException(
                "`get_num_nodes_in_cond_fd_surf_layer` expects `surface_name` as a `str` or UTF-8 encoded `bytes`, not"
                " '{}'".format(surface_name))
        if isinstance(material_name, str):
            material_name = material_name.encode('utf-8')
        elif not isinstance(material_name, bytes):
            raise EnergyPlusException(
                "`get_num_nodes_in_cond_fd_surf_layer` expects `material_name` as a `str` or UTF-8 encoded `bytes`, not"
                " '{}'".format(material_name))
        return self.api.getNumNodesInCondFDSurfaceLayer(state, surface_name, material_name)

    def request_variable(self, state: c_void_p, variable_name: Union[str, bytes],
                         variable_key: Union[str, bytes]) -> None:
        """
        Request output variables so they can be accessed during a simulation.

        In EnergyPlus, not all variables are available by default.  If they were all available, there would be a
        terrible memory impact.  Instead, only requested and necessary variables are kept in memory.  When running
        EnergyPlus as a program, including when using Python Plugins, variables are requested through input objects.
        When running EnergyPlus as a library, variables can also be requested through this function call.  This
        function has the same signature as the get_variable_handle function, which is used to then request the ID
        of a variable once the simulation has begun.  NOTE: Variables should be requested before *each* run of
        EnergyPlus, as the internal array is cleared when clearing the state of each run.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param variable_name: The name of the variable to retrieve, e.g. "Site Outdoor Air DryBulb Temperature", or
                              "Fan Air Mass Flow Rate"
        :param variable_key: The instance of the variable to retrieve, e.g. "Environment", or "Main System Fan"
        :return: Nothing
        """
        if isinstance(variable_name, str):
            variable_name = variable_name.encode('utf-8')
        elif not isinstance(variable_name, bytes):
            raise EnergyPlusException(
                "`request_variable` expects `component_type` as a `str` or UTF-8 encoded `bytes`, not "
                "'{}'".format(variable_name))
        if isinstance(variable_key, str):
            variable_key = variable_key.encode('utf-8')
        elif not isinstance(variable_key, bytes):
            raise EnergyPlusException(
                "`request_variable` expects `component_type` as a `str` or UTF-8 encoded `bytes`, not "
                "'{}'".format(variable_key))
        self.api.requestVariable(state, variable_name, variable_key)

    def get_variable_handle(self, state: c_void_p, variable_name: Union[str, bytes],
                            variable_key: Union[str, bytes]) -> int:
        """
        Get a handle to an output variable in a running simulation.

        The arguments passed into this function do not need to be a particular case, as the EnergyPlus API
        automatically converts values to upper-case when finding matches to internal variables in the simulation.

        Note also that the arguments passed in here can be either strings or bytes, as this wrapper handles conversion
        as needed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param variable_name: The name of the variable to retrieve, e.g. "Site Outdoor Air DryBulb Temperature", or
                              "Fan Air Mass Flow Rate"
        :param variable_key: The instance of the variable to retrieve, e.g. "Environment", or "Main System Fan"
        :return: An integer ID for this output variable, or -1 if one could not be found.
        """
        if isinstance(variable_name, str):
            variable_name = variable_name.encode('utf-8')
        elif not isinstance(variable_name, bytes):
            raise EnergyPlusException(
                "`get_variable_handle` expects `component_type` as a `str` or UTF-8 encoded `bytes`, not "
                "'{}'".format(variable_name))
        if isinstance(variable_key, str):
            variable_key = variable_key.encode('utf-8')
        elif not isinstance(variable_key, bytes):
            raise EnergyPlusException(
                "`get_variable_handle` expects `component_type` as a `str` or UTF-8 encoded `bytes`, not "
                "'{}'".format(variable_key))
        return self.api.getVariableHandle(state, variable_name, variable_key)

    def get_meter_handle(self, state: c_void_p, meter_name: Union[str, bytes]) -> int:
        """
        Get a handle to a meter in a running simulation.

        The meter name passed into this function do not need to be a particular case, as the EnergyPlus API
        automatically converts values to upper-case when finding matches to internal variables in the simulation.

        Note also that the meter name passed in here can be either strings or bytes, as this wrapper handles conversion
        as needed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param meter_name: The name of the variable to retrieve, e.g. "Electricity:Facility", or "Fans:Electricity"
        :return: An integer ID for this meter, or -1 if one could not be found.
        """
        meter_name = meter_name.upper()
        if isinstance(meter_name, str):
            meter_name = meter_name.encode('utf-8')
        elif not isinstance(meter_name, bytes):
            raise EnergyPlusException(
                "`get_meter_handle` expects `component_type` as a `str` or UTF-8 encoded `bytes`, not "
                "'{}'".format(meter_name))
        return self.api.getMeterHandle(state, meter_name)

    def get_actuator_handle(
            self,
            state: c_void_p,
            component_type: Union[str, bytes],
            control_type: Union[str, bytes],
            actuator_key: Union[str, bytes]
    ) -> int:
        """
        Get a handle to an available actuator in a running simulation.

        The arguments passed into this function do not need to be a particular case, as the EnergyPlus API
        automatically converts values to upper-case when finding matches to internal variables in the simulation.

        Note also that the arguments passed in here can be either strings or bytes, as this wrapper handles conversion
        as needed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param component_type: The actuator category, e.g. "Weather Data"
        :param control_type: The name of the actuator to retrieve, e.g. "Outdoor Dew Point"
        :param actuator_key: The instance of the variable to retrieve, e.g. "Environment"
        :return: An integer ID for this output variable, or -1 if one could not be found.
        """
        if isinstance(component_type, str):
            component_type = component_type.encode('utf-8')
        elif not isinstance(component_type, bytes):
            raise EnergyPlusException(
                "`get_actuator_handle` expects `component_type` as a `str` or UTF-8 encoded `bytes`, not "
                "'{}'".format(component_type))
        if isinstance(control_type, str):
            control_type = control_type.encode('utf-8')
        elif not isinstance(control_type, bytes):
            raise EnergyPlusException(
                "`get_actuator_handle` expects `component_type` as a `str` or UTF-8 encoded `bytes`, not "
                "'{}'".format(control_type))
        if isinstance(actuator_key, str):
            actuator_key = actuator_key.encode('utf-8')
        elif not isinstance(actuator_key, bytes):
            raise EnergyPlusException(
                "`get_actuator_handle` expects `component_type` as a `str` or UTF-8 encoded `bytes`, not "
                "'{}'".format(actuator_key))
        return self.api.getActuatorHandle(state, component_type, control_type, actuator_key)

    def get_variable_value(self, state: c_void_p, variable_handle: int) -> float:
        """
        Get the current value of a variable in a running simulation.  The `get_variable_handle` function is first used
        to get a handle to the variable by name.  Then once the handle is retrieved, it is passed into this function to
        then get the value of the variable.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param variable_handle: An integer returned from the `get_variable_handle` function.
        :return: Floating point representation of the current variable value.  Returns zero if the handle is invalid.
                 Use the api_error_flag function to disambiguate between valid zero returns and error states.
        """
        if not is_number(variable_handle):
            raise EnergyPlusException(
                "`get_variable_value` expects `variable_handle` as an `int`, not "
                "'{}'".format(variable_handle))
        return self.api.getVariableValue(state, variable_handle)

    def get_meter_value(self, state: c_void_p, meter_handle: int) -> float:
        """
        Get the current value of a meter in a running simulation.  The `get_meter_handle` function is first used
        to get a handle to the meter by name.  Then once the handle is retrieved, it is passed into this function to
        then get the value of the meter.

        Caution: This function currently returns the instantaneous value of a meter, not the cumulative value.
        This will change in a future version of the API.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param meter_handle: An integer returned from the `get_meter_handle` function.
        :return: Floating point representation of the current meter value.  Returns zero if the handle is invalid.
                 Use the api_error_flag function to disambiguate between valid zero returns and error states.
        """
        if not is_number(meter_handle):
            raise EnergyPlusException(
                "`get_meter_value` expects `meter_handle` as an `int`, not "
                "'{}'".format(meter_handle))
        return self.api.getMeterValue(state, meter_handle)

    def set_actuator_value(self, state: c_void_p, actuator_handle: int, actuator_value: float) -> None:
        """
        Sets the value of an actuator in a running simulation.  The `get_actuator_handle` function is first used
        to get a handle to the actuator by name.  Then once the handle is retrieved, it is passed into this function,
        along with the value to assign, to then set the value of the actuator.  Internally, actuators can alter floating
        point, integer, and boolean operational values.  The API only exposes this set function with a floating point
        argument.  For floating point types, the value is assigned directly.  For integer types, the value is rounded
        to the nearest integer, with the halfway point rounded away from zero (2.5 becomes 3), then cast to a plain
        integer.  For logical values, the original EMS convention is kept, where a value of 1.0 means TRUE, and a value
        of 0.0 means FALSE -- and any other value defaults to FALSE.  A small tolerance is applied internally to allow
        for small floating point round-off.  A value *very close* to 1.0 will still evaluate to TRUE.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param actuator_handle: An integer returned from the `get_actuator_handle` function.
        :param actuator_value: The floating point value to assign to the actuator
        :return: Nothing
        """
        if not is_number(actuator_handle):
            raise EnergyPlusException(
                "`set_actuator_value` expects `actuator_handle` as an `int`, not "
                "'{}'".format(actuator_handle))
        if not is_number(actuator_value):
            raise EnergyPlusException(
                "`set_actuator_value` expects `actuator_value` as a `float`, not "
                "'{}'".format(actuator_value))
        self.api.setActuatorValue(state, actuator_handle, actuator_value)

    def reset_actuator(self, state: c_void_p, actuator_handle: int) -> None:
        """
        Resets the actuator internally to EnergyPlus.  This allows subsequent calculations to be used for the actuator
        instead of the externally set actuator value.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param actuator_handle: An integer returned from the `get_actuator_handle` function.
        :return: Nothing
        """
        if not is_number(actuator_handle):
            raise EnergyPlusException(
                "`reset_actuator` expects `actuator_handle` as an `int`, not "
                "'{}'".format(actuator_handle))
        self.api.resetActuator(state, actuator_handle)

    def get_actuator_value(self, state: c_void_p, actuator_handle: int) -> float:
        """
        Gets the most recent value of an actuator.  In some applications, actuators are altered by multiple scripts, and
        this allows getting the most recent value.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param actuator_handle: An integer returned from the `get_actuator_handle` function.
        :return: A floating point of the actuator value.  For boolean actuators returns 1.0 for true and 0.0 for false.
                 Returns zero if the handle is invalid.  Use the api_error_flag function to disambiguate between valid
                 zero returns and error states.
        """
        if not is_number(actuator_handle):
            raise EnergyPlusException(
                "`get_actuator_value` expects `actuator_handle` as an `int`, not "
                "'{}'".format(actuator_handle))
        return self.api.getActuatorValue(state, actuator_handle)

    def get_internal_variable_handle(self, state: c_void_p, variable_type: Union[str, bytes],
                                     variable_key: Union[str, bytes]) -> int:
        """
        Get a handle to an internal variable in a running simulation.

        The arguments passed into this function do not need to be a particular case, as the EnergyPlus API
        automatically converts values to upper-case when finding matches to internal variables in the simulation.

        Note also that the arguments passed in here can be either strings or bytes, as this wrapper handles conversion
        as needed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param variable_type: The name of the variable to retrieve, e.g. "Zone Air Volume", or "Zone Floor Area"
        :param variable_key: The instance of the variable to retrieve, e.g. "Zone 1"
        :return: An integer ID for this output variable, or -1 if one could not be found.
        """
        if isinstance(variable_type, str):
            variable_type = variable_type.encode('utf-8')
        elif not isinstance(variable_type, bytes):
            raise EnergyPlusException(
                "`get_internal_variable_handle` expects `component_type` as a `str` or UTF-8 encoded `bytes`, not "
                "'{}'".format(variable_type))
        if isinstance(variable_key, str):
            variable_key = variable_key.encode('utf-8')
        elif not isinstance(variable_key, bytes):
            raise EnergyPlusException(
                "`get_internal_variable_handle` expects `component_type` as a `str` or UTF-8 encoded `bytes`, not "
                "'{}'".format(variable_key))
        return self.api.getInternalVariableHandle(state, variable_type, variable_key)

    def get_internal_variable_value(self, state: c_void_p, variable_handle: int) -> float:
        """
        Get the value of an internal variable in a running simulation.  The `get_internal_variable_handle` function is
        first used to get a handle to the variable by name.  Then once the handle is retrieved, it is passed into this
        function to then get the value of the variable.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param variable_handle: An integer returned from the `get_internal_variable_handle` function.
        :return: Floating point representation of the internal variable value.  Returns zero if the handle is invalid.
                 Use the api_error_flag function to disambiguate between valid zero returns and error states.
        """
        if not is_number(variable_handle):
            raise EnergyPlusException(
                "`get_internal_variable_value` expects `variable_handle` as an `int`, not "
                "'{}'".format(variable_handle))
        return self.api.getInternalVariableValue(state, variable_handle)

    def get_construction_handle(self, state: c_void_p, var_name: Union[str, bytes]) -> int:
        """
        Get a handle to a constructions in a running simulation.

        Some actuators allow specifying different constructions to allow switchable construction control.
        This function returns an index that can be used in those functions.  The construction is specified by name.

        The arguments passed into this function do not need to be a particular case, as the EnergyPlus API
        automatically converts values to upper-case when finding matches to internal variables in the simulation.

        Note also that the arguments passed in here can be either strings or bytes, as this wrapper handles conversion
        as needed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param var_name: Name of the construction to look up
        :return: An integer ID for this construction, or -1 if one could not be found.
        """
        if isinstance(var_name, str):
            var_name = var_name.encode('utf-8')
        elif not isinstance(var_name, bytes):
            raise EnergyPlusException(
                "`get_construction_handle` expects `component_type` as a `str` or UTF-8 encoded `bytes`, not "
                "'{}'".format(var_name))
        return self.api.getConstructionHandle(state, var_name)

    def get_ems_global_handle(self, state: c_void_p, var_name: Union[str, bytes]) -> int:
        """
        Get a handle to an EMS global variable in a running simulation.

        EMS global variables are used as a way to share data between running EMS programs.  First a global variable must
        be declared in the input file using the EnergyManagementSystem:GlobalVariable object.  Once a name has been
        declared, it can be accessed by EMS programs by name, and through the Python API.  For API usage, the client
        should get a handle to the variable using this get_global_handle function, then
        using the get_ems_global_value and set_ems_global_value functions as needed.  Note all global variables are
        floating point values.

        The arguments passed into this function do not need to be a particular case, as the EnergyPlus API
        automatically converts values to upper-case when finding matches to internal variables in the simulation.

        Note also that the arguments passed in here can be either strings or bytes, as this wrapper handles conversion
        as needed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param var_name: The name of the EMS global variable to retrieve, this name must be listed in an IDF object:
                         `EnergyManagementSystem:GlobalVariable`
        :return: An integer ID for this EMS global variable, or -1 if one could not be found.
        """
        if isinstance(var_name, str):
            var_name = var_name.encode('utf-8')
        elif not isinstance(var_name, bytes):
            raise EnergyPlusException(
                "`get_ems_global_handle` expects `component_type` as a `str` or UTF-8 encoded `bytes`, not "
                "'{}'".format(var_name))
        return self.api.getEMSGlobalVariableHandle(state, var_name)

    def get_ems_global_value(self, state: c_void_p, handle: int) -> float:
        """
        Get the current value of an EMS global variable in a running simulation.

        EMS global variables are used as a way to share data between running EMS programs.  First a global variable must
        be declared in the input file using the EnergyManagementSystem:GlobalVariable object.  Once a name has been
        declared, it can be accessed by EMS programs by name, and through the Python API.  For API usage, the client
        should get a handle to the variable using this get_global_handle function, then
        using the get_ems_global_value and set_ems_global_value functions as needed.  Note all global variables are
        floating point values.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param handle: An integer returned from the `get_ems_global_handle` function.
        :return: Floating point representation of the EMS global variable value
        """
        if not is_number(handle):
            raise EnergyPlusException(
                "`get_ems_global_value` expects `handle` as an `int`, not "
                "'{}'".format(handle))
        return self.api.getEMSGlobalVariableValue(state, handle)

    def set_ems_global_value(self, state: c_void_p, handle: int, value: float) -> None:
        """
        Set the current value of an EMS global variable in a running simulation.

        EMS global variables are used as a way to share data between running EMS programs.  First a global variable must
        be declared in the input file using the EnergyManagementSystem:GlobalVariable object.  Once a name has been
        declared, it can be accessed by EMS programs by name, and through the Python API.  For API usage, the client
        should get a handle to the variable using this get_global_handle function, then
        using the get_ems_global_value and set_ems_global_value functions as needed.  Note all global variables are
        floating point values.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param handle: An integer returned from the `get_ems_global_handle` function.
        :param value: Floating point value to assign to the EMS global variable
        """
        if not is_number(handle):
            raise EnergyPlusException(
                "`set_ems_global_value` expects `variable_handle` as an `int`, not "
                "'{}'".format(handle))
        if not is_number(value):
            raise EnergyPlusException(
                "`set_ems_global_value` expects `value` as a `float`, not "
                "'{}'".format(value))
        self.api.setEMSGlobalVariableValue(state, handle, value)

    def get_global_handle(self, state: c_void_p, var_name: Union[str, bytes]) -> int:
        """
        Get a handle to a global variable in a running simulation.  This is only used for Python Plugin applications!

        Global variables are used as a way to share data between running Python Plugins.  First a global variable must
        be declared in the input file using the PythonPlugin:GlobalVariables object.  Once a name has been declared, it
        can be accessed in the Plugin by getting a handle to the variable using this get_global_handle function, then
        using the get_global_value and set_global_value functions as needed.  Note all global variables are
        floating point values.

        The arguments passed into this function do not need to be a particular case, as the EnergyPlus API
        automatically converts values to upper-case when finding matches to internal variables in the simulation.

        Note also that the arguments passed in here can be either strings or bytes, as this wrapper handles conversion
        as needed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param var_name: The name of the global variable to retrieve, this name must be listed in the IDF object:
                         `PythonPlugin:GlobalVariables`
        :return: An integer ID for this global variable, or -1 if one could not be found.
        """
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_global_handle is only available as part of a Python Plugin workflow")
        if isinstance(var_name, str):
            var_name = var_name.encode('utf-8')
        elif not isinstance(var_name, bytes):
            raise EnergyPlusException(
                "`get_global_handle` expects `component_type` as a `str` or UTF-8 encoded `bytes`, not "
                "'{}'".format(var_name))
        return self.api.getPluginGlobalVariableHandle(state, var_name)

    def get_global_value(self, state: c_void_p, handle: int) -> float:
        """
        Get the current value of a plugin global variable in a running simulation.  This is only used for Python Plugin
        applications!

        Global variables are used as a way to share data between running Python Plugins.  First a global variable must
        be declared in the input file using the PythonPlugin:GlobalVariables object.  Once a name has been declared, it
        can be accessed in the Plugin by getting a handle to the variable using the get_global_handle function, then
        using this get_global_value and the set_global_value functions as needed.  Note all global variables are
        floating point values.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param handle: An integer returned from the `get_global_handle` function.
        :return: Floating point representation of the global variable value
        """
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_global_value is only available as part of a Python Plugin workflow")
        if not is_number(handle):
            raise EnergyPlusException(
                "`get_global_value` expects `handle` as an `int`, not "
                "'{}'".format(handle))
        return self.api.getPluginGlobalVariableValue(state, handle)

    def set_global_value(self, state: c_void_p, handle: int, value: float) -> None:
        """
        Set the current value of a plugin global variable in a running simulation.  This is only used for Python Plugin
        applications!

        Global variables are used as a way to share data between running Python Plugins.  First a global variable must
        be declared in the input file using the PythonPlugin:GlobalVariables object.  Once a name has been declared, it
        can be accessed in the Plugin by getting a handle to the variable using the get_global_handle function, then
        using the get_global_value and this set_global_value functions as needed.  Note all global variables are
        floating point values.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param handle: An integer returned from the `get_global_handle` function.
        :param value: Floating point value to assign to the global variable
        """
        if not self.running_as_python_plugin:
            raise EnergyPlusException("set_global_handle is only available as part of a Python Plugin workflow")
        if not is_number(handle):
            raise EnergyPlusException(
                "`set_global_value` expects `variable_handle` as an `int`, not "
                "'{}'".format(handle))
        if not is_number(value):
            raise EnergyPlusException(
                "`get_global_value` expects `value` as a `float`, not "
                "'{}'".format(value))
        self.api.setPluginGlobalVariableValue(state, handle, value)

    def get_trend_handle(self, state: c_void_p, trend_var_name: Union[str, bytes]) -> int:
        """
        Get a handle to a trend variable in a running simulation.  This is only used for Python Plugin applications!

        Trend variables are used as a way to track history of a PythonPlugin:Variable over time.  First a trend variable
        must be declared in the input file using the PythonPlugin:TrendVariable object.  Once a variable has been
        declared there, it can be accessed in the Plugin by getting a handle to the variable using this get_trend_handle
        function, then using the other trend variable worker functions as needed.

        The arguments passed into this function do not need to be a particular case, as the EnergyPlus API
        automatically converts values to upper-case when finding matches to internal variables in the simulation.

        Note also that the arguments passed in here can be either strings or bytes, as this wrapper handles conversion
        as needed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param trend_var_name: The name of the global variable to retrieve, this name must match the name of a
                               `PythonPlugin:TrendVariable` IDF object.
        :return: An integer ID for this trend variable, or -1 if one could not be found.
        """
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_trend_handle is only available as part of a Python Plugin workflow")
        if isinstance(trend_var_name, str):
            trend_var_name = trend_var_name.encode('utf-8')
        elif not isinstance(trend_var_name, bytes):
            raise EnergyPlusException(
                "`get_trend_handle` expects `component_type` as a `str` or UTF-8 encoded `bytes`, not "
                "'{}'".format(trend_var_name))
        return self.api.getPluginTrendVariableHandle(state, trend_var_name)

    def get_trend_value(self, state: c_void_p, trend_handle: int, time_index: int) -> float:
        """
        Get the value of a plugin trend variable at a specific history point.  The time_index argument specifies how
        many time steps to go back in the trend history.  A value of 1 indicates taking the most recent value.  The
        value of time_index must be less than or equal to the number of history terms specified in the matching
        PythonPlugin:TrendVariable object declaration in the input file.  This is only used for Python Plugin
        applications!

        Trend variables are used as a way to track history of a PythonPlugin:Variable over time.  First a trend variable
        must be declared in the input file using the PythonPlugin:TrendVariable object.  Once a variable has been
        declared there, it can be accessed in the Plugin by getting a handle to the variable using the get_trend_handle
        function, then using the other trend variable worker functions as needed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param trend_handle: An integer returned from the `get_trend_handle` function.
        :param time_index: The number of time steps to search back in history to evaluate this function.
        :return: Floating point value representation of the specific evaluation.
        """
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_trend_value is only available as part of a Python Plugin workflow")
        if not is_number(trend_handle):
            raise EnergyPlusException(
                "`get_trend_value` expects `trend_handle` as an `int`, not "
                "'{}'".format(trend_handle))
        if not is_number(time_index):
            raise EnergyPlusException(
                "`get_trend_value` expects `time_index` as an `int`, not "
                "'{}'".format(time_index))
        return self.api.getPluginTrendVariableValue(state, trend_handle, time_index)

    def get_trend_average(self, state: c_void_p, trend_handle: int, count: int) -> float:
        """
        Get the average of a plugin trend variable over a specific history set.  The count argument specifies how
        many time steps to go back in the trend history.  A value of 1 indicates averaging just the most recent value.
        The value of time_index must be less than or equal to the number of history terms specified in the matching
        PythonPlugin:TrendVariable object declaration in the input file.  This is only used for Python Plugin
        applications!

        Trend variables are used as a way to track history of a PythonPlugin:Variable over time.  First a trend variable
        must be declared in the input file using the PythonPlugin:TrendVariable object.  Once a variable has been
        declared there, it can be accessed in the Plugin by getting a handle to the variable using the get_trend_handle
        function, then using the other trend variable worker functions as needed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param trend_handle: An integer returned from the `get_trend_handle` function.
        :param count: The number of time steps to search back in history to evaluate this function.
        :return: Floating point value representation of the specific evaluation.
        """
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_trend_average is only available as part of a Python Plugin workflow")
        if not is_number(trend_handle):
            raise EnergyPlusException(
                "`get_trend_average` expects `trend_handle` as an `int`, not "
                "'{}'".format(trend_handle))
        if not is_number(count):
            raise EnergyPlusException(
                "`get_trend_average` expects `count` as an `int`, not "
                "'{}'".format(count))
        return self.api.getPluginTrendVariableAverage(state, trend_handle, count)

    def get_trend_min(self, state: c_void_p, trend_handle: int, count: int) -> float:
        """
        Get the minimum of a plugin trend variable over a specific history set.  The count argument specifies how
        many time steps to go back in the trend history.  A value of 1 indicates sweeping just the most recent value.
        The value of time_index must be less than or equal to the number of history terms specified in the matching
        PythonPlugin:TrendVariable object declaration in the input file.  This is only used for Python Plugin
        applications!

        Trend variables are used as a way to track history of a PythonPlugin:Variable over time.  First a trend variable
        must be declared in the input file using the PythonPlugin:TrendVariable object.  Once a variable has been
        declared there, it can be accessed in the Plugin by getting a handle to the variable using the get_trend_handle
        function, then using the other trend variable worker functions as needed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param trend_handle: An integer returned from the `get_trend_handle` function.
        :param count: The number of time steps to search back in history to evaluate this function.
        :return: Floating point value representation of the specific evaluation.
        """
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_trend_min is only available as part of a Python Plugin workflow")
        if not is_number(trend_handle):
            raise EnergyPlusException(
                "`get_trend_min` expects `trend_handle` as an `int`, not "
                "'{}'".format(trend_handle))
        if not is_number(count):
            raise EnergyPlusException(
                "`get_trend_min` expects `count` as an `int`, not "
                "'{}'".format(count))
        return self.api.getPluginTrendVariableMin(state, trend_handle, count)

    def get_trend_max(self, state: c_void_p, trend_handle: int, count: int) -> float:
        """
        Get the maximum of a plugin trend variable over a specific history set.  The count argument specifies how
        many time steps to go back in the trend history.  A value of 1 indicates sweeping just the most recent value.
        The value of time_index must be less than or equal to the number of history terms specified in the matching
        PythonPlugin:TrendVariable object declaration in the input file.  This is only used for Python Plugin
        applications!

        Trend variables are used as a way to track history of a PythonPlugin:Variable over time.  First a trend variable
        must be declared in the input file using the PythonPlugin:TrendVariable object.  Once a variable has been
        declared there, it can be accessed in the Plugin by getting a handle to the variable using the get_trend_handle
        function, then using the other trend variable worker functions as needed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param trend_handle: An integer returned from the `get_trend_handle` function.
        :param count: The number of time steps to search back in history to evaluate this function.
        :return: Floating point value representation of the specific evaluation.
        """
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_trend_max is only available as part of a Python Plugin workflow")
        if not is_number(trend_handle):
            raise EnergyPlusException(
                "`get_trend_max` expects `trend_handle` as an `int`, not "
                "'{}'".format(trend_handle))
        if not is_number(count):
            raise EnergyPlusException(
                "`get_trend_max` expects `count` as an `int`, not "
                "'{}'".format(count))
        return self.api.getPluginTrendVariableMax(state, trend_handle, count)

    def get_trend_sum(self, state: c_void_p, trend_handle: int, count: int) -> float:
        """
        Get the summation of a plugin trend variable over a specific history set.  The count argument specifies how
        many time steps to go back in the trend history.  A value of 1 indicates sweeping just the most recent value.
        The value of time_index must be less than or equal to the number of history terms specified in the matching
        PythonPlugin:TrendVariable object declaration in the input file.  This is only used for Python Plugin
        applications!

        Trend variables are used as a way to track history of a PythonPlugin:Variable over time.  First a trend variable
        must be declared in the input file using the PythonPlugin:TrendVariable object.  Once a variable has been
        declared there, it can be accessed in the Plugin by getting a handle to the variable using the get_trend_handle
        function, then using the other trend variable worker functions as needed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param trend_handle: An integer returned from the `get_trend_handle` function.
        :param count: The number of time steps to search back in history to evaluate this function.
        :return: Floating point value representation of the specific evaluation.
        """
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_trend_sum is only available as part of a Python Plugin workflow")
        if not is_number(trend_handle):
            raise EnergyPlusException(
                "`get_trend_sum` expects `trend_handle` as an `int`, not "
                "'{}'".format(trend_handle))
        if not is_number(count):
            raise EnergyPlusException(
                "`get_trend_sum` expects `count` as an `int`, not "
                "'{}'".format(count))
        return self.api.getPluginTrendVariableSum(state, trend_handle, count)

    def get_trend_direction(self, state: c_void_p, trend_handle: int, count: int) -> float:
        """
        Get the trajectory of a plugin trend variable over a specific history set.  The count argument specifies how
        many time steps to go back in the trend history.  A value of 1 indicates sweeping just the most recent value.
        A linear regression is performed over the swept values and the slope of the regression line is returned as a
        representation of the average trajectory over this range.
        The value of time_index must be less than or equal to the number of history terms specified in the matching
        PythonPlugin:TrendVariable object declaration in the input file.  This is only used for Python Plugin
        applications!

        Trend variables are used as a way to track history of a PythonPlugin:Variable over time.  First a trend variable
        must be declared in the input file using the PythonPlugin:TrendVariable object.  Once a variable has been
        declared there, it can be accessed in the Plugin by getting a handle to the variable using the get_trend_handle
        function, then using the other trend variable worker functions as needed.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param trend_handle: An integer returned from the `get_trend_handle` function.
        :param count: The number of time steps to search back in history to evaluate this function.
        :return: Floating point value representation of the specific evaluation.
        """
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_trend_direction is only available as part of a Python Plugin workflow")
        if not is_number(trend_handle):
            raise EnergyPlusException(
                "`get_trend_direction` expects `trend_handle` as an `int`, not "
                "'{}'".format(trend_handle))
        if not is_number(count):
            raise EnergyPlusException(
                "`get_trend_direction` expects `count` as an `int`, not "
                "'{}'".format(count))
        return self.api.getPluginTrendVariableDirection(state, trend_handle, count)

    def year(self, state: c_void_p) -> int:
        """
        Get the "current" year of the simulation, read from the EPW. All simulations operate at a real year, either user
        specified or automatically selected by EnergyPlus based on other data (start day of week + leap year option).

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: An integer year (2020, for example)
        """
        return self.api.year(state)

    def calendar_year(self, state: c_void_p) -> int:
        """
        Get the "current" calendar year of the simulation.

        Only valid for weather file run periods.

        All simulations operate at a real year, either user
        specified or automatically selected by EnergyPlus based on other data (start day of week + leap year option).

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: An integer year (2020, for example)
        """
        return self.api.calendarYear(state)

    def month(self, state: c_void_p) -> int:
        """
        Get the current month of the simulation (1-12)

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: An integer month (1-12)
        """
        return self.api.month(state)

    def day_of_month(self, state: c_void_p) -> int:
        """
        Get the current day of month (1-31)

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: An integer day of the month (1-31)
        """
        return self.api.dayOfMonth(state)

    def hour(self, state: c_void_p) -> int:
        """
        Get the current hour of the simulation (0-23)

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: An integer hour of the day (0-23)
        """
        return self.api.hour(state)

    def current_time(self, state: c_void_p) -> float:
        """
        Get the current time of day in hours, where current time represents the end time of the current time step.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: A floating point representation of the current time in hours
        """
        return self.api.currentTime(state)

    def minutes(self, state: c_void_p) -> int:
        """
        Get the current minutes into the hour (1-60)

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: An integer number of minutes into the current hour (1-60)
        """
        return self.api.minutes(state)

    def num_time_steps_in_hour(self, state: c_void_p) -> int:
        """
        Returns the number of zone time steps in an hour, which is currently a constant value throughout a simulation.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: An integer representation of the number of time steps in an hour
        """
        return self.api.numTimeStepsInHour(state)

    def zone_time_step_number(self, state: c_void_p) -> int:
        """
        The current zone time step index, from 1 to the number of zone time steps per hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: The integer index of the current time step
        """
        return self.api.zoneTimeStepNum(state)

    def day_of_week(self, state: c_void_p) -> int:
        """
        Get the current day of the week (1-7)

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: An integer day of week (1-7)
        """
        return self.api.dayOfWeek(state)

    def day_of_year(self, state: c_void_p) -> int:
        """
        Get the current day of the year (1-366)

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: An integer day of the year (1-366)
        """
        return self.api.dayOfYear(state)

    def daylight_savings_time_indicator(self, state: c_void_p) -> bool:
        """
        Get the current daylight savings time indicator as a logical value.  The C API returns an integer where 1 is
        yes and 0 is no, this simply wraps that with a bool conversion.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: A boolean DST indicator for the current time.
        """
        return self.api.daylightSavingsTimeIndicator(state) == 1

    def holiday_index(self, state: c_void_p) -> int:
        """
        Gets a flag for the current day holiday type: 0 is no holiday, 1 is holiday type #1, etc.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: An integer indicator for current day holiday type.
        """
        if self.api.holidayIndex(state) == 0:
            return 0
        else:
            return self.api.holidayIndex(state) - 7

    def sun_is_up(self, state: c_void_p) -> bool:
        """
        Gets a flag for whether the sun is currently up.  The C API returns an integer where 1 is yes and 0 is no, this
        simply wraps that with a bool conversion.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: A boolean indicating whether the sun is currently up.
        """
        return self.api.sunIsUp(state) == 1

    def is_raining(self, state: c_void_p) -> bool:
        """
        Gets a flag for whether the it is currently raining.  The C API returns an integer where 1 is yes and 0 is no,
        this simply wraps that with a bool conversion.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: A boolean indicating whether it is currently raining.
        """
        return self.api.isRaining(state) == 1

    def warmup_flag(self, state: c_void_p) -> bool:
        """
        Gets a flag for whether the warmup flag is currently on, signaling that EnergyPlus is still in the process of
        converging on warmup days.  The C API returns an integer where 1 is yes and 0 is no, this simply wraps that
        with a bool conversion.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: A boolean indicating whether the warmup flag is on.
        """
        return self.api.warmupFlag(state) == 1

    def zone_time_step(self, state: c_void_p) -> float:
        """
        Gets the current zone time step value in EnergyPlus.  The zone time step is variable and fluctuates
        during the simulation.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: The current zone time step in fractional hours.
        """
        return self.api.zoneTimeStep(state)

    def system_time_step(self, state: c_void_p) -> float:
        """
        Gets the current system time step value in EnergyPlus.  The system time step is variable and fluctuates
        during the simulation.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: The current system time step in fractional hours.
        """
        return self.api.systemTimeStep(state)

    def current_environment_num(self, state: c_void_p) -> int:
        """
        Gets the current environment index.  EnergyPlus environments are design days, run periods, etc.  This function
        is only expected to be useful in very specialized applications where you control the environment order
        carefully.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: The current environment number.
        """
        return self.api.currentEnvironmentNum(state)

    def actual_time(self, state: c_void_p) -> int:
        """
        Gets a simple sum of the values of the time part of the date/time function. Could be used in random seeding.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: Integer value of time portion of the date/time function.
        """
        return self.api.actualTime(state)

    def actual_date_time(self, state: c_void_p) -> int:
        """
        Gets a simple sum of the values of the date/time function. Could be used in random seeding.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: Integer value of the date/time function.
        """
        return self.api.actualDateTime(state)

    def kind_of_sim(self, state: c_void_p) -> int:
        """
        Gets the current environment number.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: Integer value of current environment.
        """
        return self.api.kindOfSim(state)

    def today_weather_is_raining_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> bool:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: A true/false for whether the weather condition is active at the specified time
        """
        return self.api.todayWeatherIsRainAtTime(state, hour, time_step_number) == 1

    def today_weather_is_snowing_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> bool:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: A true/false for whether the weather condition is active at the specified time
        """
        return self.api.todayWeatherIsSnowAtTime(state, hour, time_step_number) == 1

    def today_weather_outdoor_dry_bulb_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.todayWeatherOutDryBulbAtTime(state, hour, time_step_number)

    def today_weather_outdoor_dew_point_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.todayWeatherOutDewPointAtTime(state, hour, time_step_number)

    def today_weather_outdoor_barometric_pressure_at_time(self, state: c_void_p, hour: int,
                                                          time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.todayWeatherOutBarometricPressureAtTime(state, hour, time_step_number)

    def today_weather_outdoor_relative_humidity_at_time(self, state: c_void_p, hour: int,
                                                        time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.todayWeatherOutRelativeHumidityAtTime(state, hour, time_step_number)

    def today_weather_wind_speed_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.todayWeatherWindSpeedAtTime(state, hour, time_step_number)

    def today_weather_wind_direction_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.todayWeatherWindDirectionAtTime(state, hour, time_step_number)

    def today_weather_sky_temperature_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.todayWeatherSkyTemperatureAtTime(state, hour, time_step_number)

    def today_weather_horizontal_ir_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.todayWeatherHorizontalIRSkyAtTime(state, hour, time_step_number)

    def today_weather_beam_solar_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.todayWeatherBeamSolarRadiationAtTime(state, hour, time_step_number)

    def today_weather_diffuse_solar_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.todayWeatherDiffuseSolarRadiationAtTime(state, hour, time_step_number)

    def today_weather_albedo_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.todayWeatherAlbedoAtTime(state, hour, time_step_number)

    def today_weather_liquid_precipitation_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.todayWeatherLiquidPrecipitationAtTime(state, hour, time_step_number)

    def tomorrow_weather_is_raining_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> bool:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: A true/false for whether the weather condition is active at the specified time
        """
        return self.api.tomorrowWeatherIsRainAtTime(state, hour, time_step_number) == 1

    def tomorrow_weather_is_snowing_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> bool:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: A true/false for whether the weather condition is active at the specified time
        """
        return self.api.tomorrowWeatherIsSnowAtTime(state, hour, time_step_number) == 1

    def tomorrow_weather_outdoor_dry_bulb_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.tomorrowWeatherOutDryBulbAtTime(state, hour, time_step_number)

    def tomorrow_weather_outdoor_dew_point_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.tomorrowWeatherOutDewPointAtTime(state, hour, time_step_number)

    def tomorrow_weather_outdoor_barometric_pressure_at_time(self, state: c_void_p, hour: int,
                                                             time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.tomorrowWeatherOutBarometricPressureAtTime(state, hour, time_step_number)

    def tomorrow_weather_outdoor_relative_humidity_at_time(self, state: c_void_p, hour: int,
                                                           time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.tomorrowWeatherOutRelativeHumidityAtTime(state, hour, time_step_number)

    def tomorrow_weather_wind_speed_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.tomorrowWeatherWindSpeedAtTime(state, hour, time_step_number)

    def tomorrow_weather_wind_direction_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.tomorrowWeatherWindDirectionAtTime(state, hour, time_step_number)

    def tomorrow_weather_sky_temperature_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.tomorrowWeatherSkyTemperatureAtTime(state, hour, time_step_number)

    def tomorrow_weather_horizontal_ir_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.tomorrowWeatherHorizontalIRSkyAtTime(state, hour, time_step_number)

    def tomorrow_weather_beam_solar_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.tomorrowWeatherBeamSolarRadiationAtTime(state, hour, time_step_number)

    def tomorrow_weather_diffuse_solar_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.tomorrowWeatherDiffuseSolarRadiationAtTime(state, hour, time_step_number)

    def tomorrow_weather_albedo_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.tomorrowWeatherAlbedoAtTime(state, hour, time_step_number)

    def tomorrow_weather_liquid_precipitation_at_time(self, state: c_void_p, hour: int, time_step_number: int) -> float:
        """
        Gets the specified weather data at the specified hour and time step index within that hour

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param hour: Integer hour of day (0 to 23)
        :param time_step_number: Time step index in hour, from 1 to the number of zone time steps per hour
        :return: Value of the weather condition at the specified time
        """
        return self.api.tomorrowWeatherLiquidPrecipitationAtTime(state, hour, time_step_number)

    def current_sim_time(self, state: c_void_p) -> float:
        """
        Returns the cumulative simulation time from the start of the environment, in hours

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: Value of the simulation time from the start of the environment in fractional hours
        """
        return self.api.currentSimTime(state)
