#!/usr/bin/env python3
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

"""
This file is a standalone EnergyPlusPlugin script tester.
It is called using a single argument, the path to an EnergyPlus plugin file
"""

import inspect
import os
import sys
from importlib import util as import_util
from unittest.mock import Mock

from pyenergyplus.plugin import EnergyPlusPlugin


def generate_mock_api(bare_mock_api_instance: Mock) -> Mock:
    # Functional methods
    bare_mock_api_instance.functional.glycol.return_value.specific_heat.return_value = 3.14
    bare_mock_api_instance.functional.glycol.return_value.density.return_value = 3.14
    bare_mock_api_instance.functional.glycol.return_value.conductivity.return_value = 3.14
    bare_mock_api_instance.functional.glycol.return_value.viscosity.return_value = 3.14
    bare_mock_api_instance.functional.refrigerant.return_value.saturation_pressure.return_value = 3.14
    bare_mock_api_instance.functional.refrigerant.return_value.saturation_temperature.return_value = 3.14
    bare_mock_api_instance.functional.refrigerant.return_value.saturated_enthalpy.return_value = 3.14
    bare_mock_api_instance.functional.refrigerant.return_value.saturated_density.return_value = 3.14
    bare_mock_api_instance.functional.refrigerant.return_value.saturated_specific_heat.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.density.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.latent_energy_of_air.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.latent_energy_of_moisture_in_air.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.enthalpy.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.enthalpy_b.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.specific_heat.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.dry_bulb.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.vapor_density.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.relative_humidity.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.relative_humidity_b.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.wet_bulb.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.specific_volume.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.saturation_pressure.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.saturation_temperature.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.vapor_density_b.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.humidity_ratio.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.humidity_ratio_b.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.humidity_ratio_c.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.humidity_ratio_d.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.dew_point.return_value = 3.14
    bare_mock_api_instance.functional.psychrometrics.return_value.dew_point_b.return_value = 3.14
    bare_mock_api_instance.functional.callback_error.return_value = None
    # Runtime methods
    bare_mock_api_instance.runtime.run_energyplus.return_value = 1
    bare_mock_api_instance.runtime.issue_warning.return_value = None
    bare_mock_api_instance.runtime.issue_severe.return_value = None
    bare_mock_api_instance.runtime.issue_text.return_value = None
    bare_mock_api_instance.runtime.callback_progress.return_value = None
    bare_mock_api_instance.runtime.callback_message.return_value = None
    bare_mock_api_instance.runtime.callback_begin_new_environment.return_value = None
    bare_mock_api_instance.runtime.callback_after_new_environment_warmup_complete.return_value = None
    bare_mock_api_instance.runtime.callback_begin_zone_timestep_before_init_heat_balance.return_value = None
    bare_mock_api_instance.runtime.callback_begin_zone_timestep_after_init_heat_balance.return_value = None
    bare_mock_api_instance.runtime.callback_begin_system_timestep_before_predictor.return_value = None
    bare_mock_api_instance.runtime.callback_after_predictor_before_hvac_managers.return_value = None
    bare_mock_api_instance.runtime.callback_after_predictor_after_hvac_managers.return_value = None
    bare_mock_api_instance.runtime.callback_inside_system_iteration_loop.return_value = None
    bare_mock_api_instance.runtime.callback_end_zone_timestep_before_zone_reporting.return_value = None
    bare_mock_api_instance.runtime.callback_end_zone_timestep_after_zone_reporting.return_value = None
    bare_mock_api_instance.runtime.callback_end_system_timestep_before_hvac_reporting.return_value = None
    bare_mock_api_instance.runtime.callback_end_system_timestep_after_hvac_reporting.return_value = None
    bare_mock_api_instance.runtime.callback_end_zone_sizing.return_value = None
    bare_mock_api_instance.runtime.callback_end_system_sizing.return_value = None
    bare_mock_api_instance.runtime.callback_after_component_get_input.return_value = None
    bare_mock_api_instance.runtime.callback_user_defined_component_model.return_value = None
    bare_mock_api_instance.runtime.callback_unitary_system_sizing.return_value = None
    bare_mock_api_instance.runtime.clear_callbacks.return_value = None
    # Data exchange methods
    bare_mock_api_instance.exchange.get_actuator_handle.return_value = 0
    bare_mock_api_instance.exchange.hour.return_value = 1
    bare_mock_api_instance.exchange.request_variable.return_value = None
    bare_mock_api_instance.exchange.get_variable_handle.return_value = 1
    bare_mock_api_instance.exchange.et_meter_handle.return_value = 1
    bare_mock_api_instance.exchange.get_actuator_handle.return_value = 1
    bare_mock_api_instance.exchange.get_variable_value.return_value = 3.14
    bare_mock_api_instance.exchange.get_meter_value.return_value = 3.14
    bare_mock_api_instance.exchange.set_actuator_value.return_value = None
    bare_mock_api_instance.exchange.reset_actuator.return_value = None
    bare_mock_api_instance.exchange.get_internal_variable_handle.return_value = 1
    bare_mock_api_instance.exchange.get_internal_variable_value.return_value = 3.14
    bare_mock_api_instance.exchange.get_global_handle.return_value = 1
    bare_mock_api_instance.exchange.get_global_value.return_value = 3.14
    bare_mock_api_instance.exchange.set_global_value.return_value = None
    bare_mock_api_instance.exchange.year.return_value = 1
    bare_mock_api_instance.exchange.calendarYear.return_value = 1
    bare_mock_api_instance.exchange.month.return_value = 1
    bare_mock_api_instance.exchange.day_of_month.return_value = 1
    bare_mock_api_instance.exchange.hour.return_value = 1
    bare_mock_api_instance.exchange.current_time.return_value = 3.14
    bare_mock_api_instance.exchange.minutes.return_value = 1
    bare_mock_api_instance.exchange.day_of_week.return_value = 1
    bare_mock_api_instance.exchange.day_of_year.return_value = 1
    bare_mock_api_instance.exchange.daylight_savings_time_indicator.return_value = True
    bare_mock_api_instance.exchange.holiday_index.return_value = 1
    bare_mock_api_instance.exchange.sun_is_up.return_value = True
    bare_mock_api_instance.exchange.is_raining.return_value = True
    bare_mock_api_instance.exchange.warmup_flag.return_value = 3.14
    bare_mock_api_instance.exchange.system_time_step.return_value = 3.14
    bare_mock_api_instance.exchange.current_environment_num.return_value = 1
    # Return the now fully populated mock
    return bare_mock_api_instance


class EnergyPlusPluginTesting(object):

    @staticmethod
    def plugin_file_tester(file_path):
        modules = []

        if os.path.exists(file_path):
            print("   OK : File path exists at: " + file_path)
        else:
            print("ERROR : File path does not exist!  Path: " + file_path)
            return 1

        if file_path.endswith(".py"):
            print("   OK : File ends with .py")
        else:
            print("ERROR : File path does NOT end with .py")
            return 1

        module_spec = import_util.spec_from_file_location("eplus_plugin_module", file_path)
        this_module = import_util.module_from_spec(module_spec)
        try:
            modules.append(this_module)
            module_spec.loader.exec_module(this_module)
            print("   OK : Python import succeeded")
        except ImportError as ie:
            # this error generally means they have a bad plugin class or something
            print("ERROR : Import error occurred on plugin file %s: %s" % (file_path, str(ie)))
            return 1
        except SyntaxError as se:
            # syntax errors are, well, syntax errors in the Python code itself
            print("ERROR : Syntax error occurred on plugin file %s, line %s: %s" % (file_path, se.lineno, se.msg))
            return 1
        except Exception as e:
            # there's always the potential of some other unforeseen thing going on when a plugin is executed
            print("ERROR : Unexpected error occurred trying to import plugin: %s: %a" % (file_path, str(e)))
            return 1

        successful_classes = []
        for this_module in modules:
            class_members = inspect.getmembers(this_module, inspect.isclass)
            for this_class in class_members:
                this_class_name, this_class_type = this_class
                print(' INFO : Encountered class: "' + this_class_name + '", testing now...')
                if this_class_type is EnergyPlusPlugin:
                    print(" INFO : Skipping the actual plugin base class: " + this_class_name)
                    continue
                elif not issubclass(this_class_type, EnergyPlusPlugin):
                    print(" INFO : Skipping class that does not inherit plugin base class: " + this_class_name)
                    continue
                else:  # we found one!
                    print("   OK : Basic inheritance checks out OK for class: " + this_class_name)

                    try:
                        plugin_instance = this_class_type()
                        print("   OK : Instantiation of derived class works")
                    except Exception as e:
                        print("ERROR : Instantiation of derived class malfunctioning; reason: " + str(e))
                        return 1

                    # now use a Mock API to test the script in isolated fashion
                    plugin_instance.api = generate_mock_api(Mock())

                    # it's possible that you could override the API methods further here if you wanted to test
                    # the script in a very custom fashion

                    # check each overridden function and call it
                    # noinspection PyProtectedMember
                    functions_overridden = plugin_instance._detect_overridden()

                    expected_overrides = [
                        "on_begin_new_environment",
                        "on_after_new_environment_warmup_is_complete",
                        "on_begin_zone_timestep_before_init_heat_balance",
                        "on_begin_zone_timestep_after_init_heat_balance",
                        "on_begin_timestep_before_predictor",
                        "on_after_predictor_before_hvac_managers",
                        "on_after_predictor_after_hvac_managers",
                        "on_inside_hvac_system_iteration_loop",
                        "on_end_of_zone_timestep_before_zone_reporting",
                        "on_end_of_zone_timestep_after_zone_reporting",
                        "on_end_of_system_timestep_before_hvac_reporting",
                        "on_end_of_system_timestep_after_hvac_reporting",
                        "on_end_of_zone_sizing",
                        "on_end_of_system_sizing",
                        "on_end_of_component_input_read_in",
                        "on_user_defined_component_model",
                        "on_unitary_system_sizing",
                    ]

                    for func in functions_overridden:
                        if func in expected_overrides:
                            method_to_call = getattr(plugin_instance, func)
                            try:
                                dummy_state_arg = 0
                                response = method_to_call(dummy_state_arg)
                                print("   OK : Overridden %s() function execution works" % func)
                            except Exception as e:
                                print("ERROR : %s() function not overridden, or is broken; reason: %s" % (func, str(e)))
                                return 1
                            if isinstance(response, int):
                                print("   OK : %s() returns an int, this is the expected condition" % func)
                            else:
                                print("ERROR : Bad return from %s(); it must return an integer!" % func)
                                return 1

                    successful_classes.append(this_class_name)

        if len(successful_classes) > 0:
            print("   OK : Found %s successful EnergyPlusPlugin classes:" % len(successful_classes))
            for c in successful_classes:
                print("   OK :   " + c)
            return 0
        else:
            print("ERROR : Did not find ANY successful EnergyPlusPlugin imports in this file!")
            return 1


def main():
    if len(sys.argv) != 2:
        print("Bad call to tester, give one command line argument, the full path to an EnergyPlus plugin file")
        sys.exit(2)
    else:
        arg_file_path = sys.argv[1]
        response = EnergyPlusPluginTesting.plugin_file_tester(arg_file_path)
        sys.exit(response)


if __name__ == "__main__":
    main()
