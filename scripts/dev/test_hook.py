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
from pathlib import Path

from base_hook import (
    SRC_DIR,
    ErrorMessage,
    LogLevel,
    LogMessage,
    exit_hook,
    flatten_list_of_lists,
    get_base_parser,
    parallel_apply,
    report_log_messages,
)


def function_that_issues_several_errors(filepath: Path, is_error: bool = False) -> list[LogMessage]:
    errors: list[LogMessage] = []
    if is_error:
        errors.append(
            ErrorMessage(
                tool="test_hook",
                filepath=filepath,
                line_number=10,
                line="This is a dummy line content",
                message="I'm issuing an error",
            )
        )
        errors.append(
            ErrorMessage(
                tool="test_hook",
                filepath=filepath,
                line_number=250,
                line="This is another dummy line content",
                message="I'm issuing a second error",
            )
        )
    return errors


def function_that_issues_one_error(filepath: Path, is_error: bool = False) -> LogMessage | None:
    if is_error:
        return ErrorMessage(
            tool="test_hook",
            filepath=filepath,
            line_number=10,
            line="This is a dummy line content",
            message="I'm issuing a unique error",
        )
    return None


if __name__ == "__main__":
    parser = get_base_parser(description="Test Hooks")
    parser.add_argument("--single-error", action="store_true", help="Issue a single error")
    parser.add_argument("--gha", action="store_true", help="Fake Being on Github Actions")
    parser.add_argument("--debug", action="store_true", help="Print files and exit")
    args = parser.parse_args()

    if args.gha:
        import os
        import tempfile

        os.environ["GITHUB_ACTIONS"] = "true"
        step_summary = Path(tempfile.mkdtemp()) / "step_summary.md"
        os.environ["GITHUB_STEP_SUMMARY"] = str(step_summary)

    # exts = {".cc", ".hh"}
    exts = None
    if len(args.files) > 0:
        n_ori = len(args.files)
        if exts is None:
            files = args.files
        else:
            files = [f for f in args.files if f.suffix in exts]
        if args.verbose:
            print(f"Checking {len(files)} of {n_ori} specified files")
    else:
        files = []
        if exts is None:
            exts = {".*"}
        for e in exts:
            files += list(SRC_DIR.glob(f"**/*{e}"))
        if args.verbose:
            print(f"Checking {len(files)} files")
    if len(files) == 0:
        print("No files to check")
        exit(0)

    if args.debug:
        print(files)
        exit(0)

    if args.single_error:
        log_messages = [function_that_issues_one_error(filepath=files[0], is_error=True)]
        log_messages += parallel_apply(func=function_that_issues_one_error, filepaths=files[1:], is_error=False)
        success = report_log_messages(log_messages=log_messages, fail_threshold=LogLevel.ERROR, verbose=args.verbose)
        if args.gha:
            print("\n====== content of GITHUB_STEP_SUMMARY ======")
            print(step_summary.read_text())
            print("============================================")
        exit_hook(success=success)
    else:
        errors_list_of_lists = [function_that_issues_several_errors(filepath=files[0], is_error=True)]
        errors_list_of_lists += parallel_apply(
            func=function_that_issues_several_errors, filepaths=files[1:], is_error=False
        )
        log_messages = flatten_list_of_lists(list_of_lists=errors_list_of_lists)
        success = report_log_messages(log_messages=log_messages, fail_threshold=LogLevel.ERROR, verbose=args.verbose)

        if args.gha:
            print("\n====== content of GITHUB_STEP_SUMMARY ======")
            print(step_summary.read_text())
            print("============================================")

        exit_hook(success=success)
