#!/usr/bin/env python3
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

import argparse
import os
import subprocess
from pathlib import Path
from typing import List, Set

import requests
from packaging.version import Version

ROOT_DIR = Path(__file__).parent.parent.parent
WORKFLOW_DIR = ROOT_DIR / ".github/workflows"


def get_github_token():
    if "GITHUB_TOKEN" in os.environ:
        return os.environ["GITHUB_TOKEN"]

    try:
        return subprocess.check_output(["gh", "auth", "token"], universal_newlines=True, encoding="utf-8").strip()
    except Exception:
        print("GITHUB_TOKEN not in ENV variable, and `gh` CLI not available")
        return None


def get_uses(workflows: List[Path]) -> Set[str]:
    uses = []
    for workflow_path in workflows:
        content = workflow_path.read_text()
        lines = content.splitlines()
        for i, line in enumerate(lines):
            if "uses" in line:
                action = line.split("uses: ")[1].split("#")[0].strip()
                uses.append(action)
    return set(uses)


def convert_to_version(tag_name):
    try:
        return Version(tag_name.replace("v", ""))
    except:
        return Version("0.0.0")


def check_latest(uses: Set[str]) -> dict[str, Version]:
    unique_actions = set([use.split("@")[0] for use in set(uses)])

    headers = None
    gha_token = get_github_token()
    if gha_token is not None:
        headers = {"Authorization": f"token {gha_token}"}
    latests = {}
    for action in unique_actions:
        r = requests.get(f"https://api.github.com/repos/{action}/releases", headers=headers)
        r.raise_for_status()
        latests[action] = next(reversed(sorted([convert_to_version(x["tag_name"]) for x in r.json()])))
    return latests


def get_replacements(uses: Set[str], latests: dict[str, Version]) -> dict[str, str]:
    replacements = {}
    for use in uses:
        action, version = use.split("@")
        assert action in latests, f"{action} not found in latests: {latests}"
        replacement_v: str = ""
        if "." in version:
            replacement_v = str(latests[action])
        else:
            replacement_v = str(latests[action].major)
        replacement = f"{action}@v{replacement_v}"
        if replacement == use:
            print(f"No updates found for {use}")
            continue
        replacements[use] = replacement
    return replacements


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Update GHA Actions.")
    parser.add_argument("--safe", action="store_true", default=False, help="Only update official GHA actions/xxx")
    args = parser.parse_args()

    workflows = list(WORKFLOW_DIR.glob("*.yml"))
    uses = get_uses(workflows=workflows)
    if args.safe:
        print("Limiting to official GHA actions/xxx")
        uses = set([x for x in uses if x.startswith("actions/")])

    latests = check_latest(uses=uses)
    replacements = get_replacements(uses=uses, latests=latests)
    for workflow_path in workflows:
        content = workflow_path.read_text()
        for ori, new in replacements.items():
            content = content.replace(ori, new)
        workflow_path.write_text(content)
