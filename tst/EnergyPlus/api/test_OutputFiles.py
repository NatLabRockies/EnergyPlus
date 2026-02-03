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

import glob as gb
import os
import tempfile
import unittest

from pyenergyplus.api import EnergyPlusAPI


def parent(x):
    return os.path.dirname(os.path.realpath(x))


this_file = os.path.realpath(__file__)  # returns C:\EPRepo\tst\EnergyPlus\api\test_OutputFiles.py
repo_root = parent(parent(parent(parent(this_file))))  # returns C:\EPRepo
# something **very** odd was happening with os.path.join here on Windows.  Putting a couple separators manually works OK
idf_path = repo_root + os.sep + os.sep + "testfiles" + os.sep + os.sep + "1ZoneUncontrolled.idf"
# at this point, idf_path should be something like: C:\\EPRepo\\testfiles\\1ZoneUncontrolled.idf


class TestAPIFlushOutput(unittest.TestCase):
    """
    unittest on EnergyPlusAPI for #8484 - [pyenergyplus] EnergyPlus output not
    flushed to eplusout.err
    """

    @classmethod
    def setUpClass(cls):
        """
        A class method called before tests in an individual class are run
        """
        cls.api = EnergyPlusAPI()

    def setUp(self):
        """
        Prepare each test fixture, with a new state
        """
        self.state = self.api.state_manager.new_state()

    def test_proper_run(self):
        """
        Tests the case where the simulation ends properly and EndEnergyPlus is
        called
        """
        tempdir = tempfile.mkdtemp()
        cmd_args = ["-d", tempdir, "-D", idf_path]
        return_code = self.api.runtime.run_energyplus(self.state, cmd_args)
        self.assertEqual(return_code, 0)
        out_files = gb.glob(os.path.join(tempdir, "*"))
        # We need to find out files, and we expect NONE of them to be empty
        self.assertGreater(len(out_files), 10)
        empty_files = list(filter(lambda f: os.stat(f).st_size == 0, out_files))
        msg = f"Did not expect empty files, but found {len(empty_files)}:"
        for f in empty_files:
            msg += f"\n * {os.path.relpath(f, tempdir)}"
        self.assertFalse(empty_files, msg)

        # Check that the eplusout.err is not truncated, that the last line
        # is what we expect it to be
        with open(os.path.join(tempdir, "eplusout.err"), "r") as f:
            lines = f.read().splitlines()
        self.assertTrue(lines, "eplusout.err is empty!")
        self.assertIn("EnergyPlus Completed Successfully", lines[-1])

    def test_wrong_version_api_output(self):
        """
        Test for #8484 - When the simulation starts, but fails, we should still
        find proper content in the eplusout.err (AbortEnergyPlus is called)
        """

        idf_content = """
  Version," + DataStringGlobals::MatchVersion + ";"

  Timestep,4;

  Building,
    Simple One Zone (Wireframe DXF),  !- Name
    0,                       !- North Axis {deg}
    Suburbs,                 !- Terrain
    0.04,                    !- Loads Convergence Tolerance Value {W}
    0.004,                   !- Temperature Convergence Tolerance Value {dC}
    MinimalShadowing,        !- Solar Distribution
    30,                      !- Maximum Number of Warmup Days
    6;                       !- Minimum Number of Warmup Days

  HeatBalanceAlgorithm,ConductionTransferFunction;

  SurfaceConvectionAlgorithm:Inside,TARP;

  SurfaceConvectionAlgorithm:Outside,DOE-2;

  GlobalGeometryRules,
    UpperLeftCorner,         !- Starting Vertex Position
    CounterClockWise,        !- Vertex Entry Direction
    World;                   !- Coordinate System
        """

        tempdir = tempfile.mkdtemp()
        idf_file = os.path.join(tempdir, "wrong_version.idf")

        with open(idf_file, "w") as f:
            f.write(idf_content)

        cmd_args = ["-d", tempdir, "-D", idf_file]
        return_code = self.api.runtime.run_energyplus(self.state, cmd_args)
        self.assertEqual(return_code, 1)
        with open(os.path.join(tempdir, "eplusout.err"), "r") as f:
            lines = f.read().splitlines()
        self.assertTrue(lines, "eplusout.err is empty!")
        self.assertIn("EnergyPlus Terminated", lines[-1])


if __name__ == "__main__":
    unittest.main()
