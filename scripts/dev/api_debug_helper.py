#!/usr/bin/env python3
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

# This is just a small script that allows for easily running EnergyPlus for API calling debugging purposes
# It is currently set up for a Unix-ish environment, but could be adapted for all platforms
# To start debugging, follow these simple steps
# - You probably want to run from within a Python venv, so that you can quickly install custom dependencies.
#   To do that, just run `python -m venv /path/to/venv`
#   Then activate it with `. /path/to/venv/bin/activate`
#   Then pip install whatever you want with `pip install something` or `pip install -r requirements.txt`
# - Next prepare this file, setting the build directory, products directory, and IDF to test
# - Next prepare the build folder, make sure cmake is set up and run and it builds OK already
# - Now run the debugger by simply passing the Python binary to it: `gdb /path/to/venv/bin/python`
# - Inside the debugger, you set the command line args to run this file with `set args /path/to/this/file.py`
# - You can try to immediately run and see what happens by entering `r`, it should run EnergyPlus straight through
# - You can add breakpoints at any time, like `break SimulationManager.cc:188`, though it may ask you to confirm
# - You can change the E+ code in this repo, kill the current Python run with `k`, and then re-run this script with `r`
#   Because this script starts with a make command, it will then build the updated code before trying to call the API
#   This allows for rapid debugging iteration

import os
from pathlib import Path
from subprocess import check_call
from sys import exit, path
from tempfile import mkdtemp

DO_BUILD = False

repo_root = Path(__file__).resolve().parent.parent.parent
file_to_run = repo_root / "testfiles" / "PythonPluginCustomOutputVariable.idf"

if DO_BUILD:
    build_dir = repo_root / "cmake-build-debug"
    products_dir = build_dir / "Products"
    make_tool = "/snap/clion/current/bin/ninja/linux/x64/ninja"  # 'make'

    # os.cpu_count() returns Optional[int]
    cpu_count = max(1, (os.cpu_count() or 1) - 2)

    # this will automatically build E+ each run, so you can quickly make changes and re-execute inside the debugger
    check_call([make_tool, "-j", str(cpu_count), "energyplus"], cwd=str(build_dir))
else:
    products_dir = Path("/tmp/EnergyPlus-24.1.0-241fc81186-Linux-Ubuntu22.04-x86_64")


path.insert(0, str(products_dir))
from pyenergyplus.api import EnergyPlusAPI  # type: ignore[import]

api = EnergyPlusAPI()
state = api.state_manager.new_state()
run_dir = mkdtemp()
print(f"EnergyPlus starting with outputs in directory: {run_dir}")
return_value = api.runtime.run_energyplus(state, ["-d", run_dir, "-D", str(file_to_run)])
print(f"EnergyPlus finished with outputs in directory: {run_dir}")
exit(return_value)
