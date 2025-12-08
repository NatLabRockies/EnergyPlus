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
import platform
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Tuple

from eplaunch.workflows.base import BaseEPLaunchWorkflow1, EPLaunchWorkflowResponse1


class ColumnNames(object):
    Errors = "Errors"
    Warnings = "Warnings"
    Runtime = "Runtime [s]"
    Version = "Version"


class Version:

    @staticmethod
    def check_energyplus_version(file_path: str) -> Tuple[bool, str, int]:
        """Gets the version number information for a given EnergyPlus input file"""
        extension = os.path.splitext(file_path)[1].upper()
        if extension == ".IDF" or extension == ".IMF":
            return Version.check_idf_imf_energyplus_version(file_path)
        elif extension == ".EPJSON":
            return Version.check_json_energyplus_version(file_path)
        else:
            return False, "", 0

    @staticmethod
    def check_idf_imf_energyplus_version(file_path: str) -> Tuple[bool, str, int]:
        """Attempts to read a version number from an IDF syntax file"""
        file_path_object = Path(file_path)
        # noinspection PyBroadException
        try:
            file_contents = file_path_object.read_text()
            file_lines = [x.strip() for x in file_contents.split("\n")]
            for index, cur_line in enumerate(file_lines):
                if len(cur_line) > 0 and cur_line[0] != "!" and "VERSION" in cur_line.upper():
                    trimmed_line = Version.line_with_no_comment(cur_line)
                    if ";" in trimmed_line:  # one liner version object ("Version, 8.4;")
                        poss_obj = trimmed_line
                    else:  # hoping for a two-liner version object ("Version,\n  8.4;")
                        if index + 1 <= len(file_lines):
                            next_line_trimmed = Version.line_with_no_comment(file_lines[index + 1])
                            poss_obj = trimmed_line + next_line_trimmed
                        else:
                            return False, "<invalid>", 0  # hit the end of the file in the middle of the version object
                    hopeful_object = poss_obj[:-1] if poss_obj[-1] == ";" else poss_obj
                    fields = hopeful_object.split(",")
                    return True, fields[1], Version.numeric_version_from_string(fields[1])
            else:
                return False, "<missing>", 0
        except:
            return False, "<error>", 0

    @staticmethod
    def line_with_no_comment(in_string: str) -> str:
        """Returns an IDF line with any comments removed"""
        return in_string[0 : in_string.find("!")].strip() if in_string.find("!") >= 0 else in_string.strip()

    @staticmethod
    def numeric_version_from_string(string_version: str) -> int:
        """Gets the coded version string from a version string like 5.0.0-abcdef"""
        parts = [int(x) for x in string_version.split("-")[0].split(".")]
        return 10000 * parts[0] + 100 * parts[1]

    @staticmethod
    def numeric_version_from_dash_string(string_version: str) -> int:
        """Gets the coded version string from a version string like V5-0-0"""
        string_version = string_version[1:] if string_version[0] == "V" else string_version
        # the rest of the version number should just be separated by periods
        parts = [int(x) for x in string_version.split("-")]
        return 10000 * parts[0] + 100 * parts[1]

    @staticmethod
    def string_version_from_number(version_number: int) -> str:
        """Converts a coded number like 50200 (fictional version 5.2) to string with leading zeros 'V050200'"""
        return "V" + str(version_number).zfill(6)

    @staticmethod
    def check_json_energyplus_version(file_path: str) -> Tuple[bool, str, int]:
        """Reads a version number from an EpJSON file"""
        # noinspection PyBroadException
        try:
            with open(file_path, "r") as readfile:
                data = json.load(readfile)
            current_version = data["Version"]["Version 1"]["version_identifier"]
            return True, current_version, Version.numeric_version_from_string(current_version)
        except:  # could be a file-not-found, a permission issue, a key error, a JSON exception.....
            return False, "", 0


class EPlusRunManager(object):

    @staticmethod
    def get_end_summary_from_err(err_file_path):
        contents = open(err_file_path, "r").readlines()
        last_line = contents[-1]
        if "EnergyPlus" not in last_line and "Elapsed Time" not in last_line:
            return False, None, None, None
        last_line_tokens = last_line.split(" ")
        num_warnings = int(last_line_tokens[7])
        num_errors = int(last_line_tokens[9])
        time_position_marker = last_line.index("Time=")
        time_string = last_line[time_position_marker:]
        num_hours = int(time_string[5:7])
        num_minutes = int(time_string[10:12])
        num_seconds = float(time_string[16:21])
        runtime_seconds = num_seconds + num_minutes / 60 + num_hours / 3600
        return True, num_errors, num_warnings, runtime_seconds

    @staticmethod
    def eplus_suffixes():
        return [
            # the following are in the same order as the buttons in EP-Launch 2
            "Table.htm",
            ".csv",
            "Meter.csv",
            ".err",
            ".rdd",
            ".mdd",
            ".eio",
            ".svg",
            ".dxf",
            ".mtd",
            "Zsz.csv",
            "Ssz.csv",
            "Spsz.csv",
            "Psz.csv",
            "DElight.in",
            "DElight.out",
            "Map.csv",
            "DElight.eldmp",
            "DElight.dfdmp",
            "Screen.csv",
            ".expidf",
            ".epmidf",
            ".epmdet",
            ".shd",
            ".wrl",
            ".audit",
            ".bnd",
            ".dbg",
            ".sln",
            "_bsmt.out",
            ".bsmt",
            "_bsmt.audit",
            ".eso",
            ".mtr",
            "Proc.csv",
            "_slab.out",
            ".slab",
            "_slab.ger",
            "_bsmt.csv",
            ".edd",
            "Table.xml",
            "_perflog.csv",
            ".end",
            ".sci",
            ".rvaudit",
            ".sql",
            ".log",
            "Table.csv",
            "Table.tab",
            "Table.txt",
            ".tab",
            ".txt",
            "Meter.tab",
            "Meter.txt",
            "Zsz.tab",
            "Zsz.txt",
            "Ssz.tab",
            "Ssz.txt",
            "Spsz.tab",
            "Spsz.txt",
            "Psz.tab",
            "Psz.txt",
            "Map.tab",
            "Map.txt",
        ]

    @staticmethod
    def run_energyplus(workflow_instance, is_ip, run_directory, file_name, args, by_api):

        full_file_path = os.path.join(run_directory, file_name)
        file_name_no_ext, extension = os.path.splitext(file_name)
        output_directory = os.path.join(run_directory, f"EPLaunchRun_{file_name_no_ext}")
        if os.path.exists(output_directory):
            shutil.rmtree(output_directory)
        os.makedirs(output_directory)

        def delete_if_exists(file_path):
            if os.path.exists(file_path):
                os.remove(file_path)

        energyplus_root_folder, _ = os.path.split(args["workflow location"])

        # Run EnergyPlus binary
        if platform.system() == "Windows":
            energyplus_binary = os.path.join(energyplus_root_folder, "energyplus.exe")
        else:
            energyplus_binary = os.path.join(energyplus_root_folder, "energyplus")
        if not os.path.exists(energyplus_binary):
            return EPLaunchWorkflowResponse1(
                success=False, message="EnergyPlus binary not found: {}!".format(energyplus_binary), column_data=[]
            )

        v = Version()
        is_found, current_version, numeric_version = v.check_energyplus_version(full_file_path)
        if not is_found:
            workflow_instance.callback("***Error occurred processing version, attempting to continue")

        if by_api:
            # if calling through API, then the arguments don't include the E+ binary
            command_line_args = []
        else:
            command_line_args = [energyplus_binary]

        if extension == ".imf":
            command_line_args += ["--epmacro"]

        # So...ExpandObjects is weird in E+.  It doesn't like running things in directories and accidentally symlinks
        # things into the current working directory even if you specify an output directory
        # We're just going to run it ourselves.  (Side note it could be a similar problem for EPMacro)
        if extension != ".epJSON":
            if platform.system() == "Windows":
                expand_objects = os.path.join(energyplus_root_folder, "ExpandObjects.exe")
            else:
                expand_objects = os.path.join(energyplus_root_folder, "ExpandObjects")
            if not os.path.exists(expand_objects):
                return EPLaunchWorkflowResponse1(
                    success=False, message="ExpandObjects binary not found: {}!".format(expand_objects), column_data=[]
                )
            # go ahead and copy the target IDF and original IDD into the output_dir/in.idf for running ExpandObjects
            idf_path_in_output_dir = os.path.join(output_directory, "in.idf")
            shutil.copy(full_file_path, idf_path_in_output_dir)
            idd_path = os.path.join(energyplus_root_folder, "Energy+.idd")  # yes for now we still need it
            idd_path_in_output_dir = os.path.join(output_directory, "Energy+.idd")
            shutil.copy(idd_path, idd_path_in_output_dir)
            # run ExpandObjects
            try:
                for message in workflow_instance.execute_for_callback([expand_objects], output_directory):
                    workflow_instance.callback(message)
            except subprocess.CalledProcessError:
                workflow_instance.callback("E+ FAILED")
                return EPLaunchWorkflowResponse1(
                    success=False, message="EnergyPlus failed for file: %s!" % full_file_path, column_data={}
                )
            # regardless of what happened, clean up the temp files
            delete_if_exists(idf_path_in_output_dir)
            delete_if_exists(idd_path_in_output_dir)
            # if expanded.idf does not exist then we don't need to do anything, assume it just didn't have any templates
            # but if it does exist, then this needs to be the IDF that we run, *however* we can't just leave it here
            # as it may reference some other files back in the original directory.  I'm going to go with a new file name
            # in the original directory of "FileName_Expanded.idf"
            expanded_path_in_output_dir = os.path.join(output_directory, "expanded.idf")
            if os.path.exists(expanded_path_in_output_dir):
                target_expanded_file_name = os.path.join(run_directory, f"{file_name_no_ext}_Expanded.idf")
                shutil.copy(expanded_path_in_output_dir, target_expanded_file_name)
                full_file_path = target_expanded_file_name

        # Run EnergyPlus in a subdirectory
        command_line_args += ["-d", output_directory]

        # if not isIP:  # if using SI output units just use readvars CLI option
        #     command_line_args += ['--readvars']

        # add some config parameters
        command_line_args += ["--output-prefix", file_name_no_ext, "--output-suffix", "C"]

        # add in simulation control args
        if "weather" in args and args["weather"]:
            command_line_args += ["--weather", args["weather"]]
        else:
            command_line_args += ["--design-day"]

        # and at the very end, add the file to run
        command_line_args += [full_file_path]

        # make sure we clean up any existing csv file to detect whether E+ created it natively
        vari_csv_path = os.path.join(output_directory, file_name_no_ext + ".csv")  # TODO: What is this?
        delete_if_exists(vari_csv_path)

        # run E+
        if by_api:
            # if by API then find the API wrapper relative to this workflow, import it, set up a callback, and run
            eplus_dir = Path(__file__).parent.parent.absolute()
            sys.path.insert(0, str(eplus_dir))
            from pyenergyplus.api import EnergyPlusAPI

            x = lambda msg: workflow_instance.callback("(E+API) " + msg.decode("utf-8", errors="ignore"))
            # x = lambda msg: print("(E+ API) : " + str(msg))
            api = EnergyPlusAPI()
            state = api.state_manager.new_state()
            api.runtime.callback_message(state, x)
            api.runtime.set_console_output_status(state, False)
            # cur_dir = os.getcwd()
            # os.chdir(output_directory)
            eplus_return = api.runtime.run_energyplus(state, command_line_args)
            # os.chdir(cur_dir)
            if eplus_return != 0:
                workflow_instance.callback("E+ FAILED")
                return EPLaunchWorkflowResponse1(
                    success=False, message="EnergyPlus failed for file: %s!" % full_file_path, column_data={}
                )
        else:
            # if by CLI then just execute the full command line
            try:
                for message in workflow_instance.execute_for_callback(command_line_args, output_directory):
                    workflow_instance.callback(message)
            except subprocess.CalledProcessError:
                workflow_instance.callback("E+ FAILED")
                return EPLaunchWorkflowResponse1(
                    success=False, message="EnergyPlus failed for file: %s!" % full_file_path, column_data={}
                )

        if os.path.exists(vari_csv_path):
            # need to skip the read vars stuff, E+ must have created this automatically.
            pass
        else:
            # if isIP:
            # set up the ESO and MTR output files for either unit conversion or just ReadVarsESO
            # *.eso back to eplusout.eso
            eso_path = os.path.join(output_directory, file_name_no_ext + ".eso")
            eplusouteso_path = os.path.join(output_directory, "eplusout.eso")
            if os.path.exists(eso_path):
                shutil.copy(eso_path, eplusouteso_path)
            # *.mtr back to eplusout.mtr
            mtr_path = os.path.join(output_directory, file_name_no_ext + ".mtr")
            eplusoutmtr_path = os.path.join(output_directory, "eplusout.mtr")
            if os.path.exists(mtr_path):
                shutil.copy(mtr_path, eplusoutmtr_path)

            if is_ip:
                # run the ConvertESOMTR program to create IP versions of the timestep based output files
                if platform.system() == "Windows":
                    converter = os.path.join(
                        energyplus_root_folder, "PostProcess", "convertESOMTRpgm", "convertESOMTR.exe"
                    )
                else:
                    converter = os.path.join(energyplus_root_folder, "PostProcess", "convertESOMTRpgm", "convertESOMTR")
                if os.path.exists(converter):
                    txt_orig_path = os.path.join(
                        energyplus_root_folder, "PostProcess", "convertESOMTRpgm", "convert.txt"
                    )
                    txt_run_path = os.path.join(output_directory, "convert.txt")
                    shutil.copy(txt_orig_path, txt_run_path)

                    command_line_args = [converter]
                    try:
                        for message in workflow_instance.execute_for_callback(command_line_args, output_directory):
                            workflow_instance.callback(message)
                    except subprocess.CalledProcessError:
                        workflow_instance.callback("ConvertESOMTR FAILED")
                        return EPLaunchWorkflowResponse1(
                            success=False, message="ConvertESOMTR failed for file: %s!" % full_file_path, column_data={}
                        )
                    # copy converted IP version of ESO file to users *.eso file
                    ipeso_path = os.path.join(output_directory, "ip.eso")
                    if os.path.exists(ipeso_path):
                        shutil.copy(ipeso_path, eso_path)
                        os.replace(ipeso_path, eplusouteso_path)
                    # copy converted IP version of MTR file to users *.mtr file
                    ipmtr_path = os.path.join(output_directory, "ip.mtr")
                    if os.path.exists(ipmtr_path):
                        shutil.copy(ipmtr_path, mtr_path)
                        os.replace(ipmtr_path, eplusoutmtr_path)
                    os.remove(txt_run_path)

            # run ReadVarsESO to convert the timestep based output files to CSV files
            if platform.system() == "Windows":
                readvarseso_binary = os.path.join(energyplus_root_folder, "PostProcess", "ReadVarsESO.exe")
            else:
                readvarseso_binary = os.path.join(energyplus_root_folder, "PostProcess", "ReadVarsESO")
            if os.path.exists(readvarseso_binary):

                command_line_args = [readvarseso_binary]
                rvi_path = os.path.join(run_directory, file_name_no_ext + ".rvi")
                temp_rvi_path = os.path.join(output_directory, "temp.rvi")
                eplusout_rvi_path = os.path.join(output_directory, "eplusout.rvi")
                if os.path.exists(rvi_path):
                    shutil.copy(rvi_path, eplusout_rvi_path)
                    command_line_args.append("eplusout.rvi")
                else:
                    with open(temp_rvi_path, "w") as f:
                        f.write("eplusout.eso \n")
                        f.write("eplusout.csv \n")
                    command_line_args.append("temp.rvi")
                command_line_args.append("unlimited")  # no number of column limit

                try:
                    for message in workflow_instance.execute_for_callback(command_line_args, output_directory):
                        workflow_instance.callback(message)
                except subprocess.CalledProcessError:
                    workflow_instance.callback("ReadVarsESO FAILED on ESO file")
                    return EPLaunchWorkflowResponse1(
                        success=False, message="ReadVarsESO failed for ESO file: %s!" % full_file_path, column_data={}
                    )
                eplusout_csv_path = os.path.join(output_directory, "eplusout.csv")
                if os.path.exists(eplusout_csv_path):
                    os.replace(eplusout_csv_path, vari_csv_path)

                command_line_args = [readvarseso_binary]
                mvi_path = os.path.join(run_directory, file_name_no_ext + ".mvi")
                temp_mvi_path = os.path.join(output_directory, "temp.mvi")
                eplusout_mvi_path = os.path.join(output_directory, "eplusout.mvi")
                if os.path.exists(mvi_path):
                    shutil.copy(mvi_path, eplusout_mvi_path)
                    command_line_args.append("eplusout.mvi")
                else:
                    with open(temp_mvi_path, "w+") as f:
                        f.write("eplusout.mtr \n")
                        f.write("eplusmtr.csv \n")
                    command_line_args.append("temp.mvi")
                command_line_args.append("unlimited")  # no number of column limit

                try:
                    for message in workflow_instance.execute_for_callback(command_line_args, output_directory):
                        workflow_instance.callback(message)
                except subprocess.CalledProcessError:
                    workflow_instance.callback("ReadVarsESO FAILED on MTR file")
                    return EPLaunchWorkflowResponse1(
                        success=False, message="ReadVarsESO failed for MTR file: %s!" % full_file_path, column_data={}
                    )
                mtr_csv_path = os.path.join(output_directory, file_name_no_ext + "Meter.csv")
                eplusmtr_csv_path = os.path.join(output_directory, "eplusmtr.csv")
                if os.path.exists(eplusmtr_csv_path):
                    os.replace(eplusmtr_csv_path, mtr_csv_path)

                readvars_audit_path = os.path.join(output_directory, "readvars.audit")
                rv_audit_path = os.path.join(output_directory, file_name_no_ext + ".rvaudit")
                if os.path.exists(readvars_audit_path):
                    os.replace(readvars_audit_path, rv_audit_path)

                # clean up things inside this IF block
                delete_if_exists(temp_rvi_path)
                delete_if_exists(temp_mvi_path)
                delete_if_exists(eplusout_rvi_path)
                delete_if_exists(eplusout_mvi_path)

            # clean up more things
            delete_if_exists(eplusouteso_path)
            delete_if_exists(eplusoutmtr_path)

        audit_out_path = os.path.join(output_directory, "audit.out")
        delete_if_exists(audit_out_path)
        expanded_idf_path = os.path.join(output_directory, "expanded.idf")
        delete_if_exists(expanded_idf_path)
        out_idf_path = os.path.join(output_directory, "out.idf")
        delete_if_exists(out_idf_path)

        # run HVAC-Diagram
        if platform.system() == "Windows":
            hvac_diagram_binary = os.path.join(energyplus_root_folder, "PostProcess", "HVAC-Diagram.exe")
        else:
            hvac_diagram_binary = os.path.join(energyplus_root_folder, "PostProcess", "HVAC-Diagram")
        if os.path.exists(hvac_diagram_binary):
            bnd_path = os.path.join(output_directory, file_name_no_ext + ".bnd")
            eplusout_bnd_path = os.path.join(output_directory, "eplusout.bnd")
            if os.path.exists(bnd_path):
                shutil.copy(bnd_path, eplusout_bnd_path)
                command_line_args = [hvac_diagram_binary]
            try:
                for message in workflow_instance.execute_for_callback(command_line_args, output_directory):
                    workflow_instance.callback(message)
            except subprocess.CalledProcessError:
                workflow_instance.callback("HVAC-Diagram FAILED on BND file")
                return EPLaunchWorkflowResponse1(
                    success=False, message="HVAC-Diagram failed for BND file: %s!" % full_file_path, column_data={}
                )
            svg_path = os.path.join(output_directory, file_name_no_ext + ".svg")
            eplusout_svg_path = os.path.join(output_directory, "eplusout.svg")
            if os.path.exists(eplusout_svg_path):
                os.replace(eplusout_svg_path, svg_path)
            if os.path.exists(eplusout_bnd_path):
                os.remove(eplusout_bnd_path)

        # Now the last thing to do is to collect all the relevant output files for this simulation and put them back
        # in the original run directory.  This is because EP Launch will expect to find them right there.
        file_names_to_copy = [f for f in os.listdir(output_directory) if f.startswith(file_name_no_ext)]
        for f in file_names_to_copy:
            shutil.copy(os.path.join(output_directory, f), os.path.join(run_directory, f))

        # check on .end file and finish up
        err_file_name = "{0}.err".format(file_name_no_ext)
        err_file_path = os.path.join(output_directory, err_file_name)
        success, errors, warnings, runtime = EPlusRunManager.get_end_summary_from_err(err_file_path)

        column_data = {
            ColumnNames.Errors: errors,
            ColumnNames.Warnings: warnings,
            ColumnNames.Runtime: runtime,
            ColumnNames.Version: current_version,
        }

        # now leave
        return EPLaunchWorkflowResponse1(
            success=True, message="Ran EnergyPlus OK for file: %s!" % file_name, column_data=column_data
        )


class EnergyPlusWorkflowSI(BaseEPLaunchWorkflow1):

    def name(self):
        return "EnergyPlus-${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}.${CMAKE_VERSION_PATCH} SI"

    def context(self):
        return "EnergyPlus-${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}.${CMAKE_VERSION_PATCH}-${CMAKE_VERSION_BUILD}"

    def description(self):
        return "Run EnergyPlus with SI unit system"

    def uses_weather(self):
        return True

    def get_file_types(self):
        return ["*.idf", "*.imf", "*.epJSON"]

    def get_output_suffixes(self):
        return EPlusRunManager.eplus_suffixes()

    def get_extra_data(self):
        return {"Hey, it's extra": "data"}

    def get_interface_columns(self):
        return [ColumnNames.Errors, ColumnNames.Warnings, ColumnNames.Runtime, ColumnNames.Version]

    def main(self, run_directory, file_name, args):
        response = EPlusRunManager.run_energyplus(self, False, run_directory, file_name, args, False)
        if type(response) is not EPLaunchWorkflowResponse1:
            response = EPLaunchWorkflowResponse1(
                success=False,
                message="Current workflow run_energyplus function did not respond properly",
                column_data=None,
            )
        return response


class EnergyPlusWorkflowIP(BaseEPLaunchWorkflow1):

    def name(self):
        return "EnergyPlus-${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}.${CMAKE_VERSION_PATCH} IP"

    def context(self):
        return "EnergyPlus-${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}.${CMAKE_VERSION_PATCH}-${CMAKE_VERSION_BUILD}"

    def description(self):
        return "Run EnergyPlus with IP unit system"

    def uses_weather(self):
        return True

    def get_file_types(self):
        return ["*.idf", "*.imf", "*.epJSON"]

    def get_output_suffixes(self):
        return EPlusRunManager.eplus_suffixes()

    def get_extra_data(self):
        return {"Hey, it's extra": "data"}

    def get_interface_columns(self):
        return [ColumnNames.Errors, ColumnNames.Warnings, ColumnNames.Runtime, ColumnNames.Version]

    def main(self, run_directory, file_name, args):
        response = EPlusRunManager.run_energyplus(self, True, run_directory, file_name, args, False)
        if type(response) is not EPLaunchWorkflowResponse1:
            response = EPLaunchWorkflowResponse1(
                success=False,
                message="Current workflow run_energyplus function did not respond properly",
                column_data=None,
            )
        return response


class EnergyPlusWorkflowSIByAPI(BaseEPLaunchWorkflow1):

    def name(self):
        return "EnergyPlus-${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}.${CMAKE_VERSION_PATCH} SI (Call via API)"

    def context(self):
        return "EnergyPlus-${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}.${CMAKE_VERSION_PATCH}-${CMAKE_VERSION_BUILD}"

    def description(self):
        return "Run EnergyPlus by API with SI unit system"

    def uses_weather(self):
        return True

    def get_file_types(self):
        return ["*.idf", "*.imf", "*.epJSON"]

    def get_output_suffixes(self):
        return EPlusRunManager.eplus_suffixes()

    def get_extra_data(self):
        return {"Hey, it's extra": "data"}

    def get_interface_columns(self):
        return [ColumnNames.Errors, ColumnNames.Warnings, ColumnNames.Runtime, ColumnNames.Version]

    def main(self, run_directory, file_name, args):
        response = EPlusRunManager.run_energyplus(self, False, run_directory, file_name, args, True)
        if type(response) is not EPLaunchWorkflowResponse1:
            response = EPLaunchWorkflowResponse1(
                success=False,
                message="Current workflow run_energyplus function did not respond properly",
                column_data=None,
            )
        return response
