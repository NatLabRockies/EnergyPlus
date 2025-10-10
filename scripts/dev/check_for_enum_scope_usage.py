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

# This script is used to find enum declarations that are declared specifically inside header files,
# and only used in a single source file.  This indicates enums that should be declared at the top of the source
# file rather than exported through the header

import unittest
from os import walk
from pathlib import Path
from re import finditer
from sys import argv, exit
from typing import List, Set

verbose = False


class PotentialUsage:
    def __init__(self, file_path: Path, line_number: int, line: str):
        self.file_path: Path = file_path
        self.line_number: int = line_number
        self.line: str = line


class EnumDeclaration:
    def __init__(self, file_path: Path, line_number: int, enum_name: str):
        self.file_path: Path = file_path
        self.line_number: int = line_number
        self.enum_name: str = enum_name.strip()
        self.usages: List[PotentialUsage] = []

    def describe(self):
        return f"{self.file_path.name} : {self.line_number + 1} :: {self.enum_name} ({len(self.usages)} usages)"

    def __str__(self):
        return self.describe()


class SingleHeaderFile:
    def __init__(self, file_path: Path):
        self.full_path: Path = file_path
        self.enum_declarations: List[EnumDeclaration] = []

    def run(self):
        with open(self.full_path, "r") as f:
            self.process_lines(f.readlines())

    def process_lines(self, lines: List[str]):
        for idx, line in enumerate(lines):

            # remove trailing comments, which could include the whole line
            if "//" in line:
                tokens = line.split("//")
                line = tokens[0].strip()

            # skip blank lines
            if line == "":
                continue

            # find enum class, if it doesn't exist, just ignore
            enum_index = line.find("enum class")
            if enum_index == -1:
                continue

            # ok, so now we have an enum class, but we can get rid of the enum class portion
            enum_name = line[enum_index + 11 :].strip()

            # find open brace, if it doesn't exist, assume it's on the next line
            open_brace_index = enum_name.find("{")
            if open_brace_index != -1:
                enum_name = enum_name[:open_brace_index]

            # find enum subtype specifier, if it doesn't exist, assume it's on the next line
            type_delimiter_index = enum_name.find(":")
            if type_delimiter_index != -1:
                enum_name = enum_name[:type_delimiter_index].strip()

            # finally, if this is an enum class forward declaration, we should skip it
            if enum_name.endswith(";"):
                continue

            self.enum_declarations.append(EnumDeclaration(self.full_path, idx, enum_name))


class EnumScopeEvaluator:
    def __init__(self, source_dir: Path, unit_test_dir: Path):
        self.source_dir: Path = source_dir
        self.test_dir: Path = unit_test_dir
        self.error_count = 0
        self.all_enum_declarations = []
        self.all_source_file_contents = dict()

    def run(self):
        if verbose:
            print("Creating list of files for searching")
        source_files_to_search = []
        header_files_to_search = []
        for p in [self.source_dir, self.test_dir]:
            for root, dirs, files in walk(p):
                for file in files:
                    f_path = Path(root) / Path(file)
                    f_extension = f_path.suffix
                    if f_extension == ".hh":
                        header_files_to_search.append(f_path)
                    elif f_extension == ".cc":
                        source_files_to_search.append(f_path)
        source_files_to_search.sort()
        header_files_to_search.sort()

        if verbose:
            print("Processing header file data")
        for file in header_files_to_search:
            s = SingleHeaderFile(file)
            s.run()
            self.all_enum_declarations.extend(s.enum_declarations)

        if verbose:
            print("Processing source file into lists of lines")
        for file in source_files_to_search + header_files_to_search:
            with open(file, "r") as f:
                original_lines = f.readlines()
            new_lines = list()
            for line in original_lines:
                if "//" in line:
                    tokens = line.split("//")
                    line = tokens[0].strip()
                if "::" not in line and " " not in line:
                    line = ""  # ignore lines without namespace qualifier to save space for later searching
                new_lines.append(line.strip())
            self.all_source_file_contents[file] = new_lines

        if verbose:
            print("Checking source file lines for usages")
        for file_path, file_lines in self.all_source_file_contents.items():
            for line_num, line in enumerate(file_lines):
                self.check_single_line_for_usage(file_path, line_num, line)

        if verbose:
            print("Reconciling usages")
        apparent_enums_in_only_one_source_file: List[str] = list()
        apparent_enums_in_zero_source_files: List[str] = list()
        for e in self.all_enum_declarations:
            if len(e.usages) == 0:
                apparent_enums_in_zero_source_files.append(e.describe())
            unique_files_in_usages: Set[str] = set()
            # exceptions listed by <FILE>:<ENUM NAME>
            exceptions = ["DataGlobalConstants.hh:ePollutant", "RefrigeratedCase.hh:CriticalType"]
            if f"{e.file_path.name}:{e.enum_name}" not in exceptions:
                for u in e.usages:
                    unique_files_in_usages.add(u.file_path.name)
                if len(unique_files_in_usages) == 1:
                    apparent_enums_in_only_one_source_file.append(
                        f"{e.describe()} in {next(iter(unique_files_in_usages))}"
                    )

        if verbose:
            print("Reporting results")
        if len(apparent_enums_in_zero_source_files) > 0:
            print(f"Detected {len(apparent_enums_in_zero_source_files)} enums in ZERO source files:")
            for e in apparent_enums_in_zero_source_files:
                print(f" - {e}")
        if len(apparent_enums_in_only_one_source_file) > 0:
            print(f"\nDetected {len(apparent_enums_in_only_one_source_file)} enums in ONE source file:")
            for e in apparent_enums_in_only_one_source_file:
                print(f" - {e}")

        self.error_count = len(apparent_enums_in_zero_source_files) + len(apparent_enums_in_only_one_source_file)

    def check_single_line_for_usage(self, file_path: Path, line_num: int, line: str):
        # search for usages of Enum:: first
        for match in finditer(r"(\w*::)", line):
            g = match.group(1)
            if g != "std::":
                scope = g[:-2]
                for e in self.all_enum_declarations:
                    if e.enum_name == scope:
                        e.usages.append(PotentialUsage(file_path, line_num, line))
        # also search for declarations of the enum type as in EnumType e;
        if " " in line:
            for e in self.all_enum_declarations:
                if f"{e.enum_name} " in line:
                    e.usages.append(PotentialUsage(file_path, line_num, line))
        # finally it might also be used as a template type:
        if ">" in line:
            for e in self.all_enum_declarations:
                if f"{e.enum_name}>" in line:
                    e.usages.append(PotentialUsage(file_path, line_num, line))


class TestEnumStuff(unittest.TestCase):
    def test_process_usage(self):
        e = EnumScopeEvaluator(Path(), Path())
        e.all_enum_declarations.append(EnumDeclaration(Path(), 0, "OutConvClass"))
        e.check_single_line_for_usage(
            Path(),
            0,
            "state.dataSurface->SurfOutConvClassification(SurfNum) = ConvectionConstants::OutConvClass::RoofStable;",
        )
        self.assertEqual(1, len(e.all_enum_declarations))
        self.assertEqual(1, len(e.all_enum_declarations[0].usages))

    def test_line_processor(self):
        contents = [
            "// Hello",
            "",
            "// commented enum class Whatever {",
            " enum class Hello : int {",
            " enum class Types",
            "   enum class Token : size_t",
            " {",
            "  NONE = 0",
        ]
        h = SingleHeaderFile(Path())
        h.process_lines(contents)
        self.assertEqual(3, len(h.enum_declarations))


if __name__ == "__main__":
    if len(argv) > 1 and argv[1] == "test":
        del argv[1:]
        unittest.main(exit=False, verbosity=0)
    root_path = Path(__file__).parent.parent.parent
    src_path = root_path / "src" / "EnergyPlus"
    tst_path = root_path / "tst" / "EnergyPlus" / "unit"
    evaluator = EnumScopeEvaluator(src_path, tst_path)
    evaluator.run()
    if evaluator.error_count > 0:
        raise exit(1)
