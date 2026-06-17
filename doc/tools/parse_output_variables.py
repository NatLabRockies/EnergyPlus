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

# python 2/3 compatibility imports
from __future__ import absolute_import, print_function, unicode_literals

import csv
import fnmatch
import glob
import io
import os
import re
import sys
from os import path

# generates a csv summary of the output variables in E+
# assumption: doesn't check for commented lines
# assumption: no /* */ style commenting
# To Call: 3 ARGUMENTS:
# the first is the source dir to find *.cc files
# the second is the csv output file name
# the third is the md output file name

# example arguments here
# source_dir = "<repo_source_dir>\src\EnergyPlus"
# csv_file = "<repo_build_dir>\auto_docs\SetupOutputVariableCalls.csv"
# md_file = "<repo_build_dir>\auto_docs\SetupOutputVariableCalls.csv"

if len(sys.argv) == 4:
    source_dir = sys.argv[1]
    output_file = sys.argv[2]
    md_output_file = sys.argv[3]
else:
    print("Bad usage, three command line arguments are required")
    print("  The first is the source dir to find .cc files,")
    print("  the second is the output csv file,")
    print("  and the third is the output md file")
    sys.exit(1)


def do_initial_line_trimming(working_line):
    # trim leading/trailing whitespace
    working_line = working_line.strip()
    # cut the call to SetupOutputVariable off
    sov_index = working_line.index("SetupOutputVariable")
    working_line = working_line[sov_index + 20 :]
    # cut the last 3 characters off
    working_line = working_line[:-2]
    return working_line


def get_level_zero_comma_locations(line):
    current_char_location = -1
    current_parentheses_level = 0
    level_zero_comma_locations = []
    for char in line:
        current_char_location += 1
        if char == "," and current_parentheses_level == 0:
            level_zero_comma_locations.append(current_char_location)
        elif char == "(":
            current_parentheses_level += 1
        elif char == ")":
            current_parentheses_level -= 1
            # print "Character: " + char + "; paren_level: " + str(current_parentheses_level)
            # print (char + " : " + str(level_zero_comma_locations) )
    return level_zero_comma_locations


def get_arguments_based_on_comma_locations(line, comma_locations):
    try:
        arguments = []
        comma_locations_length = len(comma_locations)
        for i in range(comma_locations_length + 1):
            if i == 0:
                arguments.append(line[0 : comma_locations[i]].strip())
            elif i == comma_locations_length:
                arguments.append(line[comma_locations[i - 1] + 1 :].strip())
            else:
                arguments.append(line[comma_locations[i - 1] + 1 : comma_locations[i]].strip())
        return arguments
    except:
        print(line)
        return None


def process_variable_name_and_units(argument):
    # parse out data from the variable name first
    first_bracket = argument.find("[")
    # variable_name = ""
    variable_units = ""
    if first_bracket == -1:
        variable_name = argument.strip()
    else:
        variable_name = argument[0:first_bracket].strip() + '"'
        variable_units = argument[first_bracket + 1 : -2].strip()
    return variable_name, variable_units


class OutputVariableCall:
    # constructor
    def __init__(
        self, file_name, variable_name, units, variable_itself, index_type_key, variable_type_key, keyed_value
    ):
        self.file_name = file_name
        self.variable_name = variable_name
        self.units = units
        self.variable = variable_itself
        self.index_type_key = index_type_key
        self.variable_type_key = variable_type_key
        self.keyed_value = keyed_value

    # csv methods
    @staticmethod
    def spew_header_to_csv(csv_file_object):
        csv_file_object.writerow(
            (
                "Filename",
                "Variable name",
                "Units",
                "Variable reference",
                "Index type key",
                "Variable type key",
                "Keyed value",
            )
        )

    def spew_to_csv(self, csv_file_object):
        csv_file_object.writerow(
            [
                self.file_name,
                self.variable_name,
                self.units,
                self.variable,
                self.index_type_key,
                self.variable_type_key,
                self.keyed_value,
            ]
        )

    # markdown methods
    def spew_to_md(self, md_file_object):
        md_file_object.write("|")
        md_file_object.write(
            "|".join(
                (
                    str(x)
                    for x in (
                        self.variable_name,
                        self.units,
                        self.variable,
                        self.index_type_key,
                        self.variable_type_key,
                        self.keyed_value,
                    )
                )
            )
        )
        md_file_object.write("|")
        md_file_object.write("\n")

    def write_file_header(self, md_file_object):
        md_file_object.write("\n")
        md_file_object.write("## %s\n" % self.file_name)
        md_file_object.write("|Variable name|Units|Variable reference|Index type key|Variable type key|Keyed value|\n")
        md_file_object.write("|-------------|-----|------------------|--------------|-----------------|-----------|\n")


def main():
    # this is the    master list of calls
    output_variables = []

    files = []
    for root, _, filenames in os.walk(source_dir):
        for filename in fnmatch.filter(filenames, "*.cc"):
            files.append(os.path.join(root, filename))

    for this_file in files:

        file_name = path.basename(this_file)
        if file_name == "OutputProcessor.cc":
            continue

        with io.open(this_file, "r", encoding="latin-1") as f:

            file_contents = f.read()

            # first warn about commented lines
            p = re.compile(r"//\s*SetupOutputVariable")
            matches = p.findall(file_contents)

            if len(matches) > 0:
                print("File %s contains commented SetupOutputVariable calls; output may be flawed" % file_name)

            p = re.compile(r"SetupOutputVariable\([^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^;]*;")
            matches = p.findall(file_contents)

            for match in matches:

                setup_call = match.replace("\n", "")

                # trim off the leading/trailing stuff to just get args
                working_line = do_initial_line_trimming(setup_call)

                # get all the zero parentheses level comma locations to parse actual arguments,
                #  even if the arguments have embedded commas
                comma_locations = get_level_zero_comma_locations(working_line)

                # get a list of the actual arguments
                arguments = get_arguments_based_on_comma_locations(working_line, comma_locations)

                # parse out data from the variable name first
                variable_name = arguments[1]
                variable_units = arguments[2]

                try:

                    # the second can be taken as-is
                    actual_variable = arguments[3]

                    # the third should be processed for quotes
                    index_type_key = arguments[4]

                    # same for the fourth
                    variable_type_key = arguments[5]

                    # same for the fifth I guess
                    keyed_value = arguments[6]

                except:

                    print("Something went wrong processing this variable line:")
                    print(working_line)
                    continue

                # add to the array
                this_o_v = OutputVariableCall(
                    file_name,
                    variable_name,
                    variable_units,
                    actual_variable,
                    index_type_key,
                    variable_type_key,
                    keyed_value,
                )
                output_variables.append(this_o_v)

                # print "Finished with file %s; %i calls processed so far" % (file_name, len(output_variables))

    mode = "w"
    if sys.version_info.major < 3:
        mode += "b"

    # output the data to csv
    with io.open(output_file, mode) as csv_file:
        csv_writer = csv.writer(csv_file)
        OutputVariableCall.spew_header_to_csv(csv_writer)
        for call in output_variables:
            call.spew_to_csv(csv_writer)

    with io.open(md_output_file, mode) as md_file:
        md_file.write("# Internal EnergyPlus Output Variable Definitions\n")
        headers_written = []
        for call in output_variables:
            if call.file_name not in headers_written:
                call.write_file_header(md_file)
                headers_written.append(call.file_name)
            call.spew_to_md(md_file)

    print(" +++ AutoDocs: Completed processing output variable audit: processed %i calls" % len(output_variables))


# run main documentation code
main()
