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

# Attempt to automatically change the version number of all idf and imf files in a repo
# Two arguments: old version number and new version number

import codecs
import fnmatch
import json
import os
import sys

EPJSON_INDENT = 4  # Note, this essentially standardizes our indent for EpJSON files


# provide a nice usage function
def usage():
    print(
        """Call this script with three command line arguments:
 $ change_version.py <path to repo> <old version number> <new version number>
 $ change_version.py /repos/eplus 8.7 8.8"""
    )


# check the command line argument status
if not len(sys.argv) == 4:
    print("Invalid command line arguments")
    usage()
    sys.exit(1)

# store the command line arguments since they appear valid so far
repo = sys.argv[1]
v_old = sys.argv[2]
v_new = sys.argv[3]

# walk across the idf and imf files
for extension in ["*.idf", "*.imf"]:
    for folder in ["testfiles", "performance_tests", "datasets", os.path.join("testfiles", "BasicsFiles")]:
        this_dir = os.path.join(repo, folder)
        for root, dir_names, file_names in os.walk(this_dir):
            for filename in fnmatch.filter(file_names, extension):
                with codecs.open(os.path.join(root, filename), encoding="utf-8", errors="ignore") as input_file:
                    file_data = input_file.read()
                    file_data = file_data.replace("Version," + v_old, "Version," + v_new)
                    file_data = file_data.replace("VERSION," + v_old, "Version," + v_new)
                with codecs.open(os.path.join(root, filename), "w", encoding="utf-8") as output_file:
                    output_file.write(file_data)

# also epJSON files
for folder in ["testfiles", "performance_tests", "datasets", os.path.join("testfiles", "BasicsFiles")]:
    this_dir = os.path.join(repo, folder)
    for root, dir_names, file_names in os.walk(this_dir):
        for filename in fnmatch.filter(file_names, "*.epJSON"):
            with codecs.open(os.path.join(root, filename), encoding="utf-8", errors="ignore") as input_file:
                file_data = input_file.read()
                json_data = json.loads(file_data)
                json_data["Version"]["Version 1"]["version_identifier"] = v_new
            with codecs.open(os.path.join(root, filename), "w", encoding="utf-8") as output_file:
                output_file.write(json.dumps(json_data, indent=EPJSON_INDENT))

# then walk across all the unit test files too
for folder in [os.path.join("tst", "EnergyPlus", "unit")]:
    this_dir = os.path.join(repo, folder)
    for root, dir_names, file_names in os.walk(this_dir):
        for filename in fnmatch.filter(file_names, "*.cc"):
            with codecs.open(os.path.join(root, filename), encoding="utf-8", errors="ignore") as input_file:
                file_data = input_file.read()
                file_data = file_data.replace("Version," + v_old, "Version," + v_new)
                file_data = file_data.replace("VERSION," + v_old, "Version," + v_new)
            with codecs.open(os.path.join(root, filename), "w", encoding="utf-8") as output_file:
                output_file.write(file_data)
