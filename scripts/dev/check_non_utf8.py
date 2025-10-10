#!/usr/bin/env python
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

import io  # For Python 2 compat
import json
import os
import sys

DIRS_TO_SKIP = [
    ".git",
    "build",
    "builds",
    "cmake-build-debug",
    "cmake-build-release",
    "design",
    "release",
]

# these CC files purposefully have bad characters
# not sure what to do besides ignore them
FILE_NAMES_TO_SKIP = [
    # 'InputProcessor.unit.cc', 'EconomicTariff.cc',
    # 'OutputReportTabular.cc'
]

# tex files are included here, but the docs folder is ignored,
# so it has no effect right now
FILE_PATTERNS = [".cc", ".hh", ".tex", ".cpp", ".hpp", ".idd"]

current_script_dir = os.path.dirname(os.path.realpath(__file__))
repo_root = os.path.abspath(os.path.join(current_script_dir, "..", ".."))
full_path_dirs_to_skip = [os.path.join(repo_root, d) for d in DIRS_TO_SKIP]

num_warnings = 0
num_errors = 0

for root, dirs, filenames in os.walk(repo_root):
    if any(root.startswith(f) for f in full_path_dirs_to_skip):
        continue
    for filename in filenames:
        if not any(filename.endswith(f) for f in FILE_PATTERNS):
            continue
        if any(f in filename for f in FILE_NAMES_TO_SKIP):
            continue
        file_path = os.path.join(root, filename)
        relative_file_path = os.path.relpath(file_path, repo_root)

        # High level check: try to open the file as utf-8 in full
        try:
            with io.open(file_path, encoding="utf-8", errors="strict") as f_idf:
                idf_text = f_idf.read()
        except UnicodeDecodeError:
            ci_msg = {
                "tool": "verify_file_encodings",
                "filename": filename,
                "file": relative_file_path,
                "messagetype": "error",
                "message": ("{} isn't UTF-8 encoded" "".format(relative_file_path)),
            }
            print(json.dumps(ci_msg))
            num_errors += 1

            # Now you try to give a better error message by pointing at the
            # lines that are guilty. To do so, you open as binary, and try to
            # decode each line as utf-8
            with io.open(file_path, "rb") as f_idf:
                binary_lines = f_idf.readlines()

            for line_num, line in enumerate(binary_lines):
                try:
                    line.decode(encoding="utf-8", errors="strict")
                    # codecs.decode(line, encoding='utf-8', errors='strict')
                except (UnicodeDecodeError, UnicodeEncodeError):
                    _l = line.decode(encoding="utf-8", errors="replace")

                    # line contains non-utf8 character
                    print(
                        json.dumps(
                            {
                                "tool": "check_non_utf8",
                                "filename": relative_file_path,
                                "file": relative_file_path,
                                "line": line_num,
                                "messagetype": "warning",
                                # " {}".format(_l)) # only works python3
                                "message": ("Line has invalid characters/encoding: " + _l),
                            }
                        )
                    )
                    num_warnings += 1

if num_errors + num_warnings > 0:
    sys.exit(1)
