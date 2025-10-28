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


import argparse
import os
import platform
from enum import Enum
from pathlib import Path
from re import search
from shutil import rmtree
from subprocess import STDOUT, CalledProcessError, check_call
from sys import exit, path
from typing import Optional

this_file_path = Path(__file__).resolve()
package_test_root_dir = this_file_path.parent
path.insert(0, str(package_test_root_dir))

from ep_testing.constants import MSVC, MSVC_ALIAS_MAPPING, OS, Bitness
from ep_testing.tests.api import TestCAPIAccess, TestCppAPIDelayedAccess, TestPythonAPIAccess
from ep_testing.tests.energyplus import TestPlainDDRunEPlusFile
from ep_testing.tests.expand_objects import TestExpandObjectsAndRun
from ep_testing.tests.hvacdiagram import HVACDiagram
from ep_testing.tests.transition import TransitionOldFile

# fmt: off
CONFIGURATIONS = {
    'ubuntu2004': {
        'os': OS.Linux, 'bitness': Bitness.X64, 'asset_pattern': 'Linux-Ubuntu20.04-x86_64.tar.gz', 'os_version': '20.04'
    },
    'ubuntu2204': {
        'os': OS.Linux, 'bitness': Bitness.X64, 'asset_pattern': 'Linux-Ubuntu22.04-x86_64.tar.gz', 'os_version': '22.04'
    },
    'ubuntu2404': {
        'os': OS.Linux, 'bitness': Bitness.X64, 'asset_pattern': 'Linux-Ubuntu24.04-x86_64.tar.gz', 'os_version': '24.04'
    },
    'ubuntu2204-arm64': {
        'os': OS.Linux, 'bitness': Bitness.ARM64, 'asset_pattern': 'Linux-Ubuntu22.04-arm64.tar.gz', 'os_version': '22.04'
    },
    'ubuntu2404-arm64': {
        'os': OS.Linux, 'bitness': Bitness.ARM64, 'asset_pattern': 'Linux-Ubuntu24.04-arm64.tar.gz', 'os_version': '24.04'
    },
    'mac11': {
        'os': OS.Mac, 'bitness': Bitness.X64, 'asset_pattern': 'Darwin-macOS11.6-x86_64.tar.gz', 'os_version': '11.6'
    },
    'mac12': {
        'os': OS.Mac, 'bitness': Bitness.X64, 'asset_pattern': 'Darwin-macOS12.1-x86_64.tar.gz', 'os_version': '12.1'
    },
    'mac13-arm64': {
        'os': OS.Mac, 'bitness': Bitness.ARM64, 'asset_pattern': 'Darwin-macOS13-arm64.tar.gz', 'os_version': '13'
    },
    'win32': {
        'os': OS.Windows, 'bitness': Bitness.X86, 'asset_pattern': 'Windows-i386.zip', 'os_version': '2022'
    },
    'win64': {
        'os': OS.Windows, 'bitness': Bitness.X64, 'asset_pattern': 'Windows-x86_64.zip', 'os_version': '2022'
    },
}
# fmt: on


class TestRunner:

    def __init__(
        self,
        run_config_key: str,
        this_version: str,
        last_version: str,
        last_tag: str,
        msvc_version: Optional[MSVC] = None,
    ):

        # invalid keys are protected in the command's finalize_options method
        this_config = CONFIGURATIONS[run_config_key]
        self.os_version = this_config["os_version"]
        self.os = this_config["os"]
        self.msvc_version = None
        if self.os == OS.Windows:
            self.msvc_version = msvc_version
            if not isinstance(self.msvc_version, MSVC):
                raise ValueError("On Windows, an MSVC (Enum) version must be provided.")

        self.asset_pattern = this_config["asset_pattern"]
        self.bitness = this_config["bitness"]

        self.this_version = this_version
        # self.tag_this_version = 'v24.1.0-RC2'
        self.last_version = last_version
        self.tag_last_version = last_tag

    def find_and_extract_package(self, artifact_folder: Path) -> str:
        extract_path = Path.cwd() / "ep_package"
        saved_working_directory = Path.cwd()
        os.chdir(artifact_folder)
        package_file_name = str(list(artifact_folder.rglob("*"))[0])
        if self.os == OS.Linux:
            # tar -xzf ep.tar.gz -C ep_package
            extract_command = ["tar", "-xzf", package_file_name, "-C", str(extract_path)]
        elif self.os == OS.Mac:
            # tar -xzf ep.tar.gz -C ep_package
            extract_command = ["tar", "-xzf", package_file_name, "-C", str(extract_path)]
        else:  # if config.os == OS.Windows:
            # 7z x ep.zip -oep_package
            extract_command = ["7z.exe", "x", package_file_name, "-o" + str(extract_path)]
        if extract_path.exists():
            rmtree(extract_path)
        try:
            extract_path.mkdir(parents=True)
        except Exception as e:
            raise Exception("Could not create extraction path at %s; error: %s" % (extract_path, str(e)))
        try:
            print("Extracting asset...")
            dev_null = open(os.devnull, "w")
            check_call(extract_command, stdout=dev_null, stderr=STDOUT)
            print(" ...Extraction Complete")
        except CalledProcessError as e:
            raise Exception(f"Extraction failed with this error: {e}\nCommand: {extract_command}")
        # should result in a single new directory inside the extract path, like: /extract/path/EnergyPlus-V1-abc-Linux
        all_sub_folders = [f.path for f in os.scandir(extract_path) if f.is_dir()]
        if len(all_sub_folders) > 1:
            raise Exception("Extracted EnergyPlus package has more than one directory, problem.")
        os.chdir(saved_working_directory)
        return all_sub_folders[0]

    def run_all_tests(self, install_path_path: Path, verbose: bool) -> int:
        saved_path = os.getcwd()
        install_path = str(install_path_path)
        TestPlainDDRunEPlusFile().run(install_path, verbose, {"test_file": "1ZoneUncontrolled.idf"})
        TestPlainDDRunEPlusFile().run(install_path, verbose, {"test_file": "PythonPluginCustomOutputVariable.idf"})
        TestExpandObjectsAndRun().run(install_path, verbose, {"test_file": "HVACTemplate-5ZoneFanCoil.idf"})
        TransitionOldFile().run(install_path, verbose, {"last_version": self.tag_last_version})
        HVACDiagram().run(install_path, verbose, {})
        if self.os == OS.Windows:
            print("Windows Symlink runs are not testable on Travis, I think the user needs symlink privilege.")
        else:
            TestPlainDDRunEPlusFile().run(
                install_path, verbose, {"test_file": "1ZoneUncontrolled.idf", "binary_sym_link": True}
            )
        TestCAPIAccess(os=self.os, bitness=self.bitness, msvc_version=self.msvc_version).run(
            install_path,
            verbose,
        )
        TestCppAPIDelayedAccess(os=self.os, bitness=self.bitness, msvc_version=self.msvc_version).run(
            install_path,
            verbose,
        )
        if self.bitness == Bitness.X86:
            print("Travis does not have a 32-bit Python package readily available, so not testing Python API")
        else:
            TestPythonAPIAccess().run(install_path, verbose, {"os": self.os})
        os.chdir(saved_path)
        return 0  # do better?


def get_version_info() -> tuple:
    repo_root = package_test_root_dir.parent.parent
    version_file = repo_root / "cmake" / "Version.cmake"
    contents = version_file.read_text()
    major = search(r"CMAKE_VERSION_MAJOR [0-9]+", contents).group(0).split(" ")[1]
    minor = search(r"CMAKE_VERSION_MINOR [0-9]+", contents).group(0).split(" ")[1]
    last_major = search(r"PREV_RELEASE_MAJOR [0-9]+", contents).group(0).split(" ")[1]
    last_minor = search(r"PREV_RELEASE_MINOR [0-9]+", contents).group(0).split(" ")[1]
    last_patch = search(r"PREV_RELEASE_PATCH [0-9]+", contents).group(0).split(" ")[1]
    this_version = f"{major}.{minor}"
    last_version = f"{last_major}.{last_minor}"
    last_tag = f"v{last_major}.{last_minor}.{last_patch}"
    return this_version, last_version, last_tag


def handle_msvc_arg(arg: str) -> MSVC:
    # Check for aliases first
    if arg in MSVC_ALIAS_MAPPING:
        return MSVC_ALIAS_MAPPING[arg]

    try:
        return MSVC(int(arg))
    except ValueError:
        possible_values = list(map(str, MSVC))
        aliases = [f"{k} -> {v}" for k, v in MSVC_ALIAS_MAPPING.items()]
        msg = f"Invalid MSVC version argument: '{arg}' (choose from {possible_values} or aliases {aliases})"
        raise argparse.ArgumentTypeError(msg)


def main() -> int:
    # Handle some command line arguments
    parser = argparse.ArgumentParser(description="Run Package Tests on EnergyPlus")
    parser.add_argument("config", choices=CONFIGURATIONS.keys(), help="Specify the run configuration")
    parser.add_argument("package_dir", help="Path to the extracted EnergyPlus package directory")
    parser.add_argument(
        "--msvc",
        type=handle_msvc_arg,
        choices=list(MSVC),
        default=None,
        help="For MSVC builds, this is the Visual Studio version ('16', '17')",
    )
    parser.add_argument("--verbose", action="store_true", help="If specified, get verbose output")
    args = parser.parse_args()
    if platform.system() == "Windows" and args.msvc is None:
        parser.error("On Windows, the --msvc argument is required to specify the Visual Studio version.")
    # Dynamically get the version information instead of hard-coding it
    this, last, last_tag = get_version_info()
    # Validate the path and prepare it for operations
    tentative_path = Path(args.package_dir)
    raw_artifact_path = tentative_path if tentative_path.is_absolute() else Path.cwd() / tentative_path
    # Instantiate the runner class based on the command line arguments
    runner = TestRunner(
        run_config_key=args.config, this_version=this, last_version=last, last_tag=last_tag, msvc_version=args.msvc
    )
    # Extract the package using the runner utility
    extracted_package_dir = runner.find_and_extract_package(raw_artifact_path)
    # Run all tests and return an exit code
    return runner.run_all_tests(Path(extracted_package_dir), args.verbose)


if __name__ == "__main__":
    exit(main())
