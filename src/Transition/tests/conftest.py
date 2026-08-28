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

import difflib
import subprocess
from pathlib import Path

import pytest


def validate_dir(arg) -> Path:
    if (dirpath := Path(arg).expanduser()).is_dir():
        return dirpath
    raise NotADirectoryError(arg)


def pytest_addoption(parser):
    parser.addoption(
        "--transition-cli-dir", type=validate_dir, help="Directory containing Transition-VXXX-to-VYYY executables"
    )


@pytest.fixture(scope="module")
def transitionclidir(request) -> Path:
    cli_dir = request.config.getoption("--transition-cli-dir")
    if cli_dir is None:
        raise ValueError("You must supply --transition-cli-dir [Path]")
    return cli_dir


def add_required_objects(idf_text: str, version: tuple[int, int, int]) -> str:

    new_idf_text = ""

    if "Version" not in idf_text:
        v_str = ".".join(map(str, version[:2]))
        new_idf_text += f"""
  Version,{v_str};
"""
    if "Building" not in idf_text:
        new_idf_text += """
  Building,
    Bldg,                    !- Name
    0,                       !- North Axis {deg}
    Suburbs,                 !- Terrain
    0.04,                    !- Loads Convergence Tolerance Value {W}
    0.4000,                  !- Temperature Convergence Tolerance Value {deltaC}
    FullExterior,            !- Solar Distribution
    25,                      !- Maximum Number of Warmup Days
    6;                       !- Minimum Number of Warmup Days
"""
    if "GlobalGeometryRules" not in idf_text:
        new_idf_text += """
  GlobalGeometryRules,
    UpperLeftCorner,         !- Starting Vertex Position
    Counterclockwise,        !- Vertex Entry Direction
    Relative,                !- Coordinate System
    Relative,                !- Daylighting Reference Point Coordinate System
    Relative;                !- Rectangular Surface Coordinate System
"""
    new_idf_text += f"""
  {idf_text.strip()}
"""
    return f"""  {new_idf_text.strip()}
"""


@pytest.fixture
def prepare_and_run_transition(transitionclidir: Path, tmp_path: Path):
    def _run(
        ori_idf_text: str,
        expected_idf_text: str,
        version_ori: tuple[int, int, int],
        version_new: tuple[int, int, int],
    ):
        v_ori = "-".join(map(str, version_ori))
        v_new = "-".join(map(str, version_new))
        cli_path = transitionclidir / f"Transition-V{v_ori}-to-V{v_new}"
        if not cli_path.is_file():
            raise FileNotFoundError(f"Transition binary not found: {cli_path}")
        idf_file_path = tmp_path / "in.idf"
        idf_file_path.write_text(add_required_objects(idf_text=ori_idf_text, version=version_ori))
        subprocess.check_call([cli_path, idf_file_path], cwd=transitionclidir)
        new_text = (tmp_path / "in.idfnew").read_text()
        assert "Version,{};\n".format(".".join(map(str, version_new[:2]))) in new_text

        expected_idf_text = add_required_objects(idf_text=expected_idf_text, version=version_new)
        # If we can't find the expected_idf_text in the new text, print a diff
        if expected_idf_text not in new_text:
            diff_str = "".join(
                difflib.unified_diff(
                    expected_idf_text.splitlines(keepends=True),
                    new_text.splitlines(keepends=True),
                    fromfile="expected",
                    tofile=str(tmp_path / "in.idfnew"),
                )
            )
            assert False, diff_str
        else:
            assert expected_idf_text in new_text

    return _run
