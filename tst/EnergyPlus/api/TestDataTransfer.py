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

import sys

from pyenergyplus.api import EnergyPlusAPI

one_time = True
outdoor_temp_sensor = -1
outdoor_dew_point_sensor = -1
outdoor_dew_point_actuator = -1
zone1_wall1_actuator_handle = -1
wall_construction_handle = -1
floor_construction_handle = -1
ok = True
exception = None


def time_step_handler(state):
    global one_time, outdoor_temp_sensor, outdoor_dew_point_sensor, outdoor_dew_point_actuator, zone1_wall1_actuator_handle, wall_construction_handle, floor_construction_handle, ok, exception
    sys.stdout.flush()
    # If API isn't ready yet, or we know it failed already, just return
    if not api.exchange.api_data_fully_ready(state) or not ok:
        return

    # Get the handles
    if one_time:
        one_time = False
        try:
            # val = api.exchange.list_available_api_data_csv()
            # with open('/tmp/data.csv', 'w') as f:
            #     f.write(val.decode(encoding='utf-8'))
            api.exchange.get_api_data(state)  # inspect this to see what's available to exchange
            surfaces = api.exchange.get_object_names(state, "BuildingSurface:Detailed")
            if len(surfaces) == 0:
                print("Called TestDataTransfer.py with an input file that has no BuildingSurface:Detailed, aborting")
                raise RuntimeError("Bad input file for TestDataTransfer.py")
            outdoor_temp_sensor = api.exchange.get_variable_handle(
                state, "SITE OUTDOOR AIR DRYBULB TEMPERATURE", "ENVIRONMENT"
            )
            outdoor_dew_point_sensor = api.exchange.get_variable_handle(
                state, "SITE OUTDOOR AIR DEWPOINT TEMPERATURE", "ENVIRONMENT"
            )
            outdoor_dew_point_actuator = api.exchange.get_actuator_handle(
                state, "Weather Data", "Outdoor Dew Point", "Environment"
            )
            zone1_wall1_actuator_handle = api.exchange.get_actuator_handle(
                state, "Surface", "Construction State", surfaces[0]
            )

            wall_construction_handle = api.exchange.get_construction_handle(state, "R13WALL")
            floor_construction_handle = api.exchange.get_construction_handle(state, "FLOOR")

            idf_path = api.exchange.get_input_file_path(state)
            epw_path = api.exchange.get_weather_file_path(state)
            print(f"Got an input file path of: {idf_path}, and weather file path of: {epw_path}")

            # checking for EMS globals
            ems_global_valid = api.exchange.get_ems_global_handle(state, "MaximumEffort")
            ems_global_invalid = api.exchange.get_ems_global_handle(state, "4or5moments")
            ems_global_builtin = api.exchange.get_ems_global_handle(state, "WARMUPFLAG")
            if ems_global_valid > 0 and ems_global_invalid == 0 and ems_global_builtin == 0:
                print("EMS Global handle lookups worked just fine!")
            else:
                raise Exception(
                    "EMS Global handle lookup failed.  Make sure to call this with _1ZoneUncontrolled_ForAPITesting.idf"
                )
            api.exchange.set_ems_global_value(state, ems_global_valid, 2.0)
            val = api.exchange.get_ems_global_value(state, ems_global_valid)
            if val < 1.9999 or val > 2.0001:
                raise Exception("EMS Global assignment/lookup didn't seem to work, odd")

        except Exception as e:
            # Capture ok and exception message
            exception = e
            ok = False
            raise e

        # If the one_time block failed before, E+ swallowed the exception, but our handles will be wrong
        if (
            outdoor_temp_sensor == -1
            or outdoor_dew_point_sensor == -1
            or outdoor_dew_point_actuator == -1
            or zone1_wall1_actuator_handle == -1
            or wall_construction_handle == -1
            or floor_construction_handle == -1
        ):
            ok = False
            return

    api.exchange.set_actuator_value(state, outdoor_dew_point_actuator, -25)
    oa_temp = api.exchange.get_variable_value(state, outdoor_temp_sensor)
    print("Reading outdoor temp via getVariable, value is: %s" % oa_temp)
    dp_temp = api.exchange.get_variable_value(state, outdoor_dew_point_sensor)
    print("Actuated Dew Point temp value is: %s" % dp_temp)
    sim_time = api.exchange.current_sim_time(state)
    print("Current sim time is: %f" % sim_time)
    epwYear = api.exchange.year(state)
    runPeriodYear = api.exchange.calendar_year(state)
    print("year: %i, calendarYear: %i\n", epwYear, runPeriodYear)

    if api.exchange.zone_time_step_number(state) == 1:
        n = api.exchange.num_time_steps_in_hour(state)
        tomorrow_db = api.exchange.tomorrow_weather_outdoor_dry_bulb_at_time(state, 3, 2)
        print(f"Num time steps in hour = {n}; Tomorrow's hour 3, timestep 2 temp is: {tomorrow_db}")
    if oa_temp > 10:
        print(
            f"Setting Zn001:Wall001 construction ({zone1_wall1_actuator_handle}) to R13WALL ({wall_construction_handle}"
        )
        api.exchange.set_actuator_value(state, zone1_wall1_actuator_handle, wall_construction_handle)
    else:
        print(
            f"Setting Zn001:Wall001 construction ({zone1_wall1_actuator_handle}) to FLOOR ({floor_construction_handle}"
        )
        api.exchange.set_actuator_value(state, zone1_wall1_actuator_handle, floor_construction_handle)


api = EnergyPlusAPI()
state = api.state_manager.new_state()
api.runtime.callback_end_zone_timestep_after_zone_reporting(state, time_step_handler)
api.exchange.request_variable(state, "SITE OUTDOOR AIR DRYBULB TEMPERATURE", "ENVIRONMENT")
api.exchange.request_variable(state, "SITE OUTDOOR AIR DEWPOINT TEMPERATURE", "ENVIRONMENT")
# trim off this python script name when calling the run_energyplus function so you end up with just
# the E+ args, like: -d /output/dir -D /path/to/input.idf
return_code = api.runtime.run_energyplus(state, sys.argv[1:])
if return_code != 0:
    raise RuntimeError("E+ Simulation failed to run")
if not ok:
    raise RuntimeError("DataTransfer failed") from exception
