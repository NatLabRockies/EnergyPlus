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

import os
import sys
import unittest
from math import floor, log10
from typing import Set, TextIO, Tuple

"""Processes a diff between DD and ReverseDD runs, returning 0 for match and 1 for differences or error"""


def process_command_arguments() -> str:
    """Verify the number of command line arguments is correct then return the relevant ones"""
    if len(sys.argv) != 2:
        print(f"Invalid arguments passed to {sys.argv[0]}")
        sys.exit(1)
    return sys.argv[1]


def configure_root_dirs(test_dir: str) -> Tuple[str, str]:
    """Set up the test directories based on prescribed names"""
    return os.path.join(test_dir, "Regular"), os.path.join(test_dir, "Reversed")


def configure_paths(b_dir: str, r_dir: str, filename: str) -> Tuple[str, str]:
    """Set all the relative file paths for the base, reversed, and reverse-backup files"""
    return (
        os.path.join(b_dir, f"{filename}.csv"),
        os.path.join(r_dir, f"{filename}.csv"),
    )


def round_to_sig_digits(x, digits):
    """Rounds a floating point number to a specific number of significant digits"""
    # strip the negative sign
    absolute_value_of_x = abs(x)
    # the base 10 logarithm can tell us how many decimal places to move to get a single digit whole number
    log_x = log10(absolute_value_of_x)
    # scrape off the mantissa to get just an integer number of decimal places to move to get a single digit whole number
    log_characteristic = floor(log_x)
    # then calculate the place to round to
    round_argument = digits - int(log_characteristic) - 1
    # pass that to the built-in round function
    return round(x, round_argument)


def get_processed_rows(file_object: TextIO) -> Set[str]:
    """Process each row by rounding individual tokens and then building back into row strings"""
    rows = set()
    for line_num, li in enumerate(file_object.readlines()):
        if line_num == 0:
            rows.add(li)
            continue
        tokens = []
        for i, v in enumerate(li.split(",")):
            if i == 0:
                tokens.append(v)
            try:
                number = float(v)  # will throw ValueError if not numeric
                rounded = round_to_sig_digits(number, 2)
                tokens.append(rounded)
            except ValueError:
                continue
        rows.add(",".join([str(x) for x in tokens]))
    return rows


def files_match(base_file_path: str, mod_file_path: str) -> bool:
    """Gather processed rows from each file and then return the comparison"""
    with open(base_file_path) as f_base:
        with open(mod_file_path) as f_reversed:
            base_rows = get_processed_rows(f_base)
            reversed_rows = get_processed_rows(f_reversed)
            # for br in base_rows:
            #     if br in reversed_rows:
            #         pass
            #     else:
            #         print(f"Found bad base row: {br}")
            # for mr in reversed_rows:
            #     if mr in base_rows:
            #         pass
            #     else:
            #         print(f"Found bad mod row:  {mr}")
            return base_rows == reversed_rows


def both_csv_files_missing(b: str, r: str) -> bool:
    base_missing = not os.path.exists(b)
    rev_missing = not os.path.exists(r)
    return base_missing and rev_missing


def main():
    """Handles the main entry point into this script"""

    # validate and get command line arguments
    reverse_dd_test_dir = process_command_arguments()

    # configure up all the paths
    base_dir, reversed_dir = configure_root_dirs(reverse_dd_test_dir)

    # configure paths for both csv and mtr outputs
    base_csv, reversed_csv = configure_paths(base_dir, reversed_dir, "eplusout")
    base_mtr, rev_mtr = configure_paths(base_dir, reversed_dir, "eplusmtr")

    if both_csv_files_missing(base_csv, reversed_csv):
        return 0  # assume everything is fine if the CSV is missing in *both* builds

    # do comparison of the outputs
    csv_match = files_match(base_csv, reversed_csv)
    mtr_match = files_match(base_mtr, rev_mtr) if os.path.exists(rev_mtr) else True

    # report the results of the comparisons
    if csv_match and mtr_match:
        return 0
    return 1


if __name__ == "__main__":
    sys.exit(main())


class Test1(unittest.TestCase):
    def test_function(self):
        f1 = "/eplus/repos/8eplus/builds/r/testfiles_ReverseDD/1ZoneUncontrolled/Regular/eplusout.csv"
        f2 = "/eplus/repos/8eplus/builds/r/testfiles_ReverseDD/1ZoneUncontrolled/Reversed/eplusout.csv"
        self.assertTrue(files_match(f1, f2))
