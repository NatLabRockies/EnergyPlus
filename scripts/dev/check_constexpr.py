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

import os
import re
import sys
import unittest
from pathlib import Path

sci_notation_str = r"[+-]?\d+\.?(\d+)?e[+-]?\d+"
sci_notation_pattern = re.compile(sci_notation_str)

int_real_str = r"[+-]?\d+\.?(\d+)?"
int_real_pattern = re.compile(int_real_str)

const_type_str = (
    r"(const int|int const|const bool|bool const|const Real64|Real64 const|const double|double const)(?!(expr))"
)
const_type_pattern = re.compile(const_type_str)

var_name_str = r"[_a-zA-Z][_a-zA-Z0-9]*"
var_name_pattern = re.compile(var_name_str)

const_num_paren_str = r"CONSTTYPE VARNAME\((INTREAL|SCI)\);"
const_num_paren_str = const_num_paren_str.replace("CONSTTYPE", const_type_str)
const_num_paren_str = const_num_paren_str.replace("VARNAME", var_name_str)
const_num_paren_str = const_num_paren_str.replace("INTREAL", int_real_str)
const_num_paren_str = const_num_paren_str.replace("SCI", sci_notation_str)
const_num_paren_pattern = re.compile(const_num_paren_str)

const_num_equal_str = r"CONSTTYPE VARNAME = (INTREAL|SCI);"
const_num_equal_str = const_num_equal_str.replace("CONSTTYPE", const_type_str)
const_num_equal_str = const_num_equal_str.replace("VARNAME", var_name_str)
const_num_equal_str = const_num_equal_str.replace("INTREAL", int_real_str)
const_num_equal_str = const_num_equal_str.replace("SCI", sci_notation_str)
const_num_equal_pattern = re.compile(const_num_equal_str)

const_num_str = r"(PAREN|EQUAL)"
const_num_str = const_num_str.replace("PAREN", const_num_paren_str)
const_num_str = const_num_str.replace("EQUAL", const_num_equal_str)
const_num_pattern = re.compile(const_num_str)

# const_num_wrapped_str = r"CONSTTYPE VARNAME\((?!.)"
# const_num_wrapped_str = const_num_wrapped_str.replace("CONSTTYPE", const_type_str)
# const_num_wrapped_str = const_num_wrapped_str.replace("VARNAME", var_name_str)
# const_num_wrapped_pattern = re.compile(const_num_wrapped_str)

array_const_str = r"(static )?Array[12345]D<(int|bool|double|Real64)> const VARNAME\("
array_const_str = array_const_str.replace("VARNAME", var_name_str)
array_const_pattern = re.compile(array_const_str)

array_us_const_str = r"(static )?Array[12345]D_(int|bool|double) const VARNAME\("
array_us_const_str = array_us_const_str.replace("VARNAME", var_name_str)
array_us_const_pattern = re.compile(array_us_const_str)


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
            self.assertTrue(re.match(array_us_const_pattern, y))
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
        ]
        for n in no_match:
            self.assertFalse(re.match(array_us_const_pattern, n))

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
            self.assertTrue(re.match(array_const_pattern, y))
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
        ]
        for n in no_match:
            self.assertFalse(re.match(array_const_pattern, n))

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
            self.assertTrue(re.match(const_num_pattern, y))
        # don't match these
        no_match = [
            "Real64 constexpr VarName = 1.e1;",
            "constexpr int VarName = 1;",
            "Real64 constexpr VarName(1.e1);",
            "constexpr int VarName(1);",
        ]
        for n in no_match:
            self.assertFalse(re.match(const_num_pattern, n))

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
            self.assertTrue(re.match(const_num_equal_pattern, y))
        # don't match these
        no_match = ["Real64 constexpr VarName = 1.e1;", "constexpr int VarName = 1;"]
        for n in no_match:
            self.assertFalse(re.match(const_num_equal_pattern, n))

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
            self.assertTrue(re.match(const_num_paren_pattern, y))
        # don't match these
        no_match = ["Real64 constexpr VarName(1.e1);", "constexpr int VarName(1);"]
        for n in no_match:
            self.assertFalse(re.match(const_num_paren_pattern, n))

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
            self.assertTrue(re.match(var_name_pattern, y))
        # don't match these
        no_match = ["123", "9.81", "9.81e0"]
        for n in no_match:
            self.assertFalse(re.match(var_name_pattern, n))

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
            self.assertTrue(re.match(const_type_pattern, y))
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
            self.assertFalse(re.match(const_type_pattern, n))

    def test_int_real(self):
        # match these
        yes_match = ["1", "+1", "-1", "1.0", "+1.0", "-1.0"]
        for y in yes_match:
            self.assertTrue(re.match(int_real_pattern, y))
            # don't match these
        no_match = ["Var", "Var123"]
        for n in no_match:
            self.assertFalse(re.match(int_real_pattern, n))

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
            self.assertTrue(re.match(sci_notation_pattern, y))
            # don't match these
        no_match = ["VarName"]
        for n in no_match:
            self.assertFalse(re.match(sci_notation_pattern, n))


def constexpr_check(search_path: Path) -> int:
    """Checks for missing constexpr tags, returns the number of tags found that should be changed"""
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
        with open(file, "r") as f:
            lines = f.readlines()

        lines = [x.strip() for x in lines]

        bracket_count = 0

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

            re_match_1 = re.match(const_num_pattern, line)
            re_match_2 = re.match(array_const_pattern, line)
            re_match_3 = re.match(array_us_const_pattern, line)
            if (re_match_1 or re_match_2 or re_match_3) and bracket_count == 0:
                print("ERROR: 'const' found, convert to 'constexpr'")
                s = f"{file.name}: {idx + 1} - {line}"
                print(s)
                num_errors += 1

            # count brackets in line
            bracket_count += line.count("(")
            bracket_count -= line.count(")")

    return num_errors


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "test":
        del sys.argv[1:]
        unittest.main(exit=False, verbosity=0)
    root_path = Path(__file__).parent.parent.parent
    src_path = root_path / "src" / "EnergyPlus"
    tst_path = root_path / "tst" / "EnergyPlus" / "unit"
    errors_found = constexpr_check(src_path) + constexpr_check(tst_path)
    if errors_found > 0:
        raise sys.exit(1)
