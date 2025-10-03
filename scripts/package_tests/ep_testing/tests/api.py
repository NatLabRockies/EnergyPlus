# EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University
# of Illinois, The Regents of the University of California, through Lawrence
# Berkeley National Laboratory (subject to receipt of any required approvals
# from the U.S. Dept. of Energy), Oak Ridge National Laboratory, managed by UT-
# Battelle, Alliance for Sustainable Energy, LLC, and other contributors. All
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


import os
import platform
import subprocess
import sys
from tempfile import mkdtemp, mkstemp
from typing import List, Optional

from ep_testing.constants import MSVC, OS, Bitness
from ep_testing.tests.base import BaseTest


def api_resource_dir() -> str:
    this_file_path = os.path.realpath(__file__)
    this_directory = os.path.dirname(this_file_path)
    templates_dir = os.path.join(this_directory, "api_templates")
    return templates_dir


def my_check_call(verbose: bool, command_line: List[str], **kwargs) -> None:

    r = subprocess.run(command_line, stdout=subprocess.PIPE, stderr=subprocess.PIPE, **kwargs)
    if r.returncode != 0:
        raise Exception(
            f"Command {command_line} failed with exit status {r.returncode}!\n"
            "stderr:\n"
            f"{r.stderr.decode().strip()}"
            "\n\n"
            "stdout:\n"
            f"{r.stdout.decode().strip()}"
        )
    elif verbose:
        if len(r.stderr) > 0:
            print(r.stderr.decode().strip())


class TestPythonAPIAccess(BaseTest):

    def __init__(self):
        super().__init__()
        self.os = None

    def name(self):
        return "Test running an API script against pyenergyplus"

    @staticmethod
    def _api_script_content(install_root: str) -> str:
        if platform.system() in ["Linux", "Darwin"]:
            pass
        else:  # windows
            install_root = install_root.replace("\\", "\\\\")
        template_file = os.path.join(api_resource_dir(), "python_link.py")
        template = open(template_file).read()
        return template % install_root

    def run(self, install_root: str, verbose: bool, kwargs: dict):
        self.verbose = verbose
        print('* Running test class "%s"... ' % self.__class__.__name__, end="")
        if "os" not in kwargs:
            raise Exception("Bad call to %s -- must pass os in kwargs" % self.__class__.__name__)
        self.os = kwargs["os"]
        handle, python_file_path = mkstemp(suffix=".py")
        with os.fdopen(handle, "w") as f:
            f.write(self._api_script_content(install_root))
        print(" [FILE WRITTEN] ", end="")
        try:
            # TODO: why do we have to pass the full path to python on darwin/Windows but not linux?
            # Anyhoo, let's just use sys.executable everywhere
            # if platform.system() == 'Linux':
            #     py = 'python3'
            # elif platform.system() == 'Darwin':
            #     py = '/usr/local/bin/python3'
            # else:  # windows
            #    py = 'C:\\Python36\\Python.exe'
            py = sys.executable
            my_env = os.environ.copy()
            if platform.system() == "Windows":  # my local comp didn't have cmake in path except in interact shells
                my_env["PATH"] = install_root + ";" + my_env["PATH"]
            if platform.system() == "Darwin":
                # while it runs OK locally, for some reason on GHA, running a Plugin file from the Python API seg-faults
                idf_to_run = os.path.join(install_root, "ExampleFiles", "1ZoneUncontrolled.idf")
            else:
                idf_to_run = os.path.join(
                    install_root, "ExampleFiles", "1ZoneUncontrolled.idf"
                )  # PythonPluginCustomOutputVariable
            my_check_call(self.verbose, [py, python_file_path, "-D", idf_to_run], env=my_env)
            print(" [DONE]!")
        except Exception as e:
            print("Python API Wrapper Script failed!")
            raise e


def make_build_dir_and_build(cmake_build_dir: str, verbose: bool, bitness: Bitness, msvc_version: MSVC):

    os.makedirs(cmake_build_dir)
    my_env = os.environ.copy()
    if platform.system() == "Darwin":  # my local comp didn't have cmake in path except in interact shells
        my_env["PATH"] = "/usr/local/bin:" + my_env["PATH"]
    command_line = ["cmake", ".."]
    is_windows = platform.system() == "Windows"
    if is_windows:
        command_line.extend(["-G", msvc_version.generator_name(), "-A", bitness.value])

    try:
        my_check_call(verbose, command_line, cwd=cmake_build_dir, env=my_env)
    except Exception as e:
        print("C API Wrapper Configuration Failed!")
        raise e

    command_line = ["cmake", "--build", "."]
    if platform.system() == "Windows":
        command_line.extend(["--config", "Release"])
    try:
        my_check_call(verbose, command_line, env=my_env, cwd=cmake_build_dir)
    except Exception as e:
        print("C API Wrapper Compilation Failed!")
        raise e
    print(" [COMPILED] ", end="")


class TestCAPIAccess(BaseTest):

    def __init__(self, os: OS, bitness: Bitness, msvc_version: Optional[MSVC] = None):
        super().__init__()
        self.os = os
        self.bitness = bitness
        self.msvc_version = msvc_version
        self.source_file_name = "func.c"
        self.target_name = "TestCAPIAccess"
        if self.os == OS.Windows and self.msvc_version is None:
            raise Exception("Must pass msvc_version if os is Windows")

    def name(self):
        return "Test running an API script against energyplus in C"

    @staticmethod
    def _api_fixup_content() -> str:
        template_file = os.path.join(api_resource_dir(), "eager_cpp_fixup.txt")
        template = open(template_file).read()
        return template

    def _api_cmakelists_content(self, install_path: str) -> str:
        if platform.system() == "Linux":
            lib_file_name = "libenergyplusapi.so"
        elif platform.system() == "Darwin":
            lib_file_name = "libenergyplusapi.dylib"
        else:  # windows
            lib_file_name = "energyplusapi.lib"
            install_path = install_path.replace("\\", "\\\\")
        template_file = os.path.join(api_resource_dir(), "eager_cpp_cmakelists.txt")
        template = open(template_file).read()
        return template.format(
            EPLUS_INSTALL_NO_SLASH=install_path,
            LIB_FILE_NAME=lib_file_name,
            TARGET_NAME=self.target_name,
            SOURCE_FILE=self.source_file_name,
        )

    @staticmethod
    def _api_script_content() -> str:
        template_file = os.path.join(api_resource_dir(), "eager_cpp_source.cpp")
        template = open(template_file).read()
        return template

    def run(self, install_root: str, verbose: bool):
        self.verbose = verbose
        print('* Running test class "%s"... ' % self.__class__.__name__, end="")
        build_dir = mkdtemp()
        c_file_name = self.source_file_name
        c_file_path = os.path.join(build_dir, c_file_name)
        with open(c_file_path, "w") as f:
            f.write(self._api_script_content())
        print(" [SRC FILE WRITTEN] ", end="")
        cmake_lists_path = os.path.join(build_dir, "CMakeLists.txt")
        with open(cmake_lists_path, "w") as f:
            f.write(self._api_cmakelists_content(install_root))
        print(" [CMAKE FILE WRITTEN] ", end="")
        fixup_cmake_path = os.path.join(build_dir, "fixup.cmake")
        with open(fixup_cmake_path, "w") as f:
            f.write(self._api_fixup_content())
        print(" [FIXUP CMAKE WRITTEN] ", end="")
        cmake_build_dir = os.path.join(build_dir, "build")
        make_build_dir_and_build(
            cmake_build_dir=cmake_build_dir, verbose=self.verbose, bitness=self.bitness, msvc_version=self.msvc_version
        )
        try:
            new_binary_path = os.path.join(cmake_build_dir, self.target_name)
            if platform.system() == "Windows":  # override the path/name for Windows
                new_binary_path = os.path.join(cmake_build_dir, "Release", self.target_name + ".exe")
            command_line = [new_binary_path]
            my_check_call(self.verbose, command_line, cwd=install_root)
        except Exception as e:
            print("C API Wrapper Execution failed!")
            raise e
        print(" [DONE]!")


class TestCppAPIDelayedAccess(BaseTest):

    def __init__(self, os: OS, bitness: Bitness, msvc_version: Optional[MSVC] = None):
        super().__init__()
        self.os = os
        self.bitness = bitness
        self.msvc_version = msvc_version
        self.source_file_name = "func.cpp"
        self.target_name = "TestCAPIAccess"
        if self.os == OS.Windows and self.msvc_version is None:
            raise Exception("Must pass msvc_version if os is Windows")

    def name(self):
        return "Test running an API script against energyplus in C++ but with delayed DLL loading"

    def _api_cmakelists_content(self) -> str:
        template_file = os.path.join(api_resource_dir(), "delayed_cpp_cmakelists.txt")
        template = open(template_file).read()
        return template.format(TARGET_NAME=self.target_name, SOURCE_FILE=self.source_file_name)

    @staticmethod
    def _api_script_content(install_path: str) -> str:
        if platform.system() == "Linux":
            lib_file_name = "/libenergyplusapi.so"
        elif platform.system() == "Darwin":
            lib_file_name = "/libenergyplusapi.dylib"
        else:  # windows
            raise Exception("Dont call TestCAPIDelayedAccess._api_script_content for Windows")
        template_file = os.path.join(api_resource_dir(), "delayed_cpp_source_linux_mac.cpp")
        template = open(template_file).read()
        return template.replace("{EPLUS_INSTALL_NO_SLASH}", install_path).replace("{LIB_FILE_NAME}", lib_file_name)

    @staticmethod
    def _api_script_content_windows(install_path: str) -> str:
        lib_file_name = "\\\\energyplusapi.dll"
        install_path = install_path.replace("\\", "\\\\")
        template_file = os.path.join(api_resource_dir(), "delayed_cpp_source_windows.cpp")
        template = open(template_file).read()
        return template.replace("{EPLUS_INSTALL_NO_SLASH}", install_path).replace("{LIB_FILE_NAME}", lib_file_name)

    def run(self, install_root: str, verbose: bool):
        self.verbose = verbose
        print('* Running test class "%s"... ' % self.__class__.__name__, end="")
        build_dir = mkdtemp()
        c_file_name = "func.cpp"
        c_file_path = os.path.join(build_dir, c_file_name)
        with open(c_file_path, "w") as f:
            if platform.system() == "Linux" or platform.system() == "Darwin":
                f.write(self._api_script_content(install_root))
            else:
                f.write(self._api_script_content_windows(install_root))
        print(" [SRC FILE WRITTEN] ", end="")
        cmake_lists_path = os.path.join(build_dir, "CMakeLists.txt")
        with open(cmake_lists_path, "w") as f:
            f.write(self._api_cmakelists_content())
        print(" [CMAKE FILE WRITTEN] ", end="")
        cmake_build_dir = os.path.join(build_dir, "build")
        make_build_dir_and_build(
            cmake_build_dir=cmake_build_dir, verbose=self.verbose, bitness=self.bitness, msvc_version=self.msvc_version
        )
        if platform.system() == "Windows":
            built_binary_path = os.path.join(cmake_build_dir, "Release", "TestCAPIAccess")
        else:
            built_binary_path = os.path.join(cmake_build_dir, "TestCAPIAccess")
        my_env = os.environ.copy()
        if platform.system() == "Windows":  # my local comp didn't have cmake in path except in interact shells
            my_env["PATH"] = install_root + ";" + my_env["PATH"]
        try:
            my_check_call(self.verbose, [built_binary_path], env=my_env)
        except Exception as e:
            print("Delayed C API Wrapper execution failed")
            raise e
        print(" [DONE]!")
