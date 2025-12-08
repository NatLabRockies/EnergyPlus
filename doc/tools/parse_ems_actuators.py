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
This modules parses the C++ source Code for EnergyPlus and returns
some info about the EMS Actuators and Internal Variables available
in the program. It will output a CSV file and a summary to stdout.

Args:
-----
You can do `python parse_ems_actuators --help` to see extra arguments you can
customize such as the path to the `EnergyPlus/src/EnergyPlus` directory or the
name and path of the output CSV file, and set verbosity. If you don't supply
any parameters it will default to `../../src/EnergyPlus`,
`./SetupEMSActuatorAndInternalVariableCalls.csv` and non-verbose.


Known limitations:
-------------------
This is a pretty dumb script that only parses lines that match a given pattern.
There is no semantic analysis, so if a variable is used in the call to
`SetupEMSActuator` instead of a string literal, this script doesn't know what's
inside this variable
(eg: it will report 'cCMO_BBRadiator_Electric' for 'Name' when really this
variable stores the string "ZoneHVAC:Baseboard:Convective:Electric").
The only thing of that sort that it tries to do is to parse the list
of 'cCurrentModuleObject'.


Needed modules:
---------------
The only module that isn't part of the standard library is pandas.
You can install it via `pip install pandas` (or `conda install pandas` if you
use conda).
"""

__author__ = "Julien Marrec"
__organization__ = "EffiBEM, EURL, www.effibem.com"
__copyright__ = "Copyright 2018, EffiBEM EURL"
__email__ = "julien@effibem.com"
__license__ = "MIT"
__version__ = "0.1.1"
__status__ = "Production"


import csv
import glob as gb
import io
import os
import re
from itertools import groupby


def parse_ems_var(source_dir, verbose=False):
    """
    Parses all *.cc files in the supplied source_dir
    and looks for 'SetupEMSVariable' and 'SetupEMSInternalVariable' statements.
    Search for cc files is non-recursive

    Args:
    -----
        * source_dir (str): path to the local `EnergyPlus/src/EnergyPlus`
            directory

    Returns:
    --------

        * ems_arr_dict (list): a list of dict of the parsed results

    Needs:
    -------
        import os, re, io
        import glob as gb
        import csv
    """

    # Regex
    pat_cModule_Objects = re.compile(
        r"\s*static Array1D_string const " r"cModuleObjects\(\s*([0-9]{1,2})," r"\s*{\s*(.*?)}\s*\);"
    )

    cCurrentModuleObject = ""
    cModuleObjects = ""

    ems_arr_dict = []

    # For each cc file in source_dir
    for file_path in gb.iglob(os.path.join(source_dir, "*.cc")):

        file_name = os.path.basename(file_path)

        # Skip this file
        if file_name == "EMSManager.cc":
            continue

        if verbose:
            print("\nProcessing File: {}".format(file_name))

        # Open a read lines
        with io.open(file_path, "r", encoding="latin-1") as f:
            lines = f.readlines()

        # Loop on each line
        for i, line in enumerate(lines):

            # create a copy of line to work on
            working_line = line

            # trim off in-line comments first, then strip
            working_line = working_line.split("//")[0].strip()

            m = pat_cModule_Objects.match(working_line)
            if m:
                n = int(m.group(1))
                cModuleObjects = [x.replace('"', "").strip() for x in m.group(2).split(",")]
                if len(cModuleObjects) != n:
                    print("\nPROBLEM")
                    print(n, cModuleObjects)

            if working_line[: len("cCurrentModuleObject")] == "cCurrentModuleObject":
                if "=" in working_line:
                    cCurrentModuleObject = working_line.split("=")[1].replace('"', "").replace(";", "").strip()
                if "cModuleObjects" not in cCurrentModuleObject:
                    cModuleObjects = [cCurrentModuleObject]

            # If we have either of these two function calls, we concat as many
            # lines as needed by counting the opening and closing parentheses
            # so we get the full function call
            if ("SetupEMSActuator" in working_line) | ("SetupEMSInternalVariable" in working_line):
                match_braces = working_line.count("(") - working_line.count(")")
                j = i + 1
                while match_braces != 0:
                    _line = lines[j].split("//")[0].strip()
                    match_braces += _line.count("(")
                    match_braces -= _line.count(")")
                    working_line += _line
                    j += 1

            if "SetupEMSActuator" in working_line:
                # Try naively, that is assume it's all on one line
                # print("{}: {}".format(cModuleObjects, working_line))
                c_l = [
                    x.replace('"', "").strip()
                    for x in working_line.replace("SetupEMSActuator(", "").replace(");", "").split(",")
                ]

                if len(c_l) != 6:
                    print("Problem: In file {}, Line {}: " "{}".format(file_name, i, working_line))
                    continue

                (cComponentTypeName, cUniqueIDName, cControlTypeName, cUnits, lEMSActuated, irlValue) = c_l

                for cModuleObject in cModuleObjects:

                    d = {
                        "Type": "Actuator",
                        "Name": cModuleObject,
                        "FileName": file_name,
                        "Line": i,
                        "cComponentTypeName": cComponentTypeName,
                        "cUniqueIDName": cUniqueIDName,
                        "cControlTypeName / cDataTypeName": cControlTypeName,
                        "cUnits": cUnits,
                        "lEMSActuated": lEMSActuated,
                        "Value": irlValue,
                    }

                    ems_arr_dict.append(d)

            if "SetupEMSInternalVariable" in working_line:
                c_l = [
                    x.replace('"', "").strip()
                    for x in working_line.replace("SetupEMSInternalVariable(", "").replace(");", "").split(",")
                ]
                if len(c_l) != 4:
                    print("Problem: In file {}, Line {}: " "{}".format(file_name, i, working_line))
                    continue

                (cDataTypeName, cUniqueIDName, cUnits, irValue) = c_l

                for cModuleObject in cModuleObjects:

                    d = {
                        "Type": "InternalVariable",
                        "Name": cModuleObject,
                        "FileName": file_name,
                        "Line": i,
                        "cControlTypeName / cDataTypeName": cDataTypeName,
                        "cUniqueIDName": cUniqueIDName,
                        "cUnits": cUnits,
                        "Value": irValue,
                    }

                    ems_arr_dict.append(d)

    # Sort by Name primarily, type secondarily
    ems_arr_dict = sorted(ems_arr_dict, key=lambda k: k["Type"])
    ems_arr_dict = sorted(ems_arr_dict, key=lambda k: k["Name"])

    return ems_arr_dict


def summary(ems_arr_dict):
    """
    Create high-level summary info on EMS variables found

    Args:
    -----
        * ems_arr_dict (list): the list of dict of the parsed results@

    Returns:
    --------
        * out (str): a formatted string ready for output in a file
        or the stdout


    """

    def keyfunc(x):
        return x["Type"]

    data = sorted(ems_arr_dict, key=keyfunc)

    out_str = ""
    for k, v in groupby(data, key=keyfunc):
        gp = list(v)
        n_vars = len(gp)
        n_files = len(set([x["FileName"] for x in gp]))
        n_unique_objects = len(set([x["Name"] for x in gp]))
        out_str += (
            "\nWe found {v} unique EMS {t}s defined, "
            "in {f} files, affecting {o} unique "
            "objects".format(v=n_vars, t=k, f=n_files, o=n_unique_objects)
        )

    return out_str


# If launched from the command line
if __name__ == "__main__":

    import argparse

    parser = argparse.ArgumentParser()

    parser.add_argument("-s", "--source_dir", help=("Local path to the EnergyPlus/src/EnergyPlus/ " "directory"))

    parser.add_argument("-o", "--output", help="Local path Output CSV")

    parser.add_argument("-v", "--verbose", help="increase output verbosity", action="store_true")

    args = parser.parse_args()

    if args.source_dir:
        source_dir = args.source_dir
    else:
        source_dir = "../../src/EnergyPlus/"
        source_dir = os.path.abspath(source_dir)
        print("Defaulting source_dir to {}".format(source_dir))

    if args.output:
        output_file = args.output
    else:
        output_file = "SetupEMSActuatorAndInternalVariableCalls.csv"
        print("Defaulting output csv file to " "{}".format(os.path.abspath(output_file)))

    ems_arr_dict = parse_ems_var(source_dir, verbose=args.verbose)

    # Print output summary
    out_str = summary(ems_arr_dict)
    print(out_str)

    # Output csv file
    col_order = [
        "Name",
        "Type",
        "cComponentTypeName",
        "cControlTypeName / cDataTypeName",
        "cUniqueIDName",
        "cUnits",
        "lEMSActuated",
        "Value",
        "FileName",
        "Line",
    ]

    with open(output_file, "w") as f:
        dict_writer = csv.DictWriter(f, col_order)
        dict_writer.writeheader()
        dict_writer.writerows(ems_arr_dict)

    print("\nSaving to {}".format(os.path.abspath(output_file)))
