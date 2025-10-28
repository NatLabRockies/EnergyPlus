#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

import argparse
import re
from collections import Counter
from pathlib import Path

RE_FULLNAME = re.compile(r"<!-- FullName:(?P<activeReportName>[^_]+)_(?P<activeForName>[^_]+)_(?P<subtitle>[^_]+)-->")


def format_row(row, col_widths):
    return "| " + " | ".join(f"{str(cell):<{col_widths[i]}}" for i, cell in enumerate(row)) + " |"


def safe_read_text(path: Path) -> str:
    """Read text from a file, trying UTF-8 first, then cp1252 as fallback.  If neither works, it will just fail"""
    try:
        return path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return path.read_text(encoding="cp1252")


def ensure_unique_html_tables(html_path: Path):
    """
    Ensure that each HTML table in the report has a unique name.
    If duplicates are found, append a number to the duplicate names.
    """
    content = safe_read_text(html_path)

    matches = RE_FULLNAME.findall(content)
    if not matches:
        raise ValueError("No HTML tables found with FullName pattern")

    # Filter only duplicates (count > 1)
    duplicates = [
        (activeReportName, activeForName, subtitle, count)
        for (activeReportName, activeForName, subtitle), count in Counter(matches).items()
        if count > 1
    ]

    # Include header row in width calculation
    if duplicates:
        print(f"Found {len(duplicates)} duplicate HTML table(s) based on FullName\n")
        header = [("activeReportName", "activeForName", "subtitle", "count")]
        rows = header + duplicates
        # Compute max width for each column
        col_widths = [max(len(str(row[i])) for row in rows) for i in range(len(header[0]))]

        print(format_row(rows[0], col_widths))
        print("|-" + "-|-".join("-" * w for w in col_widths) + "-|")
        for row in duplicates:
            print(format_row(row, col_widths))

        exit(1)
    else:
        print("No duplicates found.")
        exit(0)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Compare Output Reports.")
    parser.add_argument("out_dir", type=Path, help="Output directory where to find the reports")
    parser.add_argument(
        "--skip-missing", action="store_true", default=False, help="Do not raise if the eplustbl.htm isn't found"
    )
    args = parser.parse_args()

    out_dir = args.out_dir.resolve()
    if not (out_dir.exists() and out_dir.is_dir()):
        raise IOError(f"'{out_dir}' is not a valid directory")
    html_path = out_dir / "eplustbl.htm"
    if not html_path.exists():
        if args.skip_missing:
            print(f"Skipping missing HTML file: {html_path}")
            exit(0)
        raise IOError(f"HTML '{html_path}' does not exist")

    ensure_unique_html_tables(html_path=html_path)
