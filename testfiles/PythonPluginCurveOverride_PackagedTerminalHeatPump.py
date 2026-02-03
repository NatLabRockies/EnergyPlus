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


class CurveOverwriteMGR(EnergyPlusPlugin):

    def __init__(self):
        super().__init__()
        self.need_to_get_handles = True
        self.CoilInletDBT_handle = None
        self.CoilInletW_handle = None
        self.Pressure_handle = None
        self.OAT_handle = None
        self.CurveOverwrite_handle = None
        self.psych = None

    def on_end_of_zone_timestep_before_zone_reporting(self, state) -> int:
        if not self.psych:
            self.psych = self.api.functional.psychrometrics(state)
        # API is ready to execute
        if self.api.exchange.api_data_fully_ready(state):
            # get handles if needed
            if self.need_to_get_handles:
                self.CoilInletDBT_handle = self.api.exchange.get_variable_handle(
                    state, "System Node Temperature", "Zone1PTHPFanOutletNode"
                )

                self.CoilInletW_handle = self.api.exchange.get_variable_handle(
                    state, "System Node Humidity Ratio", "Zone1PTHPFanOutletNode"
                )

                self.Pressure_handle = self.api.exchange.get_variable_handle(
                    state, "System Node Pressure", "Zone1PTHPOAInNode"
                )

                self.CurveOverwrite_handle = self.api.exchange.get_actuator_handle(
                    state, "Curve", "Curve Result", "HPACCOOLCAPFT"
                )

                self.OAT_handle = self.api.exchange.get_variable_handle(
                    state, "System Node Temperature", "Zone1PTHPOAInNode"
                )

                self.need_to_get_handles = False

            # calculations
            TTmp = self.api.exchange.get_variable_value(state, self.CoilInletDBT_handle)
            WTmp = self.api.exchange.get_variable_value(state, self.CoilInletW_handle)
            PTmp = self.api.exchange.get_variable_value(state, self.Pressure_handle)
            MyWB = self.psych.wet_bulb(state, TTmp, WTmp, PTmp)
            IVOnea = MyWB
            OAT = self.api.exchange.get_variable_value(state, self.OAT_handle)
            IVTwo = OAT
            IVThree = IVOnea * IVTwo
            C1 = 0.942567793
            C2 = 0.009543347
            C2a = 0.009543347
            C3 = 0.00068377e0
            C4 = 0.011042676
            C5 = 0.000005249
            C6 = 0.000009720
            CurveInput = (
                C1 + (C2 * IVOnea) + (C3 * IVOnea * IVOnea) - (C4 * IVTwo) + (C5 * IVTwo * IVTwo) - (C6 * IVThree)
            )
            if OAT > 31.0:
                CurveInput = (
                    C1 - (C2a * IVOnea) + (C3 * IVOnea * IVOnea) - (C4 * IVTwo) + (C5 * IVTwo * IVTwo) - (C6 * IVThree)
                )

            CurveOverwrite = CurveInput
            self.api.exchange.set_actuator_value(state, self.CurveOverwrite_handle, CurveOverwrite)

            return 0
        else:
            # API not ready, return
            return 0
