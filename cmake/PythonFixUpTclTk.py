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

import platform
import re
import shutil
import subprocess
import sys
from pathlib import Path
import os
import stat


def locate_tk_so(python_dir: Path) -> Path:
    print(f"Searching for _tkinter so in {python_dir}")
    sos = list(python_dir.glob("lib-dynload/_tkinter*.so"))
    assert len(sos) == 1, "Unable to locate _tkinter so"
    return sos[0]


LINKED_RE = re.compile(
    r"(?P<libname>.*) \(compatibility version (?P<compat_version>\d+\.\d+\.\d+), "
    r"current version (?P<current_version>\d+\.\d+\.\d+)(?:, \w+)?\)"
)

LINKED_RE_ARM64 = re.compile(r"(?P<libname>.*) \(architecture arm64\)")


def get_linked_libraries(p: Path):
    linked_libs = []
    lines = subprocess.check_output(["otool", "-L", str(p)], encoding="utf-8", universal_newlines=True).splitlines()
    if "is not an object file" in lines[0]:
        return None
    lines = [x.strip() for x in lines[1:]]

    print(f"Processing otool -L {p}, output lines =")
    [print(f"  {l}") for l in lines]

    for line in lines:
        if 'compatibility version' in line and (m := LINKED_RE.match(line)):
            linked_libs.append(m.groupdict())
        elif 'architecture arm64' in line and (m := LINKED_RE_ARM64.match(line)):
            linked_libs.append(m.groupdict())  # it will only have a libname key, I think that's fine
        else:
            raise ValueError(f"For {p}, cannot parse line: '{line}'")
    return linked_libs


if __name__ == "__main__":

    if platform.system() != "Darwin":
        sys.exit(0)

    print("PYTHON: Copying and fixing up Tcl/Tk")

    if len(sys.argv) == 2:
        python_dir = Path(sys.argv[1])
    else:
        print("Must call " + sys.argv[0] + "with one command line argument: the path to the python_lib directory")
        sys.exit(1)

    assert python_dir.is_dir()
    lib_dynload_dir = python_dir / "lib-dynload"

    tk_so = locate_tk_so(python_dir)
    tcl_tk_sos = [Path(t["libname"]) for t in get_linked_libraries(tk_so) if "libt" in t["libname"]]

    any_missing = False
    for tcl_tk_so in tcl_tk_sos:
        new_tcl_tk_so = lib_dynload_dir / tcl_tk_so.name
        if str(tcl_tk_so).startswith('@loader_path'):
            assert new_tcl_tk_so.is_file(), f"{new_tcl_tk_so} missing when the tkinter so is already adjusted. Wipe the dir"
            print("Already fixed up the libtcl and libtk, nothing to do here")
            continue
        elif not str(tcl_tk_so).startswith('@') and not tcl_tk_so.exists():
            print(f"Hmmm... could not find the dependency shared object at {tcl_tk_so}; script will continue but fail")
            any_missing = True
            continue
        shutil.copy(tcl_tk_so, new_tcl_tk_so)
        # during testing, the brew installed tcl and tk libraries were installed without write permission
        # the workaround was to manually chmod u+w those files in the brew install folder
        # instead let's just fix them up once we copy them here
        current_perms = os.stat(str(new_tcl_tk_so)).st_mode
        os.chmod(str(new_tcl_tk_so), current_perms | stat.S_IWUSR)
        # now that it can definitely be written, we can run install_name_tool on it
        subprocess.check_output(
            ["install_name_tool", "-change", str(tcl_tk_so), f"@loader_path/{new_tcl_tk_so.name}", str(tk_so)]
        )
        # Change the id that's the first line of the otool -L in this case and it's confusing
        subprocess.check_output(["install_name_tool", "-id", str(new_tcl_tk_so.name), str(new_tcl_tk_so)])

    if any_missing:
        sys.exit(1)
