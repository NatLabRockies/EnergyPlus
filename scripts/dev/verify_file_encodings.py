#!/usr/bin/env python
# -*- coding: utf-8 -*-
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

"""
This script ensures that files are encoded in UTF-8.

cf: https://github.com/NREL/EnergyPlus/issues/7213
Winter in Winter 2019
"""

__author__ = "Julien Marrec, EffiBEM"
__email__ = "julien@effibem.com"

import glob as gb
import io  # For Python 2 compat
import json
import os
import sys

# Get a path that'll work if run directly from this folder (when running
# locally usually) or the root of the repo (decent_ci for eg)
ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), "../../"))

# If you want this script to fix the encoding problems it finds
DO_FIX = False


def check_file_encoding(idf_path):
    """
    Verify that the file can be loaded with strict UTF-8 encoding,
    print a json message to console otherwise for decent_ci to consume
    """
    try:
        with io.open(idf_path, "r", encoding="utf-8", errors="strict") as f:
            f.read()
        return True
    except UnicodeDecodeError:
        rel_file = os.path.relpath(idf_path, start=ROOT_DIR)
        ci_msg = {
            "tool": "verify_file_encodings",
            "filename": idf_path,
            "file": idf_path,
            "messagetype": "error",
            "message": "{} isn't UTF-8 encoded".format(rel_file),
        }
        print(json.dumps(ci_msg))
        return False


def fix_encoding(idf_path):
    try:
        with io.open(idf_path, "r", encoding="latin-1", errors="strict") as f_in:
            idf_text = f_in.read()
        with io.open(idf_path, "w", encoding="utf-8") as f_out:
            f_out.write(idf_text)

    except ValueError:
        rel_file = os.path.relpath(idf_path, start=ROOT_DIR)
        raise ValueError("Cannot fix encoding for {}".format(rel_file))


if __name__ == "__main__":
    # Glob all .idf / .imf
    # Glob recursive Works in python3.4 and above only...
    if sys.version_info > (3, 4):
        test_files = gb.glob(os.path.join(ROOT_DIR, "**/*.i[d|m]f"), recursive=True)
    else:
        import fnmatch

        test_files = []
        for root, dirnames, filenames in os.walk(ROOT_DIR):
            for filename in fnmatch.filter(filenames, "*.i[d|m]f"):
                test_files.append(os.path.join(root, filename))

    overall_success = True
    for test_file in test_files:
        success = check_file_encoding(test_file)
        if not success:
            overall_success = False
            if DO_FIX:
                # Fix encoding
                fix_encoding(test_file)

    if not overall_success:
        sys.exit(1)
