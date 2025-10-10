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

import difflib
import os
import sys
import unittest
from pathlib import Path

valid_null_enum_value_names = ["INVALID"]
valid_num_enum_value_names = ["NUM"]


def process_enum_str(input_str: str, file_name: str, line_no: int, print_errors: bool = True) -> int:
    """Process enum string, return true false for errors found flag"""

    # skip "enum class SomeEnum;"
    if "{" not in input_str:
        return 0

    error_str = ""
    input_str = input_str.replace("enum class", "")
    input_str = input_str.replace("};", "")
    tokens = input_str.split("{")
    if ":" in tokens[0]:
        tokens[0] = tokens[0].replace(" ", "").split(":")[0]

    name = tokens[0].strip()
    tokens = tokens[1].split(",")
    tokens = [x.strip() for x in tokens]

    if tokens[-1] == "":
        tokens.pop(-1)

    # split into names and integer values, in present
    keys = []
    keys_uc = []
    values = []
    for e in tokens:
        if "=" in e:
            tokens = e.replace(" ", "").split("=")
            keys.append(tokens[0])
            keys_uc.append(tokens[0].upper())
            try:
                values.append(int(tokens[1]))
            except ValueError:
                values.append(tokens[1])
        else:
            keys.append(e)
            keys_uc.append(e.upper())
            values.append("")

    # check for null names at 0-th position
    if keys_uc[0] not in valid_null_enum_value_names:
        # exceptions listed by <FILE>:<ENUM NAME>
        exceptions = [
            "CsvParser.hh:Token",
            "IdfParser.hh:Token",
            "OutputProcessor.hh:ReportingFrequency",
            "OutputProcessor.hh:ReportFreqSOV",
            "HVACInterfaceManager.cc:UpdateType",
            "DataHeatBalance.hh:PERptVars",
            "EconomicTariff.hh:StepType",
            "LowTempRadiantSystem.hh:OpMode",
        ]
        if f"{file_name}:{name}" not in exceptions:
            error_str += "\tMissing 'Invalid' at position 0\n"

    # check for null value = -1 at 0-th position
    if keys_uc[0] in valid_null_enum_value_names and values[0] != -1:
        # exceptions listed by <FILE>:<ENUM NAME>
        exceptions = ["HVACInterfaceManager.cc:UpdateType"]
        if f"{file_name}:{name}" not in exceptions:
            error_str += f"\t{keys_uc[0]} must = -1\n"

    # check for num names at N-th position
    if keys_uc[-1] not in valid_num_enum_value_names:
        # exceptions listed by <FILE>:<ENUM NAME>
        exceptions = [
            "HVACInterfaceManager.cc:UpdateType",
            "IdfParser.hh:Token",
            "EconomicTariff.hh:StepType",
            "LowTempRadiantSystem.hh:OpMode",
        ]
        if f"{file_name}:{name}" not in exceptions:
            error_str += "\tMissing 'Num' at position N\n"

    # check for "unassigned" in names
    if "UNASSIGNED" in keys_uc:
        error_str += "\tUNASSIGNED in enum names\n"

    # check for "unknown" in names
    if "UNKNOWN" in keys_uc:
        # exceptions listed by <FILE>:<ENUM NAME>
        exceptions = ["DataGlobalConstants.hh:Units"]
        if f"{file_name}:{name}" not in exceptions:
            error_str += "\tUNKNOWN in enum names\n"

    # check for proper casing
    if str(name[0]).islower():
        # exceptions listed by <FILE>:<ENUM NAME>
        exceptions = [
            "DataGlobalConstants.hh:eResource",
            "DataGlobalConstants.hh:eFuel",
            "DataGlobalConstants.hh:ePollutant",
            "OutputProcessor.hh:eResourceSOV",
        ]
        if f"{file_name}:{name}" not in exceptions:
            error_str += "\tenum name must begin with upper case letter\n"

    if "ENUM" in str(name).upper():
        error_str += "\tenum name should not contain 'enum'\n"

    if any([str(x[0]).islower() for x in keys]):
        # exceptions listed by <FILE>:<ENUM NAME>
        exceptions = ["FileSystem.hh:FileTypes", "DataGlobalConstants.hh:Units"]
        if f"{file_name}:{name}" not in exceptions:
            error_str += "\tenum keys must begin with upper case letter\n"

    if difflib.get_close_matches(name, keys, cutoff=0.7):
        exceptions = [
            "DataGlobalConstants.hh:HeatOrCool",
            "DataHVACGlobals.hh:UnitarySysType",
            "DataGenerators.hh:WaterTempMode",
        ]
        if f"{file_name}:{name}" not in exceptions:
            error_str += "\tenum keys are too similar to enum name\n"

    # # check for non-allowed enum values
    # if any([x != -1 for x in values if type(x) == int]):
    #     error_str += "\texplicit numbers not allowed in enum values except 'Invalid=-1'\n"

    if error_str:
        if print_errors:
            print(f"ERROR: malformed 'enum class'")
            print(f"{file_name}: {line_no} - {name}")
            print(error_str)
        return 1
    else:
        return 0


def find_enums(search_path: Path) -> int:
    """Checks for malformed enums, returns the number of errors found"""
    num_errors = 0

    files_to_search = []
    for p in [search_path]:
        for root, dirs, files in os.walk(p):
            for file in files:
                f_path = Path(root) / Path(file)
                f_extension = f_path.suffix
                if f_extension == ".hh":
                    files_to_search.append(f_path)
                elif f_extension == ".cc":
                    files_to_search.append(f_path)

    files_to_search.sort()

    for file in files_to_search:

        try:
            with open(file, "r") as f:
                lines = f.readlines()
        except UnicodeDecodeError:
            with open(file, "r", encoding="utf-8") as f:
                lines = f.readlines()

        lines = [x.strip() for x in lines]

        start_found = False
        start_line = 0
        end_found = False
        enum_str = ""

        for idx, line in enumerate(lines):

            # skip blank lines
            if line == "":
                continue

            # skip comment lines
            if line[0:2] == "//":
                continue

            # strip trailing comments
            if "//" in line:
                tokens = line.split("//")
                line = tokens[0].strip()

            if "enum class" in line:
                start_found = True
                start_line = idx + 1

            if start_found and (";" in line):
                end_found = True

            if start_found:
                enum_str += line

            if end_found:
                num_errors += process_enum_str(enum_str, file.name, start_line)
                start_found = False
                end_found = False
                enum_str = ""

    return num_errors


class TestProcessEnums(unittest.TestCase):
    def test_process_enum_str(self):
        # forward decl
        s = "enum class SomeType;"
        self.assertFalse(process_enum_str(s, "DummyFile", 1, False))

        # proper format
        s = "enum class SomeType : int {Invalid = -1, Valid, Num};"
        self.assertFalse(process_enum_str(s, "DummyFile", 1, False))

        # missing 'invalid'
        s = "enum class SomeType {Valid, Num};"
        self.assertTrue(process_enum_str(s, "DummyFile", 1, False))

        # missing 'num'
        s = "enum class SomeType {Invalid = -1, Valid};"
        self.assertTrue(process_enum_str(s, "DummyFile", 1, False))


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "test":
        del sys.argv[1:]
        unittest.main(exit=False, verbosity=0)
    root_path = Path(__file__).parent.parent.parent
    src_path = root_path / "src" / "EnergyPlus"
    errors_found = find_enums(src_path)
    if errors_found > 0:
        raise sys.exit(1)
