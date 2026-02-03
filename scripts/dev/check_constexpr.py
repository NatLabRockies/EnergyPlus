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

import re
import sys
import unittest
from pathlib import Path

from base_hook import (
    SRC_DIR,
    TST_DIR,
    ErrorMessage,
    InfoMessage,
    LogLevel,
    LogMessage,
    exit_hook,
    flatten_list_of_lists,
    get_base_parser,
    parallel_apply,
    report_log_messages,
)

SCI_NOTATION_STR = r"[+-]?\d+\.?(\d+)?e[+-]?\d+"
SCI_NOTATION_PATTERN = re.compile(SCI_NOTATION_STR)

INT_REAL_STR = r"[+-]?\d+\.?(\d+)?"
INT_REAL_PATTERN = re.compile(INT_REAL_STR)

CONST_TYPE_STR = (
    r"(const int|int const|const bool|bool const|const Real64|Real64 const|const double|double const)(?!(expr))"
)
CONST_TYPE_PATTERN = re.compile(CONST_TYPE_STR)

VAR_NAME_STR = r"[_a-zA-Z][_a-zA-Z0-9]*"
VAR_NAME_PATTERN = re.compile(VAR_NAME_STR)

CONST_NUM_PAREN_STR = (
    r"CONSTTYPE VARNAME\((INTREAL|SCI)\);".replace("CONSTTYPE", CONST_TYPE_STR)
    .replace("VARNAME", VAR_NAME_STR)
    .replace("INTREAL", INT_REAL_STR)
    .replace("SCI", SCI_NOTATION_STR)
)
CONST_NUM_PAREN_PATTERN = re.compile(CONST_NUM_PAREN_STR)

CONST_NUM_EQUAL_STR = (
    r"CONSTTYPE VARNAME = (INTREAL|SCI);".replace("CONSTTYPE", CONST_TYPE_STR)
    .replace("VARNAME", VAR_NAME_STR)
    .replace("INTREAL", INT_REAL_STR)
    .replace("SCI", SCI_NOTATION_STR)
)
CONST_NUM_EQUAL_PATTERN = re.compile(CONST_NUM_EQUAL_STR)

CONST_NUM_STR = r"(PAREN|EQUAL)".replace("PAREN", CONST_NUM_PAREN_STR).replace("EQUAL", CONST_NUM_EQUAL_STR)
CONST_NUM_PATTERN = re.compile(CONST_NUM_STR)

# CONST_NUM_WRAPPED_STR = (
#     r"CONSTTYPE VARNAME\((?!.)"
#     .replace("CONSTTYPE", CONST_TYPE_STR)
#     .replace("VARNAME", VAR_NAME_STR)
# )
# CONST_NUM_WRAPPED_PATTERN = re.compile(CONST_NUM_WRAPPED_STR)

# NOTE: Avoid matching declarations that include an initializer-list ({...})
# by adding a negative lookahead after the opening parenthesis.
ARRAY_CONST_STR = (r"(static )?Array[12345]D<(int|bool|double|Real64)> const VARNAME\((?!.*\{)").replace(
    "VARNAME", VAR_NAME_STR
)
ARRAY_CONST_PATTERN = re.compile(ARRAY_CONST_STR)

ARRAY_US_CONST_STR = (r"(static )?Array[12345]D_(int|bool|double) const VARNAME\((?!.*\{)").replace(
    "VARNAME", VAR_NAME_STR
)
ARRAY_US_CONST_PATTERN = re.compile(ARRAY_US_CONST_STR)

# ---------------------------------------------------------------------------
# Helpers to strip string literals so checks don't trigger inside strings
# ---------------------------------------------------------------------------

STRING_LITERAL_PATTERN = re.compile(
    r"""
    # Raw string literals (simple single-line heuristic)
    R"[^"]*"                           |
    # Normal double-quoted strings
    "(?:\\.|[^"\\])*"                  |
    # Normal single-quoted strings
    '(?:\\.|[^'\\])*'
    """,
    re.VERBOSE,
)


def strip_string_literals(text: str) -> str:
    """Remove C++-style string literals from a line."""
    return STRING_LITERAL_PATTERN.sub("", text)


class TestMatching(unittest.TestCase):

    def test_array_underscore_const(self):
        # match these
        yes_match = [
            "Array1D_int const A(",
            "Array1D_int const Abc(",
            "Array1D_bool const A(",
            "Array1D_bool const Abc(",
            "Array1D_double const A(",
            "Array1D_double const Abc(",
            "Array2D_int const A(",
            "Array2D_int const Abc(",
            "Array2D_bool const A(",
            "Array2D_bool const Abc(",
            "Array2D_double const A(",
            "Array2D_double const Abc(",
            "static Array1D_int const A(",
            "static Array1D_int const Abc(",
            "static Array1D_bool const A(",
            "static Array1D_bool const Abc(",
            "static Array1D_double const A(",
            "static Array1D_double const Abc(",
            "static Array2D_int const A(",
            "static Array2D_int const Abc(",
            "static Array2D_bool const A(",
            "static Array2D_bool const Abc(",
            "static Array2D_double const A(",
            "static Array2D_double const Abc(",
        ]
        for y in yes_match:
            self.assertTrue(re.match(ARRAY_US_CONST_PATTERN, y))
        # don't match these
        no_match = [
            "Array1D_int A(",
            "Array1D_int Abc(",
            "Array1D_bool A(",
            "Array1D_bool Abc(",
            "Array1D_double A(",
            "Array1D_double Abc(",
            "static Array2D_int A(",
            "static Array2D_int Abc(",
            "static Array2D_bool A(",
            "static Array2D_bool Abc(",
            "static Array2D_double A(",
            "static Array2D_double Abc(",
            # should not match these real code lines:
            "static Array1D<Real64> const OutdoorUnitInletAirDryBulbTempPLTestPoint(3, {27.5, 20.0, 18.3});",
            "static Array1D<Real64> const NetCapacityFactorPLTestPoint(3, {0.75, 0.50, 0.25});",
        ]
        for n in no_match:
            self.assertFalse(re.match(ARRAY_US_CONST_PATTERN, n))

    def test_array_const(self):
        # match these
        yes_match = [
            "Array1D<int> const A(",
            "Array1D<int> const Abc(",
            "Array1D<bool> const A(",
            "Array1D<bool> const Abc(",
            "Array1D<double> const A(",
            "Array1D<double> const Abc(",
            "Array2D<int> const A(",
            "Array2D<int> const Abc(",
            "Array2D<bool> const A(",
            "Array2D<bool> const Abc(",
            "Array2D<double> const A(",
            "Array2D<double> const Abc(",
            "static Array1D<int> const A(",
            "static Array1D<int> const Abc(",
            "static Array1D<bool> const A(",
            "static Array1D<bool> const Abc(",
            "static Array1D<double> const A(",
            "static Array1D<double> const Abc(",
            "static Array2D<int> const A(",
            "static Array2D<int> const Abc(",
            "static Array2D<bool> const A(",
            "static Array2D<bool> const Abc(",
            "static Array2D<double> const A(",
            "static Array2D<double> const Abc(",
        ]
        for y in yes_match:
            self.assertTrue(re.match(ARRAY_CONST_PATTERN, y))
        # don't match these
        no_match = [
            "Array1D<int> A(",
            "Array1D<int> Abc(",
            "Array1D<bool> A(",
            "Array1D<bool> Abc(",
            "Array1D<double> A(",
            "Array1D<double> Abc(",
            "static Array2D<int> A(",
            "static Array2D<int> Abc(",
            "static Array2D<bool> A(",
            "static Array2D<bool> Abc(",
            "static Array2D<double> A(",
            "static Array2D<double> Abc(",
            # should not match these real code lines:
            "static Array1D<Real64> const OutdoorUnitInletAirDryBulbTempPLTestPoint(3, {27.5, 20.0, 18.3});",
            "static Array1D<Real64> const NetCapacityFactorPLTestPoint(3, {0.75, 0.50, 0.25});",
        ]
        for n in no_match:
            self.assertFalse(re.match(ARRAY_CONST_PATTERN, n))

    # def test_const_num_wrapped(self):
    #     # match these
    #     yes_match = [
    #         "const int VarName(",
    #         "const Real64 VarName(",
    #         "const int A("
    #     ]
    #     for y in yes_match:
    #         self.assertTrue(re.match(const_num_wrapped_pattern, y))

    def test_const_num(self):
        # match these
        yes_match = ["const int VarName = 1;", "const int VarName(1);", "const int A(1);"]
        for y in yes_match:
            self.assertTrue(re.match(CONST_NUM_PATTERN, y))
        # don't match these
        no_match = [
            "Real64 constexpr VarName = 1.e1;",
            "constexpr int VarName = 1;",
            "Real64 constexpr VarName(1.e1);",
            "constexpr int VarName(1);",
        ]
        for n in no_match:
            self.assertFalse(re.match(CONST_NUM_PATTERN, n))

    def test_const_num_equal(self):
        # match these
        yes_match = [
            "const int VarName = 1;",
            "const int VarName = 1.0;",
            "const int VarName = 1.0e1;",
            "const int VarName = 1.e1;",
            "int const VarName = 1;",
            "int const VarName = 1.0;",
            "int const VarName = 1.0e1;",
            "int const VarName = 1.e1;",
            "const bool VarName = 1;",
            "const bool VarName = 1.0;",
            "const bool VarName = 1.0e1;",
            "const bool VarName = 1.e1;",
            "bool const VarName = 1;",
            "bool const VarName = 1.0;",
            "bool const VarName = 1.0e1;",
            "bool const VarName = 1.e1;",
            "const double VarName = 1;",
            "const double VarName = 1.0;",
            "const double VarName = 1.0e1;",
            "const double VarName = 1.e1;",
            "double const VarName = 1;",
            "double const VarName = 1.0;",
            "double const VarName = 1.0e1;",
            "double const VarName = 1.e1;",
            "const Real64 VarName = 1;",
            "const Real64 VarName = 1.0;",
            "const Real64 VarName = 1.0e1;",
            "const Real64 VarName = 1.e1;",
            "Real64 const VarName = 1;",
            "Real64 const VarName = 1.0;",
            "Real64 const VarName = 1.0e1;",
            "Real64 const VarName = 1.e1;",
        ]
        for y in yes_match:
            self.assertTrue(re.match(CONST_NUM_EQUAL_PATTERN, y))
        # don't match these
        no_match = ["Real64 constexpr VarName = 1.e1;", "constexpr int VarName = 1;"]
        for n in no_match:
            self.assertFalse(re.match(CONST_NUM_EQUAL_PATTERN, n))

    def test_const_num_paren(self):
        # match these
        yes_match = [
            "const int VarName(1);",
            "const int VarName(1.0);",
            "const int VarName(1.0e1);",
            "const int VarName(1.e1);",
            "int const VarName(1);",
            "int const VarName(1.0);",
            "int const VarName(1.0e1);",
            "int const VarName(1.e1);",
            "const bool VarName(1);",
            "const bool VarName(1.0);",
            "const bool VarName(1.0e1);",
            "const bool VarName(1.e1);",
            "bool const VarName(1);",
            "bool const VarName(1.0);",
            "bool const VarName(1.0e1);",
            "bool const VarName(1.e1);",
            "const double VarName(1);",
            "const double VarName(1.0);",
            "const double VarName(1.0e1);",
            "const double VarName(1.e1);",
            "double const VarName(1);",
            "double const VarName(1.0);",
            "double const VarName(1.0e1);",
            "double const VarName(1.e1);",
            "const Real64 VarName(1);",
            "const Real64 VarName(1.0);",
            "const Real64 VarName(1.0e1);",
            "const Real64 VarName(1.e1);",
            "Real64 const VarName(1);",
            "Real64 const VarName(1.0);",
            "Real64 const VarName(1.0e1);",
            "Real64 const VarName(1.e1);",
        ]
        for y in yes_match:
            self.assertTrue(re.match(CONST_NUM_PAREN_PATTERN, y))
        # don't match these
        no_match = ["Real64 constexpr VarName(1.e1);", "constexpr int VarName(1);"]
        for n in no_match:
            self.assertFalse(re.match(CONST_NUM_PAREN_PATTERN, n))

    def test_var_name(self):
        # match these
        yes_match = [
            "VarName",
            "VarName_",
            "_VarName_",
            "_VarName",
            "_VarName123",
            "VarName123",
            "VarName123_",
            "_VarName123_",
        ]
        for y in yes_match:
            self.assertTrue(re.match(VAR_NAME_PATTERN, y))
        # don't match these
        no_match = ["123", "9.81", "9.81e0"]
        for n in no_match:
            self.assertFalse(re.match(VAR_NAME_PATTERN, n))

    def test_const_type(self):
        # match these
        yes_match = [
            "const int",
            "const bool",
            "const double",
            "const Real64",
            "int const",
            "bool const",
            "double const",
            "Real64 const",
        ]
        for y in yes_match:
            self.assertTrue(re.match(CONST_TYPE_PATTERN, y))
        # don't match these
        no_match = [
            "constexpr int",
            "constexpr bool",
            "constexpr double",
            "constexpr Real64",
            "int constexpr",
            "bool constexpr",
            "double constexpr",
            "Real64 constexpr",
        ]
        for n in no_match:
            self.assertFalse(re.match(CONST_TYPE_PATTERN, n))

    def test_int_real(self):
        # match these
        yes_match = ["1", "+1", "-1", "1.0", "+1.0", "-1.0"]
        for y in yes_match:
            self.assertTrue(re.match(INT_REAL_PATTERN, y))
        # don't match these
        no_match = ["Var", "Var123"]
        for n in no_match:
            self.assertFalse(re.match(INT_REAL_PATTERN, n))

    def test_sci_notation(self):
        # match these
        yes_match = [
            "9.81e0",
            "9.81e-0",
            "9.81e+0",
            "+9.81e0",
            "+9.81e-0",
            "+9.81e+0",
            "-9.81e0",
            "-9.81e-0",
            "-9.81e+0",
            "9.e0",
            "9.e-0",
            "9.e+0",
            "+9.e0",
            "+9.e-0",
            "+9.e+0",
            "-9.e0",
            "-9.e-0",
            "-9.e+0",
        ]
        for y in yes_match:
            self.assertTrue(re.match(SCI_NOTATION_PATTERN, y))
        # don't match these
        no_match = ["VarName"]
        for n in no_match:
            self.assertFalse(re.match(SCI_NOTATION_PATTERN, n))


def constexpr_check(filepath: Path) -> list[LogMessage]:
    # Keep original lines (for reporting), but strip leading/trailing whitespace
    raw_lines = filepath.read_text(encoding="utf-8").splitlines()
    lines = [x.strip() for x in raw_lines]

    bracket_count = 0
    errors: list[LogMessage] = []

    for idx, line in enumerate(lines, start=1):

        # skip blank lines
        if line == "":
            continue

        # skip comment lines (full-line C++ style)
        if line.startswith("//"):
            continue

        # strip trailing comments (simple heuristic)
        if "//" in line:
            tokens = line.split("//", 1)
            line = tokens[0].strip()

        # remove string literals so we don't match inside them
        line_no_strings = strip_string_literals(line)

        # evaluate patterns on code-without-strings
        re_match_1 = re.match(CONST_NUM_PATTERN, line_no_strings)
        re_match_2 = re.match(ARRAY_CONST_PATTERN, line_no_strings)
        re_match_3 = re.match(ARRAY_US_CONST_PATTERN, line_no_strings)

        if (re_match_1 or re_match_2 or re_match_3) and bracket_count == 0:
            errors.append(
                ErrorMessage(
                    tool="check_constexpr",
                    filepath=filepath,
                    line_number=idx,
                    line=line,  # report the non-comment original line
                    message="Use 'constexpr' instead of 'const' for variable declaration",
                )
            )

        # count brackets in the code portion (not in strings)
        bracket_count += line_no_strings.count("(")
        bracket_count -= line_no_strings.count(")")

    return errors


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "test":
        del sys.argv[1:]
        unittest.main(exit=False, verbosity=0)

    parser = get_base_parser(description="Check Constexpr Usage")
    args = parser.parse_args()

    exts = {".cc", ".hh"}
    if args.files:
        n_ori = len(args.files)
        files = [f for f in args.files if f.suffix in exts]
        if args.verbose:
            print(f"Checking {len(files)} of {n_ori} specified files")
    else:
        files = []
        for d in [SRC_DIR, TST_DIR]:
            for e in exts:
                files += list(d.glob(f"**/*{e}"))
        if args.verbose:
            print(f"Checking {len(files)} files")

    log_messages = flatten_list_of_lists(list_of_lists=parallel_apply(func=constexpr_check, filepaths=files))
    success = report_log_messages(log_messages=log_messages, fail_threshold=LogLevel.ERROR, verbose=args.verbose)
    exit_hook(success=success)
