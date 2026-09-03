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

"""Check that DataErrorTracking::ErrorSummaryType stays in sync with its call sites.

- DataErrorTracking.hh's enum entries carry a short truncated "search string" in a
  trailing comment (eg `// "Node Connection Error"`). That string is a verbatim substring
  of the actual ShowXXX() message printed at the real point of call - it predates this
  refactor (it used to be used to scan the whole error file for these strings) and is now
  repurposed purely as a lookup key for this script.
- UtilityRoutines.cc's `ErrorSummaries` array must have one entry per enum value (Num
  excluded), in the same order, each starting with `// EnumName` followed by a
  `{"Summary", "MoreDetails"}` entry. That .Summary text is what gets printed in the
  "Final Error Summary"; it is not expected to match the call site's message.
- Every "live" entry must have, somewhere in the .cc sources, a
  `++state.dataErrTracking->ErrorSummaryCount[static_cast<size_t>(DataErrorTracking::ErrorSummaryType::X)];`
  near a ShowXXX() call whose message contains X's search string (order between the two
  varies, and there's sometimes a block of unrelated ShowContinueError() calls in between).
- An enum entry with no such increment anywhere is dead: it should be removed from both
  the enum (DataErrorTracking.hh) and the ErrorSummaries array (UtilityRoutines.cc).
- A ShowXXX() call whose message contains a search string but has no matching increment
  nearby is suspicious: either the increment was forgotten, or the search string is
  stale/coincidental.
"""

import re
import sys
import unittest
from dataclasses import dataclass
from pathlib import Path

from base_hook import (
    SRC_DIR,
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

DATA_ERROR_TRACKING_HH = SRC_DIR / "DataErrorTracking.hh"
UTILITY_ROUTINES_CC = SRC_DIR / "UtilityRoutines.cc"

ENUM_ENTRY_RE = re.compile(r"^\s*(\w+)\s*(?:=\s*\d+)?\s*,?\s*(?://\s*\"([^\"]*)\")?\s*$")

SUMMARIES_ENTRY_RE = re.compile(r"//\s*(\w+)\s*\n\s*\{\s*\"([^\"]*)\"")

INCREMENT_RE = re.compile(
    r"\+\+\s*state\s*\.\s*dataErrTracking\s*->\s*ErrorSummaryCount\s*\[\s*static_cast<size_t>\s*\(\s*"
    r"DataErrorTracking::ErrorSummaryType::(\w+)\s*\)\s*\]\s*;"
)

SHOW_CALL_RE = re.compile(r"\bShow\w*\s*\(")

STRIP_RE = re.compile(r'"(?:\\.|[^"\\])*"|/\*.*?\*/|//[^\n]*', re.DOTALL)

# How far apart (in characters, either direction) an ErrorSummaryCount increment and the
# ShowXXX() call it belongs to are allowed to be. Generous because a call is sometimes
# followed by a block of unrelated ShowContinueError() diagnostics before a final
# ShowRecurringXXXErrorAtEnd() call that repeats the same search string, and the
# increment itself sometimes precedes and sometimes follows the call.
WINDOW_CHARS = 4000


def strip_comments(text: str) -> str:
    """Blank out // and /* */ comments (keeping length and newlines) so they can't produce
    matches; string literals are left untouched."""

    def _replace(m: re.Match) -> str:
        matched = m.group(0)
        if matched.startswith('"'):
            return matched
        return "".join(c if c == "\n" else " " for c in matched)

    return STRIP_RE.sub(_replace, text)


@dataclass
class EnumEntry:
    name: str
    line_number: int
    search_string: str | None


@dataclass
class SummaryEntry:
    name: str
    summary: str


@dataclass
class IncrementSite:
    enum_name: str
    line_number: int
    matched: bool  # whether a nearby ShowXXX() call was found containing the search string


def line_of(text: str, index: int) -> int:
    return text.count("\n", 0, index) + 1


def parse_enum_entries(hh_path: Path) -> list[EnumEntry]:
    """Parse the ErrorSummaryType enum (name, line number, and trailing "search string" comment)."""
    text = hh_path.read_text(encoding="utf-8")
    lines = text.splitlines()

    start_idx = next(i for i, line in enumerate(lines) if "enum class ErrorSummaryType" in line)
    brace_idx = next(i for i in range(start_idx, len(lines)) if "{" in lines[i])

    entries: list[EnumEntry] = []
    for i in range(brace_idx + 1, len(lines)):
        line = lines[i]
        if "}" in line:
            break
        stripped = line.strip()
        if not stripped or stripped.startswith("//"):
            continue
        m = ENUM_ENTRY_RE.match(line)
        if not m:
            raise ValueError(f"{hh_path}:{i + 1}: could not parse enum entry: {line!r}")
        name = m.group(1)
        if name == "Num":
            continue
        entries.append(EnumEntry(name=name, line_number=i + 1, search_string=m.group(2)))
    return entries


def parse_error_summaries(cc_path: Path) -> list[SummaryEntry]:
    """Parse the `// EnumName` / `{"Summary", ...}` entries of the ErrorSummaries array."""
    text = cc_path.read_text(encoding="utf-8")
    start = text.index("ErrorSummaries{{")
    end = text.index("}};", start)
    block = text[start:end]
    return [SummaryEntry(name=name, summary=summary) for name, summary in SUMMARIES_ENTRY_RE.findall(block)]


def find_show_call_spans(text: str) -> list[tuple[int, int]]:
    """Return (start, end) spans of every ShowXXX(...) call in `text`, end being just past
    the call's closing paren (found by paren-balancing from the opening one)."""
    spans: list[tuple[int, int]] = []
    for m in SHOW_CALL_RE.finditer(text):
        depth = 1
        pos = m.end()
        end = len(text)
        while pos < len(text):
            c = text[pos]
            if c == "(":
                depth += 1
            elif c == ")":
                depth -= 1
                if depth == 0:
                    end = pos + 1
                    break
            pos += 1
        spans.append((m.start(), end))
    return spans


def find_key_occurrences(text: str, search_string: str, show_call_spans: list[tuple[int, int]]) -> list[int]:
    """Return start indices of `search_string` occurrences that sit inside one of `show_call_spans`
    (ie really are part of a ShowXXX(...) message, not some unrelated string literal)."""
    occurrences: list[int] = []
    for call_start, call_end in show_call_spans:
        idx = text.find(search_string, call_start, call_end)
        if idx != -1:
            occurrences.append(idx)
    return occurrences


def check_text(
    raw_text: str, filepath: Path, enum_entries_by_name: dict[str, EnumEntry]
) -> tuple[list[IncrementSite], list[LogMessage]]:
    """Scan one .cc file's text: collect every ErrorSummaryCount increment, and flag ShowXXX
    calls whose message contains a search string with no ErrorSummaryCount increment nearby
    (or vice versa). Proximity is checked in both directions since the increment sometimes
    precedes and sometimes follows the ShowXXX() call it belongs to."""
    text = strip_comments(raw_text)
    log_messages: list[LogMessage] = []
    sites: list[IncrementSite] = []

    increment_spans = list(INCREMENT_RE.finditer(text))
    show_call_spans = find_show_call_spans(text)
    key_occurrences_by_name = {
        entry.name: find_key_occurrences(text, entry.search_string, show_call_spans)
        for entry in enum_entries_by_name.values()
        if entry.search_string is not None
    }

    for m in increment_spans:
        enum_name = m.group(1)
        entry = enum_entries_by_name.get(enum_name)
        matched = False
        if entry is not None and entry.search_string is not None:
            matched = any(abs(occ - m.start()) <= WINDOW_CHARS for occ in key_occurrences_by_name.get(enum_name, []))
        sites.append(IncrementSite(enum_name=enum_name, line_number=line_of(text, m.start()), matched=matched))
        if entry is not None and entry.search_string is not None and not matched:
            log_messages.append(
                WarningMessage(
                    tool="check_error_summary_increment",
                    filepath=filepath,
                    line_number=line_of(text, m.start()),
                    line=raw_text.splitlines()[line_of(text, m.start()) - 1].strip(),
                    message=(
                        f"++ErrorSummaryCount[...ErrorSummaryType::{enum_name}] has no nearby ShowXXX() call containing "
                        f"the expected search string {entry.search_string!r} (see DataErrorTracking.hh:{entry.line_number})"
                    ),
                )
            )

    # Reverse direction: a ShowXXX() call containing a search string with no matching
    # increment nearby likely means the increment was forgotten.
    increment_positions_by_name: dict[str, list[int]] = {}
    for m in increment_spans:
        increment_positions_by_name.setdefault(m.group(1), []).append(m.start())

    for enum_name, occurrences in key_occurrences_by_name.items():
        entry = enum_entries_by_name[enum_name]
        increment_positions = increment_positions_by_name.get(enum_name, [])
        for occ in occurrences:
            if any(abs(occ - pos) <= WINDOW_CHARS for pos in increment_positions):
                continue
            log_messages.append(
                WarningMessage(
                    tool="check_error_summary_increment",
                    filepath=filepath,
                    line_number=line_of(text, occ),
                    line=raw_text.splitlines()[line_of(text, occ) - 1].strip(),
                    message=(
                        f"ShowXXX() call contains {entry.search_string!r}, the search string for "
                        f"ErrorSummaryType::{enum_name} (see DataErrorTracking.hh:{entry.line_number}), "
                        "but has no ++ErrorSummaryCount[...] increment nearby - forgot to add it?"
                    ),
                )
            )

    return sites, log_messages


def check_file(
    filepath: Path, enum_entries_by_name: dict[str, EnumEntry]
) -> tuple[list[IncrementSite], list[LogMessage]]:
    """Read and scan a single .cc file. See `check_text` for the actual logic."""
    return check_text(filepath.read_text(encoding="utf-8"), filepath, enum_entries_by_name)


class TestCheckErrorSummaryIncrement(unittest.TestCase):
    def setUp(self) -> None:
        self.dummy_file = Path("DummyFile.cc")
        # Two enum entries, both with a search string.
        self.enum_entries_by_name = {
            "FooError": EnumEntry(name="FooError", line_number=10, search_string="Foo happened"),
            "BarError": EnumEntry(name="BarError", line_number=11, search_string="Bar happened"),
        }

    def test_valid_increment_immediately_before_call(self) -> None:
        text = """
        ++state.dataErrTracking->ErrorSummaryCount[static_cast<size_t>(DataErrorTracking::ErrorSummaryType::FooError)];
        ShowSevereError(state, "Foo happened here");
        """
        sites, log_messages = check_text(text, self.dummy_file, self.enum_entries_by_name)
        self.assertEqual(len(sites), 1)
        self.assertTrue(sites[0].matched)
        self.assertEqual(log_messages, [])

    def test_valid_increment_after_call(self) -> None:
        # The increment comes after the ShowXXX() call it belongs to (eg SimulationManager.cc's
        # NodeConnectionErrors), which must be accepted just like the more common order.
        text = """
        ShowWarningError(state, std::format("Foo happened for object {}", CType));
        ShowContinueError(state, "some detail");
        ++state.dataErrTracking->ErrorSummaryCount[static_cast<size_t>(DataErrorTracking::ErrorSummaryType::FooError)];
        """
        sites, log_messages = check_text(text, self.dummy_file, self.enum_entries_by_name)
        self.assertEqual(len(sites), 1)
        self.assertTrue(sites[0].matched)
        self.assertEqual(log_messages, [])

    def test_valid_increment_several_lines_above(self) -> None:
        # A ShowSevereMessage() followed by several unrelated ShowContinueError() diagnostics
        # before a final ShowRecurringSevereErrorAtEnd() that repeats the search string (eg
        # HeatBalanceSurfaceManager.cc's TemperatureLowOutOfBounds) must not be flagged.
        filler = "\n".join(f'ShowContinueError(state, "...filler detail {i}");' for i in range(20))
        text = f"""
        ++state.dataErrTracking->ErrorSummaryCount[static_cast<size_t>(DataErrorTracking::ErrorSummaryType::FooError)];
        ShowSevereMessage(state, "Foo happened here");
        {filler}
        ShowRecurringSevereErrorAtEnd(state, "Foo happened for zone=X", count, low, high, _, "C", "C");
        """
        sites, log_messages = check_text(text, self.dummy_file, self.enum_entries_by_name)
        self.assertEqual(len(sites), 1)
        self.assertTrue(sites[0].matched)
        self.assertEqual(log_messages, [])

    def test_dead_entry_has_no_increment_site(self) -> None:
        # BarError's search string never shows up at all: check_text should report neither
        # an increment site nor a missing-increment warning for it - dead-entry detection
        # itself happens one level up, across all files, once no site is found anywhere.
        text = """
        ++state.dataErrTracking->ErrorSummaryCount[static_cast<size_t>(DataErrorTracking::ErrorSummaryType::FooError)];
        ShowSevereError(state, "Foo happened here");
        """
        sites, log_messages = check_text(text, self.dummy_file, self.enum_entries_by_name)
        self.assertEqual({s.enum_name for s in sites}, {"FooError"})
        self.assertEqual(log_messages, [])

    def test_call_without_nearby_increment_is_flagged(self) -> None:
        text = """
        ShowWarningError(state, "Bar happened but the counter was forgotten");
        """
        sites, log_messages = check_text(text, self.dummy_file, self.enum_entries_by_name)
        self.assertEqual(sites, [])
        self.assertEqual(len(log_messages), 1)
        self.assertIn("BarError", log_messages[0].message)
        self.assertIn("forgot to add it", log_messages[0].message)

    def test_increment_without_nearby_call_is_flagged(self) -> None:
        text = """
        ++state.dataErrTracking->ErrorSummaryCount[static_cast<size_t>(DataErrorTracking::ErrorSummaryType::FooError)];
        // the ShowXXX() call was removed but the increment was left behind
        """
        sites, log_messages = check_text(text, self.dummy_file, self.enum_entries_by_name)
        self.assertEqual(len(sites), 1)
        self.assertFalse(sites[0].matched)
        self.assertEqual(len(log_messages), 1)
        self.assertIn("has no nearby ShowXXX() call", log_messages[0].message)

    def test_search_string_outside_a_show_call_is_ignored(self) -> None:
        # A plain string literal that happens to contain the search string (eg an object
        # name in an array) must not count as a call site.
        text = """
        Array1D_string const cMaterialGroupType({"Foo happened elsewhere, not in a Show call"});
        """
        sites, log_messages = check_text(text, self.dummy_file, self.enum_entries_by_name)
        self.assertEqual(sites, [])
        self.assertEqual(log_messages, [])

    def test_commented_out_call_is_ignored(self) -> None:
        text = """
        // CALL ShowFatalError('Foo happened, node=' // NodeName // &
        // ', check for details in the following messages.')
        """
        sites, log_messages = check_text(text, self.dummy_file, self.enum_entries_by_name)
        self.assertEqual(sites, [])
        self.assertEqual(log_messages, [])


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "test":
        del sys.argv[1:]
        unittest.main(exit=True, verbosity=0)

    parser = get_base_parser(
        description="Check DataErrorTracking::ErrorSummaryType entries against their ErrorSummaryCount call sites",
        include_files_arg=False,
    )
    args = parser.parse_args()

    enum_entries = parse_enum_entries(DATA_ERROR_TRACKING_HH)
    enum_entries_by_name = {e.name: e for e in enum_entries}
    summary_entries = parse_error_summaries(UTILITY_ROUTINES_CC)

    log_messages: list[LogMessage] = []

    # Sanity check: the ErrorSummaries array must mirror the enum, in order.
    enum_names = [e.name for e in enum_entries]
    summary_names = [s.name for s in summary_entries]
    if enum_names != summary_names:
        log_messages.append(
            ErrorMessage(
                tool="check_error_summary_increment",
                filepath=UTILITY_ROUTINES_CC,
                message=(
                    "ErrorSummaries array order/contents does not match the ErrorSummaryType enum.\n"
                    f"  enum:     {enum_names}\n"
                    f"  array:    {summary_names}"
                ),
            )
        )

    for entry in enum_entries:
        if entry.search_string is None:
            log_messages.append(
                ErrorMessage(
                    tool="check_error_summary_increment",
                    filepath=DATA_ERROR_TRACKING_HH,
                    line_number=entry.line_number,
                    message=f'ErrorSummaryType::{entry.name} has no trailing `// "search string"` comment to check it against',
                )
            )

    files = list(collect_files(base_dir=SRC_DIR, extensions={".cc"}, recursive=True, dirs_to_skip=[]))
    if args.verbose:
        print(f"Checking {len(files)} .cc files under {SRC_DIR}")

    results = parallel_apply(func=check_file, filepaths=files, enum_entries_by_name=enum_entries_by_name)
    all_sites: list[IncrementSite] = flatten_list_of_lists([sites for sites, _ in results])
    log_messages += flatten_list_of_lists([msgs for _, msgs in results])

    live_enum_names = {site.enum_name for site in all_sites}
    for entry in enum_entries:
        if entry.name not in live_enum_names:
            log_messages.append(
                ErrorMessage(
                    tool="check_error_summary_increment",
                    filepath=DATA_ERROR_TRACKING_HH,
                    line_number=entry.line_number,
                    message=(
                        f"ErrorSummaryType::{entry.name} has no ++ErrorSummaryCount[...] call site anywhere under {SRC_DIR}; "
                        "it is dead and should be removed from the ErrorSummaryType enum and the ErrorSummaries array"
                    ),
                )
            )

    success = report_log_messages(log_messages=log_messages, fail_threshold=LogLevel.ERROR, verbose=args.verbose)
    exit_hook(success=success)
