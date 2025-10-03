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

from ep_testing.tests.base import BaseTest


class TestVersionInfoInDocumentation(BaseTest):

    def name(self):
        return "Verify contents in a PDF"

    def run(self, install_root: str, verbose: bool, kwargs: dict):
        if "pdf_file" not in kwargs:
            raise Exception("Bad call to %s -- must pass pdf_file in kwargs" % self.__class__.__name__)
        if "version_string" not in kwargs:
            raise Exception("Bad call to %s -- must pass version_string in kwargs" % self.__class__.__name__)
        pdf_file = kwargs["pdf_file"]
        version_string = kwargs["version_string"]
        print('* Running test class "%s" on file "%s"... ' % (self.__class__.__name__, pdf_file), end="")
        documentation_dir = os.path.join(install_root, "Documentation")
        saved_dir = os.getcwd()
        os.chdir(documentation_dir)
        original_pdf_path = os.path.join(documentation_dir, pdf_file)
        target_pdf_path = os.path.join(documentation_dir, "FirstPage_%s" % pdf_file)
        dev_null = open(os.devnull, "w")
        try:
            check_call(
                ["pdftk", original_pdf_path, "cat", "1", "output", target_pdf_path], stdout=dev_null, stderr=STDOUT
            )
            print(" [PAGE1_EXTRACTED] ", end="")
        except CalledProcessError:
            raise Exception("PdfTk Page 1 extraction failed!")
        target_txt_path = target_pdf_path + ".txt"
        try:
            check_call(["pdftotext", target_pdf_path, target_txt_path], stdout=dev_null, stderr=STDOUT)
            print(" [PAGE1_CONVERTED] ", end="")
        except CalledProcessError:
            raise Exception("PdfToText Page 1 conversion failed!")
        with open(target_txt_path) as f:
            contents = f.read()
            if version_string in contents:
                print(" [FOUND VERSION STRING, DONE]!")
            else:
                raise Exception(
                    "Did not find matching version string in PDF front page, page contents = \n%s" % contents
                )
        os.chdir(saved_dir)
