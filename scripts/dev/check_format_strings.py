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

import os
import re
import sys
import tempfile
import unittest
from pathlib import Path
from typing import Any, Dict, List, Tuple

# ----------- Match ...::format( followed by a string literal ("..." or R"...") -----------
FORMAT_START = re.compile(r'\b(?:\w+::)*format\(\s*(?=(?:"|R"))', re.S)

# Count each placeholder start {  (ignores literal {{ )
PLACEHOLDER_START = re.compile(r"(?<!{)\{(?!\{)")


# ========================= Low-level scanners =========================
def _scan_cxx_string(src: str, i: int) -> int:
    """Return index just after a normal C++ string starting at src[i] == '\"'."""
    i += 1
    n = len(src)
    while i < n:
        ch = src[i]
        if ch == "\\":
            i += 2
        elif ch == '"':
            return i + 1
        else:
            i += 1
    return i


def _scan_cxx_char(src: str, i: int) -> int:
    """Return index just after a C++ char literal starting at src[i] == '\\''."""
    i += 1
    n = len(src)
    while i < n:
        ch = src[i]
        if ch == "\\":
            i += 2
        elif ch == "'":
            return i + 1
        else:
            i += 1
    return i


def _scan_cxx_raw_string_end(src: str, i: int) -> int:
    """
    Return index just after a C++ raw string literal starting at src[i] == 'R' and src[i+1] == '"'.
    Supports arbitrary delimiters: R"delim( ... )delim"
    """
    n = len(src)
    i += 2  # skip R"
    delim_start = i
    while i < n and src[i] != "(":
        i += 1
    delim = src[delim_start:i]
    if i < n and src[i] == "(":
        i += 1
    closing = ")" + delim + '"'
    l = len(closing)
    while i + l <= n:
        if src[i : i + l] == closing:
            return i + l
        i += 1
    return n


def _grab_balanced_call(src: str, open_paren_idx: int) -> int | None:
    """
    Given src with src[open_paren_idx] == '(',
    return the index AFTER the matching closing ')', scanning strings/chars/raw strings properly.
    """
    i = open_paren_idx + 1
    n = len(src)
    depth = 1
    while i < n:
        ch = src[i]
        if ch == '"':
            i = _scan_cxx_string(src, i)
            continue
        if ch == "'":
            i = _scan_cxx_char(src, i)
            continue
        if ch == "R" and i + 1 < n and src[i + 1] == '"':
            i = _scan_cxx_raw_string_end(src, i)
            continue
        if ch == "(":
            depth += 1
            i += 1
            continue
        if ch == ")":
            depth -= 1
            i += 1
            if depth == 0:
                return i
            continue
        i += 1
    return None


def _split_top_level_commas(s: str) -> List[str]:
    """Split s on commas not inside strings, raw strings, char literals, or parentheses."""
    parts, buf = [], []
    depth = 0
    i, n = 0, len(s)
    while i < n:
        ch = s[i]
        if ch == '"':
            j = _scan_cxx_string(s, i)
            buf.append(s[i:j])
            i = j
            continue
        if ch == "'":
            j = _scan_cxx_char(s, i)
            buf.append(s[i:j])
            i = j
            continue
        if ch == "R" and i + 1 < n and s[i + 1] == '"':
            j = _scan_cxx_raw_string_end(s, i)
            buf.append(s[i:j])
            i = j
            continue
        if ch == "(":
            depth += 1
            buf.append(ch)
            i += 1
            continue
        if ch == ")":
            depth -= 1
            buf.append(ch)
            i += 1
            continue
        if ch == "," and depth == 0:
            parts.append("".join(buf).strip())
            buf = []
            i += 1
            continue
        buf.append(ch)
        i += 1
    if buf:
        parts.append("".join(buf).strip())
    return [p for p in parts if p]


# ========================= Comment removal (preserve newlines) =========================
def remove_cpp_comments(f_path: Path) -> Tuple[str, List[int]]:
    """
    Remove all C/C++ comments while preserving ALL original newlines.
    Returns:
      cleaned             – comment-free source (same line count as original)
      non_comment_lines   – list of original 1-based line numbers that contain non-comment text
    """
    s = f_path.read_text(encoding="utf-8")
    i, n = 0, len(s)
    out: List[str] = []
    non_comment_lines: List[int] = []
    line_idx = 1
    line_has_code = False

    def push(ch: str):
        nonlocal line_idx, line_has_code
        out.append(ch)
        if ch == "\n":
            if line_has_code:
                non_comment_lines.append(line_idx)
            line_idx += 1
            line_has_code = False
        elif not ch.isspace():
            line_has_code = True

    def push_chunk(chunk: str):
        for ch in chunk:
            push(ch)

    while i < n:
        ch = s[i]

        # Always keep newlines as-is for perfect line mapping
        if ch == "\n":
            push("\n")
            i += 1
            continue

        # Line comment //
        if ch == "/" and i + 1 < n and s[i + 1] == "/":
            i += 2
            while i < n and s[i] != "\n":
                i += 1
            # newline (if any) will be handled by the main loop
            continue

        # Block comment /* ... */
        if ch == "/" and i + 1 < n and s[i + 1] == "*":
            i += 2
            while i < n:
                if s[i] == "\n":
                    push("\n")
                    i += 1
                    continue
                if s[i] == "*" and i + 1 < n and s[i + 1] == "/":
                    i += 2
                    break
                i += 1
            continue

        # Normal / Raw strings / Char literals
        if ch == '"':
            j = _scan_cxx_string(s, i)
            push_chunk(s[i:j])
            i = j
            continue
        if ch == "'":
            j = _scan_cxx_char(s, i)
            push_chunk(s[i:j])
            i = j
            continue
        if ch == "R" and i + 1 < n and s[i + 1] == '"':
            j = _scan_cxx_raw_string_end(s, i)
            push_chunk(s[i:j])
            i = j
            continue

        # Regular code
        push(ch)
        i += 1

    if line_has_code:
        non_comment_lines.append(line_idx)

    cleaned = "".join(out)
    return cleaned, non_comment_lines


# ========================= Format finder & parser =========================
def _format_matches_iter(text: str):
    """
    Yield FORMAT_START matches that are NOT:
      - after '//' on the same line, or
      - raw strings starting with R"idf(
    """
    for m in FORMAT_START.finditer(text):
        bol = text.rfind("\n", 0, m.start()) + 1
        # Skip if there's a '//' comment before on the same line
        if text.find("//", bol, m.start()) != -1:
            continue
        # Skip if it's a raw string with an idf-style delimiter like R"idf(
        raw_prefix = text[m.end() : m.end() + 20]  # small window after 'format('
        if re.match(r'\s*R"idf', raw_prefix):
            continue
        yield m


def find_and_parse_format_calls(text: str) -> List[Dict[str, Any]]:
    """
    Find calls and return dicts:
      start, end, line, full, fmt_expr, args_text, args_list
    """
    results = []
    for m in _format_matches_iter(text):
        open_paren = m.end() - 1
        end_idx = _grab_balanced_call(text, open_paren)
        if end_idx is None:
            continue

        # Cleaned text preserves all original newlines => direct line computation is accurate
        line_num = text.count("\n", 0, m.start()) + 1

        full = text[m.start() : end_idx]
        inner = text[open_paren + 1 : end_idx - 1].strip()

        if not inner:
            results.append(
                {
                    "start": m.start(),
                    "end": end_idx,
                    "line": line_num,
                    "full": full,
                    "fmt_expr": "",
                    "args_text": None,
                    "args_list": [],
                }
            )
            continue

        pieces = _split_top_level_commas(inner)
        fmt_expr = pieces[0] if pieces else ""
        args_text = inner[len(fmt_expr) :].lstrip()
        if args_text.startswith(","):
            args_text = args_text[1:].lstrip()
        args_list = _split_top_level_commas(args_text) if args_text else []

        results.append(
            {
                "start": m.start(),
                "end": end_idx,
                "line": line_num,
                "full": full,
                "fmt_expr": fmt_expr,
                "args_text": args_text,
                "args_list": args_list,
            }
        )
    return results


# ========================= Placeholder counting & checks =========================
def count_placeholders(fmt_string: str) -> int:
    """
    Count placeholders by counting unescaped '{' after removing literal braces.
    """
    fmt_no_literals = fmt_string.replace("{{", "").replace("}}", "")
    return len(PLACEHOLDER_START.findall(fmt_no_literals))


def check_format_statement(f_path: Path, fmt_dict: Dict[str, Any]) -> int:
    num_placeholders = count_placeholders(fmt_dict["fmt_expr"])
    num_args = len(fmt_dict["args_list"])
    if num_args != num_placeholders:
        print(f"{f_path}:{fmt_dict['line']}: placeholders={num_placeholders}, args={num_args}")
        print(f"  {fmt_dict['full']}")
        return 1
    return 0


def check_format_statements(file_to_check: List[Path]) -> int:
    num_errors = 0
    for f_path in file_to_check:
        try:
            cleaned, _ = remove_cpp_comments(f_path)
        except UnicodeDecodeError:
            # Skip files with unexpected encoding
            continue
        for fmt_dict in find_and_parse_format_calls(cleaned):
            num_errors += check_format_statement(f_path, fmt_dict)
    return 1 if num_errors > 0 else 0


# ========================= File discovery & CLI =========================
def get_sorted_file_list(search_path: Path) -> List[Path]:
    out: List[Path] = []
    for root, _, files in os.walk(search_path):
        for file in files:
            if file.endswith((".hh", ".cc")):
                out.append(Path(root) / file)
    out.sort()
    return out


class TestCheckFormatStrings(unittest.TestCase):
    PASS = 0
    FAIL = 1

    def test_valid_cases(self):
        test_options = [
            'format("{}", arg1)',
            'format("{}{}="{}"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1))',
            'fmt::format("PLR          = {:7." + std::to_string(DecimalPrecision) + "F}", fmt::join(PLRArray, ","))',
            'format("...{} is < 2 {{C}}. Freezing could occur.", cNumericFields(17))',
            'format(R"({}="{}" invalid {}="{}" not found.)",\n CurrentModuleObject,\n ventSlab.Name,\n cAlphaFields(4),\n state.dataIPShortCut->cAlphaArgs(4))',
            'format("{}{}{}{}{}{}", "Occurs for Node=", NodeName, ", ObjectType=", ObjectType, ", ObjectName=", ObjectName)',
            'format("{}{}", RoutineName, "Node registered for both Parent and "not" Parent")'
            'format("{}\n", EnergyPlus::Constant::unitNames[(int)meter->units])',
        ]

        tmp_dir = Path(tempfile.mkdtemp())
        tmp_file = tmp_dir / "valid.cc"

        with open(tmp_file, "w") as f:
            for line in test_options:
                f.write(line)

        self.assertEqual(self.PASS, check_format_statements([tmp_file]))

    def test_invalid_cases(self):
        test_options = [
            'format("{}", arg1, arg2)',
            'format("{}{}="{}"")',
        ]

        tmp_dir = Path(tempfile.mkdtemp())
        tmp_file = tmp_dir / "invalid.cc"

        with open(tmp_file, "w") as f:
            for line in test_options:
                f.write(line)

        self.assertEqual(self.FAIL, check_format_statements([tmp_file]))


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "test":
        del sys.argv[1:]
        unittest.main(exit=False, verbosity=0)

    root_path = Path(__file__).parent.parent.parent
    src_path = root_path / "src" / "EnergyPlus"
    files = get_sorted_file_list(src_path)
    tst_path = root_path / "tst" / "EnergyPlus"
    files.extend(get_sorted_file_list(tst_path))
    sys.exit(check_format_statements(files))
