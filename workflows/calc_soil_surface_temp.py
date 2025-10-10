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

import os
import platform
import shutil
from subprocess import PIPE, Popen

from eplaunch.workflows.base import BaseEPLaunchWorkflow1, EPLaunchWorkflowResponse1


class ColumnNames(object):
    AverageSurfTemp = "Avg Surf Temp [C]"
    AmplitudeSurfTemp = "Amp Surf Temp [C]"
    PhaseConstant = "Phase Constant [days]"


class CalcSoilSurfTempWorkflow(BaseEPLaunchWorkflow1):

    def name(self):
        return "CalcSoilSurfTemp-${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}.${CMAKE_VERSION_PATCH}"

    def context(self):
        return "EnergyPlus-${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}.${CMAKE_VERSION_PATCH}-${CMAKE_VERSION_BUILD}"

    def description(self):
        return "Run CalcSoilSurfTemp Preprocessor"

    def get_file_types(self):
        return ["*.epw"]

    def get_output_suffixes(self):
        return [".out"]

    def get_extra_data(self):
        return {"Hey, it's extra": "data"}

    def get_interface_columns(self):
        return [ColumnNames.AverageSurfTemp, ColumnNames.AmplitudeSurfTemp, ColumnNames.PhaseConstant]

    def main(self, run_directory, file_name, args):
        if "workflow location" in args:
            energyplus_root_folder, _ = os.path.split(args["workflow location"])
            preprocess_folder = os.path.join(energyplus_root_folder, "PreProcess")
            calc_soil_surf_temp_folder = os.path.join(preprocess_folder, "CalcSoilSurfTemp")
            if platform.system() == "Windows":
                calc_soil_surf_temp_binary = os.path.join(calc_soil_surf_temp_folder, "CalcSoilSurfTemp.exe")
            else:
                calc_soil_surf_temp_binary = os.path.join(calc_soil_surf_temp_folder, "CalcSoilSurfTemp")
            if not os.path.exists(calc_soil_surf_temp_binary):
                return EPLaunchWorkflowResponse1(
                    success=False,
                    message="CalcSoilSurfTemp binary not found: {}!".format(calc_soil_surf_temp_binary),
                    column_data=[],
                )
        else:
            return EPLaunchWorkflowResponse1(
                success=False, message="Workflow location missing: {}!".format(args["worflow location"]), column_data=[]
            )

        full_file_path = os.path.join(run_directory, file_name)
        full_file_no_ext, _ = os.path.splitext(full_file_path)
        out_file_path = full_file_no_ext + ".out"

        csst_out = os.path.join(run_directory, "CalcSoilSurfTemp.out")
        if os.path.exists(csst_out):
            self.callback("Removing previous output file")
            os.remove(csst_out)

        if os.path.exists(out_file_path):
            try:
                os.remove(out_file_path)
            except OSError:
                return EPLaunchWorkflowResponse1(
                    success=False,
                    message="Could not delete prior CalcSoilSurfTemp results file: %s!" % out_file_path,
                    column_data={},
                )

        # run E+ and gather (for now fake) data
        self.callback("Running CalcSoilSurfTemp!")
        p1 = Popen([calc_soil_surf_temp_binary, file_name], stdout=PIPE, stdin=PIPE, cwd=run_directory)
        p1.communicate(input=b"1\n1\n")
        self.callback("CalcSoilSurfTemp Completed!")

        if not os.path.exists(csst_out):
            return EPLaunchWorkflowResponse1(
                success=False, message="CalcSoilSurfTemp failed for file: %s!" % full_file_path, column_data={}
            )

        if os.path.exists(out_file_path):
            os.remove(out_file_path)

        # Copy output files to input/output path
        shutil.copy2(csst_out, out_file_path)

        # Clean up directory.
        if os.path.exists(csst_out):
            os.remove(csst_out)

        try:
            self.callback("Processing output file...")
            with open(out_file_path, "r") as f:
                lines = f.readlines()
                avg_temp = float(lines[1][40:-1])
                amp_temp = float(lines[2][38:-1])
                phase_constant = int(lines[3][42:-1])
                column_data = {
                    ColumnNames.AverageSurfTemp: avg_temp,
                    ColumnNames.AmplitudeSurfTemp: amp_temp,
                    ColumnNames.PhaseConstant: phase_constant,
                }
                self.callback("   ...DONE!")
                return EPLaunchWorkflowResponse1(
                    success=True, message="Ran EnergyPlus OK for file: %s!" % file_name, column_data=column_data
                )
        except Exception:  # noqa -- there could be lots of issues here, file existence, file contents, float conversion
            self.callback("   ...FAILED!")
            return EPLaunchWorkflowResponse1(
                success=False,
                message="CalcSoilSurfTemp seemed to run, but post processing failed for file: %s!" % full_file_path,
                column_data={},
            )
