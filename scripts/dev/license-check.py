#!/usr/bin/env python3
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
from pathlib import Path

import licensetext
from base_hook import (
    ROOT_DIR,
    LogLevel,
    LogMessage,
    exit_hook,
    get_base_parser,
    relative_path_from_root,
    report_log_messages,
)

TOOL_NAME = "license-check"

# Directories to check
CPP_DIRS = [ROOT_DIR / "src", ROOT_DIR / "tst/EnergyPlus/"]
PYTHON_DIRS = [ROOT_DIR]

CPP_CURRENT_LICENSE = licensetext.current()
PYTHON_CURRENT_LICENSE = licensetext.current_python()

CPP_CHECKER = licensetext.Checker(CPP_CURRENT_LICENSE, toolname=TOOL_NAME)
PYTHON_CHECKER = licensetext.Checker(
    PYTHON_CURRENT_LICENSE, offset=2, extensions=["py"], toolname=TOOL_NAME, shebang=True, empty_passes=True
)

# Relative from ROOT_DIR patterns
PYTHON_EXCLUDE_PATTERNS = [
    r".*third_party.*",
    r"^build.*",
    r"^bin.*",
    r".*readthedocs.*",
    r".*venv.*",
    r".*cmake-build-.*",
    r".*colorize_cppcheck_results.py.*",
    r".*__init__.py",
]


def check_root_license_txt() -> LogMessage | None:
    # Check LICENSE.txt
    # Create the text as it should be
    license_txt = licensetext.merge_paragraphs(CPP_CURRENT_LICENSE)
    # Load the text file
    license_txt_path = ROOT_DIR / "LICENSE.txt"
    assert license_txt_path.is_file(), f"'{license_txt_path}' not found"
    # Compare the two strings
    return licensetext.check_license(
        filepath=license_txt_path, possible=license_txt_path.read_text(), correct=license_txt, toolname=TOOL_NAME
    )


def check_full_repo() -> bool:
    """Check LICENSE.txt, then scan all C++ and Python dirs."""
    ok = True
    for base in CPP_DIRS:
        if not CPP_CHECKER.visit(path=base):
            ok = False

    for base in PYTHON_DIRS:
        if not PYTHON_CHECKER.visit(path=base, exclude_patterns=PYTHON_EXCLUDE_PATTERNS):
            ok = False

    return ok


def check_files(filepaths: list[Path]) -> bool:
    """Check only the given files (used in pre-commit mode)."""
    ok = True
    for path in filepaths:
        if not path.exists():
            continue

        ext = path.suffix.lower()
        is_python = ext == ".py"
        checker = PYTHON_CHECKER if is_python else CPP_CHECKER
        if ext in {".cpp", ".hpp", ".h", ".c", ".cc", ".cxx"}:
            checker = CPP_CHECKER
        elif ext == ".py":
            checker = PYTHON_CHECKER
        else:
            # Skip unknown file types
            continue

        if not checker.visit_file(filepath=path):
            ok = False

    return ok


def report_status(opt_base_msg: LogMessage | None, verbose: bool = False):
    all_log_messages: list[LogMessage] = []
    if opt_base_msg is not None:
        all_log_messages = [opt_base_msg]
    all_log_messages += CPP_CHECKER.log_messages + PYTHON_CHECKER.log_messages
    report_log_messages(log_messages=all_log_messages, fail_threshold=LogLevel.ERROR, verbose=verbose)

    if verbose:
        print(f"C++: visited {len(CPP_CHECKER.visited_files)} files")
        print(f"Python: visited {len(PYTHON_CHECKER.visited_files)} files")


if __name__ == "__main__":
    parser = get_base_parser(description="License checker")

    opt_base_msg = check_root_license_txt()

    args = parser.parse_args()
    if not args.files:
        success = check_full_repo()
    else:
        files = args.files
        files = [f for f in files if f.suffix == ".py" or any(f.is_relative_to(d) for d in CPP_DIRS)]
        for pattern in PYTHON_EXCLUDE_PATTERNS:
            matcher = re.compile(pattern)
            files = [f for f in files if not matcher.match(str(relative_path_from_root(f)))]
        success = check_files(filepaths=files)
    success = success and opt_base_msg is None
    report_status(opt_base_msg=opt_base_msg, verbose=args.verbose)

    exit_hook(success)
