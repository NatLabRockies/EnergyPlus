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

import argparse
import shutil
import subprocess
import tempfile
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path

from tqdm.auto import tqdm

ROOT_DIR = Path(__file__).parent.parent.parent


def stringify_cmd(cmd) -> list[str]:
    """Takes a list of arguments that may be Path or str or int and turns into a List[str].
    So that is it suitable to pass to subprocess.
    """
    return list(map(str, cmd))


def _prepare_temp_dir_for_transition(idd_path: Path, old_idd_path: Path | None = None) -> Path:
    """Prepare a temporary directory for running the transition tool

    Copies the IDD files into the temporary directory so we can run the transition tool there and avoid some
    concurrency issues.
    """
    temp_dir = Path(tempfile.mkdtemp(prefix="transition-"))
    shutil.copy(idd_path, temp_dir)
    if old_idd_path is not None:
        shutil.copy(old_idd_path, temp_dir)
    return temp_dir


def _cleanup_transition_artifacts(idf_path: Path) -> None:
    for suffix in {".VCpErr", ".idfnew", ".idfold", ".imfnew", ".imfold"}:
        artifact = idf_path.with_suffix(suffix)
        if artifact.is_file():
            artifact.unlink()


def translate_file_parallelizable(
    idf_path: Path, formatter_only_exe: Path, idd_path: Path, old_idd_path: Path | None = None
) -> int:
    assert idf_path.is_file(), f"Could not find {idf_path}"

    temp_dir = _prepare_temp_dir_for_transition(idd_path=idd_path, old_idd_path=old_idd_path)
    cmd = stringify_cmd([formatter_only_exe, idf_path])
    r = subprocess.run(
        cmd,
        capture_output=True,
        shell=False,
        encoding="utf-8",
        cwd=temp_dir,
    )

    try:
        if r.returncode == 0:
            # Keep only the new files and remove the old ones
            for x in ["idf", "rvi", "mvi", "imf"]:
                new_file = idf_path.with_suffix(f".{x}new")
                if x == "idf":
                    assert new_file.is_file(), f"Could not find {new_file}, {temp_dir=}"
                if new_file.is_file():
                    shutil.move(new_file, idf_path.with_suffix(f".{x}"))

                old_file = idf_path.with_suffix(f".{x}old")
                if old_file.is_file():
                    old_file.unlink()

            # print('Done for {}.idf - {}'.format(eplus_file, path))
        else:
            print(f"Error for {idf_path}: {temp_dir=}")
            print(r.stdout)
            print(r.stderr)
    finally:
        _cleanup_transition_artifacts(idf_path)

    # https://github.com/NatLabRockies/EnergyPlus/issues/9590
    # Remove weird extra comments. E.g.,
    #      !- Field 4
    #      !- Field 6
    #      !- Field 8
    # ---
    #          !- X,Y,Z  1 {m}
    #          !- X,Y,Z  2 {m}
    #          !- X,Y,Z  3 {m}
    with open(idf_path, "r") as f:
        lines = f.readlines()

    with open(idf_path, "w") as f:
        prev_line_blank = False
        for line in lines:
            line_strip = line.strip()
            line_lstrip = line.lstrip()
            this_line_blank = not line_strip

            # Assume any line starting with "!-" and more than 4 spaces indented can be removed.
            if not (prev_line_blank and this_line_blank) and not (
                line_strip.startswith("!-") and (len(line) - len(line_lstrip) > 4)
            ):
                f.write(line)
                prev_line_blank = this_line_blank

    return r.returncode


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Make IDF files pretty using EnergyPlus transition tool.")
    parser.add_argument(
        "--root-dir",
        type=Path,
        default=ROOT_DIR,
        help="Directory to the root where IDF files will be globbed. Default: '%(default)s'.",
    )
    parser.add_argument("formatter_only_exe", type=Path, help="Path to the Formatter_only-VX-Y-Z executable.")
    args = parser.parse_args()

    root_dir = args.root_dir.resolve()

    idf_paths = []
    for ext in ["idf", "imf"]:
        idf_paths.extend(root_dir.glob(f"**/*.{ext}"))
    print(f"Found {len(idf_paths)} IDF files in {root_dir}")

    # Something like '/path/to/Formatter_only-V25-2-0'
    formatter_only_exe = args.formatter_only_exe.resolve()
    if not formatter_only_exe.is_file():
        raise ValueError(f"Could not find {formatter_only_exe}")

    # Extract version from the executable name: V25-2-0
    v_name = formatter_only_exe.stem.split("-", maxsplit=1)[1]
    # V25-2-0-Energy+.idd
    idd_path = formatter_only_exe.parent / f"{v_name}-Energy+.idd"
    if not idd_path.is_file():
        raise ValueError(f"Could not find {idd_path}")

    # Use a Pool to process files in parallel
    with tqdm(total=len(idf_paths)) as pbar:
        with ProcessPoolExecutor() as executor:
            futures = {
                executor.submit(
                    translate_file_parallelizable,
                    idf_path=idf_path,
                    formatter_only_exe=formatter_only_exe,
                    idd_path=idd_path,
                ): idf_path
                for idf_path in idf_paths
            }
            for future in as_completed(futures):
                idf_path = futures[future]
                try:
                    future.result()
                except Exception as exc:
                    print(f"File {idf_path} generated an exception: {exc}")
                pbar.update(1)
