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

"""CondFD Sky Longwave Radiation Override — Python Plugin Example.

Overrides the sky LW radiation term on a CondFD exterior surface with a
constant Enet value (W/m2). Positive = heat INTO surface.

To use with any IDF:
1. Set SURFACE_NAME to your target exterior CondFD surface
2. Set ENET_VALUE to desired sky LW flux (W/m2)
3. Add to your IDF:

    PythonPlugin:SearchPaths,
        Yes,                     !- Add Current Working Directory
        Yes;                     !- Add Input File Directory

    PythonPlugin:Instance,
        EnetOverridePlugin,      !- Name
        Yes,                     !- Run During Warmup Days
        PythonPluginCondFD_Enet, !- Module Name
        EnetSkyOverride;         !- Class Name
"""

from pyenergyplus.plugin import EnergyPlusPlugin

# === Configuration ===
SURFACE_NAME = "Zn001:Roof001"
ENET_VALUE = -200.0  # W/m2
MATERIAL_NAME = "C5 - 4 IN HW CONCRETE"
THERMAL_ABSORPTANCE_VALUE = 0.75


class EnetSkyOverride(EnergyPlusPlugin):

    def __init__(self):
        super().__init__()
        self.need_to_get_handles = True
        self.need_to_get_material_handle = True
        self.enet_handle = None
        self.thermal_absorptance_handle = None

    def on_begin_zone_timestep_before_init_heat_balance(self, state) -> int:
        if self.need_to_get_material_handle:
            self.thermal_absorptance_handle = self.api.exchange.get_actuator_handle(
                state,
                "Material",
                "Surface Property Thermal Absorptance",
                MATERIAL_NAME,
            )
            if self.thermal_absorptance_handle == -1:
                self.api.runtime.issue_severe(
                    state,
                    f"EnetSkyOverride: no thermal absorptance actuator for material '{MATERIAL_NAME}'.",
                )
                return 1
            self.need_to_get_material_handle = False

        self.api.exchange.set_actuator_value(
            state,
            self.thermal_absorptance_handle,
            THERMAL_ABSORPTANCE_VALUE,
        )
        return 0

    def on_begin_timestep_before_predictor(self, state) -> int:
        if self.need_to_get_handles:
            if not self.api.exchange.api_data_fully_ready(state):
                return 0
            self.enet_handle = self.api.exchange.get_actuator_handle(
                state,
                "CondFD Surface",
                "Sky Longwave Radiation Override",
                SURFACE_NAME,
            )
            if self.enet_handle == -1:
                self.api.runtime.issue_severe(
                    state,
                    f"EnetSkyOverride: no actuator for surface '{SURFACE_NAME}'. "
                    "Surface must be exterior + ConductionFiniteDifference.",
                )
                return 1
            self.need_to_get_handles = False

        self.api.exchange.set_actuator_value(state, self.enet_handle, ENET_VALUE)
        return 0
