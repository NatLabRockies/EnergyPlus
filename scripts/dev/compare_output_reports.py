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

"""
This script aims to check that the Tabular Reports are the same between the
HTML, SQL, and JSON outputs.

5ZoneCAV_MaxTemp_JSON_Outputs.idf has all the required
objects (Output:SQLite, Output:JSON) with 'AllSummary' enabled.

cf: https://github.com/NREL/EnergyPlus/issues/9419
"""

__author__ = "Julien Marrec, EffiBEM"
__email__ = "julien@effibem.com"

import argparse
import json
import re
import sqlite3
from pathlib import Path

REPO_ROOT = Path(__file__).parent.parent.parent


def MakeAnchorName(s):
    validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_:."
    for c in s:
        if c not in validChars:
            s = s.replace(c, "")
    return s


def parse_json(out_dir: Path):
    with open(out_dir / "eplusout.json", "rb") as f:
        json_data = json.load(f)

    all_json_report_names = []
    json_report_to_tables_list = []
    empty_report_to_tables_list = []
    for report in json_data["TabularReports"]:
        report_name = report["ReportName"]
        for table in report["Tables"]:
            table_name = table["TableName"]

            # Filter out the empty tables, as they do not show up in the SQL
            if table["Rows"] is None:
                print("Filtering empty JSON table " f"'{report_name} - {table_name}'")
                empty_report_to_tables_list.append((report_name, table_name))
            else:
                all_json_report_names.append(report_name)
                json_report_to_tables_list.append((report_name, table_name))

    return (
        set(all_json_report_names),
        set(json_report_to_tables_list),
        set(empty_report_to_tables_list),
    )


def parse_sql(out_dir: Path):
    query = """
    SELECT DISTINCT(ReportName) FROM TabularDataWithStrings
    """

    abs_sql_path = (out_dir / "eplusout.sql").resolve()
    sql_uri = "{}?mode=ro".format(abs_sql_path.as_uri())

    with sqlite3.connect(sql_uri, uri=True) as con:
        cursor = con.cursor()
        r = cursor.execute(query)
        all_sql_report_names = [x[0] for x in r.fetchall()]

    query = """
    SELECT DISTINCT(TableName) FROM TabularDataWithStrings
    WHERE ReportName = "{}"
    """

    abs_sql_path = (out_dir / "eplusout.sql").resolve()
    sql_uri = "{}?mode=ro".format(abs_sql_path.as_uri())

    sql_report_to_tables_list = []
    for report_name in all_sql_report_names:
        with sqlite3.connect(sql_uri, uri=True) as con:
            cursor = con.cursor()
            r = cursor.execute(query.format(report_name))
            all_sql_table_names = [x[0] for x in r.fetchall()]
            for table_name in all_sql_table_names:
                sql_report_to_tables_list.append((report_name, table_name))

    return set(all_sql_report_names), set(sql_report_to_tables_list)


def find_manual_mapping_html():
    re_predef_report = re.compile(
        r'= newPreDefReport\(state, "(?P<inReportName>.+?)", "(?P<inReportAbrev>.+?)", "(?P<inReportNamewithSpaces>.+?)"'
    )

    with open(REPO_ROOT / "src/EnergyPlus/OutputReportPredefined.cc", "r") as f:
        content = f.read()
    lines = content.splitlines()

    manual_html_mapping_dict = {}
    for line in lines:
        if "= newPreDefReport" in line:
            m = re_predef_report.search(line)
            if m:
                d = m.groupdict()
                inReportName = d["inReportName"]
                anchor = MakeAnchorName(d["inReportNamewithSpaces"])
                if anchor != inReportName:
                    manual_html_mapping_dict[anchor] = inReportName
    return manual_html_mapping_dict


def parse_html(out_dir):
    # Lazy load bs4 as its not in the standard library
    try:
        from bs4 import BeautifulSoup
    except ModuleNotFoundError:
        print("bs4 (BeautifulSoup) is not installed, not checking HTML")
        return None

    with open(out_dir / "eplustbl.htm", "rb") as f:
        soup = BeautifulSoup(f.read(), features="html.parser")

    manual_html_mapping_dict = find_manual_mapping_html()
    html_report_to_tables_list = []
    for table in soup.find_all("table"):
        table_name = table.find_previous("b").text.strip()
        for previous_p in table.find_all_previous("p"):
            if "Report" in previous_p.text:
                report_name = previous_p.find("b").text.strip()
                break

        html_report_to_tables_list.append((report_name, table_name))

    html_report_to_tables_list_without_spaces = []

    for report_name, table_name in html_report_to_tables_list:
        report_name = report_name.replace(" ", "")
        table_name = table_name.replace(" ", "")
        if report_name in manual_html_mapping_dict:
            # print(f"Override for {report_name}")
            report_name = manual_html_mapping_dict[report_name]

        html_report_to_tables_list_without_spaces.append((report_name, table_name))

    return set(html_report_to_tables_list_without_spaces)


def compare_sets(lhs: set, lhs_name: str, rhs: set, rhs_name: str, type_str: str) -> bool:

    success = True

    extra_lhs = lhs - rhs
    if extra_lhs:
        print(f"{type_str} that are in the {lhs_name} but not in {rhs_name}:")
        print(extra_lhs)
        success = False

    extra_rhs = rhs - lhs
    if extra_rhs:
        print(f"{type_str} that are in the {rhs_name} but not in {lhs_name}:")
        print(extra_rhs)
        success = False

    return success


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Compare Output Reports.")
    parser.add_argument("out_dir", type=Path, help="Output directory where to find the reports")
    args = parser.parse_args()

    out_dir = args.out_dir.resolve()
    if not (out_dir.exists() and out_dir.is_dir()):
        raise IOError(f"{out_dir} is not a valid directory")

    (
        json_report_names_set,
        json_report_to_tables_set,
        json_empty_report_to_tables_set,
    ) = parse_json(out_dir)
    sql_report_names_set, sql_report_to_tables_set = parse_sql(out_dir)

    success = True
    success &= compare_sets(
        lhs=sql_report_names_set,
        lhs_name="SQL",
        rhs=json_report_names_set,
        rhs_name="JSON",
        type_str="Reports",
    )

    success &= compare_sets(
        lhs=sql_report_to_tables_set,
        lhs_name="SQL",
        rhs=json_report_to_tables_set,
        rhs_name="JSON",
        type_str="Tables",
    )

    if success:
        print("JSON and SQL reports and tables align")
    else:
        print("JSON and SQL reports and tables do not align")

    html_report_to_tables_nospaces_set = parse_html(out_dir)
    if html_report_to_tables_nospaces_set is not None:
        json_report_to_tables_nospaces_set = set(
            [
                (x[0].replace(" ", ""), x[1].replace(" ", ""))
                for x in json_report_to_tables_set.union(json_empty_report_to_tables_set)
            ]
        )

        html_success = compare_sets(
            lhs=json_report_to_tables_nospaces_set,
            lhs_name="JSON",
            rhs=html_report_to_tables_nospaces_set,
            rhs_name="HTML",
            type_str="Tables",
        )
        if html_success:
            print("JSON and HTML reports and tables align")
        else:
            print("JSON and HTML reports and tables do not align")
            success = False

    if not success:
        exit(1)

    print("Check successful")
