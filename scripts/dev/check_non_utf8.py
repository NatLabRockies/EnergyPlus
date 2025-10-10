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

from pathlib import Path

from base_hook import (
    ROOT_DIR,
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

DIRS_TO_SKIP: list[Path] = [
    ".git",
    "build",
    "builds",
    "cmake-build-debug",
    "cmake-build-release",
    "design",
    "release",
    "__pycache__",
    ".mypy_cache",
    "_build",
]

# these CC files purposefully have bad characters
# not sure what to do besides ignore them
FILE_NAMES_TO_SKIP = [
    # 'InputProcessor.unit.cc', 'EconomicTariff.cc',
    # 'OutputReportTabular.cc'
]

# tex files are included here, but the docs folder is ignored,
# so it has no effect right now
FILE_PATTERNS = {".cc", ".hh", ".tex", ".cpp", ".hpp", ".idd", ".idf", ".imf"}


def is_file_kept(filepath: Path) -> bool:
    if filepath.suffix not in FILE_PATTERNS:
        return False
    if filepath.name in FILE_NAMES_TO_SKIP:
        return False

    if any(filepath.is_relative_to(ROOT_DIR / d) for d in DIRS_TO_SKIP):
        return False

    return True


def check_is_utf8(filepath: Path, do_fix: bool = False) -> list[LogMessage]:
    log_messages: list[LogMessage] = []

    try:
        filepath.read_text(encoding="utf-8", errors="strict")
    except UnicodeDecodeError:
        log_messages.append(
            ErrorMessage(
                tool="check_non_utf8",
                filepath=filepath,
                message="File isn't UTF-8 encoded",
            )
        )
        # Now you try to give a better error message by pointing at the
        # lines that are guilty. To do so, you open as binary, and try to
        # decode each line as utf-8
        binary_lines = filepath.read_bytes().splitlines()

        for line_num, line in enumerate(binary_lines):
            try:
                line.decode(encoding="utf-8", errors="strict")
                # codecs.decode(line, encoding='utf-8', errors='strict')
            except (UnicodeDecodeError, UnicodeEncodeError):
                replaced_line = line.decode(encoding="utf-8", errors="replace")

                log_messages.append(
                    WarningMessage(
                        tool="check_non_utf8",
                        filepath=filepath,
                        line_number=line_num,
                        line=line,
                        message=f"Line has invalid characters/encoding: {replaced_line}",
                    )
                )

        if do_fix:
            fix_encoding(filepath=filepath)

    return log_messages


def fix_encoding(filepath: Path) -> None:
    try:
        idf_text = filepath.read_text(encoding="latin-1", errors="strict")
        filepath.write_text(idf_text, encoding="utf-8")

    except ValueError:
        print(f"Cannot fix encoding for {filepath.relative_to(ROOT_DIR)}")


if __name__ == "__main__":
    parser = get_base_parser(description="Check files are UTF-8")
    parser.add_argument(
        "--fix", dest="do_fix", action="store_true", default=False, help="fix files with latin-1 encoding"
    )

    args = parser.parse_args()
    if args.files:
        n_ori = len(args.files)
        files = [f for f in args.files if is_file_kept(f)]
        if args.verbose:
            print(f"Checking {len(files)} of {n_ori} specified files")
    else:
        files = collect_files(base_dir=ROOT_DIR, extensions=FILE_PATTERNS, recursive=True, dirs_to_skip=DIRS_TO_SKIP)
        if args.verbose:
            print(f"Found {len(files)} files to check")

    errors_list_of_lists = parallel_apply(func=check_is_utf8, filepaths=files, do_fix=args.do_fix)
    log_messages = flatten_list_of_lists(list_of_lists=errors_list_of_lists)
    success = report_log_messages(log_messages=log_messages, fail_threshold=LogLevel.ERROR, verbose=args.verbose)
    exit_hook(success=success)
