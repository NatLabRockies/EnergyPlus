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

import math

from pyenergyplus.plugin import EnergyPlusPlugin


class SetShadeControlState(EnergyPlusPlugin):
    ShadeStatusInteriorBlindOn = 6
    ShadeStatusOff = 0

    def __init__(self):
        super().__init__()
        self.handles_set = False
        self.handle_zone_sensible_cooling_rate = None
        self.handle_solar_beam_incident_cosine = None
        self.handle_shading_deploy_status = None
        self.handle_global_shading_actuator_status = None  # only because actuators can be output vars ... yet

    def on_begin_timestep_before_predictor(self, state) -> int:
        if not self.handles_set:
            self.handle_zone_sensible_cooling_rate = self.api.exchange.get_variable_handle(
                state, "Zone Air System Sensible Cooling Rate", "West Zone"
            )
            self.handle_solar_beam_incident_cosine = self.api.exchange.get_variable_handle(
                state, "Surface Outside Face Beam Solar Incident Angle Cosine Value", "Zn001:Wall001:Win001"
            )
            self.handle_shading_deploy_status = self.api.exchange.get_actuator_handle(
                state, "Window Shading Control", "Control Status", "Zn001:Wall001:Win001"
            )
            self.handle_global_shading_actuator_status = self.api.exchange.get_global_handle(
                state, "Zn001_Wall001_Win001_Shading_Deploy_Status"
            )
            self.handles_set = True
        current_incident_angle = self.api.exchange.get_variable_value(state, self.handle_solar_beam_incident_cosine)
        current_sensible_cool_rate = self.api.exchange.get_variable_value(state, self.handle_zone_sensible_cooling_rate)
        incident_angle_rad = math.acos(current_incident_angle)
        incident_angle_degrees = math.degrees(incident_angle_rad)
        value_to_assign = self.ShadeStatusOff
        if incident_angle_degrees < 45 or current_sensible_cool_rate > 20:
            value_to_assign = self.ShadeStatusInteriorBlindOn
        self.api.exchange.set_actuator_value(state, self.handle_shading_deploy_status, value_to_assign)
        self.api.exchange.set_global_value(state, self.handle_global_shading_actuator_status, value_to_assign)
        return 0
