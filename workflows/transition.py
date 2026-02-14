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

import os
import shutil
import subprocess
from pathlib import Path
from typing import Tuple

from eplaunch.workflows.base import BaseEPLaunchWorkflow1, EPLaunchWorkflowResponse1


class Version:
    """A reduced version of the original Version class, only including the functions needed for this workflow"""

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


class TransitionWorkflow(BaseEPLaunchWorkflow1):

    def name(self):
        return "Transition-${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}.${CMAKE_VERSION_PATCH}"

    def context(self):
        return "EnergyPlus-${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}.${CMAKE_VERSION_PATCH}-${CMAKE_VERSION_BUILD}"

    def description(self):
        return "Run Version Transition"

    def get_file_types(self):
        return ["*.idf"]

    def get_output_suffixes(self):
        return [".vcperr"]

    def get_extra_data(self):
        return {"Hey, it's extra": "data"}

    def main(self, run_directory, file_name, args):
        transition_exes = TransitionWorkflow.find_transition_executable_files(args["workflow location"])
        if transition_exes:
            full_file_path = os.path.join(run_directory, file_name)
            if os.path.exists(full_file_path):
                returned_success, returned_message = self.perform_transition(full_file_path, transition_exes)
                return EPLaunchWorkflowResponse1(success=returned_success, message=returned_message, column_data=[])
            else:
                return EPLaunchWorkflowResponse1(
                    success=False, message="Transition file not found: {}!".format(""), column_data=[]
                )
        else:
            return EPLaunchWorkflowResponse1(
                success=False, message="Transition exe file not found: {}!".format(""), column_data=[]
            )

    @staticmethod
    def find_transition_executable_files(workflow_location):
        energyplus_root_folder, _ = os.path.split(workflow_location)
        idf_updater_dir = os.path.join(energyplus_root_folder, "PreProcess", "IDFVersionUpdater")
        transition_exes = [os.path.join(idf_updater_dir, f) for f in os.listdir(idf_updater_dir) if "Transition-V" in f]
        transition_exes.sort()
        transition_dict = {}
        for transition_exe in transition_exes:
            start_number, end_number = TransitionWorkflow.get_start_end_version_from_exe(transition_exe)
            transition_dict[start_number] = [end_number, transition_exe]
        return transition_dict

    @staticmethod
    def get_start_end_version_from_exe(exe_file_name):
        filename = os.path.basename(exe_file_name)
        if filename[:11] == "Transition-":
            versions_string_with_maybe_ext = filename[11:]
            if "." in versions_string_with_maybe_ext:
                versions_string, _ = versions_string_with_maybe_ext.split(".")
            else:
                versions_string = versions_string_with_maybe_ext
            start_version, end_version = versions_string.split("-to-")
            start_number = Version.numeric_version_from_dash_string(start_version)
            end_number = Version.numeric_version_from_dash_string(end_version)
            return start_number, end_number
        else:
            return 0, 0

    def perform_transition(self, path_to_old_file, transition_executable_files):
        v_found, v_start_string, v_start = Version.check_idf_imf_energyplus_version(path_to_old_file)
        if v_start in transition_executable_files:
            v_this = v_start
            while v_this in transition_executable_files:
                v_this_string = Version.string_version_from_number(v_this)
                v_next, binary = transition_executable_files[v_this]
                ok, msg = self.run_single_transition(binary, path_to_old_file, v_this_string)
                if not ok:
                    return False, f'Transition Failed! "{msg}"'
                v_this = v_next
            v_final_string = Version.string_version_from_number(v_this)
            return True, f"Version update successful {v_start_string}->{v_final_string} for IDF file {path_to_old_file}"
        else:
            return False, f"Version update failed for IDF file {path_to_old_file} (original version {v_start_string})"

    def run_single_transition(self, transition_exe_path, file_to_update, old_version_string):

        def delete_if_exists(file_path):
            if os.path.exists(file_path):
                os.remove(file_path)

        def rename_if_exists(source_file, new_file):
            if os.path.exists(source_file):
                os.rename(source_file, new_file)

        file_no_extension, _ = os.path.splitext(file_to_update)
        run_directory, _ = os.path.split(transition_exe_path)
        command_line_args = [transition_exe_path, file_to_update]
        idf_copy_of_old_file_temp = file_to_update + "_tempcopy"
        # make temporary copy that preserve file date
        shutil.copy2(file_to_update, idf_copy_of_old_file_temp)
        # see if rvi file is used
        rvi_copy_of_old_file_temp = ""
        orig_rvi_file = file_no_extension + ".rvi"
        if os.path.exists(orig_rvi_file):
            rvi_copy_of_old_file_temp = file_no_extension + ".rvi_tempcopy"
            shutil.copy2(orig_rvi_file, rvi_copy_of_old_file_temp)
        # see if mvi file is used
        mvi_copy_of_old_file_temp = ""
        orig_mvi_file = file_no_extension + ".mvi"
        if os.path.exists(orig_mvi_file):
            mvi_copy_of_old_file_temp = file_no_extension + ".mvi_tempcopy"
            shutil.copy2(orig_mvi_file, mvi_copy_of_old_file_temp)
        # perform transition
        try:
            for message in self.execute_for_callback(command_line_args, run_directory):
                self.callback(message)
        except subprocess.CalledProcessError:
            self.callback("Transition Failed for this file")
            return False, "Transition failed for file " + file_to_update
        self.callback("Conversion using %s complete! Copying files" % os.path.basename(transition_exe_path))
        # delete the extra outputs that are not needed
        idf_new_path = file_no_extension + ".idfnew"
        if os.path.exists(idf_new_path):
            os.remove(idf_new_path)
            idf_old_path = file_no_extension + ".idfold"
            delete_if_exists(idf_old_path)
            # rename the previously copied file to preserve the old version
            idf_revised_old_path = file_no_extension + "_" + old_version_string + ".idf"
            rename_if_exists(idf_copy_of_old_file_temp, idf_revised_old_path)
            # work on the rvi update
            delete_if_exists(file_no_extension + ".rvinew")
            delete_if_exists(file_no_extension + ".rviold")
            rvi_revised_old_path = file_no_extension + "_" + old_version_string + ".rvi"
            if os.path.exists(orig_rvi_file):
                os.rename(rvi_copy_of_old_file_temp, rvi_revised_old_path)
            # work on the rvi update
            delete_if_exists(file_no_extension + ".mvinew")
            delete_if_exists(file_no_extension + ".mviold")
            mvi_revised_old_path = file_no_extension + "_" + old_version_string + ".mvi"
            if os.path.exists(orig_mvi_file):
                os.rename(mvi_copy_of_old_file_temp, mvi_revised_old_path)
            # process any error file
            rename_if_exists(file_no_extension + ".vcperr", file_no_extension + "_" + old_version_string + ".vcperr")
            return True, "Successfully converted file: " + file_to_update
        else:
            return False, "Conversion problem for file: " + file_to_update
