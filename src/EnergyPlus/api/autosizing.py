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

from ctypes import c_char_p, c_int, c_void_p, cdll

from pyenergyplus.common import RealEP


class BaseSizerWorker:
    """
    This class provides utility functions that are common to all sizer types, each sizer just needs one of these.
    """

    def __init__(self, api: cdll):
        api.sizerGetLastErrorMessages.argtypes = [c_void_p]
        api.sizerGetLastErrorMessages.restype = c_char_p

    @staticmethod
    def get_error_messages(api: cdll, instance: c_void_p) -> bytes:
        """
        Lists out any error messages during sizing for this sizer, and clears the error buffer.

        :return: Returns a raw byte string of error messages.
        """
        return api.getLastErrorMessages(instance)


class HeatingAirflowUASizer:
    """
    This sizer class wraps the internal HeatingAirflowUASizer class
    """

    ZoneConfigTerminal = 0
    ZoneConfigInductionUnit = 1
    ZoneConfigFanCoil = 2

    SysConfigOutdoorAir = 0
    SysConfigMainDuct = 1
    SysConfigCoolingDuct = 2
    SysConfigHeatingDuct = 3
    SysConfigOtherDuct = 4

    def __init__(self, api: cdll):
        self.api = api
        self.api.sizerHeatingAirflowUANew.argtypes = []
        self.api.sizerHeatingAirflowUANew.restype = c_void_p
        self.api.sizerHeatingAirflowUAInitializeForZone.argtypes = [c_void_p, c_void_p, c_int, RealEP, RealEP, RealEP]
        self.api.sizerHeatingAirflowUAInitializeForZone.restype = c_void_p
        self.api.sizerHeatingAirflowUAInitializeForSystem.argtypes = [
            c_void_p,
            c_void_p,
            c_int,
            RealEP,
            RealEP,
            RealEP,
            c_int,
        ]
        self.api.sizerHeatingAirflowUAInitializeForSystem.restype = c_void_p
        self.api.sizerHeatingAirflowUADelete.argtypes = [c_void_p]
        self.api.sizerHeatingAirflowUADelete.restype = c_void_p
        self.api.sizerHeatingAirflowUASize.argtypes = [c_void_p, c_void_p]
        self.api.sizerHeatingAirflowUASize.restype = int
        self.api.sizerHeatingAirflowUAValue.argtypes = [c_void_p]
        self.api.sizerHeatingAirflowUAValue.restype = RealEP
        self.base_worker = BaseSizerWorker(self.api)
        self.instance = self.api.sizerHeatingAirflowUANew()

    def __del__(self):
        self.api.sizerHeatingAirflowUADelete(self.instance)

    def get_last_error_messages(self):
        return self.base_worker.get_error_messages(self.api, self.instance)

    def initialize_for_zone(
        self,
        state: c_void_p,
        zone_config: int,
        elevation: float,
        representative_flow_rate: float,
        reheat_multiplier: float = 1.0,
    ) -> None:
        self.api.sizerHeatingAirflowUAInitializeForZone(
            state, self.instance, zone_config, elevation, representative_flow_rate, reheat_multiplier
        )

    def initialize_for_system_outdoor_air(
        self,
        state: c_void_p,
        sys_config: int,
        elevation: float,
        representative_flow_rate: float,
        min_flow_rate_ratio: float,
        doas: bool,
    ) -> None:
        self.api.sizerHeatingAirflowUAInitializeForSystem(
            state, self.instance, sys_config, elevation, representative_flow_rate, min_flow_rate_ratio, 1 if doas else 0
        )

    def size(self, state: c_void_p) -> bool:
        """
        Performs autosizing calculations with the given initialized values

        :return: True if the sizing was successful, or False if not
        """
        return True if self.api.sizerHeatingAirflowUASize(state, self.instance) == 0 else False

    def autosized_value(self) -> float:
        """
        Returns the autosized value, assuming the calculation was successful

        :return: The autosized value
        """
        return self.api.sizerHeatingAirflowUAValue(self.instance)


class Autosizing:
    """
    A wrapper class for all the autosizing classes, acting as a factory
    """

    def __init__(self, api: cdll):
        self.api = api

    def heating_airflow_ua_sizer(self) -> HeatingAirflowUASizer:
        return HeatingAirflowUASizer(self.api)
