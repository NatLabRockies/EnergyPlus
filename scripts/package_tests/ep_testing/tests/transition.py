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
from subprocess import STDOUT, CalledProcessError, check_call
from urllib import request

from ep_testing.tests.base import BaseTest


class TransitionOldFile(BaseTest):

    def name(self):
        return "Test running 1ZoneUncontrolled.idf and make sure it exits OK"

    def run(self, install_root: str, verbose: bool, kwargs: dict):
        if "last_version" not in kwargs:
            raise Exception("Bad call to %s -- must pass last_version in kwargs" % self.__class__.__name__)
        last_version = kwargs["last_version"]
        test_file = kwargs.get("test_file", "1ZoneUncontrolled.idf")
        print('* Running test class "%s" on file "%s"... ' % (self.__class__.__name__, test_file), end="")
        transition_dir = os.path.join(install_root, "PreProcess", "IDFVersionUpdater")
        all_transition_binaries = [
            f.path for f in os.scandir(transition_dir) if f.is_file() and f.name.startswith("Transition-")
        ]
        if len(all_transition_binaries) < 1:
            raise Exception("Could not find any transition binaries...weird")
        all_transition_binaries.sort()
        most_recent_binary = all_transition_binaries[-1]
        idf_url = "https://raw.githubusercontent.com/NREL/EnergyPlus/%s/testfiles/%s" % (last_version, test_file)
        saved_dir = os.getcwd()
        os.chdir(transition_dir)
        idf_path = os.path.join(transition_dir, test_file)
        dev_null = open(os.devnull, "w")
        try:
            r = request.Request(idf_url)
            response = request.urlopen(r)
            data = response.read()
            with open(idf_path, "wb") as f:
                f.write(data)
        except Exception as e:
            raise Exception("Could not download file from prior release at %s; error: %s" % (idf_url, str(e))) from None
        try:
            check_call([most_recent_binary, os.path.basename(idf_path)], stdout=dev_null, stderr=STDOUT)
            print(" [TRANSITIONED] ", end="")
        except CalledProcessError:
            raise Exception("Transition failed!") from None
        os.chdir(install_root)
        eplus_binary = os.path.join(install_root, "energyplus")
        try:
            check_call([eplus_binary, "-D", idf_path], stdout=dev_null, stderr=STDOUT)
            print(" [DONE]!")
        except CalledProcessError:
            raise Exception("EnergyPlus failed!") from None
        os.chdir(saved_dir)
