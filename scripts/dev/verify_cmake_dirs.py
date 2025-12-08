#!/usr/bin/env python3
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

import json
import os
import sys
from pathlib import Path

current_script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
repo_root_dir = current_script_dir.parent.parent

common_build_folder_names = ["build", "builds", "cmake-build-debug", "cmake-build-release"]
common_cmake_file_names = ["CMakeLists.txt", ".cmake"]
common_tokens_to_sanitize_out = ["ORIGINAL_CMAKE_SOURCE_DIR", "ORIGINAL_CMAKE_BINARY_DIR"]

DEBUG = False

num_issues_found = 0


def custom_check_output_line(relative_file_path: str, line_num: int, message: str) -> str:
    return json.dumps(
        {
            "tool": "verify_cmake_dirs",
            "file": relative_file_path,
            "line": line_num,
            "messagetype": "error",
            "message": message,
        }
    )


for path in Path(repo_root_dir).rglob("*"):
    # only do files
    if not path.is_file():
        continue
    # get a nice relative path
    relative_path = path.relative_to(repo_root_dir)
    s_relative_path = str(relative_path)
    # skip any configured files in any known build folder
    in_build_folder = False
    for b in common_build_folder_names:
        if s_relative_path.startswith(b):
            in_build_folder = True
            break
    if in_build_folder:
        continue
    # ignore everything in third_party...right?
    if s_relative_path.startswith("third_party"):
        continue
    # try to match a common name or pattern
    for f in common_cmake_file_names:
        if s_relative_path.endswith(f):
            break
    else:
        continue
    if DEBUG:
        print("Processing %s ... " % relative_path, end="")
    contents = path.read_text(encoding="utf-8", errors="ignore")
    lines = contents.split("\n")
    for i, line in enumerate(lines):
        # now sanitize the line just a bit
        for token in common_tokens_to_sanitize_out:
            line = line.replace(token, "")
        cmake_src = "CMAKE_SOURCE_DIR" in line
        cmake_bin = "CMAKE_BINARY_DIR" in line
        if cmake_src and cmake_bin:
            print(
                custom_check_output_line(
                    s_relative_path, i + 1, "Found CMAKE_SOURCE_DIR and CMAKE_BINARY_DIR in file contents"
                )
            )
            num_issues_found += 1
        elif cmake_src:
            print(custom_check_output_line(s_relative_path, i + 1, "Found CMAKE_SOURCE_DIR in file contents"))
            num_issues_found += 1
        elif cmake_bin:
            print(custom_check_output_line(s_relative_path, i + 1, "Found CMAKE_BINARY_DIR in file contents"))
            num_issues_found += 1
        else:
            if DEBUG:
                print(" [DONE]")

if num_issues_found > 0:
    sys.exit(1)
