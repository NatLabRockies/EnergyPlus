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
from pathlib import Path
from sys import argv, float_info, path
from tempfile import mkdtemp

# convenience
big = float_info.max

# if running in an installation, it will live at the <install>/ExampleFiles/API, and you don't need any args
install_or_build_dir: Path = Path(__file__).resolve().parent.parent.parent
if len(argv) == 2:
    # if running in a build tree, pass in the path to the <build>/Products directory that contains pyenergyplus
    install_or_build_dir: Path = Path(argv[1])
example_file_to_run: str = str(Path(__file__).resolve().parent / "user_defined_equipment.idf")
try:  # This is the new way, if the user's Python environment includes the pip installed energyplus-api-helpers
    # noinspection PyUnresolvedReferences
    from energyplus_api_helpers.import_helper import EPlusAPIHelper

    e = EPlusAPIHelper(install_or_build_dir)
    api_ = e.get_api_instance()
except (ImportError, ModuleNotFoundError):  # This is the older way, directly adding E+ dir to path before importing API
    path.insert(0, str(install_or_build_dir))
    # noinspection PyUnresolvedReferences
    from pyenergyplus.api import EnergyPlusAPI

    api_ = EnergyPlusAPI()
run_directory = mkdtemp()
state_ = api_.state_manager.new_state()


class QuadraticCurve:
    def __init__(self, c1: float, c2: float, c3: float, min_x: float = -big, max_x: float = big):
        self.c = [c1, c2, c3]
        self.max_x = max_x
        self.min_x = min_x

    def curve_value(self, x: float) -> float:
        x = max(min(x, self.max_x), self.min_x)
        return self.c[0] + (self.c[1] * x) + (self.c[2] * pow(x, 2))


class BiQuadraticCurve:
    def __init__(
        self,
        c1: float,
        c2: float,
        c3: float,
        c4: float,
        c5: float,
        c6: float,
        min_x: float = -big,
        max_x: float = big,
        min_y: float = -big,
        max_y: float = big,
    ):
        self.c = [c1, c2, c3, c4, c5, c6]
        self.max_x = max_x
        self.min_x = min_x
        self.max_y = max_y
        self.min_y = min_y

    def curve_value(self, x: float, y: float) -> float:
        x = max(min(x, self.max_x), self.min_x)
        y = max(min(y, self.max_y), self.min_y)
        return (
            self.c[0]
            + (self.c[1] * x)
            + (self.c[2] * pow(x, 2))
            + (self.c[3] * y)
            + (self.c[4] * pow(y, 2))
            + (self.c[5] * x * y)
        )


class Zone1WinACModel:

    def __init__(self):
        self.q_dot_request = 0.0
        self.primary_air_mass_flow_in = 0.0
        self.primary_air_mass_flow_out = 0.0
        self.primary_air_inlet_temp = 0.0
        self.primary_air_hum_rat_inlet = 0.0
        self.primary_air_outlet_temp = 0.0
        self.primary_air_hum_rat_outlet = 0.0
        self.tot_cool_Power = 0.0
        self.electricity_power = 0.0
        self.electricity_energy = 0.0
        self.outdoor_air_mass_flow_inlet = 0.0
        self.outdoor_air_mass_flow_design = 0.0
        self.supply_air_mass_flow_design = 0.0
        self.outdoor_air_dry_bulb = 0.0
        self.outdoor_air_hum_rat = 0.0
        self.fan_efficiency = 0.0
        self.fan_delta_pressure = 0.0
        self.rated_capacity = 0.0
        self.zone_cool_thermostat = 0.0
        self.rated_eir = 0.0

        # handles
        self.need_to_get_handles = True
        self.handles = {}

        # psych api instance
        self.psych = None

        # curves
        d_cap_f_t = {
            "c1": 0.942587793,  # Constant
            "c2": 0.00954334,  # x
            "c3": 0.000683770,  # x**2
            "c4": -0.011042676,  # y
            "c5": 0.000005249,  # y**2
            "c6": -0.000009720,  # x*y
            "min_x": 12.77778,
            "max_x": 23.88889,
            "min_y": 18.0,
            "max_y": 46.11111,
        }

        self.window_ac_cool_cap_f_t = BiQuadraticCurve(*d_cap_f_t.values())

        d_cap_f_ff = {"c1": 0.8, "c2": 0.2, "c3": 0.0, "min_x": 0.5, "max_x": 1.5}

        self.window_ac_cool_cap_f_ff = QuadraticCurve(*d_cap_f_ff.values())

        d_plf_f_plr = {"c1": 0.85, "c2": 0.15, "c3": 0.0, "min_x": 0.0, "max_x": 1.0}

        self.window_ac_plf_f_plr = QuadraticCurve(*d_plf_f_plr.values())

        d_eir_f_t = {
            "c1": 0.342414409,  # Constant
            "c2": 0.034885008,  # x
            "c3": -0.000623700,  # x**2
            "c4": 0.004977216,  # y
            "c5": 0.000437951,  # y**2
            "c6": -0.000728028,  # x*y
            "min_x": 12.77778,
            "max_x": 23.88889,
            "min_y": 18.0,
            "max_y": 46.11111,
        }

        self.window_ac_eir_f_t = BiQuadraticCurve(*d_eir_f_t.values())

        d_eir_f_ff = {"c1": 1.1552, "c2": -0.1808, "c3": 0.0256, "min_x": 0.5, "max_x": 1.0}

        self.window_ac_eir_f_ff = QuadraticCurve(*d_eir_f_ff.values())

    def get_handles(self, state):
        self.handles["Zone1WinAC_PrimAir_Tinlet"] = api_.exchange.get_internal_variable_handle(
            state, "Inlet Temperature for Primary Air Connection", "Zone1WindAC"
        )

        self.handles["Zone1WinAC_PrimAir_Winlet"] = api_.exchange.get_internal_variable_handle(
            state, "Inlet Humidity Ratio for Primary Air Connection", "Zone1WindAC"
        )
        self.handles["Zone1WinAC_OA_Tdb"] = api_.exchange.get_internal_variable_handle(
            state, "Inlet Temperature for Secondary Air Connection", "Zone1WindAC"
        )
        self.handles["Zone1WinAC_OA_W"] = api_.exchange.get_internal_variable_handle(
            state, "Inlet Humidity Ratio for Secondary Air Connection", "Zone1WindAC"
        )
        self.handles["Zone1WinAC_Qdot_Request"] = api_.exchange.get_internal_variable_handle(
            state, "Remaining Sensible Load to Cooling Setpoint", "Zone1WindAC"
        )
        self.handles["Zone1_OADesign_Vdot"] = api_.exchange.get_internal_variable_handle(
            state, "Zone Outdoor Air Design Volume Flow Rate", "West Zone"
        )
        self.handles["Zone1WinAC_OA_rho"] = api_.exchange.get_internal_variable_handle(
            state, "Inlet Density for Secondary Air Connection", "Zone1WindAC"
        )
        self.handles["Zone1_CoolDesign_Mdot"] = api_.exchange.get_internal_variable_handle(
            state, "Final Zone Design Cooling Air Mass Flow Rate", "West Zone"
        )
        self.handles["Zone1_CoolDesign_Cap"] = api_.exchange.get_internal_variable_handle(
            state, "Final Zone Design Cooling Load", "West Zone"
        )
        self.handles["Zone1Cooling_Tstat"] = api_.exchange.get_variable_handle(
            state, "Zone Thermostat Cooling Setpoint Temperature", "West Zone"
        )
        self.handles["COOLINGCOILAVAILSCHED"] = api_.exchange.get_variable_handle(
            state, "Schedule Value", "COOLINGCOILAVAILSCHED"
        )
        self.handles["OA_Press"] = api_.exchange.get_variable_handle(
            state, "Site Outdoor Air Barometric Pressure", "Environment"
        )
        self.handles["Zone1WinAC_PrimAir_MdotOut"] = api_.exchange.get_actuator_handle(
            state, "Primary Air Connection", "Outlet Mass Flow Rate", "Zone1WindAC"
        )
        self.handles["Zone1WinAC_PrimAir_MdotIn"] = api_.exchange.get_actuator_handle(
            state, "Primary Air Connection", "Inlet Mass Flow Rate", "Zone1WindAC"
        )
        self.handles["Zone1WinAC_PrimAir_Tout"] = api_.exchange.get_actuator_handle(
            state, "Primary Air Connection", "Outlet Temperature", "Zone1WindAC"
        )
        self.handles["Zone1WinAC_PrimAir_Wout"] = api_.exchange.get_actuator_handle(
            state, "Primary Air Connection", "Outlet Humidity Ratio", "Zone1WindAC"
        )
        # self.handles["Zone1WinAC_ElectPower"] = api_.exchange.get_global_handle(
        #     state,
        #     "Zone1WinAC_ElectPower"
        # )
        # self.handles["Zone1WinAC_tot_cool_Power"] = api_.exchange.get_global_handle(
        #     state,
        #     "Zone1WinAC_tot_cool_Power"
        # )
        # self.handles["Zone1WinAC_ElectEnergy"] = api_.exchange.get_global_handle(
        #     state,
        #     "Zone1WinAC_ElectEnergy"
        # )
        self.handles["Zone1WinAC_OA_MdotIn"] = api_.exchange.get_actuator_handle(
            state, "Secondary Air Connection", "Inlet Mass Flow Rate", "Zone1WindAC"
        )
        self.need_to_get_handles = False

    def handles_gotten_properly(self, state):
        handles_ok = True

        for k, v in self.handles.items():
            if v == -1:
                handles_ok = False
                api_.runtime.issue_severe(state, f"Handle not found for '{k}'")

        return handles_ok

    def initialize(self, s):
        self.primary_air_inlet_temp = api_.exchange.get_internal_variable_value(
            s, self.handles["Zone1WinAC_PrimAir_Tinlet"]
        )
        self.primary_air_hum_rat_inlet = api_.exchange.get_internal_variable_value(
            s, self.handles["Zone1WinAC_PrimAir_Winlet"]
        )
        self.outdoor_air_dry_bulb = api_.exchange.get_internal_variable_value(s, self.handles["Zone1WinAC_OA_Tdb"])
        self.outdoor_air_hum_rat = api_.exchange.get_internal_variable_value(s, self.handles["Zone1WinAC_OA_W"])
        self.q_dot_request = api_.exchange.get_internal_variable_value(s, self.handles["Zone1WinAC_Qdot_Request"])
        zone_1_outside_air_design_vol_flow = api_.exchange.get_internal_variable_value(
            s, self.handles["Zone1_OADesign_Vdot"]
        )
        zone1_win_ac_oa_rho = api_.exchange.get_internal_variable_value(s, self.handles["Zone1WinAC_OA_rho"])
        self.outdoor_air_mass_flow_design = zone_1_outside_air_design_vol_flow * zone1_win_ac_oa_rho
        self.supply_air_mass_flow_design = api_.exchange.get_internal_variable_value(
            s, self.handles["Zone1_CoolDesign_Mdot"]
        )
        self.fan_efficiency = 0.5
        self.fan_delta_pressure = 75.0
        self.rated_capacity = api_.exchange.get_internal_variable_value(s, self.handles["Zone1_CoolDesign_Cap"]) / 0.75
        self.rated_eir = 1.0 / 3.0
        self.zone_cool_thermostat = api_.exchange.get_variable_value(s, self.handles["Zone1Cooling_Tstat"])

    def simulate(self, state):
        # sim block
        if self.q_dot_request < 0.0:
            q_dot_request = self.q_dot_request
        else:
            q_dot_request = 0.0

        if api_.exchange.get_variable_value(state, self.handles["COOLINGCOILAVAILSCHED"]) == 0.0:
            q_dot_request = 0.0

        if q_dot_request == 0.0:
            self.primary_air_mass_flow_in = 0.0
            self.primary_air_mass_flow_out = 0.0
            self.primary_air_outlet_temp = self.primary_air_inlet_temp
            self.primary_air_hum_rat_outlet = self.primary_air_hum_rat_inlet
            self.tot_cool_Power = 0.0
            self.electricity_power = 0.0
            self.electricity_energy = 0.0
            self.outdoor_air_mass_flow_inlet = 0.0
            return

        recirculation_mass_flow = self.supply_air_mass_flow_design - self.outdoor_air_mass_flow_design
        recirculation_enthalpy = self.psych.enthalpy(state, self.primary_air_inlet_temp, self.primary_air_hum_rat_inlet)
        outdoor_air_enthalpy = self.psych.enthalpy(state, self.outdoor_air_dry_bulb, self.outdoor_air_hum_rat)
        mixed_enthalpy = (
            (recirculation_mass_flow * recirculation_enthalpy)
            + (self.outdoor_air_mass_flow_design * outdoor_air_enthalpy)
        ) / self.supply_air_mass_flow_design
        mixed_hum_rat = (
            (recirculation_mass_flow * self.primary_air_hum_rat_inlet)
            + (self.outdoor_air_mass_flow_design * self.outdoor_air_hum_rat)
        ) / self.supply_air_mass_flow_design
        mixed_dry_bulb = self.psych.dry_bulb(state, mixed_enthalpy, mixed_hum_rat)
        outdoor_air_pressure = api_.exchange.get_variable_value(state, self.handles["OA_Press"])
        mixed_density = self.psych.density(state, outdoor_air_pressure, mixed_dry_bulb, mixed_hum_rat)
        fan_power = (self.supply_air_mass_flow_design * self.fan_delta_pressure) / (self.fan_efficiency * mixed_density)
        fan_power_to_air = fan_power
        fan_delta_h = fan_power_to_air / self.supply_air_mass_flow_design
        fan_outlet_enthalpy = mixed_enthalpy + fan_delta_h
        fan_outlet_hum_rat = mixed_hum_rat
        fan_outlet_dry_bulb = self.psych.dry_bulb(state, fan_outlet_enthalpy, fan_outlet_hum_rat)
        fan_outlet_wet_bulb = self.psych.wet_bulb(state, fan_outlet_dry_bulb, fan_outlet_hum_rat, outdoor_air_pressure)
        rated_cbf = 0.12666
        log_rated_cbf = math.log(rated_cbf)
        a0 = (0.0 - 1.0) * log_rated_cbf * self.supply_air_mass_flow_design
        a_diff = (0.0 - 1.0) * (a0 / self.supply_air_mass_flow_design)
        cbf = math.exp(a_diff)
        tot_cap_temp_mod_fac = self.window_ac_cool_cap_f_t.curve_value(fan_outlet_wet_bulb, self.outdoor_air_dry_bulb)
        tot_cap_flow_mod_fac = self.window_ac_cool_cap_f_ff.curve_value(1.0)
        tot_cap = self.rated_capacity * tot_cap_temp_mod_fac * tot_cap_flow_mod_fac
        enthalpy_difference = tot_cap / self.supply_air_mass_flow_design
        enthalpy_adp = fan_outlet_enthalpy - (enthalpy_difference / (1.0 - cbf))
        temperature_adp = self.psych.saturation_temperature(state, enthalpy_adp, outdoor_air_pressure)
        hum_rat_adp = self.psych.humidity_ratio(state, temperature_adp, enthalpy_adp)
        enthalpy_tin_w_adp = self.psych.enthalpy(state, fan_outlet_dry_bulb, hum_rat_adp)
        if (fan_outlet_enthalpy - enthalpy_adp) != 0.0:
            shr = (enthalpy_tin_w_adp - enthalpy_adp) / (fan_outlet_enthalpy - enthalpy_adp)
            shr = min(shr, 1.0)
        else:
            shr = 1.0
        full_load_out_air_enth = fan_outlet_enthalpy - (tot_cap / self.supply_air_mass_flow_design)
        h_tin_w_out = fan_outlet_enthalpy - ((1.0 - shr) * enthalpy_difference)
        if shr != 1.0:
            full_load_out_air_hum_rat = self.psych.humidity_ratio(state, fan_outlet_dry_bulb, h_tin_w_out)
        else:
            full_load_out_air_hum_rat = fan_outlet_hum_rat
        full_load_out_air_temp = self.psych.dry_bulb(state, full_load_out_air_enth, full_load_out_air_hum_rat)
        desired_zone_enthalpy = self.psych.enthalpy(state, self.zone_cool_thermostat, self.primary_air_hum_rat_inlet)
        full_tot_cap_sens = self.supply_air_mass_flow_design * (desired_zone_enthalpy - full_load_out_air_enth)
        abs_full_tot_cap_sens = abs(full_tot_cap_sens)
        abs_q_dot_req = abs(q_dot_request)
        outlet_air_temp = 0.0
        out_air_hum_rat = 0.0
        if abs_q_dot_req < abs_full_tot_cap_sens:
            plr = (abs_q_dot_req - fan_power_to_air) / (abs_full_tot_cap_sens - fan_power_to_air)
            error_tolerance = 0.0005
            iteration_num = 0
            max_iter = 30
            relax = 0.8
            abs_error = 0.002
            while (abs_error > error_tolerance) and (iteration_num < max_iter):
                out_air_enthalpy = (plr * full_load_out_air_enth) + ((1.0 - plr) * fan_outlet_enthalpy)
                out_air_hum_rat = (plr * full_load_out_air_hum_rat) + ((1.0 - plr) * fan_outlet_hum_rat)
                outlet_air_temp = self.psych.dry_bulb(state, out_air_enthalpy, out_air_hum_rat)
                tot_cap_test = self.supply_air_mass_flow_design * (desired_zone_enthalpy - out_air_enthalpy)
                error = (abs_q_dot_req - tot_cap_test) / abs_q_dot_req
                abs_error = abs(error)
                if abs_error > error_tolerance:
                    delta_plr = (abs_q_dot_req - tot_cap_test) / abs_full_tot_cap_sens
                    plr = plr + relax * delta_plr
                    plr = min(plr, 1.0)
                iteration_num = iteration_num + 1
                if iteration_num == 16:
                    relax = 0.5
        else:
            plr = 1.0
            out_air_hum_rat = full_load_out_air_hum_rat
            outlet_air_temp = full_load_out_air_temp

        plf = self.window_ac_plf_f_plr.curve_value(plr)
        coil_rtf = plr / plf
        coil_rtf = min(coil_rtf, 1.0)
        eir_temp_mod_fac = self.window_ac_eir_f_t.curve_value(fan_outlet_wet_bulb, self.outdoor_air_dry_bulb)
        eir_flow_mod_fac = self.window_ac_eir_f_ff.curve_value(1.0)
        eir = self.rated_eir * eir_temp_mod_fac * eir_flow_mod_fac
        dx_electricity_power = tot_cap * coil_rtf * eir
        self.electricity_power = dx_electricity_power + fan_power
        self.electricity_energy = self.electricity_power * api_.exchange.system_time_step(state) * 3600
        self.tot_cool_Power = abs_full_tot_cap_sens * plr
        self.primary_air_mass_flow_out = self.supply_air_mass_flow_design
        self.primary_air_mass_flow_in = self.supply_air_mass_flow_design
        self.primary_air_outlet_temp = outlet_air_temp
        self.primary_air_hum_rat_outlet = out_air_hum_rat
        self.outdoor_air_mass_flow_inlet = self.outdoor_air_mass_flow_design

    def report(self, state):
        api_.exchange.set_actuator_value(
            state, self.handles["Zone1WinAC_PrimAir_MdotOut"], self.primary_air_mass_flow_out
        )
        api_.exchange.set_actuator_value(
            state, self.handles["Zone1WinAC_PrimAir_MdotIn"], self.primary_air_mass_flow_in
        )
        api_.exchange.set_actuator_value(state, self.handles["Zone1WinAC_PrimAir_Tout"], self.primary_air_outlet_temp)
        api_.exchange.set_actuator_value(
            state, self.handles["Zone1WinAC_PrimAir_Wout"], self.primary_air_hum_rat_outlet
        )
        # api_.exchange.set_global_value(state, self.handles["Zone1WinAC_ElectPower"], self.ElectPower)
        # api_.exchange.set_global_value(state, self.handles["Zone1WinAC_tot_cool_Power"], self.tot_cool_Power)
        # api_.exchange.set_global_value(state, self.handles["Zone1WinAC_ElectEnergy"], self.ElectEnergy)
        api_.exchange.set_actuator_value(state, self.handles["Zone1WinAC_OA_MdotIn"], self.outdoor_air_mass_flow_inlet)

    def operate(self, state):
        if not self.psych:
            self.psych = api_.functional.psychrometrics(state)
        if self.need_to_get_handles:
            self.get_handles(state)
            if not self.handles_gotten_properly(state):
                return 1
            # x = api_.exchange.get_api_data(state)  #  use this to inspect api data
        self.initialize(state)
        self.simulate(state)
        self.report(state)
        return 0


def on_user_defined_component_model(s) -> int:
    return instance.operate(s)


instance = Zone1WinACModel()
api_.runtime.callback_user_defined_component_model(
    state_, on_user_defined_component_model, "ZONE 1 WINDOW AC MODEL PROGRAM MANAGER"
)
api_.runtime.run_energyplus(state_, ["-d", run_directory, "-D", example_file_to_run])
print(f"Finished running EnergyPlus, results available in {run_directory}")
