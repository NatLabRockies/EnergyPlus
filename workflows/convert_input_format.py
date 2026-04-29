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

import os
import platform
import subprocess

from eplaunch.workflows.base import BaseEPLaunchWorkflow1, EPLaunchWorkflowResponse1


class ConvertInputFormatWorkflow(BaseEPLaunchWorkflow1):

    def name(self):
        return "ConvertInputFormat-${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}.${CMAKE_VERSION_PATCH}"

    def context(self):
        return "EnergyPlus-${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}.${CMAKE_VERSION_PATCH}-${CMAKE_VERSION_BUILD}"

    def description(self):
        return "Run ConvertInputFormat"

    def get_file_types(self):
        return ["*.idf", "*.epJSON"]

    def get_output_suffixes(self):
        return [".idf", ".epJSON"]

    def get_extra_data(self):
        return {"Hey, it's extra": "data"}

    def get_interface_columns(self):
        return []

    def main(self, run_directory, file_name, args):
        if "workflow location" in args:
            energyplus_root_folder, _ = os.path.split(args["workflow location"])
            if platform.system() == "Windows":
                convertinputformat_binary = os.path.join(energyplus_root_folder, "ConvertInputFormat.exe")
            else:
                convertinputformat_binary = os.path.join(energyplus_root_folder, "ConvertInputFormat")
            if not os.path.exists(convertinputformat_binary):
                return EPLaunchWorkflowResponse1(
                    success=False,
                    message="ConvertInputFormat binary not found: {}!".format(convertinputformat_binary),
                    column_data=[],
                )
        else:
            return EPLaunchWorkflowResponse1(
                success=False, message="Workflow location missing: {}!".format(args["worflow location"]), column_data=[]
            )

        original_with_path = os.path.join(run_directory, file_name)
        converted_file_no_ext, original_ext = os.path.splitext(original_with_path)
        if original_ext.lower() == ".idf":
            converted_file_with_path = converted_file_no_ext + ".epJSON"
        elif original_ext.lower() == ".epjson":
            converted_file_with_path = converted_file_no_ext + ".idf"
        else:
            return EPLaunchWorkflowResponse1(
                success=False,
                message="Invalid extension {} on file: {}!".format(original_ext, original_with_path),
                column_data=[],
            )

        if (
            os.path.exists(original_with_path)
            and os.path.exists(convertinputformat_binary)
            and os.path.exists(run_directory)
        ):

            # execute utility
            command_line_args = [convertinputformat_binary, original_with_path]
            try:
                for message in self.execute_for_callback(command_line_args, run_directory):
                    self.callback(message)
            except subprocess.CalledProcessError:
                self.callback("ConvertInputFormat FAILED")
                return EPLaunchWorkflowResponse1(
                    success=False,
                    message="ConvertInputFormat failed for file: %s!" % original_with_path,
                    column_data={},
                )
            return EPLaunchWorkflowResponse1(
                success=True,
                message="Ran ConvertInputFormat OK for file: {}!".format(original_with_path),
                column_data=[],
            )
        else:
            return EPLaunchWorkflowResponse1(
                success=False,
                message="ConvertInputFormat file not found: {}!".format(original_with_path),
                column_data=[],
            )
