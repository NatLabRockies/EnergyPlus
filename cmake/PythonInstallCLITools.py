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
import platform
from pathlib import Path
from subprocess import run
from sys import argv, executable

PKGS = {
    "energyplus_launch": "3.7.4",
    "energyplus_transition_tools": "2.1.4",
    "ghedesigner": "2.0",
}


def install_packages(python_lib_dir: Path):
    # expecting one command line argument - the path to the python_lib folder to place the pip package
    pkgs_to_install = {k: v for k, v in PKGS.items() if not (python_lib_dir / f"{k}-{v}.dist-info").exists()}

    if not pkgs_to_install:
        print("PYTHON: All CLI packages found and up to date, no pip install needed")
        return

    to_install = [f"{n}=={v}" for n, v in pkgs_to_install.items()]
    
    is_windows = platform.system() == "Windows"
    is_arm = platform.machine().lower() in ("arm64", "aarch64")
    if "ghedesigner" in pkgs_to_install and is_windows and is_arm:
        # there are no arm64 Windows wheels for scipy on PyPi (as of 2025-09-05, Scipy 1.16.1)
        # I got a wheel built on https://github.com/scipy/scipy/actions/runs/17428824000 (weel builder)
        scipy_wheel_url = (
            "https://github.com/jmarrec/EnergyPlus/releases/download/"
            "v25.2.0-pre-IOFreeze-1/scipy-1.17.0.dev0-cp312-cp312-win_arm64.whl"
        )
        print(
            f"PYTHON: Installing scipy from {scipy_wheel_url} since this is Windows ARM64 "
            "and no official wheel exists as of 1.16.1"
        )
        to_install.append(scipy_wheel_url)

    print(f"PYTHON: CLI packages not found or out of date, pip installing these now: {to_install}")
    run([executable, "-m", "pip", "install", f"--target={python_lib_dir}", "--upgrade", *to_install], check=True)


def existing_dir(path_str):
    dir_path = Path(path_str).resolve()
    if not dir_path.is_dir():
        raise argparse.ArgumentTypeError(f"'{dir_path}' is not a valid directory")
    return dir_path


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Install required EnergyPlus Python packages into a target directory.")
    parser.add_argument(
        "python_lib_dir",
        type=existing_dir,
        help="Path to the python_lib folder where packages will be installed. Must be a valid existing directory.",
    )
    args = parser.parse_args()

    install_packages(python_lib_dir=args.python_lib_dir)
