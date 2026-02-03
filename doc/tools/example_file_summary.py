#!/usr/bin/python
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

# python 2/3 compatibility imports
from __future__ import absolute_import, print_function, unicode_literals

import glob
import io
import sys
from os import path

import idf_parser_library
import idf_summary_class

# 2 arguments: a directory to the idfs, and an output html file
if len(sys.argv) != 3:
    print("ERROR ARGUMENTS")
    sys.exit(1)
else:
    idf_dir = sys.argv[1]
    html_file = sys.argv[2]

idf_set = sorted(glob.glob(path.join(idf_dir, "*.idf")))
idf_summaries = []
processing_idf_num = 0
for idf in idf_set:
    processing_idf_num += 1
    # filename only
    idf_name = path.basename(idf)
    # create an idf library class, and a summary instance
    this_idf = idf_parser_library.idfFile(idf)
    idf_summary = idf_summary_class.IDFSummary(idf_name)
    # loop over objects, processing stuff
    for idf_object in this_idf.idf_objects:
        upperObjName = idf_object.objectName.upper()
        try:
            if "SITE:LOCATION" == upperObjName:
                idf_summary.location = idf_object.fields[0]  # weather file name **could** be inferred I guess
            elif "SIZINGPERIOD:" in upperObjName:
                idf_summary.numSizingPeriods += 1
            elif "ZONE" == upperObjName:
                idf_summary.numberZones += 1
            elif "INTERNALMASS" == upperObjName:
                idf_summary.internalMass = True
            elif "PEOPLE" == upperObjName:
                idf_summary.people = True
            elif "LIGHTS" == upperObjName:
                idf_summary.lights = True
            elif any((x in upperObjName for x in ("FENESTRATIONSURFACE", "WINDOW", "GLAZEDDOOR"))):
                idf_summary.windows = True
            elif upperObjName in ["SHADING:SITE", "SHADING:BUILDING"]:
                idf_summary.detachedShading = True
            elif "DAYLIGHTING:" in upperObjName:
                idf_summary.daylighting = True
            elif "SCHEDULE:COMPACT" in upperObjName:
                idf_summary.compactSchedules = True
            elif "SIMULATIONCONTROL" == upperObjName:
                if idf_object.fields[0].upper()[1] == "Y":
                    idf_summary.zoneAutoSize = True
                if idf_object.fields[1].upper()[1] == "Y":
                    idf_summary.systemAutoSize = True
                if idf_object.fields[2].upper()[1] == "Y":
                    idf_summary.plantAutoSize = True
            elif "ZONEHVAC:" in upperObjName:
                idf_summary.zoneEquipment = True
                if "ZONEHVAC:IDEALLOADSAIRSYSTEM" == upperObjName:
                    idf_summary.idealLoads = True
            elif "AIRLOOPHVAC" in upperObjName:
                idf_summary.centralAirHandlingEquipment = True
            elif "DISTRICTHEATING" == upperObjName:
                idf_summary.districtHeating = True
            elif "DISTRICTCOOLING" == upperObjName:
                idf_summary.districtCooling = True
            elif "COIL:" in upperObjName:
                idf_summary.coils = True
            elif any((x in upperObjName for x in ("PUMP:", "HEADEREDPUMPS:"))):
                idf_summary.pumps = True
            elif "BOILER:" in upperObjName:
                idf_summary.boilers = True
            elif "CHILLER:" in upperObjName:
                idf_summary.chillers = True
            elif "COOLINGTOWER:" in upperObjName:
                idf_summary.towers = True
            elif "OUTPUT:TABLE:MONTHLY" == upperObjName:
                idf_summary.standardReports = True
            elif "OUTPUT:VARIABLE" == upperObjName:
                if idf_object.fields[3].upper() == "HOURLY":
                    idf_summary.hourlyTimeSeries = True
            elif "OUTPUT:TABLE:TIMEBINS" == upperObjName:
                idf_summary.timeBinsReport = True
            elif "OUTPUTCONTROL:TABLE:STYLE" == upperObjName:
                if idf_object.fields[0].upper() == "HTML":
                    idf_summary.htmlReport = True
            elif "OUTPUT:SQLITE" == upperObjName:
                idf_summary.SQLReport = True
            elif "FUELFACTORS" == upperObjName:
                idf_summary.environmentEmissions = True
            elif "UTILITYCOST:TARIFF" == upperObjName:
                idf_summary.utilityTariffs = True
            elif "LIFECYCLECOST:PARAMETERS" == upperObjName:
                idf_summary.lifeCycleCosts = True
            elif "COMPONENTCOST:" in upperObjName:
                idf_summary.costEstimates = True
        except Exception as e:
            pass

    # loop over comments, looking for magic words
    reading_highlights = False
    got_highlights = False
    highlights = []
    for comment in this_idf.comments:
        try:
            if reading_highlights:
                # we'll use the big indent as the kicker
                if "                        " in comment:
                    highlights.append(comment)
                else:
                    reading_highlights = False
                    for highlight_line in highlights:
                        idf_summary.description += highlight_line[1:].strip() + " "
                    continue
            else:
                if "Highlights" in comment and not got_highlights:
                    reading_highlights = True
                    got_highlights = True
                    highlights.append(comment[comment.find(":") :])
            if "Floor Area" in comment:
                idf_summary.floorArea = float(comment[comment.find(":") + 1 : comment.find("m2")].strip())
            elif "Number of Stories" in comment:
                idf_summary.numberFloors = int(comment[comment.find(":") + 1 :])
        except Exception as e:
            pass
    # sanitize the description for kicks
    idf_summary.description = idf_summary.description.replace(",", " ").replace(";", " ")
    idf_summaries.append(idf_summary)

with io.open(html_file, "w") as f:

    def out(s):
        print(s, file=f)

    out("<html>")
    out("<head><title>EnergyPlus Example File Summary</title></head>")
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
    out("<h1>EnergyPlus Example File Summary</h1>")
    out("This file is auto-generated from the objects and comments in the example files in GitHub.")
    out('<table border="1" >')
    out(idf_summary_class.IDFSummary.summarize_header_html())
    entry = 0
    for idf_summary in idf_summaries:
        entry += 1
        if entry % 12 == 0:
            out(idf_summary_class.IDFSummary.summarize_header_html())
        out(idf_summary.summarize_html())
    out("</table>")
    out("</body>")
    out("</html>")

print(" +++ AutoDocs: Completed processing example file summary: processed %i example files" % processing_idf_num)
