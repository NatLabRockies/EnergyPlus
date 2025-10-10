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

import os
import sys
import unittest
from pathlib import Path
from typing import List

verbose = True


def process_all_format_lines(f_path: Path, lines: list, fmt_line_nos: list) -> int:
    num_errors = 0

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
            if verbose:
                print(f"File: {str(f_path)}, line: {line_no + 1}, Format '{fmt_str}' has unbalanced curly braces.")
            num_errors += 1

        # check for unbalanced curly braces placeholders and arguments
        if fmt_str.count("{") != len(args):
            if verbose:
                print(f"File: {str(f_path)}, line: {line_no + 1}, Format '{fmt_str}' arg count {args} is not matched.")
            num_errors += 1

        # check for when no args are parsed
        if len(args) == 0:
            if verbose:
                print(f"File: {str(f_path)}, line: {line_no + 1}, Format '{fmt_str}' has no arguments. Remove format.")
            num_errors += 1

    return num_errors


def get_sorted_file_list(search_path: Path) -> List[Path]:
    files_to_search = []
    for p in [search_path]:
        for root, _, files in os.walk(p):
            for file in files:
                f_path = Path(root) / Path(file)
                f_extension = f_path.suffix
                if f_extension == ".hh":
                    files_to_search.append(f_path)
                elif f_extension == ".cc":
                    files_to_search.append(f_path)
    files_to_search.sort()
    return files_to_search


def get_format_line_numbers_from_lines(lines: List[str]) -> List[int]:
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


def check_format_strings(search_path: Path) -> int:
    num_errors = 0
    files_to_search = get_sorted_file_list(search_path)
    for f_path in files_to_search:
        with open(f_path, "r") as f:
            lines = f.readlines()
            lines = [x.strip() for x in lines]
        format_line_nos = get_format_line_numbers_from_lines(lines)
        num_errors += process_all_format_lines(f_path, lines, format_line_nos)
    return num_errors


class TestFormatCheck(unittest.TestCase):
    def test_valid_format_chunk(self):
        self.assertEqual(
            0,  # number of errors encountered
            process_all_format_lines(
                Path("/dummy/path"),
                ["line 1", "line 2", 'format("hi{}", varName);'],  # actual file content lines
                [2],  # lines containing actual format statements
            ),
        )

    def test_invalid_format_chunk(self):
        self.assertEqual(
            1,  # number of errors encountered
            process_all_format_lines(
                Path("/dummy/path"),
                ["line 1", "line 2", 'format("hi{", varName);'],  # actual file content lines
                [2],  # lines containing actual format statements
            ),
        )


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "test":
        del sys.argv[1:]
        unittest.main(exit=False, verbosity=0)

    root_path = Path(__file__).parent.parent.parent
    src_path = root_path / "src" / "EnergyPlus"
    errors_found = check_format_strings(src_path)
    tst_path = root_path / "tst" / "EnergyPlus"
    errors_found += check_format_strings(tst_path)
    if errors_found > 0:
        sys.exit(1)
