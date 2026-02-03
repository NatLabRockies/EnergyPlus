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


class IDFSummary:
    def __init__(self, idf_file_name):
        self.filename = idf_file_name
        self.location = ""
        self.weatherFileName = ""  # no
        self.numSizingPeriods = 0  # it's incremented, so start at 0, not -1
        self.floorArea = -1  # it is inferred from comments, so leave -1 as a flag
        self.numberFloors = -1  # it is inferred from comments, so leave -1 as a flag
        self.numberZones = 0  # it's incremented, so start at 0, not -1
        self.internalMass = False
        self.people = False
        self.lights = False
        self.windows = False
        self.detachedShading = False
        self.daylighting = False
        self.compactSchedules = False
        self.zoneAutoSize = False
        self.idealLoads = False
        self.zoneEquipment = False
        self.centralAirHandlingEquipment = False
        self.systemAutoSize = False
        self.districtCooling = False
        self.districtHeating = False
        self.coils = False
        self.pumps = False
        self.boilers = False
        self.chillers = False
        self.towers = False
        self.plantAutosize = False
        self.standardReports = False
        self.hourlyTimeSeries = False
        self.timeBinsReport = False
        self.htmlReport = False
        self.SQLReport = False
        self.environmentEmissions = False
        self.utilityTariffs = False
        self.lifeCycleCosts = False
        self.costEstimates = False
        self.description = ""

    @staticmethod
    def summarize_header():
        return (
            ",".join(
                (
                    "Filename",
                    "Location",
                    "NumSizingPeriods",
                    "FloorArea-m2",
                    "NumStories",
                    "NumZones",
                    "InternalMass?",
                    "People?",
                    "Lights?",
                    "Windows?",
                    "DetachedShading?",
                    "Daylighting?",
                    "CompactSchedules?",
                    "ZoneAutoSized?",
                    "IdealLoads?",
                    "ZoneEquipment?",
                    "AirLoopEquipment?",
                    "SystemAutoSized?",
                    "DistrictCooling?",
                    "DistrictHeating?",
                    "Coils?",
                    "Pumps?",
                    "Boilers?",
                    "Chillers?",
                    "CoolingTowers?",
                    "PlantAutoSized?",
                    "StandardReports?",
                    "HourlyTimeSeries?",
                    "TimeBinsReport?",
                    "HTMLReport?",
                    "SQLReport?",
                    "Emissions?",
                    "UtilityTariffs?",
                    "LifeCycleCosts?",
                    "CostEstimates?",
                    "Description",
                )
            )
            + "\n"
        )

    @staticmethod
    def summarize_header_html():
        out_string = " <tr>\n"
        for headerItem in (
            "Filename",
            "Location",
            "NumSizingPeriods",
            "FloorArea-m2",
            "NumStories",
            "NumZones",
            "InternalMass?",
            "People?",
            "Lights?",
            "Windows?",
            "DetachedShading?",
            "Daylighting?",
            "CompactSchedules?",
            "ZoneAutoSized?",
            "IdealLoads?",
            "ZoneEquipment?",
            "AirLoopEquipment?",
            "SystemAutoSized?",
            "DistrictCooling?",
            "DistrictHeating?",
            "Coils?",
            "Pumps?",
            "Boilers?",
            "Chillers?",
            "CoolingTowers?",
            "PlantAutoSized?",
            "StandardReports?",
            "HourlyTimeSeries?",
            "TimeBinsReport?",
            "HTMLReport?",
            "SQLReport?",
            "Emissions?",
            "UtilityTariffs?",
            "LifeCycleCosts?",
            "CostEstimates?",
            "Description",
        ):
            out_string += "  <th>" + headerItem + "</th>\n"
        out_string += " </tr>\n"
        return out_string

    def summarize(self):
        if self.floorArea == -1:
            self.floorArea = ""
        if self.numberFloors == -1:
            self.numberFloors = ""
        return (
            ",".join(
                (
                    str(x)
                    for x in (
                        self.filename,
                        self.location,
                        self.numSizingPeriods,
                        self.floorArea,
                        self.numberFloors,
                        self.numberZones,
                        self.internalMass,
                        self.people,
                        self.lights,
                        self.windows,
                        self.detachedShading,
                        self.daylighting,
                        self.compactSchedules,
                        self.zoneAutoSize,
                        self.idealLoads,
                        self.zoneEquipment,
                        self.centralAirHandlingEquipment,
                        self.systemAutoSize,
                        self.districtCooling,
                        self.districtHeating,
                        self.coils,
                        self.pumps,
                        self.boilers,
                        self.chillers,
                        self.towers,
                        self.plantAutosize,
                        self.standardReports,
                        self.hourlyTimeSeries,
                        self.timeBinsReport,
                        self.htmlReport,
                        self.SQLReport,
                        self.environmentEmissions,
                        self.utilityTariffs,
                        self.lifeCycleCosts,
                        self.costEstimates,
                        self.description,
                    )
                )
            )
            + "\n"
        )

    def summarize_html(self):
        out_string = " <tr>\n"
        for value in (
            self.filename,
            self.location,
            self.numSizingPeriods,
            self.floorArea,
            self.numberFloors,
            self.numberZones,
            self.internalMass,
            self.people,
            self.lights,
            self.windows,
            self.detachedShading,
            self.daylighting,
            self.compactSchedules,
            self.zoneAutoSize,
            self.idealLoads,
            self.zoneEquipment,
            self.centralAirHandlingEquipment,
            self.systemAutoSize,
            self.districtCooling,
            self.districtHeating,
            self.coils,
            self.pumps,
            self.boilers,
            self.chillers,
            self.towers,
            self.plantAutosize,
            self.standardReports,
            self.hourlyTimeSeries,
            self.timeBinsReport,
            self.htmlReport,
            self.SQLReport,
            self.environmentEmissions,
            self.utilityTariffs,
            self.lifeCycleCosts,
            self.costEstimates,
            self.description,
        ):
            out_string += "  <td>" + str(value) + "</td>\n"
        out_string += " </tr>\n"
        return out_string
