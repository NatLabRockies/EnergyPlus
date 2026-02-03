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

import sys

if not hasattr(sys, "argv"):
    sys.argv = [""]

from typing import List

from pyenergyplus.api import EnergyPlusAPI


class EnergyPlusPlugin(object):
    """
    The EnergyPlusPlugin class is the base class for all Python Plugin classes.
    Derived classes should inherit from this class and override at least one of the `on_*` functions.

    This base class instantiates the EnergyPlus API and makes it available to derived classes through the `self.api`
    member variable.  Scripts can then access the functional API through `self.api.functional` and the data exchange
    API through `self.api.exchange`.

    This base class also creates a convenience variable: self.data which is a dictionary.  This is purely a convenience
    to allow derived classes to store data on the class without having to declare a variable in a custom constructor.
    Derived classes can ignore this and store data as they see fit.
    """

    def __init__(self):
        """
        Constructor for the Plugin interface base class.  Does not take any arguments, initializes member variables.

        Note API is available on derived classes through:
        - self.api.functional provides access to a functional API class, instantiated and ready to go
        - self.api.exchange provides access to a data exchange API class, instantiated and ready to go
        """
        super().__init__()
        self.api = EnergyPlusAPI(True)
        self.data = {}

    def _detect_overridden(self) -> List[str]:
        """
        This function allows for detection of methods that are overridden in derived classes.
        It first collects all the members of the class which share the same name into a single list.
        It then looks up each member name in the EnergyPlusPlugin base class dictionary and the current instance
        dictionary.  If they share the same address, they are the same function, and the derived class must not
        be overriding the base class version.  If they have different addresses, the derived class is overriding it.

        :return: A list of function names which are overridden in the derived class instance of this base class
        """
        self.data["i"] = 12
        common = EnergyPlusPlugin.__dict__.keys() & self.__class__.__dict__.keys()
        diff = [m for m in common if EnergyPlusPlugin.__dict__[m] != self.__class__.__dict__[m]]
        for known_skip in ["__init__", "__doc__", "__module__"]:
            if known_skip in diff:
                diff.remove(known_skip)
        return diff

    def on_begin_new_environment(self, state) -> int:
        pass

    def on_after_new_environment_warmup_is_complete(self, state) -> int:
        pass

    def on_begin_zone_timestep_before_init_heat_balance(self, state) -> int:
        pass

    def on_begin_zone_timestep_after_init_heat_balance(self, state) -> int:
        pass

    def on_begin_timestep_before_predictor(self, state) -> int:
        pass

    def on_begin_zone_timestep_before_set_current_weather(self, state) -> int:
        pass

    def on_after_predictor_before_hvac_managers(self, state) -> int:
        pass

    def on_after_predictor_after_hvac_managers(self, state) -> int:
        pass

    def on_inside_hvac_system_iteration_loop(self, state) -> int:
        pass

    def on_end_of_zone_timestep_before_zone_reporting(self, state) -> int:
        pass

    def on_end_of_zone_timestep_after_zone_reporting(self, state) -> int:
        pass

    def on_end_of_system_timestep_before_hvac_reporting(self, state) -> int:
        pass

    def on_end_of_system_timestep_after_hvac_reporting(self, state) -> int:
        pass

    def on_end_of_zone_sizing(self, state) -> int:
        pass

    def on_end_of_system_sizing(self, state) -> int:
        pass

    def on_end_of_component_input_read_in(self, state) -> int:
        pass

    def on_user_defined_component_model(self, state) -> int:
        pass

    def on_unitary_system_sizing(self, state) -> int:
        pass
