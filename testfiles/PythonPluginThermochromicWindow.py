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


class ZN_1_wall_south_Window_1_Control(EnergyPlusPlugin):

    def __init__(self):
        # init parent class
        super().__init__()
        self.need_to_get_handles = True
        self.Win1_Tout_handle = None
        self.Win1_Construct_handle = None
        self.TCwindow_25_handle = None
        self.TCwindow_27_handle = None
        self.TCwindow_29_handle = None
        self.TCwindow_31_handle = None
        self.TCwindow_33_handle = None
        self.TCwindow_35_handle = None
        self.TCwindow_37_handle = None
        self.TCwindow_39_handle = None
        self.TCwindow_41_handle = None
        self.TCwindow_43_handle = None
        self.TCwindow_45_handle = None
        self.TCwindow_50_handle = None
        self.TCwindow_55_handle = None
        self.TCwindow_60_handle = None
        self.TCwindow_65_handle = None
        self.TCwindow_70_handle = None
        self.TCwindow_80_handle = None
        self.TCwindow_85_handle = None

    def on_begin_timestep_before_predictor(self, state) -> int:

        # api is ready to execute
        if self.api.exchange.api_data_fully_ready(state):

            # get variable handles if needed
            if self.need_to_get_handles:
                self.Win1_Tout_handle = self.api.exchange.get_variable_handle(
                    state, "Surface Outside Face Temperature", "Perimeter_ZN_1_wall_south_Window_1"
                )

                self.Win1_Construct_handle = self.api.exchange.get_actuator_handle(
                    state, "Surface", "Construction State", "Perimeter_ZN_1_wall_south_Window_1"
                )

                self.TCwindow_25_handle = self.api.exchange.get_construction_handle(state, "TCwindow_25")

                self.TCwindow_27_handle = self.api.exchange.get_construction_handle(state, "TCwindow_27")

                self.TCwindow_29_handle = self.api.exchange.get_construction_handle(state, "TCwindow_29")

                self.TCwindow_31_handle = self.api.exchange.get_construction_handle(state, "TCwindow_31")

                self.TCwindow_33_handle = self.api.exchange.get_construction_handle(state, "TCwindow_33")

                self.TCwindow_35_handle = self.api.exchange.get_construction_handle(state, "TCwindow_35")

                self.TCwindow_37_handle = self.api.exchange.get_construction_handle(state, "TCwindow_37")

                self.TCwindow_39_handle = self.api.exchange.get_construction_handle(state, "TCwindow_39")

                self.TCwindow_41_handle = self.api.exchange.get_construction_handle(state, "TCwindow_41")

                self.TCwindow_43_handle = self.api.exchange.get_construction_handle(state, "TCwindow_43")

                self.TCwindow_45_handle = self.api.exchange.get_construction_handle(state, "TCwindow_45")

                self.TCwindow_50_handle = self.api.exchange.get_construction_handle(state, "TCwindow_50")

                self.TCwindow_55_handle = self.api.exchange.get_construction_handle(state, "TCwindow_55")

                self.TCwindow_60_handle = self.api.exchange.get_construction_handle(state, "TCwindow_60")

                self.TCwindow_65_handle = self.api.exchange.get_construction_handle(state, "TCwindow_65")

                self.TCwindow_70_handle = self.api.exchange.get_construction_handle(state, "TCwindow_70")

                self.TCwindow_75_handle = self.api.exchange.get_construction_handle(state, "TCwindow_75")

                self.TCwindow_80_handle = self.api.exchange.get_construction_handle(state, "TCwindow_80")

                self.TCwindow_85_handle = self.api.exchange.get_construction_handle(state, "TCwindow_85")

                self.need_to_get_handles = False

            # calculate
            if self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 26.0:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_25_handle)
            elif self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 28.0:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_27_handle)
            elif self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 30.0:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_29_handle)
            elif self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 32.0:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_31_handle)
            elif self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 34.0:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_33_handle)
            elif self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 36.0:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_35_handle)
            elif self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 38.0:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_37_handle)
            elif self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 40.0:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_39_handle)
            elif self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 42.0:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_41_handle)
            elif self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 44.0:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_43_handle)
            elif self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 47.5:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_45_handle)
            elif self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 52.5:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_50_handle)
            elif self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 57.5:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_55_handle)
            elif self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 62.5:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_60_handle)
            elif self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 67.5:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_65_handle)
            elif self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 72.5:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_70_handle)
            elif self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 77.5:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_75_handle)
            elif self.api.exchange.get_variable_value(state, self.Win1_Tout_handle) <= 82.5:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_80_handle)
            else:
                self.api.exchange.set_actuator_value(state, self.Win1_Construct_handle, self.TCwindow_85_handle)

            return 0

        else:
            # api not ready, return
            return 0
