#!/usr/bin/python
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

# python 2/3 compatibility imports
from __future__ import absolute_import, print_function, unicode_literals

import glob
import io
import sys
from os import path

import idd_parser_library
import idf_parser_library

# 3 arguments: a directory to the idfs, and an output html file
if len(sys.argv) != 4:
    print("ERROR ARGUMENTS")
    sys.exit(1)
else:
    idd_file = sys.argv[1]
    idf_dir = sys.argv[2]
    html_file = sys.argv[3]

mainObjectDictionary = {}
totalReferences = {}

# process the idd first
this_idd = idd_parser_library.iddFile(idd_file)
idd_objects = sorted(this_idd.iddObjects)
for idd_obj in idd_objects:
    mainObjectDictionary[idd_obj.upper()] = []
    totalReferences[idd_obj.upper()] = 0

# then loop over the idfs and create dictionaries as needed
idf_set = sorted(glob.glob(path.join(idf_dir, "*.idf")))
idf_summaries = []
processing_idf_num = 0
for idf in idf_set:
    processing_idf_num += 1
    # filename only
    idf_name = path.basename(idf)
    # create an idf library class, and a summary instance
    this_idf = idf_parser_library.idfFile(idf)

    # loop over objects, processing stuff
    for idf_object in this_idf.idf_objects:
        upperObjName = idf_object.objectName.upper()
        if idf_name == "GSHPSimple-GLHE.idf":
            pass  # print(upperObjName)
        if upperObjName not in mainObjectDictionary:
            # theres a few things we can ignore:
            if upperObjName in ["LEAD INPUT", "END LEAD INPUT", "END SIMULATION DATA", "REPORT VARIABLE DICTIONARY"]:
                continue
            print("***Found object not in IDD:")
            print("  IDF = " + idf_name)
            print("  OBJ = " + upperObjName)
            mainObjectDictionary[upperObjName] = []
            totalReferences[upperObjName] = 0
        else:
            # if the idf isn't already in the idf list, add it
            if idf_name not in mainObjectDictionary[upperObjName]:
                mainObjectDictionary[upperObjName].append(idf_name)
            # no matter what increment the total count
            totalReferences[upperObjName] += 1

with io.open(html_file, "w") as f:

    def out(s):
        print(s, file=f)

    def header():
        out(" <tr>\n")
        for headerItem in [
            "ObjectName",
            "Number of Files with Object",
            "Total References in all files",
            "First File with Object",
            "Second File with Object",
            "Third File with Object",
        ]:
            out("  <th>" + headerItem + "</th>\n")
        out(" </tr>\n")

    out("<html>")
    out("<head><title>EnergyPlus Example File Objects Link</title></head>")
    out("<body>")
    out("<style>")
    out("table, th, td {")
    out(" border: 1px solid black;")
    out(" border-collapse: collapse;")
    out("}")
    out("th,td {")
    out(" padding: 6px;")
    out("}")
    out("</style>")
    out("<h1>EnergyPlus Example File Objects Link</h1>")
    out("This file is auto-generated from the objects and comments in the example files and idd in GitHub.")
    out('<table border="1" >')
    header()
    entry = 0
    for k in sorted(mainObjectDictionary.keys()):
        v = mainObjectDictionary[k]
        entry += 1
        if entry % 20 == 0:
            header()
        out(" <tr>\n")
        out("  <td>" + k + "</td>\n")
        out("  <td>" + str(len(v)) + "</td>\n")
        out("  <td>" + str(totalReferences[k]) + "</td>\n")
        idf_num = 0
        for idf in v:
            idf_num += 1
            out("  <td>" + idf + "</td>\n")
            if idf_num == 3:
                break
        for i in range(0, 3 - idf_num):
            out("  <td></td>\n")
    out("</table>")
    out("</body>")
    out("</html>")

print(" +++ AutoDocs: Completed processing example file links summary: processed %i example files" % processing_idf_num)
