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

from pyenergyplus.plugin import EnergyPlusPlugin


class SuppCoilSpeedControl(EnergyPlusPlugin):

    def __init__(self):
        # init parent class
        super().__init__()
        self.need_to_get_handles = True
        self.zone_air_temp_handle = None
        self.heating_setpoint_handle = None
        self.cooling_setpoint_handle = None
        self.coil_speed_level_handle = None
        self.coil_speed_override_report_handle = None

    def on_inside_hvac_system_iteration_loop(self, state) -> int:

        # api is ready to execute
        if self.api.exchange.api_data_fully_ready(state):

            # get variable handles if needed
            if self.need_to_get_handles:
                self.zone_air_temp_handle = self.api.exchange.get_variable_handle(
                    state, "Zone Air Temperature", "LIVING ZONE"
                )

                self.heating_setpoint_handle = self.api.exchange.get_variable_handle(
                    state, "Zone Thermostat Heating Setpoint Temperature", "LIVING ZONE"
                )

                self.coil_speed_level_handle = self.api.exchange.get_actuator_handle(
                    state, "Coil Speed Control", "Unitary System Supplemental Coil Stage Level", "TWOSPEED HEAT PUMP 1"
                )

                self.coil_speed_override_report_handle = self.api.exchange.get_global_handle(
                    state, "CoilSpeedLevelOverrideReport"
                )
                self.coil_speed_override_value_handle = self.api.exchange.get_global_handle(
                    state, "CoilSpeedLevelOverrideValue"
                )

                self.need_to_get_handles = False

            # calculate
            zone_air_temp = self.api.exchange.get_variable_value(state, self.zone_air_temp_handle)
            heating_setpoint = self.api.exchange.get_variable_value(state, self.heating_setpoint_handle)
            if zone_air_temp < (heating_setpoint - 1.0):
                self.api.exchange.set_actuator_value(state, self.coil_speed_level_handle, 1.9)
                self.api.exchange.set_global_value(state, self.coil_speed_override_value_handle, 1.9)
                self.api.exchange.set_global_value(state, self.coil_speed_override_report_handle, 1.0)
            elif zone_air_temp < (heating_setpoint - 0.1):
                self.api.exchange.set_actuator_value(state, self.coil_speed_level_handle, 1.0)
                self.api.exchange.set_global_value(state, self.coil_speed_override_value_handle, 1.0)
                self.api.exchange.set_global_value(state, self.coil_speed_override_report_handle, 1.0)
            else:
                self.api.exchange.reset_actuator(state, self.coil_speed_level_handle)
                self.api.exchange.set_global_value(state, self.coil_speed_override_value_handle, 0.0)
                self.api.exchange.set_global_value(state, self.coil_speed_override_report_handle, 0.0)

            return 0

        else:
            # api not ready, return
            return 0
