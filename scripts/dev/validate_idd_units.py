#!/usr/bin/env python
# EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University
# of Illinois, The Regents of the University of California, through Lawrence
# Berkeley National Laboratory (subject to receipt of any required approvals
# from the U.S. Dept. of Energy), Oak Ridge National Laboratory, managed by UT-
# Battelle, Alliance for Energy Innovation, LLC, and other contributors. All
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

from enum import Enum
from pathlib import Path

from base_hook import (
    IDD_PATH,
    ErrorMessage,
    LogLevel,
    LogMessage,
    WarningMessage,
    exit_hook,
    flatten_list_of_lists,
    get_base_parser,
    parallel_apply,
    report_log_messages,
)

# There are some missing units in a large number of fields.
# I don't really want to add an ignore list, but I don't want to fix them all at the moment, either.
# Thus, here is an ignore list.  To fix these up, you could add these to the 'not-translated units' section.
IGNORE_LIST: list[str] = []  # this is empty now with the units added to the IDD itself

# There are also some lines that include more than one unit specification
# I'd like to include the warning for those, but I won't at the moment, so for now this warning is disabled.
# Later on, enable this to ensure unit strings are properly formatted
warn_for_bad_unit_tokens = False


class ReadingMode(Enum):
    FindTranslatedUnits = 1
    FindNonTranslatedUnits = 2
    ScanFieldUnits = 3


def validate_idd_units(idd_path: Path = IDD_PATH) -> list[LogMessage]:
    assert idd_path.is_file(), f"Couldn't find IDD at '{idd_path}'"

    LogMessageClass = ErrorMessage if idd_path == IDD_PATH else WarningMessage

    idd_lines = idd_path.read_text(encoding="utf-8", errors="ignore").splitlines()

    original_units = []
    reading_mode = ReadingMode.FindTranslatedUnits

    log_messages: list[LogMessage] = []

    for line_num, line in enumerate(idd_lines, start=1):
        line = line.strip()

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
                    log_messages.append(
                        LogMessageClass(
                            tool="validate_idd_units.py",
                            filepath=idd_path,
                            line_number=line_num,
                            line=line,
                            message="Unexpected number of unit specifications",
                        )
                    )
                elif real_tokens[1] not in original_units and real_tokens[1] not in IGNORE_LIST:
                    log_messages.append(
                        LogMessageClass(
                            tool="validate_idd_units.py",
                            filepath=idd_path,
                            line_number=line_num,
                            line=line,
                            message=f"Unexpected unit type found: {real_tokens[1]}",
                        )
                    )
    return log_messages


if __name__ == "__main__":
    parser = get_base_parser(
        description="Validate IDD Units", files_arg_help=f"Files to check (if omitted, checks '{IDD_PATH}')"
    )

    args = parser.parse_args()
    files = args.files
    if not files:
        files = [IDD_PATH]

    if args.verbose:
        print(f"Checking {len(files)} files")

    if len(files) == 0:
        print("No files to check")
        raise SystemExit(0)

    if len(files) == 1:
        log_messages = validate_idd_units(idd_path=files[0])
    else:
        errors_list_of_lists = parallel_apply(func=validate_idd_units, filepaths=files)
        log_messages = flatten_list_of_lists(list_of_lists=errors_list_of_lists)

    success = report_log_messages(log_messages=log_messages, fail_threshold=LogLevel.ERROR, verbose=args.verbose)
    exit_hook(success=success)
