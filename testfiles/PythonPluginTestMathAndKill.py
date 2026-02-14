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


class AverageZoneTempAndTests(EnergyPlusPlugin):

    def __init__(self):
        super().__init__()
        self.do_setup = True

    @staticmethod
    def hpac_cool_plf_f_plr_curve(x: float) -> float:
        y = 0.75 + 0.25 * x + 0.0 * x**2
        y = max(0.0, y)
        y = min(1.0, y)
        return y

    def on_end_of_zone_timestep_before_zone_reporting(self, state) -> int:
        if not self.api.exchange.api_data_fully_ready(state):
            return 0
        if self.do_setup:
            self.data["h_zone_temp_var"] = self.api.exchange.get_global_handle(state, "AverageZoneTemp")
            self.data["h_zone_temp_running_avg"] = self.api.exchange.get_global_handle(state, "RunningAveragedZoneTemp")
            self.data["h_trend"] = self.api.exchange.get_trend_handle(state, "ZoneTempTrend")
            self.data["h_zone_temp"] = self.api.exchange.get_variable_handle(
                state, "Zone Mean Air Temperature", "Core_ZN"
            )
            self.data["count"] = 0
            self.do_setup = False
        current_zone_temp = self.api.exchange.get_variable_value(state, self.data["h_zone_temp"])
        self.api.exchange.set_global_value(state, self.data["h_zone_temp_var"], current_zone_temp)

        if not self.do_trend_tests(state):
            return 1
        if not self.do_curve_and_table_tests(state):
            return 1
        if not self.do_psychrometric_tests(state):
            return 1
        if not self.kill_run_later():
            return 1
        return 0

    def do_trend_tests(self, state) -> bool:
        trend_avg = self.api.exchange.get_trend_average(state, self.data["h_trend"], 5)
        print("Trend average: " + str(trend_avg))
        self.api.exchange.set_global_value(state, self.data["h_zone_temp_running_avg"], trend_avg)
        try:
            trend_min = self.api.exchange.get_trend_min(state, self.data["h_trend"], 5)
            print("Trend min: " + str(trend_min))
            trend_max = self.api.exchange.get_trend_max(state, self.data["h_trend"], 5)
            print("Trend max: " + str(trend_max))
            trend_sum = self.api.exchange.get_trend_sum(state, self.data["h_trend"], 5)
            print("Trend sum: " + str(trend_sum))
            trend_direction = self.api.exchange.get_trend_direction(state, self.data["h_trend"], 5)
            print("Trend direction: " + str(trend_direction))
        except:
            self.api.runtime.issue_severe(state, "Problem getting trend min/max values, aborting")
            return False
        if trend_min > trend_max:
            self.api.runtime.issue_severe(state, "Trend problem, min greater than max, aborting")
            return False
        elif trend_min > trend_avg:
            self.api.runtime.issue_severe(state, "Trend problem, min greater than average, aborting")
            return False
        return True
        # EnergyManagementSystem:Subroutine,
        # DoTrendTests,            !- Name
        # Set p1 = @TrendValue PMVtrendLog1 1,  !- Program Line 1
        # Set p2 = @TrendValue PMVtrendLog1 2,  !- Program Line 2
        # Set p3 = @TrendValue PMVtrendLog1 3,  !- <none>
        # Set p4 = @TrendValue PMVtrendLog1 4,  !- <none>
        # Set p5 = @TrendValue PMVtrendLog1 5,  !- <none>
        # Set Avg1to5 = @TrendAverage PMVtrendLog1 5,  !- <none>
        # Set Avg1to20 = @TrendAverage PMVtrendLog1 20,  !- <none>
        # Set Max1to5 = @TrendMax PMVTrendLog1 5,  !- <none>
        # Set Min1to5 = @TrendMin PMVTrendLog1 5,  !- <none>
        # Set slope1to5 = @TrendDirection PMVTrendLog1 5,  !- <none>
        # Set slope1to20 = @TrendDirection PMVTrendLog1 20,  !- <none>
        # Set slopeAll = @TrendDirection PMVTrendLog1 300,  !- <none>
        # Set Sum1to5 = @TrendSum PMVTrendLog1 5,  !- <none>
        # Set Sum1to20 = @TrendSum PMVTrendLog1 20;  !- <none>

    def do_curve_and_table_tests(self, state) -> bool:
        if self.hpac_cool_plf_f_plr_curve(0) != 0.75:
            self.api.runtime.issue_severe(state, "Python Curve and Table Tests Failed, Aborting!")
            return False
        if self.hpac_cool_plf_f_plr_curve(1) != 1.0:
            self.api.runtime.issue_severe(state, "Python Curve and Table Tests Failed, Aborting!")
            return False
        return True

    def do_psychrometric_tests(self, state) -> bool:
        pressure = 101300.0
        temp = 20.0
        hum_ratio = 0.009
        try:
            p = self.api.functional.psychrometrics(state)
            p.density(state, pressure, temp, hum_ratio)
            p.specific_heat(state, hum_ratio, temp)
            p.latent_energy_of_air(state, temp)
            p.latent_energy_of_moisture_in_air(state, temp)
            wb = p.wet_bulb(state, temp, hum_ratio, pressure)
            p.dew_point(state, hum_ratio, pressure)
            p.dew_point_b(state, temp, wb, pressure)
            rh = p.relative_humidity_b(state, temp, hum_ratio, pressure)
            h = p.enthalpy(state, temp, hum_ratio)
            p.enthalpy_b(state, temp, rh, pressure)
            p.dry_bulb(state, h, hum_ratio)
            p.vapor_density(state, temp, hum_ratio, pressure)
            rhov = p.vapor_density_b(state, temp, rh)
            p.relative_humidity(state, temp, rhov)
            p.specific_volume(state, temp, hum_ratio, pressure)
            p.humidity_ratio(state, temp, h)
            p.humidity_ratio_b(state, temp, pressure)
            p.humidity_ratio_c(state, temp, rh, pressure)
            p.humidity_ratio_d(state, temp, wb, pressure)
            p.saturation_pressure(state, temp)
            p.saturation_temperature(state, h, pressure)
        except:
            self.api.runtime.issue_severe(state, "Psychrometric function evaluation failed, aborting")
            return False
        try:
            f = self.api.functional.glycol(state, "water")
            f.specific_heat(state, temp)
            f.density(state, temp)
            f.conductivity(state, temp)
            f.viscosity(state, temp)
        except:
            self.api.runtime.issue_severe(state, "Glycol property function evaluation failed, aborting")
            return False
        return True

    def kill_run_later(self) -> bool:
        self.data["count"] += 1
        if self.data["count"] > 8:
            return False
        return True
