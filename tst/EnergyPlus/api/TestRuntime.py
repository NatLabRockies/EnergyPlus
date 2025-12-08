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

import sys

from pyenergyplus.api import EnergyPlusAPI

progressValue = 0


def environment_handler(_state) -> None:
    print("OH HAI ENVIRONMENT")
    sys.stdout.flush()


def common_callback_handler(_state) -> None:
    print("OH HAI SOMETHING")
    sys.stdout.flush()


def progress_handler(progress: int) -> None:
    global progressValue
    progressValue = progress
    if 49 < progress < 51:
        print("HALFWAY THERE!!")
        sys.stdout.flush()


def error_handler(severity: int, message: bytes) -> None:
    if b"Warning" in message:
        print("GOT A WARNING UH OH!")
        sys.stdout.flush()


api = EnergyPlusAPI()
state = api.state_manager.new_state()
api.runtime.callback_begin_new_environment(state, environment_handler)
api.runtime.callback_progress(state, progress_handler)
api.functional.callback_error(state, error_handler)
api.runtime.callback_end_system_sizing(state, common_callback_handler)
api.runtime.callback_after_component_get_input(state, common_callback_handler)
api.runtime.callback_unitary_system_sizing(state, common_callback_handler)
v = api.runtime.run_energyplus(state, sys.argv[1:])
if v != 0:
    print("EnergyPlus Failed!")
    sys.exit(1)
assert progressValue == 100

print("MUTING CONSOLE OUTPUT")
state2 = api.state_manager.new_state()
api.runtime.clear_callbacks()
api.functional.clear_callbacks()
api.runtime.set_console_output_status(state2, False)
print("RUNNING MUTED ENERGYPLUS...")
v = api.runtime.run_energyplus(state2, sys.argv[1:])
if v != 0:
    print("EnergyPlus Failed!")
    sys.exit(1)
print("MUTED E+ RUN DONE")

print("Attempting a run that kills EnergyPlus")
state3 = api.state_manager.new_state()
new_api = EnergyPlusAPI()


def callback_that_kills(_state) -> None:
    print("Going to call stop_simulation!")
    api.runtime.stop_simulation(_state)


api.runtime.callback_begin_system_timestep_before_predictor(state3, callback_that_kills)
v = api.runtime.run_energyplus(state3, sys.argv[1:])
if v != 0:
    print("EnergyPlus Failed to Abort Cleanly; Task Succeeded Unsuccessfully!")
    sys.exit(1)
print("Killed EnergyPlus Run completed!")
