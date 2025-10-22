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

from pathlib import Path
from queue import Queue
from sys import argv, path
from tempfile import mkdtemp
from threading import Thread
from time import sleep
from tkinter import BOTH, HORIZONTAL, Button, Label, StringVar, Tk, ttk


class EPGui(Tk):
    def __init__(self):
        super().__init__()
        if len(argv) == 3:
            # if running in a build tree, pass in the path to the <build>/Products directory that contains pyenergyplus
            # and the full path to the IDF to run
            self.install_or_build_dir: Path = Path(argv[1])
            self.example_file_to_run: str = argv[2]
        else:
            # if running in an installation, it will live at the <install>/ExampleFiles/API, and you don't need any args
            self.install_or_build_dir: Path = Path(__file__).resolve().parent.parent.parent
            self.example_file_to_run: str = str(self.install_or_build_dir / "ExampleFiles" / "5ZoneAirCooled.idf")
        try:  # This is the new way, if the user's Python environment includes the pip installed energyplus-api-helpers
            # noinspection PyUnresolvedReferences
            from energyplus_api_helpers.import_helper import EPlusAPIHelper

            e = EPlusAPIHelper(self.install_or_build_dir)
            self.api = e.get_api_instance()
        except ImportError:  # This is the previous way, directly adding the E+ dir to the path before importing the API
            path.insert(0, str(self.install_or_build_dir))
            # noinspection PyUnresolvedReferences
            from pyenergyplus.api import EnergyPlusAPI

            self.api = EnergyPlusAPI()
        self.title("API/GUI Demonstration")
        Button(self, text="Run EnergyPlus", command=self._start_thread).pack(expand=True)
        ttk.Separator(self, orient=HORIZONTAL).pack(fill=BOTH, expand=False)
        self._tk_var_message = StringVar(value="<E+ Outputs>")
        Label(self, textvariable=self._tk_var_message).pack(fill=BOTH, expand=True)
        self.geometry("600x100")
        self._gui_queue = Queue()
        self._check_queue()
        self.mainloop()

    def _check_queue(self) -> None:
        while True:
            # noinspection PyBroadException
            try:
                task = self._gui_queue.get(block=False)
                self.after_idle(task)
            except Exception:
                break
        self.after(60, self._check_queue)

    def _callback(self, message: str) -> None:
        self._gui_queue.put(lambda: self._tk_var_message.set(message))
        sleep(0.03)  # this is purely so that the GUI messages are readable while E+ runs

    def _start_thread(self) -> None:
        Thread(target=self._run_ep, daemon=True).start()

    def _run_ep(self) -> None:
        run_directory = mkdtemp()
        state = self.api.state_manager.new_state()
        self.api.runtime.callback_message(state, self._callback)
        self.api.runtime.run_energyplus(state, ["-d", run_directory, "-D", self.example_file_to_run])
        print(f"Finished running EnergyPlus, results available in {run_directory}")


if __name__ == "__main__":
    EPGui()
