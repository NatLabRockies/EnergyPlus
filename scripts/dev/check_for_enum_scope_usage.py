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

import re
import sys
import time
import unittest
from collections import Counter
from pathlib import Path
from typing import List, Set

from base_hook import (
    SRC_DIR,
    TST_DIR,
    ErrorMessage,
    LogLevel,
    LogMessage,
    WarningMessage,
    collect_files,
    exit_hook,
    flatten_list_of_lists,
    get_base_parser,
    parallel_apply,
    report_log_messages,
)

DIRS_TO_SEARCH = [SRC_DIR, TST_DIR / "unit"]
EXTENSIONS = {".hh", ".cc"}

RE_ENUM_USAGE_ORI = re.compile(r"(\w*::)")
RE_ENUM_USAGE = re.compile(
    r"\b(?!(?:std|ObjexxFCL)::)(?:(?:\w+)::)*(?P<EnumClass>\w+)::(?P<EnumValue>\w+)\b(?!::)(?!\s*\()(?!\")"
)


class PotentialUsage:
    def __init__(self, scope: str, filepath: Path, line_number: int, line: str):
        self.scope: str = scope
        self.filepath: Path = filepath
        self.line_number: int = line_number
        self.line: str = line

    def __repr__(self):
        return f"'{self.scope}' in {self.filepath.name}:{self.line_number} :: {self.line.strip()}"


class EnumDeclaration:
    def __init__(self, filepath: Path, line_number: int, enum_name: str):
        self.filepath: Path = filepath
        self.line_number: int = line_number
        self.enum_name: str = enum_name.strip()
        self.usages: List[PotentialUsage] = []

    def describe(self):
        return f"{self.filepath.name} : {self.line_number + 1} :: {self.enum_name} ({len(self.usages)} usages)"

    def __str__(self):
        return self.describe()


class SingleHeaderFile:
    def __init__(self, filepath: Path):
        self.full_path: Path = filepath
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
    def __init__(self, source_files_to_search: list[Path], header_files_to_search: list[Path], verbose: bool = False):
        self.source_files_to_search = source_files_to_search
        self.header_files_to_search = header_files_to_search
        self.verbose = verbose

        self.error_count = 0
        self.all_enum_declarations: list[EnumDeclaration] = []
        self.all_source_file_contents: dict[Path, list[str]] = dict()

    def process_enum_declarations(self):
        if self.verbose:
            print("Processing header file data")
        start = time.time()
        for file in self.header_files_to_search:
            s = SingleHeaderFile(file)
            s.run()
            self.all_enum_declarations.extend(s.enum_declarations)
        if self.verbose:
            print(f"Found {len(self.all_enum_declarations)} enum declarations")
            print(f"Processing header files took {time.time()-start:.2f} seconds")

    def process_source_file_contents(self):
        if self.verbose:
            print("Processing source file into lists of lines")
            start = time.time()
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
        if self.verbose:
            print(f"Processing source files took {time.time()-start:.2f} seconds")

    def run(self):

        if self.verbose:
            print("Checking source file lines for usages")
            start = time.time()
        for filepath, file_lines in self.all_source_file_contents.items():
            for line_num, line in enumerate(file_lines, start=1):
                self.check_single_line_for_usage(filepath, line_num, line)
        if self.verbose:
            print(f"Checking source files took {time.time()-start:.2f} seconds")

    def check_single_line_for_usage(self, filepath: Path, line_num: int, line: str):
        # search for usages of Enum:: first
        for match in RE_ENUM_USAGE_ORI.finditer(line):
            g = match.group(1)
            if g != "std::":
                scope = g[:-2]
                for e in self.all_enum_declarations:
                    if e.enum_name == scope:
                        e.usages.append(PotentialUsage(scope=scope, filepath=filepath, line_number=line_num, line=line))
        # also search for declarations of the enum type as in EnumType e;
        if " " in line:
            for e in self.all_enum_declarations:
                if f"{e.enum_name} " in line:
                    e.usages.append(
                        PotentialUsage(scope=e.enum_name, filepath=filepath, line_number=line_num, line=line)
                    )
        # finally it might also be used as a template type:
        if ">" in line:
            for e in self.all_enum_declarations:
                if f"{e.enum_name}>" in line:
                    e.usages.append(
                        PotentialUsage(scope=e.enum_name, filepath=filepath, line_number=line_num, line=line)
                    )

    def reconcile_usages(self, usages: list[PotentialUsage]):
        if self.verbose:
            print("Reconciling usages")
            start = time.time()
        for u in usages:
            for e in self.all_enum_declarations:
                if e.enum_name == u.scope:
                    e.usages.append(u)
        if self.verbose:
            print(f"Reconciling usages took {time.time()-start:.2f} seconds")
        return

    def find_problems_and_report(self):
        if self.verbose:
            print("Reconciling usages")
        apparent_enums_in_only_one_source_file: List[str] = list()
        apparent_enums_in_zero_source_files: List[str] = list()
        for e in self.all_enum_declarations:
            if len(e.usages) == 0:
                apparent_enums_in_zero_source_files.append(e.describe())
            unique_files_in_usages: Set[str] = set()
            # exceptions listed by <FILE>:<ENUM NAME>
            exceptions = ["DataGlobalConstants.hh:ePollutant", "RefrigeratedCase.hh:CriticalType"]
            if f"{e.filepath.name}:{e.enum_name}" not in exceptions:
                for u in e.usages:
                    unique_files_in_usages.add(u.filepath.name)
                if len(unique_files_in_usages) == 1:
                    apparent_enums_in_only_one_source_file.append(
                        f"{e.describe()} in {next(iter(unique_files_in_usages))}"
                    )

        if self.verbose:
            print("Reporting results")
        if len(apparent_enums_in_zero_source_files) > 0:
            print(f"Detected {len(apparent_enums_in_zero_source_files)} enums in ZERO source files:")
            for e in apparent_enums_in_zero_source_files:
                print(f" - {e}")
        if len(apparent_enums_in_only_one_source_file) > 0:
            print(f"\nDetected {len(apparent_enums_in_only_one_source_file)} enums in ONE source file:")
            for e in apparent_enums_in_only_one_source_file:
                print(f" - {e}")
        if self.verbose:
            total_usages = sum([len(e.usages) for e in self.all_enum_declarations])
            print(f"\nTotal enum usages found: {total_usages} in {len(self.all_enum_declarations)} enum declarations")
        self.error_count = len(apparent_enums_in_zero_source_files) + len(apparent_enums_in_only_one_source_file)

    @staticmethod
    def collect_enum_usages_for_file(filepath: Path, known_enum_names: set[str]) -> List[PotentialUsage]:
        """Method that is static so it can easily be parallelizable

        Because processing the source files was taking 80 seconds, this was extracted to a static method
        """
        usages: List[PotentialUsage] = []
        content = filepath.read_text()
        # if not any([enum_name in content for enum_name in known_enum_names]):
        #     return usages

        has_enum_declaration = False
        has_enum_container = False
        for enum_name in known_enum_names:
            if f"{enum_name} " in content:
                has_enum_declaration = True
            if f"{enum_name}>" in content:
                has_enum_container = True
            if has_enum_declaration and has_enum_container:
                break

        lines = content.splitlines()

        for line_num, line in enumerate(lines, start=1):
            for match in RE_ENUM_USAGE.finditer(line):
                scope = match.groupdict()["EnumClass"]
                if scope in known_enum_names:
                    usages.append(PotentialUsage(scope=scope, filepath=filepath, line_number=line_num, line=line))
            # also search for declarations of the enum type as in EnumType e;
            if has_enum_declaration:
                for enum_name in known_enum_names:
                    if f"{enum_name} " in line:
                        usages.append(
                            PotentialUsage(scope=enum_name, filepath=filepath, line_number=line_num, line=line)
                        )
            # finally it might also be used as a template type:
            if has_enum_container and ">" in line:
                for enum_name in known_enum_names:
                    if f"{enum_name}>" in line:
                        usages.append(
                            PotentialUsage(scope=enum_name, filepath=filepath, line_number=line_num, line=line)
                        )
        return usages


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
    if len(sys.argv) > 1 and sys.argv[1] == "test":
        del sys.argv[1:]
        unittest.main(exit=False, verbosity=0)

    parser = get_base_parser(description="Check for Enum Scope Usage")
    parser.add_argument(
        "--debug",
        dest="debug",
        action="store_true",
        default=False,
        help="Enable debug mode",
    )
    args = parser.parse_args()
    if args.files:
        n_ori = len(args.files)
        files = [f for f in args.files if f.suffix in EXTENSIONS and any(f.is_relative_to(d) for d in DIRS_TO_SEARCH)]
        source_files_to_search = [x for x in files if x.suffix == ".cc"]
        header_files_to_search = [x for x in files if x.suffix == ".hh"]
        if args.verbose:
            print(f"Checking {len(files)} of {n_ori} specified files")
    else:
        source_files_to_search = []
        header_files_to_search = []
        for d in DIRS_TO_SEARCH:
            source_files_to_search += collect_files(base_dir=d, extensions={".cc"}, recursive=True, dirs_to_skip=[])
            header_files_to_search += collect_files(base_dir=d, extensions={".hh"}, recursive=True, dirs_to_skip=[])
        if args.verbose:
            print(
                f"Checking {len(header_files_to_search) + len(source_files_to_search)} files: "
                f"{len(header_files_to_search)} headers and {len(source_files_to_search)} sources"
            )

    source_files_to_search.sort()
    header_files_to_search.sort()

    evaluator = EnumScopeEvaluator(
        source_files_to_search=source_files_to_search,
        header_files_to_search=header_files_to_search,
        verbose=args.verbose,
    )
    evaluator.process_enum_declarations()
    known_enum_names = [e.enum_name for e in evaluator.all_enum_declarations]

    if args.debug:
        counter = Counter(known_enum_names)
        duplicates = Counter({k: v for k, v in counter.items() if v > 1})
        if len(duplicates) > 0:
            print("Duplicate enum names found:")
            for scope, count in duplicates.most_common():
                print(f" - {scope} : {count}")
                for e in evaluator.all_enum_declarations:
                    if e.enum_name == scope:
                        print(f"    - {e.describe()}")

    known_enum_names = set(known_enum_names)

    run_synchronously = False
    if run_synchronously:
        evaluator.process_source_file_contents()
        evaluator.run()
    else:
        start = time.time()
        usage_list_of_lists = parallel_apply(
            func=EnumScopeEvaluator.collect_enum_usages_for_file,
            filepaths=source_files_to_search + header_files_to_search,
            known_enum_names=known_enum_names,
        )
        usages = flatten_list_of_lists(list_of_lists=usage_list_of_lists)
        if args.verbose:
            print(f"Finding usages in parallel took {time.time()-start:.2f} seconds")
        if args.debug:
            print(f"Found {len(usages)} potential enum usages")
            [print(x) for x in usages[:100]]
        evaluator.reconcile_usages(usages=usages)

    evaluator.find_problems_and_report()
    if evaluator.error_count > 0:
        raise exit(1)
