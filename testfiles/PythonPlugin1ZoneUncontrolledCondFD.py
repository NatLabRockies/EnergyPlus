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

from pyenergyplus.plugin import EnergyPlusPlugin


class Material(object):
    def __init__(self, name):
        self.name = name
        self.num_nodes = 0
        self.nodes = []
        self.cp_handle = None
        self.k_handle = None
        self.flux_handle = None

    def set_num_nodes(self, num_nodes):
        self.num_nodes = num_nodes

    def set_cp_handle(self, handle):
        self.cp_handle = handle

    def set_k_handle(self, handle):
        self.k_handle = handle

    def set_flux_handle(self, handle):
        self.flux_handle = handle


class Construction(object):
    def __init__(self, name):
        self.name = name
        self.material_layers = []

    def set_material_layers(self, layers):
        for idx, l in enumerate(layers):
            self.material_layers.append(Material(l))


class CondFDSurfaceManager(EnergyPlusPlugin):

    def __init__(self):
        super().__init__()

        # test surfaces for k and cp actuators
        wall_materials = ["Vinyl&#44 Light", "WallSheathing", "WallStudAndCavityk1", "Drywall 0.5 in."]
        surfaces_names = ["Zn001:Wall001", "Zn001:Wall002", "Zn001:Wall003"]

        self.surfaces = []

        for surface_name in surfaces_names:
            constr = Construction(surface_name)
            constr.set_material_layers(wall_materials)
            self.surfaces.append(constr)

        # test surface for heat flux actuators
        wall_materials = ["Drywall 0.25 in. lay-1"]
        surfaces_names = ["Zn001:Wall004"]

        for surface_name in surfaces_names:
            constr = Construction(surface_name)
            constr.set_material_layers(wall_materials)
            self.surfaces.append(constr)

        self.zone_air_temp_handle = None
        self.outdoor_air_temp_handle = None

        self.need_to_get_handles = True

        # set flag to use EMS to modify k and cp
        self.use_ems = True

    def get_handles(self, state):

        # get air temperature handles
        self.zone_air_temp_handle = self.api.exchange.get_variable_handle(
            state, "Zone Mean Air Temperature", "Zone One"
        )
        self.outdoor_air_temp_handle = self.api.exchange.get_variable_handle(
            state, "Site Outdoor Air Drybulb Temperature", "Environment"
        )
        # get surface handles
        for surface in self.surfaces:
            for material in surface.material_layers:
                # set the total number of nodes in each material layer
                num_nodes = self.api.exchange.get_num_nodes_in_cond_fd_surf_layer(state, surface.name, material.name)
                material.set_num_nodes(num_nodes)

                # set the conductivity handle in each material layer
                k_handle = self.api.exchange.get_actuator_handle(
                    state, "CondFD Surface Material Layer", "Thermal Conductivity", f"{surface.name}:{material.name}"
                )
                material.set_k_handle(k_handle)

                # set the specific heat handle in each material layer
                cp_handle = self.api.exchange.get_actuator_handle(
                    state, "CondFD Surface Material Layer", "Specific Heat", f"{surface.name}:{material.name}"
                )
                material.set_cp_handle(cp_handle)

                # set the heat flux handle for each material layer
                flux_handle = self.api.exchange.get_actuator_handle(
                    state, "CondFD Surface Material Layer", "Heat Flux", f"{surface.name}:{material.name}"
                )
                material.set_flux_handle(flux_handle)

        self.need_to_get_handles = False

    def on_begin_timestep_before_predictor(self, state) -> int:
        if self.need_to_get_handles:
            if self.api.exchange.api_data_fully_ready(state):
                self.get_handles(state)
            else:
                return 0

        # return early if we're not using EMS
        if not self.use_ems:
            return 0

        # get temperatures
        setpoint = 20
        outdoor_air_temp = self.api.exchange.get_variable_value(state, self.outdoor_air_temp_handle)
        zone_air_temp = self.api.exchange.get_variable_value(state, self.zone_air_temp_handle)

        # check whether we need to actuate the walls
        actuate_walls = False

        # cooling condition
        if (zone_air_temp > setpoint) and (zone_air_temp > outdoor_air_temp):
            actuate_walls = True

        # heating condition
        if (zone_air_temp < setpoint) and (zone_air_temp < outdoor_air_temp):
            actuate_walls = True

        # update k and cp using EMS based on temperature conditions
        for surface in self.surfaces:
            for layer in surface.material_layers:
                if layer.name == "WallStudAndCavityk1":
                    if actuate_walls:
                        k = 10
                        cp = 1500
                    else:
                        k = 2.4
                        cp = 1179
                    self.api.exchange.set_actuator_value(state, layer.k_handle, k)
                    self.api.exchange.set_actuator_value(state, layer.cp_handle, cp)
                elif layer.name == "Drywall 0.25 in. lay-1":
                    self.api.exchange.set_actuator_value(state, layer.flux_handle, 10)

        return 0
