#!/usr/bin/env python
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

import difflib
import sys
import unittest
from collections import Counter
from pathlib import Path

from base_hook import (
    SRC_DIR,
    ErrorMessage,
    LogLevel,
    LogMessage,
    collect_files,
    exit_hook,
    flatten_list_of_lists,
    get_base_parser,
    parallel_apply,
    report_log_messages,
)

EXTENSIONS = {".hh", ".cc"}

VALID_NULL_ENUM_VALUE_NAMES = ["INVALID"]
VALID_NUM_ENUM_VALUE_NAMES = ["NUM"]


def process_enum_str(input_str: str, filepath: Path, line_no: int) -> list[LogMessage]:
    """Process enum string."""

    log_messages: list[LogMessage] = []

    # skip "enum class SomeEnum;"
    if "{" not in input_str:
        return log_messages

    file_name = filepath.name

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
    values: list[str | int] = []
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
    if keys_uc[0] not in VALID_NULL_ENUM_VALUE_NAMES:
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
            log_messages.append(
                ErrorMessage(
                    tool="check_for_malformed_enums",
                    filepath=filepath,
                    line_number=line_no,
                    line=input_str,
                    message=f"Malformed 'enum class' '{name}' - Missing 'Invalid' at position 0",
                )
            )

    # check for null value = -1 at 0-th position
    if keys_uc[0] in VALID_NULL_ENUM_VALUE_NAMES and values[0] != -1:
        # exceptions listed by <FILE>:<ENUM NAME>
        exceptions = ["HVACInterfaceManager.cc:UpdateType"]
        if f"{file_name}:{name}" not in exceptions:
            log_messages.append(
                ErrorMessage(
                    tool="check_for_malformed_enums",
                    filepath=filepath,
                    line_number=line_no,
                    line=input_str,
                    message=f"Malformed 'enum class' '{name}' - {keys_uc[0]} must = -1",
                )
            )

    # check for num names at N-th position
    if keys_uc[-1] not in VALID_NUM_ENUM_VALUE_NAMES:
        # exceptions listed by <FILE>:<ENUM NAME>
        exceptions = [
            "HVACInterfaceManager.cc:UpdateType",
            "IdfParser.hh:Token",
            "EconomicTariff.hh:StepType",
            "LowTempRadiantSystem.hh:OpMode",
        ]
        if f"{file_name}:{name}" not in exceptions:
            log_messages.append(
                ErrorMessage(
                    tool="check_for_malformed_enums",
                    filepath=filepath,
                    line_number=line_no,
                    line=input_str,
                    message=f"Malformed 'enum class' '{name}' - Missing 'Num' at position N",
                )
            )

    # check for "unassigned" in names
    if "UNASSIGNED" in keys_uc:
        log_messages.append(
            ErrorMessage(
                tool="check_for_malformed_enums",
                filepath=filepath,
                line_number=line_no,
                line=input_str,
                message=f"Malformed 'enum class' '{name}' - UNASSIGNED in enum names",
            )
        )

    # check for "unknown" in names
    if "UNKNOWN" in keys_uc:
        # exceptions listed by <FILE>:<ENUM NAME>
        exceptions = ["DataGlobalConstants.hh:Units"]
        if f"{file_name}:{name}" not in exceptions:
            log_messages.append(
                ErrorMessage(
                    tool="check_for_malformed_enums",
                    filepath=filepath,
                    line_number=line_no,
                    line=input_str,
                    message=f"Malformed 'enum class' '{name}' - UNKNOWN in enum names",
                )
            )

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
            log_messages.append(
                ErrorMessage(
                    tool="check_for_malformed_enums",
                    filepath=filepath,
                    line_number=line_no,
                    line=input_str,
                    message=f"Malformed 'enum class' '{name}' - enum name must begin with upper case letter",
                )
            )

    if "ENUM" in str(name).upper():
        log_messages.append(
            ErrorMessage(
                tool="check_for_malformed_enums",
                filepath=filepath,
                line_number=line_no,
                line=input_str,
                message=f"Malformed 'enum class' '{name}' - enum name should not contain 'enum'",
            )
        )

    if any([str(x[0]).islower() for x in keys]):
        # exceptions listed by <FILE>:<ENUM NAME>
        exceptions = ["FileSystem.hh:FileTypes", "DataGlobalConstants.hh:Units"]
        if f"{file_name}:{name}" not in exceptions:
            log_messages.append(
                ErrorMessage(
                    tool="check_for_malformed_enums",
                    filepath=filepath,
                    line_number=line_no,
                    line=input_str,
                    message=f"Malformed 'enum class' '{name}' - enum keys must begin with upper case letter",
                )
            )

    if difflib.get_close_matches(name, keys, cutoff=0.7):
        exceptions = [
            "DataGlobalConstants.hh:HeatOrCool",
            "DataHVACGlobals.hh:UnitarySysType",
            "DataGenerators.hh:WaterTempMode",
        ]
        if f"{file_name}:{name}" not in exceptions:
            log_messages.append(
                ErrorMessage(
                    tool="check_for_malformed_enums",
                    filepath=filepath,
                    line_number=line_no,
                    line=input_str,
                    message=f"Malformed 'enum class' '{name}' - enum keys are too similar to enum name",
                )
            )

    # check for non-allowed enum values
    # if any([x != -1 for x in values if type(x) == int]):
    #      error_str = f"Malformed 'enum class' '{name}' - explicit numbers not allowed in enum values except 'Invalid=-1'"
    return log_messages


def check_for_malformed_enums(filepath: Path) -> list[LogMessage]:
    """Check a single file for malformed enums."""
    lines = [x.strip() for x in filepath.read_text(encoding="utf-8").splitlines()]

    start_found = False
    start_line = 0
    end_found = False
    enum_str = ""

    log_messages: list[LogMessage] = []

    for line_num, line in enumerate(lines, start=1):

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
            start_line = line_num

        if start_found and (";" in line):
            end_found = True

        if start_found:
            enum_str += line

        if end_found:
            log_messages += process_enum_str(input_str=enum_str, filepath=filepath, line_no=start_line)
            start_found = False
            end_found = False
            enum_str = ""

    return log_messages


class TestProcessEnums(unittest.TestCase):
    def test_process_enum_str(self) -> None:

        dummy_file = Path("DummyFile")

        # forward decl
        s = "enum class SomeType;"
        log_messages: list[LogMessage] = process_enum_str(input_str=s, filepath=dummy_file, line_no=1)
        self.assertEqual(len(log_messages), 0)

        # proper format
        s = "enum class SomeType : int {Invalid = -1, Valid, Num};"
        log_messages = process_enum_str(input_str=s, filepath=dummy_file, line_no=1)
        self.assertEqual(len(log_messages), 0)

        # missing 'invalid'
        s = "enum class SomeType {Valid, Num};"
        log_messages = process_enum_str(input_str=s, filepath=dummy_file, line_no=1)
        self.assertEqual(len(log_messages), 1)
        self.assertEqual(log_messages[0].message, "Malformed 'enum class' 'SomeType' - Missing 'Invalid' at position 0")

        # missing 'num'
        s = "enum class SomeType {Invalid = -1, Valid};"
        log_messages = process_enum_str(input_str=s, filepath=dummy_file, line_no=1)
        self.assertEqual(len(log_messages), 1)
        self.assertEqual(log_messages[0].message, "Malformed 'enum class' 'SomeType' - Missing 'Num' at position N")


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "test":
        del sys.argv[1:]
        unittest.main(exit=True, verbosity=0)

    parser = get_base_parser(description="Check for malformed enums")
    args = parser.parse_args()
    if args.files:
        n_ori = len(args.files)
        files = [f for f in args.files if f.suffix in EXTENSIONS and f.is_relative_to(SRC_DIR)]
        if args.verbose:
            print(f"Checking {len(files)} of {n_ori} specified files")
    else:
        files = list(collect_files(base_dir=SRC_DIR, extensions=EXTENSIONS, recursive=True, dirs_to_skip=[]))
        if args.verbose:
            counter_info = ", ".join([f"{num} {ext}" for ext, num in Counter([f.suffix for f in files]).items()])
            print(f"Checking {len(files)} files: {counter_info}")

    run_synchronously = False
    if run_synchronously:
        log_messages: list[LogMessage] = []
        for filepath in files:
            log_messages += check_for_malformed_enums(filepath=filepath)
    else:
        errors_list_of_lists = parallel_apply(func=check_for_malformed_enums, filepaths=files)
        log_messages = flatten_list_of_lists(list_of_lists=errors_list_of_lists)
    success = report_log_messages(log_messages=log_messages, fail_threshold=LogLevel.ERROR, verbose=args.verbose)
    exit_hook(success=success)
