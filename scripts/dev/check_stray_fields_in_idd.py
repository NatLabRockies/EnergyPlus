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

import re
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

RE_FIELD = re.compile(r"\s*\\(\w+)\s*$")


def check_for_stray_fields(idd_path: Path = IDD_PATH) -> list[LogMessage]:
    """
    Verifies that there aren't any stray fields in IDD that could cause
    parsing problems

    Args:
    -----
    idd_path (str): path to the idd file to check

    Returns:
    --------
    offending_lines (list of dict): one entry per offending line,
    each entry is a dict that can be consumed by decent_ci
    """
    assert idd_path.is_file(), f"Couldn't find IDD at '{idd_path}'"

    LogMessageClass = ErrorMessage if idd_path == IDD_PATH else WarningMessage

    lines = idd_path.read_text().splitlines()

    exclude = ["autosizable", "autocalculatable", "retaincase"]

    log_messages: list[LogMessage] = []

    for line_num, line in enumerate(lines, start=1):
        m = RE_FIELD.match(line)
        if m:
            field = m.groups()[0]
            if field not in exclude:
                log_messages.append(
                    LogMessageClass(
                        tool="check_stray_fields_in_idd",
                        filepath=idd_path,
                        line_number=line_num,
                        line=line,
                        message=rf"Stray field \{field}",
                    )
                )

    return log_messages


if __name__ == "__main__":
    parser = get_base_parser(
        description="Check Stray Fields in IDD", files_arg_help=f"Files to check (if omitted, checks '{IDD_PATH}')"
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
        log_messages = check_for_stray_fields(idd_path=files[0])
    else:
        errors_list_of_lists = parallel_apply(func=check_for_stray_fields, filepaths=files)
        log_messages = flatten_list_of_lists(list_of_lists=errors_list_of_lists)

    success = report_log_messages(log_messages=log_messages, fail_threshold=LogLevel.ERROR, verbose=args.verbose)
    exit_hook(success=success)
