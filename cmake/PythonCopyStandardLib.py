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

# Copies the Python standard library to the target dir
# If the target_dir already exists, it doesn't do anything
# an easy way is to find the __init__ file for a known standard lib package and go two parents up
# should result in something like `/usr/lib/python3.7`

# this script must be called with two args:
# 1 - the path to the EnergyPlus executable in the install-tree, which is used to determine where to copy the library
#     since this is in the install-tree, you'll need to use a cmake generator expression
# 2 - name of the folder to create to store the copied in python standard library, usually python_lib
import ctypes
import glob as gb
import os
import platform
import shutil
import sys

print("PYTHON: Copying standard library files")

if len(sys.argv) == 3:
    exe_path = sys.argv[1]
    folder_name = sys.argv[2]
else:
    print(
        "Must call " + sys.argv[0] + "with two command line arguments: the path to the energyplus exe and the name "
        "of the new library directory"
    )
    sys.exit(1)
exe_dir = os.path.dirname(exe_path)
target_dir = os.path.join(exe_dir, folder_name)

ctypes_import_file = os.path.abspath(ctypes.__file__)
ctypes_package_dir = os.path.dirname(ctypes_import_file)
standard_lib_dir = os.path.dirname(ctypes_package_dir)

print(f"PYTHON: Analyzing standard library directory at {standard_lib_dir}")

if os.path.exists(target_dir):
    # Let's check the library files to see if the ABI matches
    # Otherwise if you build with say python 3.8 initially, and then switch to
    # python 3.9, your lib-dynload will still have the 38 .so files

    def find_libs(dir_path):
        sos = []
        for ext in ["a", "so", "lib"]:
            sos += gb.glob(os.path.join(dir_path, "**/*.{}*".format(ext)), recursive=True)
        return [os.path.basename(f) for f in sos]

    std_lib_ctypes_sos = find_libs(standard_lib_dir)
    this_lib_ctypes_sos = find_libs(target_dir)

    if (set(std_lib_ctypes_sos) - set(this_lib_ctypes_sos)) or (set(this_lib_ctypes_sos) - set(std_lib_ctypes_sos)):
        print("Detected changes in the python libs, wiping and recopying")
        shutil.rmtree(target_dir)
    else:
        # File names match
        sys.exit(0)

shutil.copytree(standard_lib_dir, target_dir)

# On Windows, we also need to grab the DLLs folder, which is one folder up
if platform.system() == "Windows":
    python_root_dir = os.path.dirname(standard_lib_dir)
    dll_dir = os.path.join(python_root_dir, "DLLs")
    shutil.copytree(dll_dir, target_dir, dirs_exist_ok=True)

# And also on Windows, we now need the grab the Tcl/Tk folder that contains config, scripts, and blobs
if platform.system() == "Windows":
    python_root_dir = os.path.dirname(standard_lib_dir)
    tcl_dir = os.path.join(python_root_dir, "tcl")
    shutil.copytree(tcl_dir, target_dir, dirs_exist_ok=True)

# then I'm going to try to clean up any __pycache__ folders in the target dir to reduce installer size
for root, dirs, _ in os.walk(target_dir):
    for this_dir in dirs:
        if this_dir == "__pycache__":
            shutil.rmtree(os.path.join(root, this_dir))

# on Windows the site_packages folder is inside the standard lib folder, so we need to delete that too
if os.path.exists(os.path.join(target_dir, "site-packages")):
    shutil.rmtree(os.path.join(target_dir, "site-packages"))
