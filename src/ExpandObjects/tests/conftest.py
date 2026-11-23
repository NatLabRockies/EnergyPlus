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

import inspect
import shutil
import subprocess
from pathlib import Path
from typing import Callable

import pytest


def validate_file(arg):
    if (filepath := Path(arg).expanduser()).is_file():
        return filepath
    else:
        raise FileNotFoundError(arg)


def pytest_addoption(parser):
    parser.addoption("--expandobjects-cli-path", type=validate_file, help="Path to the ExpandObjects executable")


@pytest.fixture(scope="module")
def expandobjectsclipath(request) -> Path:
    cli_path = request.config.getoption("--expandobjects-cli-path")
    if cli_path is None:
        raise ValueError("You must supply --expandobjects-cli-path [Path]")
    return cli_path


def who_called_me():
    return inspect.stack()[2].function


class ExpandObjectsResult:
    def __init__(self, run_dir: Path, returncode: int):
        self.run_dir = run_dir
        self.returncode = returncode

        self.idf_text: str = (run_dir / "expanded.idf").read_text()

        self.err_text: str | None = None
        err_path = run_dir / "expandedidf.err"
        if err_path.is_file():
            self.err_text = (run_dir / "expandedidf.err").read_text()

        self.addition_index: int = -1

        # Scan for
        # !
        # ! -------------------------------------------------------------
        # ! New objects created from ExpandObjects
        # ! -------------------------------------------------------------
        # !
        idf_lines = self.idf_text.splitlines()
        for i, line in enumerate(idf_lines):
            if line.strip() == "! New objects created from ExpandObjects":
                self.addition_index = i
                break
        assert (
            self.addition_index > 0
        ), "Could not find the section for new objects created from ExpandObjects in the expanded IDF"

        self.existing_section_lines: list[str] = idf_lines[: (self.addition_index - 2)]
        self.existing_section_text = "\n".join(self.existing_section_lines)
        self.new_section_lines: list[str] = idf_lines[(self.addition_index + 3) :]
        self.new_section_text = "\n".join(self.new_section_lines)


@pytest.fixture
def prepare_and_run_expandobjects(expandobjectsclipath: Path) -> Callable[[str], ExpandObjectsResult]:
    def _run(
        ori_idf_text: str,
    ):

        run_dir = Path(__file__).parent / "outputs" / who_called_me()
        if run_dir.is_dir():
            shutil.rmtree(run_dir)
        run_dir.mkdir(parents=True)
        idf_file_path = run_dir / "in.idf"
        idf_file_path.write_text(ori_idf_text)

        ep_idd = expandobjectsclipath.parent / "Energy+.idd"
        assert ep_idd.is_file(), f"Energy+.idd not found at {ep_idd}"
        target_ep_idd_path = run_dir / ep_idd.name
        shutil.copy(ep_idd, run_dir / target_ep_idd_path)

        completed_process = subprocess.run([str(expandobjectsclipath), idf_file_path], cwd=run_dir, check=False)

        return ExpandObjectsResult(run_dir=run_dir, returncode=completed_process.returncode)

    return _run
