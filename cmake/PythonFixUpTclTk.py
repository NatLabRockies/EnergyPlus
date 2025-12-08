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

import glob
import os
import platform
import re
import shutil
import stat
import subprocess
import sys
from pathlib import Path


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
        if "compatibility version" in line and (m := LINKED_RE.match(line)):
            linked_libs.append(m.groupdict())
        elif "architecture arm64" in line and (m := LINKED_RE_ARM64.match(line)):
            linked_libs.append(m.groupdict())  # it will only have a libname key, I think that's fine
        else:
            raise ValueError(f"For {p}, cannot parse line: '{line}'")
    return linked_libs


if __name__ == "__main__":

    if platform.system() != "Darwin" and platform.system() != "Linux":
        sys.exit(0)

    print("PYTHON: Copying and fixing up Tcl/Tk")

    if len(sys.argv) == 2:
        python_dir = Path(sys.argv[1])
    else:
        print("Must call " + sys.argv[0] + " with one command line argument: the path to the python_lib directory")
        sys.exit(1)

    assert python_dir.is_dir()
    lib_dynload_dir = python_dir / "lib-dynload"

    if platform.system() == "Darwin":
        tk_so = locate_tk_so(python_dir)
        tcl_tk_sos = [Path(t["libname"]) for t in get_linked_libraries(tk_so) if "libt" in t["libname"]]

        any_missing = False
        for tcl_tk_so in tcl_tk_sos:
            new_tcl_tk_so = lib_dynload_dir / tcl_tk_so.name
            if str(tcl_tk_so).startswith("@loader_path"):
                assert (
                    new_tcl_tk_so.is_file()
                ), f"{new_tcl_tk_so} missing when the tkinter so is already adjusted. Wipe the dir"
                print("Already fixed up the libtcl and libtk, nothing to do here")
                continue
            elif not str(tcl_tk_so).startswith("@") and not tcl_tk_so.exists():
                print(
                    f"Hmmm... could not find the dependency shared object at {tcl_tk_so}; script will continue but fail"
                )
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

            # Grab the necessary tcl and tk folders
            # At tcl8: libtcl8.6.dylib, libtk8.6.dylib           -> support folders are 'tcl8.6', 'tk8.6'
            # At tcl9: libtcl9.0.dylib, but 'libtcl9tk9.0.dylib' -> support folders are 'tcl9.0', 'tk9.0'
            support_folder = tcl_tk_so.parent / tcl_tk_so.stem.replace("lib", "").replace("tcl9tk", "tk")
            if not support_folder.is_dir():
                print(f"Could not find support folder at {support_folder}")
                any_missing = True
                continue
            target_support_folder = python_dir / support_folder.name
            if target_support_folder.is_dir():
                # Remove it first
                shutil.rmtree(target_support_folder)
            print(f"Copying support folder from {support_folder} to {target_support_folder}")
            shutil.copytree(support_folder, target_support_folder)

        if any_missing:
            sys.exit(1)

    elif platform.system() == "Linux":
        # On Linux, we need to copy the libtk.X.Y.so file from usually /usr/lib/x86_64-linux-gnu
        # along with the /usr/share/tcltk/tclX.Y and /usr/share/tcltk/tkX.Y folders.
        # while I think X.Y will be 8.6, I'll try my best to be flexible for version and architecture
        arch = platform.machine()  # x86_64, arm64, aarch64
        if platform.freedesktop_os_release()["ID"] in ["centos", "almalinux"]:
            arch_lib_dir = Path("/usr/lib64")
            tcl_tk_root_dir = Path("/usr/share")
        else:
            arch_lib_dir = Path("/usr/lib") / f"{arch}-linux-gnu"
            tcl_tk_root_dir = Path("/usr/share/tcltk")

        lib_tk_search_path = arch_lib_dir / "libtk[0-9]*.[0-9]*.so"
        tk_candidates = glob.glob(str(lib_tk_search_path))
        if not tk_candidates:
            print(f"Could not find the libtk .so file, expected it inside {arch_lib_dir}")
            sys.exit(1)
        found_tk_so = Path(tk_candidates[0])
        print(f"Found libtk at: {found_tk_so}")
        target_tk_so = lib_dynload_dir / found_tk_so.name
        print(f"Copying libtk to: {target_tk_so}")
        shutil.copy(found_tk_so, target_tk_so)
        # we also need to find the _tkinter....so file in the python_lib/lib-dynload folder, and fixup the rpath
        matches = list(lib_dynload_dir.glob("_tkinter*.so"))
        print("DEBUG Matches:", matches)
        if len(matches) == 1:
            file_path = matches[0]
            print("DEBUG Found:", file_path)
        else:
            print("Error: Found", len(matches), "matching files")
            sys.exit(1)
        # first we need to make sure we actually can fix it up...is it a stripped executable
        output = subprocess.check_output(["file", file_path])
        if b", stripped" in output:
            subprocess.check_call(["echo", "::warning::_tkinter file is stripped and can't fix up the rpath"])
        else:
            subprocess.check_output(["patchelf", "--set-rpath", "$ORIGIN", file_path])
        # and then grab the tcl and tk config/data folders as well
        if not tcl_tk_root_dir.exists():
            print(f"TclTk directory not found at expected location: {tcl_tk_root_dir}, make sure it is installed")
        tcl_search_path = tcl_tk_root_dir / "tcl[0-9]*.[0-9]*"
        tk_search_path = tcl_tk_root_dir / "tk[0-9]*.[0-9]*"
        tcl_dirs = glob.glob(str(tcl_search_path))
        tk_dirs = glob.glob(str(tk_search_path))
        if not tcl_dirs or not tk_dirs:
            print(f"Could not find tcl/tk directories, expected inside: {tcl_tk_root_dir}, make sure it is installed")
        found_tcl_dir = Path(tcl_dirs[0])
        found_tk_dir = Path(tk_dirs[0])
        target_tcl_dir = python_dir / found_tcl_dir.name
        target_tk_dir = python_dir / found_tk_dir.name
        shutil.copytree(found_tcl_dir, target_tcl_dir, dirs_exist_ok=True)
        shutil.copytree(found_tk_dir, target_tk_dir, dirs_exist_ok=True)
