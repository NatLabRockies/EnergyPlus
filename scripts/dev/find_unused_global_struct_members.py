#!/usr/bin/env python
# -*- coding: utf-8 -*-
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

"""
Heuristic finder for unused data members in `*Data : BaseGlobalStruct` structs.

We have a recurring pattern where a data member is declared in a BaseGlobalStruct-derived struct,
but never actually referenced anywhere in `src/` or `tst/`.
It might be for several reasons such as carried over from the original Fortran port,
or used at some point and deleted later.

This script:
  1. Finds every header declaring a `struct X : [public] BaseGlobalStruct`.
  2. Parses out its plain data-member declarations
     (skipping methods, nested enums/structs,and anything with `(` or `{`, since real member variable declarations
     in this codebase never have those).
  3. Builds a single whole-repo identifier occurrence count (one pass, with comments stripped) and flags any member
     whose name occurs exactly once in the whole of `src/EnergyPlus` + `tst/EnergyPlus` -- i.e. only at its
     own declaration.

This is only a crude "very obvious offender" finder, not an exhaustive one.
It under-reports (safe direction): a common short member name (`T`, `S1`, ...) can collide
with an unrelated identifier elsewhere, or with a same-named member of a different struct, making a truly
dead member look "used".

Usage:
    python find_unused_global_struct_members.py [-v] [header1.hh header2.hh ...]

If no files are given, every header under src/EnergyPlus declaring a BaseGlobalStruct-derived struct is scanned.
"""

import re
from collections import Counter
from pathlib import Path

from base_hook import (
    SRC_DIR,
    TST_DIR,
    ErrorMessage,
    LogLevel,
    LogMessage,
    collect_files,
    exit_hook,
    get_base_parser,
    parallel_apply,
    report_log_messages,
)

STRUCT_RE = re.compile(r"\b(?:struct|class)\s+(?P<struct_name>\w+)\s*:\s*(?:public\s+)?BaseGlobalStruct\b")

LEADING_ACCESS_SPECIFIER_RE = re.compile(r"^\s*(?:public|private|protected)\s*:\s*")

# First token of a top-level ';'-terminated statement that means "this isn't
# a data member declaration, skip it".
NON_MEMBER_LEADING_TOKENS = {
    "using",
    "typedef",
    "friend",
    "static_assert",
    "template",
    "namespace",
}

# Last identifier before an optional `[...]` and/or `= initializer`, anchored
# at the end of the (comment-stripped, single) declarator.
MEMBER_NAME_RE = re.compile(r"(?P<name>[A-Za-z_]\w*)\s*(?:\[[^\]]*\])?\s*(?:=\s*.*)?$")

IDENTIFIER_RE = re.compile(r"\b[A-Za-z_]\w*\b")


def strip_comments(content: str) -> str:
    """Strip // and /* */ comments, replacing them with spaces so that line
    numbers and character offsets are preserved."""

    def repl(m: re.Match) -> str:
        return "".join(c if c == "\n" else " " for c in m.group(0))

    pattern = re.compile(r"//[^\n]*|/\*.*?\*/", re.DOTALL)
    return pattern.sub(repl, content)


def split_top_level_commas(decl: str) -> list[str]:
    """Split "Type a, b, c" on commas that are not nested inside <...>."""
    parts = []
    depth = 0
    buf: list[str] = []
    for c in decl:
        if c == "<":
            depth += 1
            buf.append(c)
        elif c == ">":
            depth = max(0, depth - 1)
            buf.append(c)
        elif c == "," and depth == 0:
            parts.append("".join(buf))
            buf = []
        else:
            buf.append(c)
    parts.append("".join(buf))
    return parts


def iter_top_level_statements(body: str):
    """Yield (statement_text, start_offset) for every ';'-terminated
    statement found at brace-depth 0 relative to the struct's own opening
    brace. Stops as soon as the struct's matching closing brace is hit."""
    depth = 0
    buf: list[str] = []
    stmt_start = 0
    for i, c in enumerate(body):
        if c == "{":
            depth += 1
            buf.append(c)
        elif c == "}":
            if depth == 0:
                # Matching close of the struct itself: done.
                return
            depth -= 1
            buf.append(c)
        elif c == ";" and depth == 0:
            buf.append(c)
            yield "".join(buf), stmt_start
            buf = []
            stmt_start = i + 1
        else:
            buf.append(c)


def parse_struct_members(header_file: Path) -> list[tuple[str, str, int]]:
    """Returns a list of (struct_name, member_name, line_number) for every
    plain data member declared directly in a BaseGlobalStruct-derived struct
    in header_file."""

    raw = header_file.read_text(encoding="utf-8")
    content = strip_comments(raw)

    members: list[tuple[str, str, int]] = []

    for struct_m in STRUCT_RE.finditer(content):
        struct_name = struct_m.group("struct_name")
        brace_pos = content.find("{", struct_m.end())
        if brace_pos == -1:
            continue
        body = content[brace_pos + 1 :]

        for stmt, offset in iter_top_level_statements(body):
            if "(" in stmt or "{" in stmt:
                # Method declarations/definitions, nested enums/structs/unions.
                continue
            unstripped_stmt = LEADING_ACCESS_SPECIFIER_RE.sub("", stmt)
            stmt = unstripped_stmt.strip()
            if not stmt.endswith(";"):
                continue
            decl = stmt[:-1].strip()
            if not decl:
                continue
            # Offset of the first non-whitespace character, i.e. the actual
            # start of the declaration text, for accurate line numbers.
            decl_offset_in_stmt = len(unstripped_stmt) - len(unstripped_stmt.lstrip())
            first_token = decl.split(None, 1)[0]
            if first_token in NON_MEMBER_LEADING_TOKENS:
                continue

            for piece in split_top_level_commas(decl):
                piece = piece.strip()
                if not piece:
                    continue
                name_m = MEMBER_NAME_RE.search(piece)
                if not name_m:
                    continue
                name = name_m.group("name")
                if name in NON_MEMBER_LEADING_TOKENS or name == first_token and " " not in piece:
                    # A bare single-token piece (e.g. just a qualifier) isn't
                    # a real declarator.
                    continue
                abs_offset = brace_pos + 1 + offset + decl_offset_in_stmt
                line_number = content.count("\n", 0, abs_offset) + 1
                members.append((struct_name, name, line_number))

    return members


def find_base_global_struct_headers() -> list[Path]:
    headers = []
    for path in collect_files(base_dir=SRC_DIR, extensions=(".hh",), recursive=True):
        try:
            content = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        if "BaseGlobalStruct" in content and STRUCT_RE.search(content):
            headers.append(path)
    return sorted(headers)


def count_identifiers_in_file(path: Path) -> Counter:
    try:
        content = path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return Counter()
    content = strip_comments(content)
    return Counter(IDENTIFIER_RE.findall(content))


# Not just .hh/.cc: AirflowNetwork uses .cpp/.hpp, the public C API uses .h,
# and a handful of tst files use .c.
SCANNED_EXTENSIONS = (".hh", ".h", ".hpp", ".cc", ".cpp", ".c")


def build_repo_wide_identifier_counts() -> Counter:
    files = list(collect_files(base_dir=SRC_DIR, extensions=SCANNED_EXTENSIONS, recursive=True))
    files += list(collect_files(base_dir=TST_DIR, extensions=SCANNED_EXTENSIONS, recursive=True))

    counters = parallel_apply(func=count_identifiers_in_file, filepaths=files)
    total = Counter()
    for c in counters:
        total.update(c)
    return total


def find_unused_global_struct_members(header_files: list[Path], identifier_counts: Counter) -> list[LogMessage]:
    log_messages: list[LogMessage] = []
    for header_file in header_files:
        for struct_name, member_name, line_number in parse_struct_members(header_file):
            # The declaration itself contributes exactly one occurrence.
            if identifier_counts.get(member_name, 0) <= 1:
                log_messages.append(
                    ErrorMessage(
                        tool="find_unused_global_struct_members",
                        filepath=header_file,
                        line_number=line_number,
                        message=f"Member `{member_name}` of struct `{struct_name}` "
                        f"does not appear to be referenced anywhere else in src/ or tst/",
                    )
                )
    return log_messages


if __name__ == "__main__":
    parser = get_base_parser(
        description="Find candidate unused data members in BaseGlobalStruct-derived structs",
        files_arg_help="Header files to check (if omitted, checks every BaseGlobalStruct header under src/EnergyPlus)",
    )
    args = parser.parse_args()

    if args.files:
        header_files = [f for f in args.files if f.suffix == ".hh"]
    else:
        header_files = find_base_global_struct_headers()
        if args.verbose:
            print(f"Found {len(header_files)} headers declaring a BaseGlobalStruct-derived struct")

    if args.verbose:
        print("Building repo-wide identifier counts (single pass over src/ and tst/)...")
    identifier_counts = build_repo_wide_identifier_counts()

    log_messages = find_unused_global_struct_members(header_files=header_files, identifier_counts=identifier_counts)

    success = report_log_messages(log_messages=log_messages, fail_threshold=LogLevel.ERROR, verbose=args.verbose)
    exit_hook(success=success)

    exit_hook(success=True)
