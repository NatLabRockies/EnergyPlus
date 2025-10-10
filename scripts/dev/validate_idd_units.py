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

# Usage: No arguments necessary, this will find the local unconfigured (raw) IDD file and process it
#        The program will scan unit specifications in the idd header and then validate all field \units tags

import codecs
import json
import os
import sys

# There are some missing units in a large number of fields.
# I don't really want to add an ignore list, but I don't want to fix them all at the moment, either.
# Thus, here is an ignore list.  To fix these up, you could add these to the 'not-translated units' section.
ignore_list = []  # this is empty now with eht units added to the IDD itself

# There are also some lines that include more than one unit specification
# I'd like to include the warning for those, but I won't at the moment, so for now this warning is disabled.
# Later on, enable this to ensure unit strings are properly formatted
warn_for_bad_unit_tokens = False


class ReadingMode:
    FindTranslatedUnits = 1
    FindNonTranslatedUnits = 2
    ScanFieldUnits = 3


class Problem:
    def __init__(self, line_num, detail):
        self.line_num = line_num
        self.detail = detail

    def __str__(self):
        return "Line # %s: %s" % (self.line_num, self.detail)


current_script_dir = os.path.dirname(os.path.realpath(__file__))
idd_file = os.path.join(current_script_dir, "..", "..", "idd", "Energy+.idd.in")
idd_lines = codecs.open(idd_file, encoding="utf-8", errors="ignore").readlines()

original_units = []
reading_mode = ReadingMode.FindTranslatedUnits
line_num = 0
num_issues_found = 0
for line in idd_lines:
    line = line.strip()
    line_num += 1
    if reading_mode == ReadingMode.FindTranslatedUnits:
        if line.startswith("!      ") and "=>   " in line:
            tokens = line.split(" ")
            real_tokens = [t for t in tokens if t]
            original_units.append(real_tokens[1])
        elif "! Units fields that are not translated" in line:
            reading_mode = ReadingMode.FindNonTranslatedUnits
    elif reading_mode == ReadingMode.FindNonTranslatedUnits:
        if line.startswith("!      "):
            tokens = line.split(" ")
            real_tokens = [t for t in tokens if t]
            original_units.append(real_tokens[1])
        else:
            reading_mode = ReadingMode.ScanFieldUnits
    elif reading_mode == ReadingMode.ScanFieldUnits:
        if "\\units " in line:
            tokens = line.split(" ")
            real_tokens = [t for t in tokens if t]
            if not len(real_tokens) == 2 and warn_for_bad_unit_tokens:
                print(
                    json.dumps(
                        {
                            "tool": "validate_idd_units.py",
                            "filename": "/idd/Energy+.idd.in",
                            "file": "/idd/Energy+.idd.in",
                            "line": line_num,
                            "messagetype": "warning",
                            "message": "Unexpected number of unit specifications",
                        }
                    )
                )
                num_issues_found += 1
            elif real_tokens[1] not in original_units and real_tokens[1] not in ignore_list:
                print(
                    json.dumps(
                        {
                            "tool": "validate_idd_units.py",
                            "filename": "/idd/Energy+.idd.in",
                            "file": "/idd/Energy+.idd.in",
                            "line": line_num,
                            "messagetype": "warning",
                            "message": "Unexpected unit type found: " + real_tokens[1],
                        }
                    )
                )
                num_issues_found += 1

if num_issues_found > 0:
    sys.exit(1)
