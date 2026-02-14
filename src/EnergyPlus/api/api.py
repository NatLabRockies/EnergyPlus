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

import os
import sys
from ctypes import c_char_p, c_void_p, cdll

from pyenergyplus.datatransfer import DataExchange
from pyenergyplus.func import Functional
from pyenergyplus.runtime import Runtime
from pyenergyplus.state import StateManager

# from pyenergyplus.autosizing import Autosizing


def api_path() -> str:
    """
    This function returns a string to the EnergyPlus dynamic library.  The energyplusapi target in the build system
    depends on the Python API build script, so you shouldn't be able to generate the EnergyPlus dynamic library without
    these scripts being successfully set up in the build tree by CMake.

    :return: A string absolute path to the EnergyPlus DLL.
    """
    this_script_dir = os.path.dirname(os.path.realpath(__file__))
    api_dll_dir = os.path.dirname(os.path.normpath(this_script_dir))
    if sys.platform.startswith("linux"):
        return os.path.join(api_dll_dir, "libenergyplusapi.so")
    elif sys.platform.startswith("darwin"):
        return os.path.join(api_dll_dir, "libenergyplusapi.dylib")
    else:  # assume Windows
        return os.path.join(api_dll_dir, "EnergyPlusAPI.dll")


class EnergyPlusAPI:
    """
    This class exposes the EnergyPlus C Library API to Python.  The API is split into three categories, and this class
    exposes each API category through member variables.  If an instance of this class is created as
    `api = EnergyPlusAPI()`, then the following members are available:

    - `api.functional`: The functional API provides access to static API calls, such as thermophysical property methods.
    - `api.runtime`: The runtime API allows a user to provide Python functions as callbacks, which are then
      called from within EnergyPlus at specific points in the simulation.
    - `api.exchange` The data_exchange API allows a user to exchange data (get sensor values, set actuator values)
      from within runtime callback methods, during a simulation.  When this class is instantiated for Python Plugin use,
      this also exposes the plugin global data members to allow sharing data between plugins.

    In a makefile-style build, the API library (dll) should be in Products; for example: `/path_to_build/Products`.
    For Visual Studio, the DLL is inside of a Debug or Release folder inside that Products directory.  At build time,
    the cmake/PythonSetupAPIinBuild.cmake script is executed (the energyplusapi target depends on it).  At build time, the
    Python API files are placed inside of the Products directory on Makefile builds, and copied into *both* the Release
    and Debug folders on Visual Studio builds.  The API scripts are put into a pyenergyplus directory, so in all cases,
    the dynamic library will simply be in the current script's parent directory. In an installation, the library will be
    in the installation root, and the Python API files will be in a pyenergyplus directory inside that install root as
    well, so the binary will again just be in this script's parent directory.

    For either case, utilizing the Python API wrappers is straightforward: if executing from directly from the
    build or install folder, scripts can be imported as `from pyenergyplus.foo import bar`.  If executing from a totally
    separate directory, the build or install directory can be inserted into the beginning of sys.path so that the
    pyenergyplus directory can be found.

    To reference this in an IDE to allow writing scripts using autocomplete, etc., most IDEs allow you to add third-
    party library directories.  The directory to add would be the build or install folder, as appropriate, so that
    the `from pyenergyplus` import statements can find a pyenergyplus package inside that third-party directory.
    """

    def __init__(self, running_as_python_plugin: bool = False):
        """
        Create a new API instance with child API classes set up as members on this class.

        :param running_as_python_plugin: If running as a python plugin, pass True, and this will do two things: 1)
                                         Instantiate the plugin "global" variable methods which are meaningless in
                                         other API calling structures, and 2) Avoid re-instantiating the functional API
                                         as this is already instantiated for Plugin workflows.
        """
        self.api = cdll.LoadLibrary(api_path())
        self.api.apiVersionFromEPlus.argtypes = [c_void_p]
        self.api.apiVersionFromEPlus.restype = c_char_p
        # self.state provides access to the main EnergyPlus state management class, instantiated and ready to go
        self.state_manager = StateManager(self.api)
        # self.functional provides access to a functional API class, instantiated and ready to go
        self.functional = Functional(self.api, running_as_python_plugin)
        # self.exchange provides access to a data exchange API class, instantiated and ready to go
        self.exchange = DataExchange(self.api, running_as_python_plugin)
        # self.runtime provides access to a runtime API class, instantiated and ready to go
        self.runtime = Runtime(self.api)
        # self.autosizing provides access to the autosizing API class, instantiated and ready to go
        # self.autosizing = Autosizing(self.api)

    @staticmethod
    def api_version() -> str:
        """
        Returns a string representation of the version of this API.  The EnergyPlus API will evolve over time,
        but in most cases, it will be simply adding new functionality and methods, not breaking existing API calls.
        The fractional portion of the API will be incremented when new functionality is added, and the whole number
        portion will be incremented when an existing API is broken.
        :return:
        """
        return "${PYTHON_API_VERSION_MAJOR}.${PYTHON_API_VERSION_MINOR}"

    def verify_api_version_match(self, state: c_void_p) -> None:
        api_version_from_ep = float(self.api.apiVersionFromEPlus(state))
        api_version_defined_here = float(self.api_version())
        if api_version_defined_here != api_version_from_ep:
            raise Exception(
                "API version does not match, this API version: %s; E+ is expecting version: %s"
                % (api_version_defined_here, api_version_from_ep)
            )
