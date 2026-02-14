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


class AverageZoneTemps(EnergyPlusPlugin):

    def __init__(self):
        super().__init__()
        self.do_setup = True

    def on_end_of_zone_timestep_before_zone_reporting(self, state) -> int:
        if self.do_setup:
            self.data["zone_volumes"] = []
            self.data["zone_temps"] = []
            zone_names = ["perimeter_zn_" + str(i) for i in range(1, 5)] + ["core_zn"]
            for zone_name in zone_names:
                handle = self.api.exchange.get_internal_variable_handle(state, "Zone Air Volume", zone_name)
                zone_volume = self.api.exchange.get_internal_variable_value(state, handle)
                self.data["zone_volumes"].append(zone_volume)
                self.data["zone_temps"].append(
                    self.api.exchange.get_variable_handle(state, "Zone Mean Air Temperature", zone_name)
                )
            self.data["avg_temp_variable"] = self.api.exchange.get_global_handle(state, "AverageBuildingTemp")
            #  x = self.api.exchange.get_api_data(state)  # use this to inspect API data
            self.do_setup = False
        zone_temps = list()
        for t_handle in self.data["zone_temps"]:
            zone_temps.append(self.api.exchange.get_variable_value(state, t_handle))
        numerator = 0.0
        denominator = 0.0
        for i in range(5):
            numerator += self.data["zone_volumes"][i] * zone_temps[i]
            denominator += self.data["zone_volumes"][i]
        average_temp = numerator / denominator
        self.api.exchange.set_global_value(state, self.data["avg_temp_variable"], average_temp)
        return 0
