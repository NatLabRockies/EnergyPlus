#!/usr/bin/env python
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
import json
import os
import sys
from collections import defaultdict
from datetime import UTC, datetime
from pathlib import Path
from shutil import copy, rmtree
from traceback import print_exc
from zoneinfo import ZoneInfo
import multiprocessing
from concurrent.futures import ProcessPoolExecutor, as_completed

from energyplus_regressions.builds.base import BuildTree
from energyplus_regressions.runtests import SuiteRunner
from energyplus_regressions.structures import EndErrSummary, TestEntry, TextDifferences


def run_single_entry(
    idf_name: str, base_testfiles_dir: Path, mod_testfiles_dir: Path, threshold_file: Path
) -> TestEntry:
    # print(f"[Worker {os.getpid()}] Running {idf_name}", flush=True)
    # print(f"  Base dir: {base_testfiles_dir}", flush=True)
    # print(f"  Mod dir: {mod_testfiles_dir}", flush=True)
    # print(f"  Threshold file: {threshold_file}", flush=True)
    b1 = BuildTree()
    b1.build_dir = base_testfiles_dir / idf_name
    b2 = BuildTree()
    b2.build_dir = mod_testfiles_dir / idf_name
    entry = TestEntry(idf_name, "")
    entry, _ = SuiteRunner.process_diffs_for_one_case(
        entry, b1, b2, "", str(threshold_file), ci_mode=True
    )  # returns an updated entry
    # print(entry.table_diffs)

    return entry


class RegressionManager:

    def __init__(self):
        self.root_index_files_no_diff = []
        self.root_index_files_diffs = []
        self.root_index_files_failed = []
        self.diffs_by_idf = defaultdict(list)
        self.diffs_by_type = defaultdict(list)
        self.summary_results = {}
        self.num_idf_inspected = 0
        # self.all_files_compared = []  TODO: need to get this from regression runner
        import energyplus_regressions

        self.threshold_file = str(Path(energyplus_regressions.__file__).parent / "diffs" / "math_diff.config")

    def process_single_file_regressions(self, idf_name: str, entry: TestEntry) -> [TestEntry, bool]:

        self.num_idf_inspected += 1
        this_file_diffs = []

        self.summary_results[idf_name] = entry.summary_result

        has_diffs = False

        text_diff_results = {
            "Audit": entry.aud_diffs,
            "BND": entry.bnd_diffs,
            "DELightIn": entry.dl_in_diffs,
            "DELightOut": entry.dl_out_diffs,
            "DXF": entry.dxf_diffs,
            "EIO": entry.eio_diffs,
            "ERR": entry.err_diffs,
            "Readvars_Audit": entry.readvars_audit_diffs,
            "EDD": entry.edd_diffs,
            "WRL": entry.wrl_diffs,
            "SLN": entry.sln_diffs,
            "SCI": entry.sci_diffs,
            "MAP": entry.map_diffs,
            "DFS": entry.dfs_diffs,
            "SCREEN": entry.screen_diffs,
            "GLHE": entry.glhe_diffs,
            "MDD": entry.mdd_diffs,
            "MTD": entry.mtd_diffs,
            "RDD": entry.rdd_diffs,
            "SHD": entry.shd_diffs,
            "PERF_LOG": entry.perf_log_diffs,
            "IDF": entry.idf_diffs,
            "StdOut": entry.stdout_diffs,
            "StdErr": entry.stderr_diffs,
        }
        for diff_type, diffs in text_diff_results.items():
            if diffs is None:
                continue
            if diffs.diff_type != TextDifferences.EQUAL:
                has_diffs = True
                this_file_diffs.append(diff_type)
                self.diffs_by_type[diff_type].append(idf_name)
                self.diffs_by_idf[idf_name].append(diff_type)

        numeric_diff_results = {
            "ESO": entry.eso_diffs,
            "MTR": entry.mtr_diffs,
            "SSZ": entry.ssz_diffs,
            "ZSZ": entry.zsz_diffs,
            "JSON": entry.json_diffs,
        }
        for diff_type, diffs in numeric_diff_results.items():
            if diffs is None:
                continue
            if diffs.diff_type == "Big Diffs":
                has_diffs = True
                this_file_diffs.append(f"{diff_type} Big Diffs")
                self.diffs_by_type[f"{diff_type} Big Diffs"].append(idf_name)
                self.diffs_by_idf[idf_name].append(f"{diff_type} Big Diffs")
            elif diffs.diff_type == "Small Diffs":
                has_diffs = True
                this_file_diffs.append(f"{diff_type} Small Diffs")
                self.diffs_by_type[f"{diff_type} Small Diffs"].append(idf_name)
                self.diffs_by_idf[idf_name].append(f"{diff_type} Small Diffs")

        if entry.table_diffs:
            if entry.table_diffs.big_diff_count > 0:
                has_diffs = True
                this_file_diffs.append("Table Big Diffs")
                self.diffs_by_type["Table Big Diffs"].append(idf_name)
                self.diffs_by_idf[idf_name].append("Table Big Diffs")
            elif entry.table_diffs.small_diff_count > 0:
                has_diffs = True
                this_file_diffs.append("Table Small Diffs")
                self.diffs_by_type["Table Small Diffs"].append(idf_name)
                self.diffs_by_idf[idf_name].append("Table Small Diffs")
            if entry.table_diffs.string_diff_count > 1:  # There's always one...the time stamp
                has_diffs = True
                this_file_diffs.append("Table String Diffs")
                self.diffs_by_type["Table String Diffs"].append(idf_name)
                self.diffs_by_idf[idf_name].append("Table String Diffs")

        return entry, has_diffs

    @staticmethod
    def single_diff_html(contents: str) -> str:
        return f"""
<!doctype html>
<html>
 <head>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.6.0/styles/default.min.css">
  <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.6.0/highlight.min.js"></script>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.6.0/languages/diff.min.js"></script>
 </head>
 <body>
  <pre>
   <code class="diff">
    {contents}
   </code>
  </pre>
  <script>hljs.highlightAll();</script>
 </body>
</html>"""

    @staticmethod
    def regression_row_in_single_test_case_html(diff_file_name: str) -> str:
        return f"""
   <tr>
    <td>{diff_file_name}</td>
    <td><a href='{diff_file_name}' download='{diff_file_name}'>download</a></td>
    <td><a href='{diff_file_name}.html'>view</a></td>
   </tr>"""

    @staticmethod
    def single_test_case_html(contents: str) -> str:
        return f"""
<!doctype html>
<html>
 <head>
  <link rel="stylesheet" href="https://netdna.bootstrapcdn.com/bootstrap/3.1.1/css/bootstrap.min.css">
  <link rel="stylesheet" href="https://netdna.bootstrapcdn.com/bootstrap/3.1.1/css/bootstrap-theme.min.css">
  <script src="https://code.jquery.com/jquery-2.1.1.min.js"></script>
  <script src="https://code.jquery.com/jquery-migrate-1.2.1.min.js"></script>
  <script src="https://netdna.bootstrapcdn.com/bootstrap/3.1.1/js/bootstrap.min.js"></script>
 </head>
 <body>
  <table class='table table-hover'>
   <tr>
    <th>filename</th>
    <th></th>
    <th></th>
   </tr>
{contents}
  </table>
 </body>
</html>"""

    def bundle_root_index_html(self, header_info: list[str]) -> str:
        # set up header table
        header_content = ""
        for hi in header_info:
            header_content += f"""<li class="list-group-item">{hi}</li>\n"""

        # set up diff summary listings
        num_no_diff = len(self.root_index_files_no_diff)
        nds = "s" if num_no_diff == 0 or num_no_diff > 1 else ""
        no_diff_content = ""
        for nd in self.root_index_files_no_diff:
            no_diff_content += f"""<li class="list-group-item">{nd}</li>\n"""
        num_diff = len(self.root_index_files_diffs)
        ds = "s" if num_diff == 0 or num_diff > 1 else ""
        diff_content = ""
        for d in self.root_index_files_diffs:
            diff_content += f"""<a href="{d}/index.html" class="list-group-item list-group-item-action">{d}</a>\n"""
        num_failed = len(self.root_index_files_failed)
        nfs = "s" if num_failed == 0 or num_failed > 1 else ""
        failed_content = ""
        for nf in self.root_index_files_failed:
            failed_content += f"""<li class="list-group-item">{nf}</li>\n"""

        # set up diff type listing
        diff_type_keys = sorted(self.diffs_by_type.keys())
        num_diff_types = len(diff_type_keys)
        dt = "s" if num_diff_types == 0 or num_diff_types > 1 else ""
        diff_type_content = ""
        if num_diff_types > 0:
            for k in diff_type_keys:
                nice_type_key = k.lower().replace(" ", "")
                diffs_this_type = self.diffs_by_type[k]
                num_files_this_type = len(diffs_this_type)
                dtt = "s" if num_diff_types == 0 or num_diff_types > 1 else ""
                this_diff_type_list = ""
                for idf in diffs_this_type:
                    this_diff_type_list += (
                        f"""<a href="{idf}/index.html" class="list-group-item list-group-item-action">{idf}</a>\n"""
                    )
                diff_type_content += f"""
   <div class="panel-group">
    <div class="panel panel-default">
     <div class="panel-heading">
      <h4 class="panel-title">
       <a data-toggle="collapse" href="#{nice_type_key}">{k}: {num_files_this_type} File{dtt}</a>
      </h4>
     </div>
     <div id="{nice_type_key}" class="panel-collapse collapse">
      <div class="panel-body">
       <ul class="list-group">
{this_diff_type_list}
       </ul>
      </div>
     </div>
    </div>
   </div>"""

        # set up runtime results table
        run_time_rows_text = ""
        sum_base_seconds = 0
        sum_branch_seconds = 0
        sorted_idf_keys = sorted(self.summary_results.keys())
        for idf in sorted_idf_keys:
            summary = self.summary_results[idf]
            case_1_success = summary.simulation_status_case1 == EndErrSummary.STATUS_SUCCESS
            case_2_success = summary.simulation_status_case2 == EndErrSummary.STATUS_SUCCESS
            if case_1_success:
                base_time = summary.run_time_seconds_case1
            else:
                base_time = "N/A"
            if case_2_success:
                branch_time = summary.run_time_seconds_case2
            else:
                branch_time = "N/A"
            if case_1_success and case_2_success:
                sum_base_seconds += base_time
                sum_branch_seconds += branch_time

            run_time_rows_text += (
                f"""<tr><td><a href='{idf}/index.html'>{idf}</a></td><td>{base_time}</td><td>{branch_time}</td></tr>"""
            )
        run_time_rows_text += f"""<tr><td>Runtime Total (Successes)</td><td>{sum_base_seconds:.1f}</td><td>{sum_branch_seconds:.1f}</td></tr>"""

        return f"""
<!doctype html>
<html>
 <head>
  <link rel="stylesheet" href="https://netdna.bootstrapcdn.com/bootstrap/3.1.1/css/bootstrap.min.css">
  <link rel="stylesheet" href="https://netdna.bootstrapcdn.com/bootstrap/3.1.1/css/bootstrap-theme.min.css">
  <script src="https://code.jquery.com/jquery-2.1.1.min.js"></script>
  <script src="https://code.jquery.com/jquery-migrate-1.2.1.min.js"></script>
  <script src="https://netdna.bootstrapcdn.com/bootstrap/3.1.1/js/bootstrap.min.js"></script>
 </head>
 <body>
  <div class="container-fluid">

   <h1>EnergyPlus Regressions</h1>

   <div class="panel-group" id="accordion_header">
    <div class="panel panel-default">
     <div class="panel-heading">
      <h4 class="panel-title">
       <a data-toggle="collapse" data-parent="#accordion_header" href="#header">Header Information</a>
      </h4>
     </div>
     <div id="header" class="panel-collapse collapse">
      <div class="panel-body">
       <ul class="list-group">
{header_content}
       </ul>
      </div>
     </div>
    </div>
   </div>

   <hr>

   <h2>Summary by File</h1>

   <div class="panel-group">
    <div class="panel panel-default">
     <div class="panel-heading">
      <h4 class="panel-title">
       <a data-toggle="collapse" href="#no_diffs">{num_no_diff} File{nds} with No Diffs</a>
      </h4>
     </div>
     <div id="no_diffs" class="panel-collapse collapse">
      <div class="panel-body">
       <ul class="list-group">
{no_diff_content}
       </ul>
      </div>
     </div>
    </div>
   </div>

   <div class="panel-group">
    <div class="panel panel-default">
     <div class="panel-heading">
      <h4 class="panel-title">
       <a data-toggle="collapse" href="#diffs">{num_diff} File{ds} with Diffs</a>
      </h4>
     </div>
     <div id="diffs" class="panel-collapse collapse">
      <div class="panel-body">
       <ul class="list-group">
{diff_content}
       </ul>
      </div>
     </div>
    </div>
   </div>


   <div class="panel-group">
    <div class="panel panel-default">
     <div class="panel-heading">
      <h4 class="panel-title">
       <a data-toggle="collapse" href="#failed">{num_failed} File{nfs} Failed During Regression Processing</a>
      </h4>
     </div>
     <div id="failed" class="panel-collapse collapse">
      <div class="panel-body">
       <ul class="list-group">
{failed_content}
       </ul>
      </div>
     </div>
    </div>
   </div>

   <hr>

   <h2>Summary by Diff Type</h1>

   <div class="panel-group">
    <div class="panel panel-default">
     <div class="panel-heading">
      <h4 class="panel-title">
       <a data-toggle="collapse" href="#by_diff_type">{num_diff_types} Diff Type{dt} Encountered</a>
      </h4>
     </div>
     <div id="by_diff_type" class="panel-collapse collapse">
      <div class="panel-body">
       <ul class="list-group">
{diff_type_content}
       </ul>
      </div>
     </div>
    </div>
   </div>

   <hr>

   <h2>Run Times</h2>

   <div class="panel-group">
    <div class="panel panel-default">
     <div class="panel-heading">
      <h4 class="panel-title">
       <a data-toggle="collapse" href="#run_times">Runtime Results Table</a>
      </h4>
     </div>
     <div id="run_times" class="panel-collapse collapse">
      <div class="panel-body">
       <table class='table table-hover'>
        <tr>
         <th>Filename</th>
         <th>Base Case Runtime (seconds)</th>
         <th>Branch Case Runtime (seconds)</th>
        </tr>
{run_time_rows_text}
       </table>
      </div>
     </div>
    </div>
   </div>

  </div>
 </body>
</html>
"""

    def generate_markdown_summary(self, bundle_root: Path):
        diff_lines = ""
        for diff_type, idfs in self.diffs_by_type.items():
            diff_lines += f"  - {diff_type}: {len(idfs)}\n"
        content = f"""
<details>
  <summary>Regression Summary</summary>

{diff_lines}
</details>"""
        (bundle_root / "summary.md").write_text(content)

    def check_all_regressions(self, base_testfiles: Path, mod_testfiles: Path, bundle_root: Path) -> bool:
        any_diffs = False
        bundle_root.mkdir(exist_ok=True)
        idf_files = []
        for baseline in sorted(base_testfiles.iterdir()):
            if not baseline.is_dir():
                continue
            if baseline.name == "CMakeFiles":  # add more ignore dirs here
                continue
            modified = mod_testfiles / baseline.name
            if not modified.exists():
                # print(f"Modified directory missing: '{modified}'")
                continue  # TODO: Should we warn that it is missing?
            idf_files.append(baseline.name)

        backtrace_shown = False

        max_workers = multiprocessing.cpu_count()
        with ProcessPoolExecutor(max_workers=max_workers) as executor:

            future_to_idf = {
                executor.submit(
                    run_single_entry, idf_file, base_testfiles, mod_testfiles, self.threshold_file
                ): idf_file
                for idf_file in idf_files
            }

            for entry_num, future in enumerate(as_completed(future_to_idf)):
                idf_file = future_to_idf[future]
                try:
                    entry = future.result()

                    entry, diffs = self.process_single_file_regressions(idf_file, entry)
                    if diffs:
                        self.root_index_files_diffs.append(idf_file)
                        any_diffs = True
                        potential_diff_files = (base_testfiles / idf_file).glob(
                            "*.*.*"
                        )  # TODO: Could try to get this from the regression tool
                        target_dir_for_this_file_diffs = bundle_root / idf_file
                        if potential_diff_files:
                            if target_dir_for_this_file_diffs.exists():
                                rmtree(target_dir_for_this_file_diffs)
                            target_dir_for_this_file_diffs.mkdir()
                            index_contents_this_file = ""
                            for potential_diff_file in potential_diff_files:
                                copy(potential_diff_file, target_dir_for_this_file_diffs)
                                diff_file_with_html = target_dir_for_this_file_diffs / (
                                    potential_diff_file.name + ".html"
                                )
                                if potential_diff_file.name.endswith(".htm"):
                                    # already a html file, just upload the raw contents but renamed as ...htm.html
                                    copy(potential_diff_file, diff_file_with_html)
                                else:
                                    # it's not an HTML file, wrap it inside an HTML wrapper in a temp file and send it
                                    contents = potential_diff_file.read_text()
                                    wrapped_contents = self.single_diff_html(contents)
                                    diff_file_with_html.write_text(wrapped_contents)
                                index_contents_this_file += self.regression_row_in_single_test_case_html(
                                    potential_diff_file.name
                                )
                            index_file = target_dir_for_this_file_diffs / "index.html"
                            index_this_file = self.single_test_case_html(index_contents_this_file)
                            index_file.write_text(index_this_file)
                    else:
                        self.root_index_files_no_diff.append(idf_file)
                    so_far = " Diffs! " if any_diffs else "No diffs"
                    if entry_num % 40 == 0:
                        print(f"On file #{entry_num}/{len(idf_files)} ({idf_file}), Diff status so far: {so_far}")

                except Exception as e:
                    any_diffs = True
                    print(f"Regression run *failed* trying to process file: {idf_file}; reason: {e}")
                    if not backtrace_shown:
                        print("Traceback shown once:")
                        print_exc()
                        backtrace_shown = True
                    self.root_index_files_failed.append(idf_file)

        meta_data = [
            f"Regression time stamp in UTC: {datetime.now(UTC)}",
            f"Regression time stamp in Central Time: {datetime.now(ZoneInfo('America/Chicago'))}",
            f"Number of input files evaluated: {self.num_idf_inspected}",
        ]
        bundle_root_index_file_path = bundle_root / "index.html"
        bundle_root_index_content = self.bundle_root_index_html(meta_data)
        bundle_root_index_file_path.write_text(bundle_root_index_content)
        print()
        print(f"* Files with Diffs *:\n{"\n ".join(self.root_index_files_diffs)}\n")
        print(f"* Diffs by File *:\n{json.dumps(self.diffs_by_idf, indent=2, sort_keys=True)}\n")
        print(f"* Diffs by Type *:\n{json.dumps(self.diffs_by_type, indent=2, sort_keys=True)}\n")
        if any_diffs:
            self.generate_markdown_summary(bundle_root)
            # print("::warning title=Regressions::Diffs Detected")
        return any_diffs


if __name__ == "__main__":  # pragma: no cover - testing function, not the __main__ entry point

    if len(sys.argv) != 4:
        print("syntax: %s base_dir mod_dir regression_dir" % sys.argv[0])
        sys.exit(1)
    arg_base_dir = Path(sys.argv[1])
    arg_mod_dir = Path(sys.argv[2])
    arg_regression_dir = Path(sys.argv[3])
    rm = RegressionManager()
    response = rm.check_all_regressions(arg_base_dir, arg_mod_dir, arg_regression_dir)
    sys.exit(1 if response else 0)
