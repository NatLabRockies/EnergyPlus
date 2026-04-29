#!/usr/bin/env python
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

import codecs
from pathlib import Path

from base_hook import (
    ROOT_DIR,
    TESTFILES_DIR,
    ErrorMessage,
    LogLevel,
    LogMessage,
    exit_hook,
    get_base_parser,
    report_log_messages,
)


def has_byte_order_mark(filepath: Path, do_fix: bool = False) -> LogMessage | None:
    """Checks if the first three bytes of the file are a UTF-8 BOM."""
    with open(filepath, "rb") as f_b:
        bts = f_b.read(3)

    if bts != codecs.BOM_UTF8:
        return None

    log_message = ErrorMessage(
        tool="check_for_bom_in_idfs",
        filepath=filepath,
        line_number=1,
        message="Byte-Order-Mark sequence detected in IDF, check editor",
    )

    if do_fix:
        with open(filepath, newline="", encoding="utf-8-sig") as f:
            contents = f.read()
        with open(filepath, "w", newline="", encoding="utf-8") as f:
            f.write(contents)

    return log_message


if __name__ == "__main__":
    parser = get_base_parser(description="Verify that the IDFs don't have byte order marks")
    parser.add_argument("--fix", dest="do_fix", action="store_true", default=False, help="Remove BOM")

    args = parser.parse_args()
    if args.files:
        n_ori = len(args.files)
        files = [f for f in args.files if f.suffix in {".idf", ".imf"}]
        if args.verbose:
            print(f"Checking {len(files)} of {n_ori} specified files")
    else:
        files = list(TESTFILES_DIR.glob("**/*.idf")) + list(TESTFILES_DIR.glob("**/*.imf"))
        if args.verbose:
            print(f"Checking {len(files)} files in {TESTFILES_DIR}")

    log_messages = []
    for filepath in files:
        opt_log_message = has_byte_order_mark(filepath, do_fix=args.do_fix)
        if opt_log_message:
            log_messages.append(opt_log_message)

    success = report_log_messages(log_messages=log_messages, fail_threshold=LogLevel.ERROR, verbose=args.verbose)
    if args.do_fix and args.verbose:
        print(f"Fixed BOM issues in {len(log_messages)} files")
    exit_hook(success=success)
