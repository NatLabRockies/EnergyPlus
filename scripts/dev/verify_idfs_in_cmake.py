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

import re
from itertools import chain
from pathlib import Path

from base_hook import (
    TESTFILES_DIR,
    ErrorMessage,
    LogLevel,
    LogMessage,
    collect_files,
    exit_hook,
    get_base_parser,
    report_log_messages,
)

TEST_CMAKELISTS_FILE = TESTFILES_DIR / "CMakeLists.txt"
assert TEST_CMAKELISTS_FILE.exists(), f"Cannot find '{TEST_CMAKELISTS_FILE}'"

RE_CMAKE = re.compile(r"\(([^)]+)\)", re.MULTILINE)


def get_cmake_list_idf_files(verbose: bool = False) -> set[Path]:
    """Parse the testfiles/CMakeLists.txt file and return a set of all IDF files.

    Returns:
        set[Path]: Set of Path objects for each IDF file listed in the CMake, paths are absolute
    """
    result = set()
    content = TEST_CMAKELISTS_FILE.read_text()
    matches = RE_CMAKE.findall(content)
    for match in matches:
        if "IDF_FILE" in match:
            cleaned_match = match.replace("\n", "")
            tokens = cleaned_match.split()  # special case that allows multiple whitespace delimiters
            filename = tokens[1]
            result.add(filename)
        elif "PYTHON_FILE" in match:  # API-based IDF runs
            cleaned_match = match.replace("\n", "")
            tokens = cleaned_match.split()  # special case that allows multiple whitespace delimiters
            filename = tokens[1]
            filename = "API/" + filename.replace(".py", ".idf")  # assuming the file is named the same as the py file
            result.add(filename)
    filepaths = {TESTFILES_DIR / f for f in result}
    # CMake would throw an error if the file didn't exist, but just in case, check here too
    for f in filepaths:
        if not f.exists():
            print(f"ERROR: File listed in CMakeLists.txt but does not exist: {f}")
            raise SystemExit(1)
    if verbose:
        print(f"Found {len(filepaths)} IDF files listed in testfiles/CMakeLists.txt")
    return filepaths


# there are a few files we purposely skip
FILES_TO_SKIP = {
    "_1a-Long0.0.idf",
    "_ExternalInterface-actuator.idf",
    "_ExternalInterface-schedule.idf",
    "_ExternalInterface-variable.idf",
    "HVAC3Zone-IntGains-Def.imf",
    "HVAC3ZoneChillerSpec.imf",
    "HVAC3ZoneGeometry.imf",
    "HVAC3ZoneMat-Const.imf",
    "_1ZoneUncontrolled_ForAPITesting.idf",
}


def get_found_idf_files() -> set[Path]:
    """Recursively search the testfiles directory for all .idf and .imf files.

    It excludes any fil that are named in FILES_TO_SKIP

    Returns:
        set[Path]: Set of Path objects for each IDF file found, paths are absolute
    """
    exts = {".idf", ".imf"}
    all_files = list(chain.from_iterable([TESTFILES_DIR.glob(f"**/*.{e}") for e in exts]))
    return {f for f in all_files if f.name not in FILES_TO_SKIP}


if __name__ == "__main__":
    parser = get_base_parser(description="Verify IDFs in testfiles CMake")
    args = parser.parse_args()

    cmake_list_idf_files = get_cmake_list_idf_files(verbose=args.verbose)

    exts = {".idf", ".imf"}
    if args.files:
        n_ori = len(args.files)
        files = {f for f in args.files if f.suffix in exts and f.name not in FILES_TO_SKIP}
        if args.verbose:
            print(f"Checking {len(files)} of {n_ori} specified files")
    else:
        files = collect_files(base_dir=TESTFILES_DIR, extensions=exts, recursive=True)
        files = {f for f in files if f.name not in FILES_TO_SKIP}
        if args.verbose:
            print(f"Found {len(files)} IDF files in testfiles/")

    # check if any are missing in cmake
    need_to_add_to_cmake = files.difference(cmake_list_idf_files)
    if not need_to_add_to_cmake:
        if args.verbose:
            print("All IDF files are listed in testfiles/CMakeLists.txt")
        raise SystemExit(0)

    if args.verbose:
        print(f"Found {len(need_to_add_to_cmake)} IDF files missing from testfiles/CMakeLists.txt")

    log_messages: list[LogMessage] = []
    for filepath in need_to_add_to_cmake:
        log_messages.append(
            ErrorMessage(
                tool="verify_idfs_in_cmake",
                filepath=filepath,
                message="File missing from testfiles/CMakeLists.txt",
            )
        )

    success = report_log_messages(log_messages=log_messages, fail_threshold=LogLevel.ERROR, verbose=args.verbose)
    exit_hook(success=success)
