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

"""Set CTest COST values from measured CTestCostData.txt files.

ctest -j starts tests in descending COST order. On a fresh build directory (CI), there is no measured
data yet, so the COST properties declared in CMake decide which tests start first. A slow test that
starts late leaves the run waiting on it at the end.

This script picks the N slowest simulation tests, plus Transition.transition_tests (always, since it is
declared last), and writes their measured duration (in seconds) as the COST, removing COST from every
other test:

* add_simulation_test() calls in testfiles/CMakeLists.txt: COST (non-Debug) and DEBUG_COST (Debug)
* Transition.transition_tests in src/Transition/CMakeLists.txt

Release and Debug are independent: only the side(s) for which a CTestCostData.txt is given are
updated, the other side keeps its current values.

CTestCostData.txt is at <build dir>/Testing/Temporary/CTestCostData.txt after a ctest run.
Use a complete, recent run: tests missing from the file are treated as not slow.
"""

import argparse
import re
import sys
from dataclasses import dataclass
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parents[2]
TESTFILES_CMAKE = Path("testfiles/CMakeLists.txt")
TRANSITION_CMAKE = Path("src/Transition/CMakeLists.txt")
TRANSITION_TEST = "Transition.transition_tests"
SIDES = ("release", "debug")
CMAKE_ARG = {"release": "COST", "debug": "DEBUG_COST"}


def parse_ctest_cost_data(path: Path) -> dict[str, float]:
    """Return {test name: average duration in seconds}. Lines after '---' list failed tests and are skipped."""
    costs = {}
    for line in path.read_text().splitlines():
        line = line.strip()
        if line == "---":
            break
        if not line:
            continue
        name, _n_runs, cost = line.rsplit(maxsplit=2)
        costs[name] = float(cost)
    return costs


def to_cost(seconds: float) -> int:
    return max(1, round(seconds))


@dataclass
class SimulationTestCall:
    """One add_simulation_test(...) call in testfiles/CMakeLists.txt."""

    start: int  # offset of the text between the parentheses
    end: int
    inner: str
    test_name: str
    expect_fatal: bool

    def get(self, arg: str) -> int | None:
        match = re.search(rf"\s{arg}\s+(\S+)", self.inner)
        return int(float(match.group(1))) if match else None


def find_simulation_test_calls(text: str) -> list[SimulationTestCall]:
    calls = []
    # Only calls at the start of a line (possibly indented), so commented-out calls are skipped
    for match in re.finditer(r"^[ \t]*add_simulation_test\s*\(", text, re.M | re.I):
        start = match.end()
        end = text.index(")", start)  # arguments never contain parentheses
        inner = text[start:end]
        tokens = inner.split()
        idf_file = tokens[tokens.index("IDF_FILE") + 1]
        # Same naming as ADD_SIMULATION_TEST in cmake/ProjectMacros.cmake (get_filename_component NAME_WE)
        category = "performance" if "PERFORMANCE" in tokens else "integration"
        test_name = f"{category}.{Path(idf_file).name.split('.', 1)[0]}"
        calls.append(SimulationTestCall(start, end, inner, test_name, "EXPECT_FATAL" in tokens))
    return calls


def rewrite_inner(inner: str, values: dict[str, int | None]) -> str:
    """Remove COST / DEBUG_COST from the call arguments and append the non-empty ones, keeping the call's layout."""
    inner = re.sub(r"\s+(?:DEBUG_)?COST\s+\S+", "", inner)
    args = [(CMAKE_ARG[side], values[side]) for side in SIDES if values[side]]
    if not args:
        return inner
    if "\n" in inner:
        # One argument per line: reuse the indentation of the IDF_FILE line
        indent = re.search(r"^([ \t]*)IDF_FILE", inner, re.M).group(1)
        return inner + "".join(f"\n{indent}{key}\n{indent}{value}" for key, value in args)
    return inner + "".join(f" {key} {value}" for key, value in args)


TRANSITION_RE = re.compile(rf"(set_tests_properties\(\s*{re.escape(TRANSITION_TEST)}\s+PROPERTIES)([^)]*)\)")
GENEX_RE = re.compile(r'"\$<IF:\$<CONFIG:Debug>,(\d+),(\d+)>"')


def get_transition_costs(text: str) -> dict[str, int | None]:
    props = TRANSITION_RE.search(text).group(2)
    if match := GENEX_RE.search(props):
        return {"release": int(match.group(2)) or None, "debug": int(match.group(1)) or None}
    if match := re.search(r"\sCOST\s+(\d+)", props):
        return {"release": int(match.group(1)), "debug": int(match.group(1))}
    return {"release": None, "debug": None}


def set_transition_costs(text: str, values: dict[str, int | None]) -> str:
    def replace(match: re.Match) -> str:
        props = re.sub(r'\s+COST\s+("[^"]*"|\S+)', "", match.group(2))
        release, debug = values["release"] or 0, values["debug"] or 0
        if release == debug:
            if release:
                props += f" COST {release}"
        else:
            props += f' COST "$<IF:$<CONFIG:Debug>,{debug},{release}>"'
        return f"{match.group(1)}{props})"

    return TRANSITION_RE.sub(replace, text, count=1)


def select_tests(measured: dict[str, float], candidates: set[str], top: int) -> tuple[dict[str, int], list[str]]:
    """Return ({test: cost} for the `top` slowest simulation tests, [slower tests that cannot be annotated]).

    Transition.transition_tests does not count towards `top` and is always included: it is declared after every
    other test, so even when it is not among the slowest (Debug), it would otherwise start last and run alone at the end.
    """
    slowest = sorted((t for t in measured if t in candidates and t != TRANSITION_TEST), key=measured.get, reverse=True)[:top]
    selected = {t: to_cost(measured[t]) for t in slowest}
    if TRANSITION_TEST in measured:
        selected[TRANSITION_TEST] = to_cost(measured[TRANSITION_TEST])
    threshold = min((measured[t] for t in slowest), default=float("inf"))
    skipped = [t for t in measured if t not in candidates and measured[t] >= threshold]
    return selected, sorted(skipped, key=measured.get, reverse=True)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--release", type=Path, metavar="CTestCostData.txt", help="Cost data from a Release build (sets COST)")
    parser.add_argument("--debug", type=Path, metavar="CTestCostData.txt", help="Cost data from a Debug build (sets DEBUG_COST)")
    parser.add_argument("-n", "--top", type=int, default=30, help="Number of slowest simulation tests to give a cost to, per build type, in addition to Transition.transition_tests (default: %(default)s)")
    parser.add_argument("--source-dir", type=Path, default=ROOT_DIR, help="EnergyPlus source directory (default: %(default)s)")
    parser.add_argument("--dry-run", action="store_true", help="Report the changes without writing files")
    args = parser.parse_args()

    measured = {side: parse_ctest_cost_data(path) for side in SIDES if (path := getattr(args, side)) is not None}
    if not measured:
        parser.error("give at least one of --release / --debug")

    testfiles_path = args.source_dir / TESTFILES_CMAKE
    transition_path = args.source_dir / TRANSITION_CMAKE
    testfiles_text = testfiles_path.read_text()
    transition_text = transition_path.read_text()

    # Current values, per test (all calls for one test, e.g. in if/else branches, are kept in sync)
    calls = find_simulation_test_calls(testfiles_text)
    current: dict[str, dict[str, int | None]] = {}
    for call in calls:
        if not call.expect_fatal:
            current.setdefault(call.test_name, {side: call.get(CMAKE_ARG[side]) for side in SIDES})
    current[TRANSITION_TEST] = get_transition_costs(transition_text)

    new = {test: dict(values) for test, values in current.items()}
    for side, data in measured.items():
        selected, skipped = select_tests(data, set(current), args.top)
        for test in new:
            new[test][side] = selected.get(test)
        simulation_costs = [cost for test, cost in selected.items() if test != TRANSITION_TEST]
        print(
            f"{side}: {len(simulation_costs)} simulation tests get a {CMAKE_ARG[side]}, from {min(simulation_costs, default=0)}s"
            f" to {max(simulation_costs, default=0)}s, {TRANSITION_TEST}: {selected.get(TRANSITION_TEST, '-')}"
        )
        if skipped:
            print(f"  Not annotated (no add_simulation_test call / not supported by this script): {', '.join(skipped)}")
        changes = [
            (test, current[test][side], new[test][side]) for test in new if current[test][side] != new[test][side]
        ]
        for test, old_value, new_value in sorted(changes, key=lambda c: -(c[2] or 0)):
            print(f"  {test:75} {str(old_value or '-'):>5} -> {new_value or '-'}")

    # Rewrite testfiles/CMakeLists.txt from the end so earlier offsets stay valid
    for call in reversed(calls):
        values = {side: None for side in SIDES} if call.expect_fatal else new[call.test_name]
        testfiles_text = testfiles_text[: call.start] + rewrite_inner(call.inner, values) + testfiles_text[call.end :]
    transition_text = set_transition_costs(transition_text, new[TRANSITION_TEST])

    if args.dry_run:
        print("Dry run: no files written")
        return 0
    for path, text in ((testfiles_path, testfiles_text), (transition_path, transition_text)):
        if text != path.read_text():
            path.write_text(text)
            print(f"Updated {path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
