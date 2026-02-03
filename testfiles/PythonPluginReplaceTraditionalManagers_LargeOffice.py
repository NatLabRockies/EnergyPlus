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


class PythonSetpointManagers(EnergyPlusPlugin):

    def __init__(self):
        # init parent class
        super().__init__()

        # alias
        self.e = self.api.exchange

        # handles
        self.need_to_get_handles = True
        self.handles = {}
        self.values = {}

        # members
        self.Toffset = 0.8333
        self.NoAction = 0.0
        self.ForceOff = 1.0
        self.CycleOn = 2.0
        self.CycleOnZoneFansOnly = 3.0

    def get_handles(self, state):
        self.need_to_get_handles = False
        self.handles["Seasonal_Reset_SAT_Sched"] = self.e.get_variable_handle(
            state, "Schedule Value", "Seasonal-Reset-Supply-Air-Temp-Sch"
        )
        self.handles["VAV_5_SAT_setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "VAV_5 Supply Equipment Outlet Node"
        )
        self.handles["T_VAV5FanOut"] = self.e.get_variable_handle(
            state, "System Node Temperature", "VAV_5 Supply Equipment Outlet Node"
        )
        self.handles["T_VAV5FanIn"] = self.e.get_variable_handle(
            state, "System Node Temperature", "VAV_5_HeatC-VAV_5_FanNode"
        )
        self.handles["VAV_5_CoolC_Setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "VAV_5_CoolC-VAV_5_HeatCNode"
        )
        self.handles["VAV_5_HeatC_Setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "VAV_5_HeatC-VAV_5_FanNode"
        )
        self.handles["VAV_5_OA_Setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "VAV_5_OA-VAV_5_CoolCNode"
        )
        self.handles["VAV_1_SAT_setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "VAV_1 Supply Equipment Outlet Node"
        )
        self.handles["VAV_1_CoolC_Setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "VAV_1_CoolC-VAV_1_HeatCNode"
        )
        self.handles["T_VAV1FanOut"] = self.e.get_variable_handle(
            state, "System Node Temperature", "VAV_1 Supply Equipment Outlet Node"
        )
        self.handles["T_VAV1FanIn"] = self.e.get_variable_handle(
            state, "System Node Temperature", "VAV_1_HeatC-VAV_1_FanNode"
        )
        self.handles["VAV_1_HeatC_Setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "VAV_1_HeatC-VAV_1_FanNode"
        )
        self.handles["VAV_1_OA_Setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "VAV_1_OA-VAV_1_CoolCNode"
        )
        self.handles["VAV_3_SAT_setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "VAV_3 Supply Equipment Outlet Node"
        )
        self.handles["VAV_3_CoolC_Setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "VAV_3_CoolC-VAV_3_HeatCNode"
        )
        self.handles["T_VAV3FanOut"] = self.e.get_variable_handle(
            state, "System Node Temperature", "VAV_3 Supply Equipment Outlet Node"
        )
        self.handles["T_VAV3FanIn"] = self.e.get_variable_handle(
            state, "System Node Temperature", "VAV_3_HeatC-VAV_3_FanNode"
        )
        self.handles["VAV_2_SAT_setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "VAV_2 Supply Equipment Outlet Node"
        )
        self.handles["VAV_2_CoolC_Setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "VAV_2_CoolC-VAV_2_HeatCNode"
        )
        self.handles["T_VAV2FanOut"] = self.e.get_variable_handle(
            state, "System Node Temperature", "VAV_2 Supply Equipment Outlet Node"
        )
        self.handles["T_VAV2FanIn"] = self.e.get_variable_handle(
            state, "System Node Temperature", "VAV_2_HeatC-VAV_2_FanNode"
        )
        self.handles["VAV_2_HeatC_Setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "VAV_2_HeatC-VAV_2_FanNode"
        )
        self.handles["VAV_2_OA_Setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "VAV_2_OA-VAV_2_CoolCNode"
        )
        self.handles["CW_Loop_Temp_Schedule"] = self.e.get_variable_handle(
            state, "Schedule Value", "CW-Loop-Temp-Schedule"
        )
        self.handles["CoolSys1_Loop_Setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "CoolSys1 Supply Outlet Node"
        )
        self.handles["HW_Loop_Temp_Schedule"] = self.e.get_variable_handle(
            state, "Schedule Value", "HW-Loop-Temp-Schedule"
        )
        self.handles["HeatSys1_Loop_Setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "HeatSys1 Supply Outlet Node"
        )
        self.handles["SHWSys1_Loop_Temp_Schedule"] = self.e.get_variable_handle(
            state, "Schedule Value", "SHWSys1-Loop-Temp-Schedule"
        )
        self.handles["SHWSys1_Loop_Setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "SHWSys1 Supply Outlet Node"
        )
        self.handles["heating_setpoint"] = self.e.get_variable_handle(state, "Schedule Value", "HTGSETP_SCH")
        self.handles["cooling_setpoint"] = self.e.get_variable_handle(state, "Schedule Value", "CLGSETP_SCH")
        self.handles["TzoneVAV5"] = self.e.get_variable_handle(state, "Zone Mean Air Temperature", "Basement")
        self.handles["VAV_5_NightCycleStatus"] = self.e.get_actuator_handle(
            state, "AirLoopHVAC", "Availability Status", "VAV_5"
        )
        self.handles["TzoneVAV1_1"] = self.e.get_variable_handle(state, "Zone Mean Air Temperature", "Core_bottom")
        self.handles["TzoneVAV1_2"] = self.e.get_variable_handle(
            state, "Zone Mean Air Temperature", "Perimeter_bot_ZN_3"
        )
        self.handles["TzoneVAV1_3"] = self.e.get_variable_handle(
            state, "Zone Mean Air Temperature", "Perimeter_bot_ZN_2"
        )
        self.handles["TzoneVAV1_4"] = self.e.get_variable_handle(
            state, "Zone Mean Air Temperature", "Perimeter_bot_ZN_1"
        )
        self.handles["TzoneVAV1_5"] = self.e.get_variable_handle(
            state, "Zone Mean Air Temperature", "Perimeter_bot_ZN_4"
        )
        self.handles["VAV_1_NightCycleStatus"] = self.e.get_actuator_handle(
            state, "AirLoopHVAC", "Availability Status", "VAV_1"
        )
        self.handles["TzoneVAV2_1"] = self.e.get_variable_handle(state, "Zone Mean Air Temperature", "Core_mid")
        self.handles["TzoneVAV2_2"] = self.e.get_variable_handle(
            state, "Zone Mean Air Temperature", "Perimeter_mid_ZN_3"
        )
        self.handles["TzoneVAV2_3"] = self.e.get_variable_handle(
            state, "Zone Mean Air Temperature", "Perimeter_mid_ZN_2"
        )
        self.handles["TzoneVAV2_4"] = self.e.get_variable_handle(
            state, "Zone Mean Air Temperature", "Perimeter_mid_ZN_1"
        )
        self.handles["TzoneVAV2_5"] = self.e.get_variable_handle(
            state, "Zone Mean Air Temperature", "Perimeter_mid_ZN_4"
        )
        self.handles["VAV_2_NightCycleStatus"] = self.e.get_actuator_handle(
            state, "AirLoopHVAC", "Availability Status", "VAV_2"
        )
        self.handles["VAV_3_HeatC_Setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "VAV_3_HeatC-VAV_3_FanNode"
        )
        self.handles["VAV_3_NightCycleStatus"] = self.e.get_actuator_handle(
            state, "AirLoopHVAC", "Availability Status", "VAV_3"
        )
        self.handles["TzoneVAV3_1"] = self.e.get_variable_handle(state, "Zone Mean Air Temperature", "Core_top")
        self.handles["TzoneVAV3_2"] = self.e.get_variable_handle(
            state, "Zone Mean Air Temperature", "Perimeter_top_ZN_3"
        )
        self.handles["TzoneVAV3_3"] = self.e.get_variable_handle(
            state, "Zone Mean Air Temperature", "Perimeter_top_ZN_2"
        )
        self.handles["TzoneVAV3_4"] = self.e.get_variable_handle(
            state, "Zone Mean Air Temperature", "Perimeter_top_ZN_1"
        )
        self.handles["TzoneVAV3_5"] = self.e.get_variable_handle(
            state, "Zone Mean Air Temperature", "Perimeter_top_ZN_4"
        )
        self.handles["VAV_3_OA_Setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "VAV_3_OA-VAV_3_CoolCNode"
        )
        self.handles["HeatSys1_Boiler_Setpoint"] = self.e.get_actuator_handle(
            state, "System Node Setpoint", "Temperature Setpoint", "HeatSys1 Supply Equipment Outlet Node"
        )

    def handles_are_valid(self, state):
        handles_are_valid = True
        for k, v in self.handles.items():
            if v == -1:
                handles_are_valid = False
                self.api.runtime.issue_severe(state, f"Handle not found for '{k}'")
        return handles_are_valid

    def vav_5_sched_setpoint(self, state):
        self.values["Seasonal_Reset_SAT_Sched"] = self.e.get_variable_value(
            state, self.handles["Seasonal_Reset_SAT_Sched"]
        )
        self.e.set_actuator_value(state, self.handles["VAV_5_SAT_setpoint"], self.values["Seasonal_Reset_SAT_Sched"])

    def vav_5_mixed_air_managers(self, state):
        T_VAV5FanOut = self.e.get_variable_value(state, self.handles["T_VAV5FanOut"])
        T_VAV5FanIn = self.e.get_variable_value(state, self.handles["T_VAV5FanIn"])
        set_val = self.values["Seasonal_Reset_SAT_Sched"] - (T_VAV5FanOut - T_VAV5FanIn)
        self.e.set_actuator_value(state, self.handles["VAV_5_CoolC_Setpoint"], set_val)
        self.e.set_actuator_value(state, self.handles["VAV_5_HeatC_Setpoint"], set_val)
        self.e.set_actuator_value(state, self.handles["VAV_5_OA_Setpoint"], set_val)

    def vav_1_sched_setpoint(self, state):
        self.e.set_actuator_value(state, self.handles["VAV_1_SAT_setpoint"], self.values["Seasonal_Reset_SAT_Sched"])

    def vav_1_mixed_air_managers(self, state):
        T_VAV1FanOut = self.e.get_variable_value(state, self.handles["T_VAV1FanOut"])
        T_VAV1FanIn = self.e.get_variable_value(state, self.handles["T_VAV1FanIn"])
        set_val = self.values["Seasonal_Reset_SAT_Sched"] - (T_VAV1FanOut - T_VAV1FanIn)
        self.e.set_actuator_value(state, self.handles["VAV_1_CoolC_Setpoint"], set_val)
        self.e.set_actuator_value(state, self.handles["VAV_1_HeatC_Setpoint"], set_val)
        self.e.set_actuator_value(state, self.handles["VAV_1_OA_Setpoint"], set_val)

    def vav_3_sched_setpoint(self, state):
        self.e.set_actuator_value(state, self.handles["VAV_3_SAT_setpoint"], self.values["Seasonal_Reset_SAT_Sched"])

    def vav_3_mixed_air_managers(self, state):
        T_VAV3FanOut = self.e.get_variable_value(state, self.handles["T_VAV3FanOut"])
        T_VAV3FanIn = self.e.get_variable_value(state, self.handles["T_VAV3FanIn"])
        set_val = self.values["Seasonal_Reset_SAT_Sched"] - (T_VAV3FanOut - T_VAV3FanIn)
        self.e.set_actuator_value(state, self.handles["VAV_3_CoolC_Setpoint"], set_val)
        self.e.set_actuator_value(state, self.handles["VAV_3_HeatC_Setpoint"], set_val)
        self.e.set_actuator_value(state, self.handles["VAV_3_OA_Setpoint"], set_val)

    def vav_2_sched_setpoint(self, state):
        self.e.set_actuator_value(state, self.handles["VAV_2_SAT_setpoint"], self.values["Seasonal_Reset_SAT_Sched"])

    def vav_2_mixed_air_managers(self, state):
        T_VAV2FanOut = self.e.get_variable_value(state, self.handles["T_VAV2FanOut"])
        T_VAV2FanIn = self.e.get_variable_value(state, self.handles["T_VAV2FanIn"])
        set_val = self.values["Seasonal_Reset_SAT_Sched"] - (T_VAV2FanOut - T_VAV2FanIn)
        self.e.set_actuator_value(state, self.handles["VAV_2_CoolC_Setpoint"], set_val)
        self.e.set_actuator_value(state, self.handles["VAV_2_HeatC_Setpoint"], set_val)
        self.e.set_actuator_value(state, self.handles["VAV_2_OA_Setpoint"], set_val)

    def cool_sys_1_sched_setpoint(self, state):
        set_val = self.e.get_variable_value(state, self.handles["CW_Loop_Temp_Schedule"])
        self.e.set_actuator_value(state, self.handles["CoolSys1_Loop_Setpoint"], set_val)

    def heat_sys_1_sched_setpoint(self, state):
        set_val = self.e.get_variable_value(state, self.handles["HW_Loop_Temp_Schedule"])
        self.e.set_actuator_value(state, self.handles["HeatSys1_Loop_Setpoint"], set_val)

    def shw_sys_1_sched_setpoint(self, state):
        set_val = self.e.get_variable_value(state, self.handles["SHWSys1_Loop_Temp_Schedule"])
        self.e.set_actuator_value(state, self.handles["SHWSys1_Loop_Setpoint"], set_val)

    def vav_5_night_cycle_mgr(self, state):
        self.values["heating_setpoint"] = self.e.get_variable_value(state, self.handles["heating_setpoint"])
        self.values["cooling_setpoint"] = self.e.get_variable_value(state, self.handles["cooling_setpoint"])
        VAV5_heating_TurnOn = self.values["heating_setpoint"]
        VAV5_heating_TurnOff = self.values["heating_setpoint"] + (2 * self.Toffset)
        VAV5_cooling_TurnOn = self.values["cooling_setpoint"]
        VAV5_cooling_TurnOff = self.values["cooling_setpoint"] - (2 * self.Toffset)
        TzoneVAV5 = self.e.get_variable_value(state, self.handles["TzoneVAV5"])
        if TzoneVAV5 < VAV5_heating_TurnOn:
            self.e.set_actuator_value(state, self.handles["VAV_5_NightCycleStatus"], self.CycleOn)
            return
        else:
            self.e.set_actuator_value(state, self.handles["VAV_5_NightCycleStatus"], self.NoAction)
        if TzoneVAV5 > VAV5_cooling_TurnOn:
            self.e.set_actuator_value(state, self.handles["VAV_5_NightCycleStatus"], self.CycleOn)
        elif TzoneVAV5 < VAV5_cooling_TurnOff:
            self.e.set_actuator_value(state, self.handles["VAV_5_NightCycleStatus"], self.NoAction)

    def vav_1_night_cycle_mgr(self, state):
        VAV1_heating_TurnOn = self.values["heating_setpoint"]
        VAV1_heating_TurnOff = self.values["heating_setpoint"] + (2 * self.Toffset)
        VAV1_cooling_TurnOn = self.values["cooling_setpoint"]
        VAV1_cooling_TurnOff = self.values["cooling_setpoint"] - (2 * self.Toffset)

        TzoneVAV1_1 = self.e.get_variable_value(state, self.handles["TzoneVAV1_1"])
        TzoneVAV1_2 = self.e.get_variable_value(state, self.handles["TzoneVAV1_2"])
        TzoneVAV1_3 = self.e.get_variable_value(state, self.handles["TzoneVAV1_3"])
        TzoneVAV1_4 = self.e.get_variable_value(state, self.handles["TzoneVAV1_4"])
        TzoneVAV1_5 = self.e.get_variable_value(state, self.handles["TzoneVAV1_5"])

        Tmin = min(TzoneVAV1_1, TzoneVAV1_2)
        Tmin = min(Tmin, TzoneVAV1_3)
        Tmin = min(Tmin, TzoneVAV1_4)
        Tmin = min(Tmin, TzoneVAV1_5)
        Tmax = max(TzoneVAV1_1, TzoneVAV1_2)
        Tmax = max(Tmax, TzoneVAV1_3)
        Tmax = max(Tmax, TzoneVAV1_4)
        Tmax = max(Tmax, TzoneVAV1_5)

        if Tmin < VAV1_heating_TurnOn:
            self.e.set_actuator_value(state, self.handles["VAV_1_NightCycleStatus"], self.CycleOn)
            return
        elif Tmin > VAV1_heating_TurnOff:
            self.e.set_actuator_value(state, self.handles["VAV_1_NightCycleStatus"], self.NoAction)

        if Tmax > VAV1_cooling_TurnOn:
            self.e.set_actuator_value(state, self.handles["VAV_1_NightCycleStatus"], self.CycleOn)
        elif Tmax < VAV1_cooling_TurnOff:
            self.e.set_actuator_value(state, self.handles["VAV_1_NightCycleStatus"], self.NoAction)

    def vav_2_night_cycle_mgr(self, state):
        VAV2_heating_TurnOn = self.values["heating_setpoint"]
        VAV2_heating_TurnOff = self.values["heating_setpoint"] + (2 * self.Toffset)
        VAV2_cooling_TurnOn = self.values["cooling_setpoint"]
        VAV2_cooling_TurnOff = self.values["cooling_setpoint"] - (2 * self.Toffset)

        TzoneVAV2_1 = self.e.get_variable_value(state, self.handles["TzoneVAV2_1"])
        TzoneVAV2_2 = self.e.get_variable_value(state, self.handles["TzoneVAV2_2"])
        TzoneVAV2_3 = self.e.get_variable_value(state, self.handles["TzoneVAV2_3"])
        TzoneVAV2_4 = self.e.get_variable_value(state, self.handles["TzoneVAV2_4"])
        TzoneVAV2_5 = self.e.get_variable_value(state, self.handles["TzoneVAV2_5"])

        Tmin = min(TzoneVAV2_1, TzoneVAV2_2)
        Tmin = min(Tmin, TzoneVAV2_3)
        Tmin = min(Tmin, TzoneVAV2_4)
        Tmin = min(Tmin, TzoneVAV2_5)
        Tmax = max(TzoneVAV2_1, TzoneVAV2_2)
        Tmax = max(Tmax, TzoneVAV2_3)
        Tmax = max(Tmax, TzoneVAV2_4)
        Tmax = max(Tmax, TzoneVAV2_5)

        if Tmin < VAV2_heating_TurnOn:
            self.e.set_actuator_value(state, self.handles["VAV_2_NightCycleStatus"], self.CycleOn)
            return
        elif Tmin > VAV2_heating_TurnOff:
            self.e.set_actuator_value(state, self.handles["VAV_2_NightCycleStatus"], self.NoAction)

        if Tmax > VAV2_cooling_TurnOn:
            self.e.set_actuator_value(state, self.handles["VAV_2_NightCycleStatus"], self.CycleOn)
        elif Tmax < VAV2_cooling_TurnOff:
            self.e.set_actuator_value(state, self.handles["VAV_2_NightCycleStatus"], self.NoAction)

    def vav_3_night_cycle_mgr(self, state):
        VAV3_heating_TurnOn = self.values["heating_setpoint"]
        VAV3_heating_TurnOff = self.values["heating_setpoint"] + (2 * self.Toffset)
        VAV3_cooling_TurnOn = self.values["cooling_setpoint"]
        VAV3_cooling_TurnOff = self.values["cooling_setpoint"] - (2 * self.Toffset)

        TzoneVAV3_1 = self.e.get_variable_value(state, self.handles["TzoneVAV3_1"])
        TzoneVAV3_2 = self.e.get_variable_value(state, self.handles["TzoneVAV3_2"])
        TzoneVAV3_3 = self.e.get_variable_value(state, self.handles["TzoneVAV3_3"])
        TzoneVAV3_4 = self.e.get_variable_value(state, self.handles["TzoneVAV3_4"])
        TzoneVAV3_5 = self.e.get_variable_value(state, self.handles["TzoneVAV3_5"])

        Tmin = min(TzoneVAV3_1, TzoneVAV3_2)
        Tmin = min(Tmin, TzoneVAV3_3)
        Tmin = min(Tmin, TzoneVAV3_4)
        Tmin = min(Tmin, TzoneVAV3_5)
        Tmax = max(TzoneVAV3_1, TzoneVAV3_2)
        Tmax = max(Tmax, TzoneVAV3_3)
        Tmax = max(Tmax, TzoneVAV3_4)
        Tmax = max(Tmax, TzoneVAV3_5)

        if Tmin < VAV3_heating_TurnOn:
            self.e.set_actuator_value(state, self.handles["VAV_3_NightCycleStatus"], self.CycleOn)
            return
        elif Tmin > VAV3_heating_TurnOff:
            self.e.set_actuator_value(state, self.handles["VAV_3_NightCycleStatus"], self.NoAction)

        if Tmax > VAV3_cooling_TurnOn:
            self.e.set_actuator_value(state, self.handles["VAV_3_NightCycleStatus"], self.CycleOn)
        elif Tmax < VAV3_cooling_TurnOff:
            self.e.set_actuator_value(state, self.handles["VAV_3_NightCycleStatus"], self.NoAction)

    def on_after_predictor_after_hvac_managers(self, state) -> int:
        if not self.e.api_data_fully_ready(state):
            return 0
        if self.need_to_get_handles:
            self.get_handles(state)
            if not self.handles_are_valid(state):
                return 1
        self.vav_5_sched_setpoint(state)
        self.vav_5_mixed_air_managers(state)
        self.vav_1_sched_setpoint(state)
        self.vav_1_mixed_air_managers(state)
        self.vav_3_sched_setpoint(state)
        self.vav_3_mixed_air_managers(state)
        self.vav_2_sched_setpoint(state)
        self.vav_2_mixed_air_managers(state)
        self.cool_sys_1_sched_setpoint(state)
        self.heat_sys_1_sched_setpoint(state)
        self.shw_sys_1_sched_setpoint(state)
        self.vav_5_night_cycle_mgr(state)
        self.vav_1_night_cycle_mgr(state)
        self.vav_2_night_cycle_mgr(state)
        self.vav_3_night_cycle_mgr(state)
        return 0
