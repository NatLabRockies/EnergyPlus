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


class InitChiller(EnergyPlusPlugin):
    CWLoopExitTemp = 7.0
    CWLoopDeltaTemp = 4.0

    def __init__(self):
        # init parent class
        super().__init__()

        # get functional api glycol instance
        self.glycol = None

        # handles
        self.need_to_get_handles = True
        self.handles = {}

    def get_handles(self, state):
        self.handles["h_intvar_design_flow"] = self.api.exchange.get_internal_variable_handle(
            state, "Plant Design Volume Flow Rate", "Chilled Water Loop"
        )
        self.handles["h_act_chiller_design_flow"] = self.api.exchange.get_actuator_handle(
            state, "Plant Connection 1", "Design Volume Flow Rate", "Central Chiller"
        )
        self.handles["h_act_chiller_min_flow"] = self.api.exchange.get_actuator_handle(
            state, "Plant Connection 1", "Minimum Mass Flow Rate", "Central Chiller"
        )
        self.handles["h_act_mdot_max"] = self.api.exchange.get_actuator_handle(
            state, "Plant Connection 1", "Maximum Mass Flow Rate", "Central Chiller"
        )
        self.handles["h_act_min_cap"] = self.api.exchange.get_actuator_handle(
            state, "Plant Connection 1", "Minimum Loading Capacity", "Central Chiller"
        )
        self.handles["h_act_max_cap"] = self.api.exchange.get_actuator_handle(
            state, "Plant Connection 1", "Maximum Loading Capacity", "Central Chiller"
        )
        self.handles["h_act_opt_cap"] = self.api.exchange.get_actuator_handle(
            state, "Plant Connection 1", "Optimal Loading Capacity", "Central Chiller"
        )
        self.need_to_get_handles = False

    def handles_are_valid(self, state):
        handles_are_valid = True
        for k, v in self.handles.items():
            if v == -1:
                handles_are_valid = False
                print(k, v)
                self.api.runtime.issue_severe(state, f"Handle not found for '{k}'")
        return handles_are_valid

    def on_user_defined_component_model(self, state) -> int:
        if not self.glycol:
            self.glycol = self.api.functional.glycol(state, "water")
        # for many use cases, the client will want to check `self.api.exchange.api_data_fully_ready before
        # attempting to get handles and such.  In the case of user defined components, however, the calling structure
        # shouldn't need it, so it isn't used in this example.
        if self.need_to_get_handles:
            self.get_handles(state)
            if not self.handles_are_valid(state):
                return 1
        specific_heat = self.glycol.specific_heat(state, self.CWLoopExitTemp)
        density = self.glycol.density(state, self.CWLoopExitTemp)
        vol_flow_design = self.api.exchange.get_internal_variable_value(state, self.handles["h_intvar_design_flow"])
        self.api.exchange.set_actuator_value(state, self.handles["h_act_chiller_design_flow"], vol_flow_design)
        self.api.exchange.set_actuator_value(state, self.handles["h_act_chiller_min_flow"], 0.0)
        self.api.exchange.set_actuator_value(state, self.handles["h_act_mdot_max"], vol_flow_design * density)
        capacity = specific_heat * density * self.CWLoopDeltaTemp * vol_flow_design
        self.api.exchange.set_actuator_value(state, self.handles["h_act_min_cap"], 0.0)
        self.api.exchange.set_actuator_value(state, self.handles["h_act_max_cap"], capacity)
        self.api.exchange.set_actuator_value(state, self.handles["h_act_opt_cap"], 0.9 * capacity)
        return 0


class SimChiller(EnergyPlusPlugin):
    CWLoopExitTemp = 7.0

    def __init__(self):
        # init parent class
        super().__init__()

        self.glycol = None

        # handles
        self.need_to_get_handles = True
        self.handles = {}

    def get_handles(self, state):
        self.handles["h_act_mdot_max"] = self.api.exchange.get_actuator_handle(
            state, "Plant Connection 1", "Maximum Mass Flow Rate", "Central Chiller"
        )
        self.handles["h_act_max_cap"] = self.api.exchange.get_actuator_handle(
            state, "Plant Connection 1", "Maximum Loading Capacity", "Central Chiller"
        )
        self.handles["h_intvar_chiller_load"] = self.api.exchange.get_internal_variable_handle(
            state, "Load Request for Plant Connection 1", "Central Chiller"
        )
        self.handles["h_intvar_inlet_temp"] = self.api.exchange.get_internal_variable_handle(
            state, "Inlet Temperature for Plant Connection 1", "Central Chiller"
        )
        self.handles["h_intvar_inlet_mass_flow"] = self.api.exchange.get_internal_variable_handle(
            state, "Inlet Mass Flow Rate for Plant Connection 1", "Central Chiller"
        )
        self.handles["h_act_outlet_temp"] = self.api.exchange.get_actuator_handle(
            state, "Plant Connection 1", "Outlet Temperature", "Central Chiller"
        )
        self.handles["h_act_mass_flow_request"] = self.api.exchange.get_actuator_handle(
            state, "Plant Connection 1", "Mass Flow Rate", "Central Chiller"
        )
        self.handles["h_global_chiller_energy"] = self.api.exchange.get_global_handle(state, "Chiller_ElectEnergy")

        self.need_to_get_handles = False

    def handles_are_valid(self, state):
        handles_are_valid = True
        for k, v in self.handles.items():
            if v == -1:
                handles_are_valid = False
                print(k, v)
                self.api.runtime.issue_severe(state, f"Handle not found for '{k}'")
        return handles_are_valid

    def on_user_defined_component_model(self, state) -> int:
        if not self.glycol:
            self.glycol = self.api.functional.glycol(state, "water")
        # for many use cases, the client will want to check `self.api.exchange.api_data_fully_ready before
        # attempting to get handles and such.  In the case of user defined components, however, the calling structure
        # shouldn't need it, so it isn't used in this example.
        if self.need_to_get_handles:
            self.get_handles(state)
            if not self.handles_are_valid(state):
                return 1
        # get load, inlet temp, and mdot
        chiller_load = self.api.exchange.get_internal_variable_value(state, self.handles["h_intvar_chiller_load"])
        inlet_temp = self.api.exchange.get_internal_variable_value(state, self.handles["h_intvar_inlet_temp"])
        mass_flow_rate = self.api.exchange.get_internal_variable_value(state, self.handles["h_intvar_inlet_mass_flow"])

        # leave early if chiller is off
        if chiller_load > -100.0:
            # pass inlet temp through
            self.api.exchange.set_actuator_value(state, self.handles["h_act_outlet_temp"], inlet_temp)
            # set mass flow rate and electricity to 0
            self.api.exchange.set_actuator_value(state, self.handles["h_act_mass_flow_request"], 0.0)
            self.api.exchange.set_global_value(state, self.handles["h_global_chiller_energy"], 0.0)
            return 0

        # set chiller load
        abs_load = abs(self.api.exchange.get_internal_variable_value(state, self.handles["h_intvar_chiller_load"]))
        max_capacity = self.api.exchange.get_actuator_value(state, self.handles["h_act_max_cap"])
        if abs_load > max_capacity:
            q_act = max_capacity
        else:
            q_act = abs_load

        # set chiller mdot
        if mass_flow_rate == 0.0:
            m_dot_act = self.api.exchange.get_actuator_value(state, self.handles["h_act_mdot_max"])
            self.api.exchange.set_actuator_value(state, self.handles["h_act_mass_flow_request"], m_dot_act)
        else:
            m_dot_act = mass_flow_rate

        # set outlet temp
        specific_heat = self.glycol.specific_heat(state, self.CWLoopExitTemp)
        t_out = inlet_temp - (q_act / (m_dot_act * specific_heat))
        self.api.exchange.set_actuator_value(state, self.handles["h_act_outlet_temp"], t_out)

        # set chiller energy
        eir = 1 / 4.6
        power = q_act * eir
        energy = power * 3600.0 * self.api.exchange.system_time_step(state)
        self.api.exchange.set_global_value(state, self.handles["h_global_chiller_energy"], energy)
        return 0
