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

import sys
import unittest
from collections import Counter
from pathlib import Path

from base_hook import (
    SRC_DIR,
    TST_DIR,
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

DIRS_TO_SEARCH = [SRC_DIR, TST_DIR]
EXTENSIONS = {".hh", ".cc"}


def process_all_format_lines(filepath: Path, lines: list) -> list[LogMessage]:

    fmt_line_nos = get_format_line_numbers_from_lines(lines)

    log_messages: list[LogMessage] = []

    # process format lines
    for line_no in fmt_line_nos:
        line = ""
        line_no_counter = line_no

        # collect multiline statements
        while True:
            line += lines[line_no_counter]
            # get rid of escaped parentheses
            line = line.replace('\\"', "")
            if line[-1] != ";":
                line_no_counter += 1
            else:
                break

        # replace parens '""' next to each other in case of wrapped lines
        line = line.replace('""', "")

        # throw away front
        tokens = line.split("format", 1)
        line = tokens[1]

        # process the rest
        num_open_paren = 0
        num_close_paren = 0
        num_quote = 0
        start_fmt = 0
        end_fmt = 0
        fmt_str = ""
        args = ""
        for idx_fmt, c in enumerate(line):

            # get fmt string
            if c == '"':
                num_quote += 1
                if num_quote == 1:
                    start_fmt = idx_fmt + 1
                    continue
                elif num_quote == 2:
                    end_fmt = idx_fmt
                    fmt_str = line[start_fmt:end_fmt]
                    continue

            # skip if we're inside the fmt string
            if 0 < num_quote < 2:
                continue

            # find the end of the args
            if c == "(":
                num_open_paren += 1
            elif c == ")":
                num_close_paren += 1

            # found full args string
            if (num_open_paren - num_close_paren) == 0:
                args_str = line[end_fmt + 2 : idx_fmt]
                args_str = args_str.strip()
                num_quote = 0
                args = []
                args_idx = 0

                # partial process args
                for a in args_str:
                    if (a == '"') and (num_quote > 0):
                        num_quote -= 1
                        continue
                    elif (a == '"') and (num_quote == 0):
                        num_quote += 1

                    if (a == ",") and (num_quote == 0):
                        args_idx += 1
                        continue

                    try:
                        args[args_idx] += a
                    except IndexError:
                        args.append(a)

                break

        # fmt strings need further processing for escaped curly braces
        fmt_str = fmt_str.replace("{{", "")
        fmt_str = fmt_str.replace("}}", "")

        # args need further processing to recombine things that shouldn't have been separated
        while True:
            args_copy = args
            for idx_args, a in enumerate(args):
                if a.count("(") != a.count(")"):
                    args_copy[idx_args : idx_args + 2] = [",".join(args[idx_args : idx_args + 2])]
                    break
            args = args_copy
            if all([y == 0 for y in [x.count("(") - x.count(")") for x in args]]):
                break

        # Finally, we can do some error checking.
        # check for unbalanced curly braces
        if fmt_str.count("{") != fmt_str.count("}"):
            log_messages.append(
                ErrorMessage(
                    tool="check_format_strings",
                    filepath=filepath,
                    line_number=line_no + 1,
                    line=line,
                    message=f"Format '{fmt_str}' has unbalanced curly braces.",
                )
            )

        # check for unbalanced curly braces placeholders and arguments
        if fmt_str.count("{") != len(args):
            log_messages.append(
                ErrorMessage(
                    tool="check_format_strings",
                    filepath=filepath,
                    line_number=line_no + 1,
                    line=line,
                    message=f"Format '{fmt_str}' arg count {len(args)} is not matched.",
                )
            )

        # check for when no args are parsed
        if len(args) == 0:
            log_messages.append(
                ErrorMessage(
                    tool="check_format_strings",
                    filepath=filepath,
                    line_number=line_no + 1,
                    line=line,
                    message=f"Format '{fmt_str}' has no arguments. Remove format.",
                )
            )

    return log_messages


def get_format_line_numbers_from_lines(lines: list[str]) -> list[int]:
    format_line_nos = []
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
        # find 'format' line numbers first
        if 'format("' in line:
            format_line_nos.append(idx)
    return format_line_nos


def check_format_strings(filepath: Path) -> list[LogMessage]:
    lines = [line.strip() for line in filepath.read_text().splitlines()]
    return process_all_format_lines(filepath=filepath, lines=lines)


class TestFormatCheck(unittest.TestCase):
    def test_valid_format_chunk(self):
        log_messages = process_all_format_lines(
            SRC_DIR / "Dummyfile.cc",
            lines=["line 1", "line 2", 'format("hi{}", varName);'],  # actual file content lines
        )
        self.assertEqual(len(log_messages), 0)

    def test_invalid_format_chunk(self):
        log_messages = process_all_format_lines(
            filepath=SRC_DIR / "Dummyfile.cc",
            lines=["line 1", "line 2", 'format("hi{", varName);'],  # actual file content lines
        )
        self.assertEqual(len(log_messages), 1)
        self.assertEqual(log_messages[0].message, "Format 'hi{' has unbalanced curly braces.")


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "test":
        del sys.argv[1:]
        unittest.main(exit=True, verbosity=0)

    parser = get_base_parser(description="Find Included CC files")
    args = parser.parse_args()
    if args.files:
        n_ori = len(args.files)
        files = [f for f in args.files if f.suffix in EXTENSIONS and any(f.is_relative_to(d) for d in DIRS_TO_SEARCH)]
        if args.verbose:
            print(f"Checking {len(files)} of {n_ori} specified files")
    else:
        files = []
        for d in DIRS_TO_SEARCH:
            files += collect_files(base_dir=d, extensions=EXTENSIONS, recursive=True, dirs_to_skip=[])
        if args.verbose:
            counter_info = ", ".join([f"{num} {ext}" for ext, num in Counter([f.suffix for f in files]).items()])
            print(f"Checking {len(files)} files: {counter_info}")

    errors_list_of_lists = parallel_apply(func=check_format_strings, filepaths=files)
    log_messages = flatten_list_of_lists(list_of_lists=errors_list_of_lists)
    success = report_log_messages(log_messages=log_messages, fail_threshold=LogLevel.ERROR, verbose=args.verbose)
    exit_hook(success=success)
