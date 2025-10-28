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

"""
Cross-platform setup.py for building the EnergyPlus Python module.
Bare-bones library, no pre- or post-processing tools.
"""

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from shutil import rmtree, copy
from platform import machine, system
from os import cpu_count
from pathlib import Path
from subprocess import check_call, CalledProcessError
from re import findall

from wheel.bdist_wheel import bdist_wheel


def get_ep_version_string(repository_root_dir: Path) -> str:
    version_info_file = repository_root_dir / 'cmake' / 'Version.cmake'
    version_contents = version_info_file.read_text()
    version_major = findall(r'CMAKE_VERSION_MAJOR (\d+)', version_contents)[0]
    version_minor = findall(r'CMAKE_VERSION_MINOR (\d+)', version_contents)[0]
    version_patch = findall(r'CMAKE_VERSION_PATCH (\d+)', version_contents)[0]
    build_increment = 1
    return f"{version_major}.{version_minor}.{version_patch}.{build_increment}"  # TODO: Determine good numbering


def get_current_wheel_details():
    wheels = {
        "Darwin": {
            "x86_64": {
                "wheel": "macosx_10_13_x86_64",  # TODO: Dynamically determine Mac version numbers?
                "zip_tag": "OSX",
                "build_tool": "Unix Makefiles",
                "extension": "dylib",
            },
            "arm64": {
                "wheel": "macosx_11_0_arm64",
                "zip_tag": "OSX_arm64",
                "build_tool": "Unix Makefiles",
                "extension": "dylib",
            },
        },
        "Linux": {
            "x86_64": {
                "wheel": "manylinux_2_17_x86_64",
                "zip_tag": "Linux",
                "build_tool": "Unix Makefiles",
                "extension": "so",
            }
        },
        "Windows": {
            "i386": {  # I would love to not build 32-bit, but I know there is a good amount of 32 bit Python out there
                "wheel": "win32",
                "zip_tag": "Windows",
                "arch": "Win32",
                "build_tool": "Visual Studio 17 2022",
                "extension": "dll",
            },
            "AMD64": {
                "wheel": "win_amd64",
                "zip_tag": "Windows",
                "arch": "x64",
                "build_tool": "Visual Studio 17 2022",
                "extension": "dll",
            },
        },
    }
    return wheels[system()][machine()]


class PyenergyplusBDistWheel(bdist_wheel):
    def get_tag(self):
        return "py3", "none", get_current_wheel_details()["wheel"]


class EnergyPlusBuild(build_ext):
    @staticmethod
    def cmake_configure_command() -> list[str]:
        current_config = get_current_wheel_details()
        cmake_cmd = ["cmake", "-G", current_config["build_tool"]]
        if "arch" in current_config:
            cmake_cmd += ["-A", current_config['arch']]
        cmake_cmd.append("-DBUILD_FORTRAN=OFF")
        if system() != "Windows":
            cmake_cmd.append("-DCMAKE_BUILD_TYPE=Release")
        cmake_cmd.append(str(repo_root_directory))
        return cmake_cmd

    @staticmethod
    def cmake_build_command() -> list[str]:
        cmake_build_cmd = ["cmake", "--build", "."]
        if system() == "Windows":  # VS builds require specifying the build config
            cmake_build_cmd.extend(["--config", "Release"])
        else:  # MSBuild doesn't like the -j passed in, so only do this on Non-Windows
            cmake_build_cmd.extend(['--', '-j', f"{cpu_count() - 1}"])
        return cmake_build_cmd

    @staticmethod
    def fixup_copied_python_file(python_file: Path):
        t = python_file.read_text()
        t = t.replace('from pyenergyplus.', 'from energyplus.')
        t = t.replace(
            'api_dll_dir = os.path.dirname(os.path.normpath(this_script_dir))',
            'api_dll_dir = os.path.normpath(this_script_dir)'
        )
        python_file.write_text(t)

    def run(self):
        self.build_lib = 'build/energyplus'  # I feel like this variable has meaning on this class, so leaving it

        try:
            cmake_cmd = self.cmake_configure_command()
            check_call(cmake_cmd, cwd=build_root_directory)
        except CalledProcessError as cpe:
            raise Exception(
                f"CMake failed to configure EnergyPlus, check error logs, raw error message: {cpe}"
            ) from None

        try:
            cmake_build_cmd = self.cmake_build_command()
            check_call(cmake_build_cmd, cwd=build_root_directory)
        except CalledProcessError as cpe:
            raise Exception(
                f"CMake failed to build EnergyPlus, check error logs, raw error message: {cpe}"
            ) from None

        # while EnergyPlus is built in the repo/build-wheel folder, set up the path to the actual wheel build
        # this will be in repo/build-wheel/build/energyplus to avoid conflicting with dev's normal repo/build folders
        # we will wipe this folder each build to get a clean wheel
        wheel_build_directory = Path(build_root_directory / self.build_lib)
        if wheel_build_directory.exists():
            rmtree(wheel_build_directory)
            wheel_build_directory.unlink(missing_ok=True)
        wheel_build_directory.mkdir(parents=True)

        # Copy the shared library files and Python API files to the output directory
        products_dir = build_root_directory / 'Products'
        if system() == "Windows":
            products_dir /= 'Release'
        built_shared_libraries = products_dir.glob(f"*.{get_current_wheel_details()['extension']}*")
        for lib in built_shared_libraries:
            copy(lib, wheel_build_directory)

        # Copy the Python source code files to the output directory
        energyplus_py_dir = products_dir / "pyenergyplus"
        # TODO: Include some weather files and example files?  But where?  Inside the wheel, API functions to access?
        # TODO: Consider adding install_requires to get the extra EnergyPlus stuff...and fixing it up after install!
        # TODO: Try uploading to test-pypi and see what happens!
        for lib in energyplus_py_dir.glob("*.py"):
            copy(lib, wheel_build_directory)
            # do quick fix-ups on the copied file to work in the pip installed configuration
            self.fixup_copied_python_file(wheel_build_directory / lib.name)


# find the repository root early, and set up a new build-directory called build-wheel where all build ops will occur
repo_root_directory = Path(__file__).resolve().parent
build_root_directory = repo_root_directory / 'build-wheel'
build_root_directory.mkdir(exist_ok=True)

setup(
    name="energyplus",
    version=get_ep_version_string(repo_root_directory),
    packages=[],
    license="Modified BSD",
    author="United States Department of Energy",
    author_email="",
    url="https://github.com/NREL/EnergyPlus",
    description="EnergyPlus is a building simulation program for modeling energy and water use in buildings.",
    long_description=(repo_root_directory / "README.md").read_text(),
    long_description_content_type='text/markdown',
    ext_modules=[Extension("energyplus", sources=[])],
    cmdclass={
        "build_ext": EnergyPlusBuild,
        "bdist_wheel": PyenergyplusBDistWheel,
    },
    options={
        'bdist_wheel': {'bdist_dir': str(build_root_directory / 'build')}
    },
    build_base='.'
)
