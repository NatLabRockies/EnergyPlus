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

import argparse
from pathlib import Path
from sys import exit


def build_regression_summary(
    matrix_os: str, github_sha: str, github_run_id: str, artifact_url: str, repository: str, regressions_dir: Path
) -> None:
    summary_input_md_file = regressions_dir / "summary.md"
    summary_output_js_file = regressions_dir / "summary.js"

    if not summary_input_md_file.exists():
        print("Regression script shows failure exit code, but could not find summary file.")
        print("This generally indicates that the regression script had an unhandled failure.")
        print("Check the 'Run Regressions' GitHub Action step above for more helpful information")
        exit(1)

    md_contents = summary_input_md_file.read_text()

    fixed_up_contents = f"""
### :warning: Regressions detected on {matrix_os} for commit {github_sha}

{md_contents}

 - [View Results](https://github.com/{repository}/actions/runs/{github_run_id})
 - [Download Regressions]({artifact_url})
"""

    js_contents = f"""
module.exports = ({{github, context}}) => {{
    github.rest.issues.createComment({{
        issue_number: context.issue.number,
        owner: context.repo.owner,
        repo: context.repo.repo,
        body: `{fixed_up_contents}`
    }})
}}
"""
    summary_output_js_file.write_text(js_contents)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Build regression summary JS file for GitHub Actions comment")
    parser.add_argument("--os", required=True, dest="matrix_os", help="Matrix OS name")
    parser.add_argument("--sha", required=True, dest="github_sha", help="GitHub commit SHA")
    parser.add_argument("--run-id", required=True, dest="github_run_id", help="GitHub Actions run ID")
    parser.add_argument("--artifact-url", required=True, help="URL to download regression artifacts")
    parser.add_argument("--repository", required=True, help="GitHub repository in owner/repo format")
    parser.add_argument("--regressions-dir", required=True, type=Path, help="Path to regressions folder")
    args = parser.parse_args()

    build_regression_summary(
        matrix_os=args.matrix_os,
        github_sha=args.github_sha,
        github_run_id=args.github_run_id,
        artifact_url=args.artifact_url,
        repository=args.repository,
        regressions_dir=args.regressions_dir,
    )
