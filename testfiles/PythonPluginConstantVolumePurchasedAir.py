# EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the
# University of Illinois, The Regents of the University of California, through
# Lawrence Berkeley National Laboratory (subject to receipt of any required
# approvals from the U.S. Dept. of Energy), Oak Ridge National Laboratory,
# managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
# contributors. All rights reserved.
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

import math

from pyenergyplus.plugin import EnergyPlusPlugin


class Set_Shade_Control_State(EnergyPlusPlugin):

    def __init__(self):
        super().__init__()
        self.need_to_get_handles = True
        self.Solar_Beam_Incident_Cos_handle = None
        self.Zn001_Wall001_Win001_Shading_Deploy_Status_actuator_handle = None
        self.Zn001_Wall001_Win001_Shading_Deploy_Status_output_handle = None
        self.Zone_Sensible_Cool_Rate_handle = None
        self.Shade_Status_Off_handle = None
        self.Shade_Status_Interior_Blind_On_handle = None
        self.IncidentAngle_handle = None

    @staticmethod
    def rad_to_deg(rad):
        return 180 * rad / math.pi

    def on_begin_timestep_before_predictor(self, state) -> int:

        # check if API ready
        if self.api.exchange.api_data_fully_ready(state):
            # get handles if needed
            if self.need_to_get_handles:
                self.Solar_Beam_Incident_Cos_handle = self.api.exchange.get_variable_handle(
                    state, "Surface Outside Face Beam Solar Incident Angle Cosine Value", "Zn001:Wall001:Win001"
                )

                self.Zn001_Wall001_Win001_Shading_Deploy_Status_actuator_handle = self.api.exchange.get_actuator_handle(
                    state, "Window Shading Control", "Control Status", "Zn001:Wall001:Win001"
                )

                self.Zn001_Wall001_Win001_Shading_Deploy_Status_output_handle = self.api.exchange.get_global_handle(
                    state, "Zn001_Wall001_Win001_Shading_Deploy_Status"
                )

                self.Zone_Sensible_Cool_Rate_handle = self.api.exchange.get_variable_handle(
                    state, "Zone Air System Sensible Cooling Rate", "WEST ZONE"
                )

                self.Shade_Status_Off_handle = self.api.exchange.get_global_handle(state, "Shade_Status_Off")

                self.Shade_Status_Interior_Blind_On_handle = self.api.exchange.get_global_handle(
                    state, "Shade_Status_Interior_Blind_On"
                )

                self.IncidentAngle_handle = self.api.exchange.get_global_handle(state, "IncidentAngle")

                self.need_to_get_handles = False

            # calculations
            Solar_Beam_Incident_Cos_handle = self.api.exchange.get_variable_value(
                state, self.Solar_Beam_Incident_Cos_handle
            )
            IncidentAngleRad = math.acos(Solar_Beam_Incident_Cos_handle)
            IncidentAngle = self.rad_to_deg(IncidentAngleRad)
            self.api.exchange.set_global_value(state, self.IncidentAngle_handle, IncidentAngle)

            Zone_Sensible_Cool_Rate = self.api.exchange.get_variable_value(state, self.Zone_Sensible_Cool_Rate_handle)
            shade_status_off = self.api.exchange.get_global_value(state, self.Shade_Status_Off_handle)
            shade_status_interior_blind_on = self.api.exchange.get_global_value(
                state, self.Shade_Status_Interior_Blind_On_handle
            )

            if IncidentAngle < 45.0:
                self.api.exchange.set_actuator_value(
                    state,
                    self.Zn001_Wall001_Win001_Shading_Deploy_Status_actuator_handle,
                    shade_status_interior_blind_on,
                )
                shade_status_rpt = shade_status_interior_blind_on
            elif Zone_Sensible_Cool_Rate > 20.0:
                self.api.exchange.set_actuator_value(
                    state,
                    self.Zn001_Wall001_Win001_Shading_Deploy_Status_actuator_handle,
                    shade_status_interior_blind_on,
                )
                shade_status_rpt = shade_status_interior_blind_on
            else:
                self.api.exchange.set_actuator_value(
                    state, self.Zn001_Wall001_Win001_Shading_Deploy_Status_actuator_handle, shade_status_off
                )
                shade_status_rpt = shade_status_off

            self.api.exchange.set_global_value(
                state, self.Zn001_Wall001_Win001_Shading_Deploy_Status_output_handle, shade_status_rpt
            )
            return 0
        else:
            return 0


class InitializeShadeControlFlags(EnergyPlusPlugin):

    def __init__(self):
        super().__init__()
        self.need_to_get_handles = True
        self.Shade_Status_Off_handle = None
        self.Shade_Status_Interior_Blind_On_handle = None

    def on_begin_new_environment(self, state) -> int:

        # these are control flag values used inside EnergyPlus for window shades
        # EMS control of window shading devices involves setting the control values for shading control actuators with
        #  one of these values.  The variable names can be used or replaced, it is the whole number values that trigger
        #  changes in the modeling.
        #  Shades and Blinds are either fully on or fully off, partial positions require multiple windows.
        # the window shading control flag values follow
        #  -1: if window has no shading device
        #   0: if shading device is off
        #   1: if interior shade is on
        #   2: if glazing is switched to darker state
        #   3: if exterior shade is on
        #   6: if interior blind is on
        #   7: if exterior blind is on
        #   8: if between-glass shade is on
        #   9: if between-glass blind is on

        # check if API ready
        if self.api.exchange.api_data_fully_ready(state):
            if self.need_to_get_handles:
                self.Shade_Status_Off_handle = self.api.exchange.get_global_handle(state, "Shade_Status_Off")
                self.Shade_Status_Interior_Blind_On_handle = self.api.exchange.get_global_handle(
                    state, "Shade_Status_Interior_Blind_On"
                )
                self.need_to_get_handles = False
            self.api.exchange.set_global_value(state, self.Shade_Status_Off_handle, 0.0)
            self.api.exchange.set_global_value(state, self.Shade_Status_Interior_Blind_On_handle, 6.0)
            return 0
        else:
            # API not ready, return
            return 0


class ConstantVolumePurchasedAirExample(EnergyPlusPlugin):

    def __init__(self):
        super().__init__()
        self.need_to_get_hndls = True

    def get_handles(self, state):
        # global zone state variable handles
        self.zn_1_state_hndl = self.api.exchange.get_global_handle(state, "zn_1_state")
        self.zn_2_state_hndl = self.api.exchange.get_global_handle(state, "zn_2_state")
        self.zn_3_state_hndl = self.api.exchange.get_global_handle(state, "zn_3_state")
        # zone sensible load "sensor" handles
        self.zn_1_sen_load_hndl = self.api.exchange.get_variable_handle(
            state, "Zone Predicted Sensible Load to Setpoint Heat Transfer Rate", "West Zone"
        )
        self.zn_2_sen_load_hndl = self.api.exchange.get_variable_handle(
            state, "Zone Predicted Sensible Load to Setpoint Heat Transfer Rate", "EAST ZONE"
        )
        self.zn_3_sen_load_hndl = self.api.exchange.get_variable_handle(
            state, "Zone Predicted Sensible Load to Setpoint Heat Transfer Rate", "NORTH ZONE"
        )
        # mass flow rate actuators handles
        self.zn_1_air_mdot_hndl = self.api.exchange.get_actuator_handle(
            state, "Ideal Loads Air System", "Air Mass Flow Rate", "ZONE1AIR"
        )
        self.zn_2_air_mdot_hndl = self.api.exchange.get_actuator_handle(
            state, "Ideal Loads Air System", "Air Mass Flow Rate", "ZONE2AIR"
        )
        self.zn_3_air_mdot_hndl = self.api.exchange.get_actuator_handle(
            state, "Ideal Loads Air System", "Air Mass Flow Rate", "ZONE3AIR"
        )
        # temperature actuators handles
        self.zn_1_air_tsup_hndl = self.api.exchange.get_actuator_handle(
            state, "Ideal Loads Air System", "Air Temperature", "ZONE1AIR"
        )
        self.zn_2_air_tsup_hndl = self.api.exchange.get_actuator_handle(
            state, "Ideal Loads Air System", "Air Temperature", "ZONE2AIR"
        )
        self.zn_3_air_tsup_hndl = self.api.exchange.get_actuator_handle(
            state, "Ideal Loads Air System", "Air Temperature", "ZONE3AIR"
        )
        # humidity ratio actuators handles
        self.zn_1_air_hmrt_hndl = self.api.exchange.get_actuator_handle(
            state, "Ideal Loads Air System", "Air Humidity Ratio", "ZONE1AIR"
        )
        self.zn_2_air_hmrt_hndl = self.api.exchange.get_actuator_handle(
            state, "Ideal Loads Air System", "Air Humidity Ratio", "ZONE2AIR"
        )
        self.zn_3_air_hmrt_hndl = self.api.exchange.get_actuator_handle(
            state, "Ideal Loads Air System", "Air Humidity Ratio", "ZONE3AIR"
        )

    # state representation:  0 is off, 1 is heating, 2 is cooling
    def determine_purch_air_state(self, state):
        # zone 1
        if self.zn_1_sen_load < 0:
            self.zn_1_state = 2
        elif self.zn_1_sen_load > 0:
            self.zn_1_state = 1
        else:
            self.zn_1_state = 0
        # zone 2
        if self.zn_2_sen_load < 0:
            self.zn_2_state = 2
        elif self.zn_2_sen_load > 0:
            self.zn_2_state = 1
        else:
            self.zn_2_state = 0
        # zone 3
        if self.zn_3_sen_load < 0:
            self.zn_3_state = 2
        elif self.zn_3_sen_load > 0:
            self.zn_3_state = 1
        else:
            self.zn_3_state = 0

    def on_after_predictor_after_hvac_managers(self, state) -> int:

        if not self.api.exchange.api_data_fully_ready(state):
            return 0

        if self.need_to_get_hndls:
            self.get_handles(state)
            self.need_to_get_hndls = False

        # get zone sensible load "sensors"
        self.zn_1_sen_load = self.api.exchange.get_variable_value(state, self.zn_1_sen_load_hndl)
        self.zn_2_sen_load = self.api.exchange.get_variable_value(state, self.zn_2_sen_load_hndl)
        self.zn_3_sen_load = self.api.exchange.get_variable_value(state, self.zn_3_sen_load_hndl)

        # determine purchased air state
        self.determine_purch_air_state(state)

        # set global zone state variables
        self.api.exchange.set_global_value(state, self.zn_1_state_hndl, self.zn_1_state)
        self.api.exchange.set_global_value(state, self.zn_2_state_hndl, self.zn_2_state)
        self.api.exchange.set_global_value(state, self.zn_3_state_hndl, self.zn_3_state)

        # set zone 1 purchased air state
        if self.zn_1_state == 2:
            zn_1_air_mdot = 0.3
            zn_1_air_temp = 13
            zn_1_air_hmrt = 0.009
        elif self.zn_1_state == 1:
            zn_1_air_mdot = 0.1
            zn_1_air_temp = 50
            zn_1_air_hmrt = 0.015
        else:
            zn_1_air_mdot = 0

        # set zone 2 purchased air state
        if self.zn_2_state == 2:
            zn_2_air_mdot = 0.3
            zn_2_air_temp = 13
            zn_2_air_hmrt = 0.009
        elif self.zn_2_state == 1:
            zn_2_air_mdot = 0.1
            zn_2_air_temp = 50
            zn_2_air_hmrt = 0.015
        else:
            zn_2_air_mdot = 0

        # set zone 3 purchased air state
        if self.zn_3_state == 2:
            zn_3_air_mdot = 0.4
            zn_3_air_temp = 13
            zn_3_air_hmrt = 0.009
        elif self.zn_3_state == 1:
            zn_3_air_mdot = 0.15
            zn_3_air_temp = 50
            zn_3_air_hmrt = 0.015
        else:
            zn_3_air_mdot = 0

        # set mass flow rate actuators
        self.api.exchange.set_actuator_value(state, self.zn_1_air_mdot_hndl, zn_1_air_mdot)
        self.api.exchange.set_actuator_value(state, self.zn_2_air_mdot_hndl, zn_2_air_mdot)
        self.api.exchange.set_actuator_value(state, self.zn_3_air_mdot_hndl, zn_3_air_mdot)

        # set temperature actuators
        self.api.exchange.set_actuator_value(state, self.zn_1_air_tsup_hndl, zn_1_air_temp)
        self.api.exchange.set_actuator_value(state, self.zn_2_air_tsup_hndl, zn_2_air_temp)
        self.api.exchange.set_actuator_value(state, self.zn_3_air_tsup_hndl, zn_3_air_temp)

        # set humidity ratio actuators
        self.api.exchange.set_actuator_value(state, self.zn_1_air_hmrt_hndl, zn_1_air_hmrt)
        self.api.exchange.set_actuator_value(state, self.zn_2_air_hmrt_hndl, zn_2_air_hmrt)
        self.api.exchange.set_actuator_value(state, self.zn_3_air_hmrt_hndl, zn_3_air_hmrt)

        return 0
