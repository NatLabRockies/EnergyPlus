#!/usr/bin/python
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

from __future__ import absolute_import, print_function, unicode_literals

import io
import json
import sys
from subprocess import CalledProcessError, check_output

try:
    from urllib import urlencode
except ImportError:
    from urllib.parse import urlencode
try:
    from urllib2 import Request, urlopen
except ImportError:
    from urllib.request import Request, urlopen

# this probably won't change
RepoName = "NREL/EnergyPlus"
EPlusRepoPath = "https://github.com/" + RepoName
debug = False


def usage():
    print(
        """Script should be called with 8 positional arguments:
 - the path to a repository
 - the path to a markdown output file
 - the path to a html output file
 - the path to a local git executable
 - a github token for performing authentication API requests
 - the commit SHA for the last major release
 - a program version identifier"""
    )


# command line arguments: repo base path, output markdown and html file paths, a git exe path, and a github token
if len(sys.argv) == 7:
    repo = sys.argv[1]
    md_file = sys.argv[2]
    html_file = sys.argv[3]
    git_exe = sys.argv[4]
    github_token = sys.argv[5]
    last_commit = sys.argv[6]
    program_version = sys.argv[7]
elif len(sys.argv) == 8:
    repo = sys.argv[1]
    md_file = sys.argv[2]
    html_file = sys.argv[3]
    git_exe = sys.argv[4]
    github_token = sys.argv[5]
    last_commit = sys.argv[6]
    program_version = sys.argv[7]
else:
    usage()
    sys.exit(1)

# get the pull request numbers
try:
    log_full = check_output([git_exe, "log", last_commit + ".."]).decode("utf-8")
except CalledProcessError as ex:
    log_full = ""
    pass  # add error handling
log_full_split = log_full.split("\n")
log_merge_prs = [
    x for x in log_full_split if x.strip().startswith("Merge pull request") and not x.strip().startswith("Revert")
]
pr_tokens = [x.split(" ")[7] for x in log_merge_prs]
pr_numbers = sorted([x[1:] for x in pr_tokens])

# create and initialize the master array, with known keys plus an "Unknown" key
ValidPRTypes = ["Defect", "NewFeature", "Performance", "Refactoring", "DoNotPublish"]
PRS = {"Unknown": []}
for valid_pr_type in ValidPRTypes:
    PRS[valid_pr_type] = []

# use the GitHub API to get pull request info
for pr_num in pr_numbers:

    # we need to skip very low numbers of pull requests, for example:
    # - a user wants to contribute a change to E+, so they create a fork/branch
    # - their operations result in a pull request into their own repo, so the counting starts at #1...
    # we're at like 5000+, so if we just skip anything less than 1000, we'll be good.
    try:
        i = int(pr_num)
    except ValueError:
        print(f'WARNING: Something wrong with PR number: "{pr_num}"')
        continue
    if int(pr_num) < 1000:
        continue

    # set the url for this pull request
    github_url = "https://api.github.com/repos/NREL/EnergyPlus/issues/" + pr_num

    # make the request
    try:
        req = Request(github_url, headers={"Authorization": "token %s" % github_token})
        response = urlopen(req)
        the_page = response.read().decode("utf-8")
    except Exception as e:
        print("ERROR: " + str(e))
        continue

    # read the json response
    j = json.loads(the_page)

    # mine the data
    title = j["title"]
    labels = j["labels"]
    if len(labels) == 0:
        print("WARNING: No labels on PR #" + pr_num, file=sys.stderr)
    for label in labels:
        key = "Unknown"
        label_name = label["name"]
        if label_name in ValidPRTypes:
            PRS[label_name].append([pr_num, title])

# Now write the nice markdown output file
with io.open(md_file, "w", encoding="utf-8") as f:

    def out(s):
        print(s, file=f)

    def out_pr_class(pr_type, descriptor):
        out("")
        out("## " + descriptor)
        for pr in PRS[pr_type]:
            out(" - [#" + pr[0] + "](" + EPlusRepoPath + "/pull/" + pr[0] + ") : " + pr[1])

    out("# Changelog for EnergyPlus " + program_version)
    out(
        "Consists of pull requests merged in this release - starting with SHA [%s](https://github.com/%s/commit/%s)"
        % (last_commit, RepoName, last_commit)
    )
    out_pr_class("NewFeature", "New Features")
    out_pr_class("Performance", "Performance Enhancements")
    out_pr_class("Defect", "Defects Repaired")
    out_pr_class("Refactoring", "Under the Hood Restructuring")
    if debug:
        out_pr_class("Unknown", "Other-DevelopersFixPlease")

with io.open(html_file, "w", encoding="utf-8") as f2:

    def out(s):
        print(s, file=f2)

    def out_pr_class(pr_type, descriptor):
        out("<h2>" + descriptor + "</h2>")
        out('<table border="1" >')
        out(" <tr>")
        out("  <th>PR #</th>")
        out("  <th>Description</th>")
        out(" </tr>")
        for pr in PRS[pr_type]:
            out(" <tr>")
            out('  <td><a href="' + EPlusRepoPath + "/pull/" + pr[0] + '">' + pr[0] + "</a></td>")
            out("  <td>" + pr[1] + "</td>")
            out(" </tr>")
        out("</table>")

    out("<html>")
    out("<head><title>EnergyPlus ChangeLog</title></head>")
    out("<body>")
    out("<style>")
    out("table, th, td {")
    out(" border: 1px solid black;")
    out(" border-collapse: collapse;")
    out("}")
    out("th,td {")
    out(" padding: 6px;")
    out("}")
    out("</style>")
    out("<h1>ChangeLog for EnergyPlus " + program_version + "</h1>")
    out("<h1>Consists of pull requests merged in this release")
    out(
        '- starting with SHA <a href = "https://github.com/%s/commit/%s">%s</a></h1>'
        % (RepoName, last_commit, last_commit)
    )
    out_pr_class("NewFeature", "New Features")
    out_pr_class("Performance", "Performance Enhancements")
    out_pr_class("Defect", "Defects Repaired")
    out_pr_class("Refactoring", "Under the Hood Restructuring")
    out("</body>")
    out("</html>")

print(" +++ AutoDocs: Completed processing changelog: processed %i merged pull requests" % len(pr_numbers))
