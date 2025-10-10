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

import argparse
import json
import os
import sys
import uuid

import auditor

#
# Local definitions
#
afn_object_names = [
    "RoomAir:Node:AirflowNetwork:AdjacentSurfaceList",
    "RoomAir:Node:AirflowNetwork:InternalGains",
    "RoomAir:Node:AirflowNetwork:HVACEquipment",
    "AirflowNetwork:SimulationControl",
    "AirflowNetwork:MultiZone:Zone",
    "AirflowNetwork:MultiZone:Surface",
    "AirflowNetwork:MultiZone:ReferenceCrackConditions",
    "AirflowNetwork:MultiZone:Surface:Crack",
    "AirflowNetwork:MultiZone:Surface:EffectiveLeakageArea",
    "AirflowNetwork:MultiZone:Component:DetailedOpening",
    "AirflowNetwork:MultiZone:Component:SimpleOpening",
    "AirflowNetwork:MultiZone:Component:HorizontalOpening",
    "AirflowNetwork:MultiZone:Component:ZoneExhaustFan",
    "AirflowNetwork:MultiZone:ExternalNode",
    "AirflowNetwork:MultiZone:WindPressureCoefficientArray",
    "AirflowNetwork:MultiZone:WindPressureCoefficientValues",
    "AirflowNetwork:ZoneControl:PressureController",
    "AirflowNetwork:Distribution:Node",
    "AirflowNetwork:Distribution:Component:Leak",
    "AirflowNetwork:Distribution:Component:LeakageRatio",
    "AirflowNetwork:Distribution:Component:Duct",
    "AirflowNetwork:Distribution:Component:Fan",
    "AirflowNetwork:Distribution:Component:Coil",
    "AirflowNetwork:Distribution:Component:HeatExchanger",
    "AirflowNetwork:Distribution:Component:TerminalUnit",
    "AirflowNetwork:Distribution:Component:ConstantPressureDrop",
    "AirflowNetwork:Distribution:Component:OutdoorAirFlow",
    "AirflowNetwork:Distribution:Component:ReliefAirFlow",
    "AirflowNetwork:Distribution:Linkage",
    "AirflowNetwork:Distribution:DuctViewFactors",
    "AirflowNetwork:OccupantVentilationControl",
    "AirflowNetwork:IntraZone:Node",
    "AirflowNetwork:IntraZone:Linkage",
]

#
# Names of the multizone components
#
multizone_component_names = [
    "AirflowNetwork:MultiZone:Surface:Crack",
    "AirflowNetwork:MultiZone:Surface:EffectiveLeakageArea",
    "AirflowNetwork:MultiZone:Component:DetailedOpening",
    "AirflowNetwork:MultiZone:Component:SimpleOpening",
    "AirflowNetwork:MultiZone:Component:HorizontalOpening",
    "AirflowNetwork:MultiZone:Component:ZoneExhaustFan",
]


class AFN_Auditor(auditor.Auditor):
    def __init__(self, model):
        super().__init__(model)
        self.nodes = {}
        self.external_nodes = {}
        self.surfs = {}
        self.wpcs = {}
        self.refconds = {}
        self.afes = {}
        self.relative_geometry = False
        self.vertex_ccw = True
        # Figure out what is what
        if "GlobalGeometryRules" in self.model:
            obj = next(iter(self.model["GlobalGeometryRules"].values()))
            if "coordinate_system" in obj:
                if obj["coordinate_system"] == "Relative":
                    self.relative_geometry = True
            if "vertex_entry_direction" == "ClockWise":
                self.vertex_ccw = False
        if self.__extract():
            self.__connect_multizone()

    def __extract(self):
        lookup = {
            "AirflowNetwork:MultiZone:Zone": self.nodes,
            "AirflowNetwork:MultiZone:Surface": self.surfs,
            "AirflowNetwork:MultiZone:ReferenceCrackConditions": self.refconds,
            "AirflowNetwork:MultiZone:Surface:Crack": self.afes,
            "AirflowNetwork:MultiZone:Surface:EffectiveLeakageArea": self.afes,
            "AirflowNetwork:MultiZone:Component:DetailedOpening": self.afes,
            "AirflowNetwork:MultiZone:Component:SimpleOpening": self.afes,
            "AirflowNetwork:MultiZone:Component:HorizontalOpening": self.afes,
            "AirflowNetwork:MultiZone:Component:ZoneExhaustFan": self.afes,
            "AirflowNetwork:MultiZone:ExternalNode": self.external_nodes,
            #'AirflowNetwork:Distribution:Node':self.nodes,
            #'AirflowNetwork:IntraZone:Node':self.nodes
        }
        # Load the simcontrol object
        try:
            self.simcontrol = self.model["AirflowNetwork:SimulationControl"]
        except KeyError:
            self.json["messages"] = ["Model does not contain a AirflowNetwork:SimulationControl object, aborting audit"]
            return False
        # Handle the wind pressure coefficients, should maybe remove these from the model once we're done
        try:
            wpa = next(iter(self.model["AirflowNetwork:MultiZone:WindPressureCoefficientArray"].values()))
            keys = [el for el in wpa.keys() if el.startswith("wind_dir")]
            keys.sort()
            directions = []
            for key in keys:
                directions.append(wpa[key])

            for k, v in self.model["AirflowNetwork:MultiZone:WindPressureCoefficientValues"].items():
                keys = [el for el in v.keys() if el.startswith("wind_pres")]
                keys.sort()
                coeffs = []
                for key in keys:
                    coeffs.append(v[key])
                self.wpcs[k] = {"wind_directions": directions, "wind_pressure_coefficient_values": coeffs}
        except KeyError:
            self.wpcs = {}

        # Pull out the airflow network objects
        for key in self.model.keys():
            if key in lookup:
                thedict = lookup[key]
                for k, v in self.model[key].items():
                    thedict[k] = v
        return True

    def write_dot(self, fp):
        if self.nodes == []:
            # Have to have internal nodes
            return
        if self.surfs == []:
            # Have to have connections
            return
        #
        # Generate a graph
        #
        # Give nodes names for displaying
        count = 0
        for name, node in self.external_nodes.items():
            node["display_name"] = "E%d" % count
            count += 1

        count = 0
        for name, node in self.nodes.items():
            node["display_name"] = "I%d" % count
            count += 1

        fp.write("graph linkages {\n")
        for name, surf in self.surfs.items():
            fp.write("%s -- %s\n" % (surf.nodes[0]["display_name"], surf.nodes[1]["display_name"]))
        fp.write("}\n")

    def __compute_azimuths(self):
        if self.relative_geometry:
            for surf in self.surfs:
                htsurf = self.model["BuildingSurface:Detailed"][surf["building_surface_name"]]

    def __connect_multizone(self):
        # Link surfaces to nodes, need to automate this better at some point
        for name, node in self.nodes.items():
            node["link_count"] = 0
            node["external_connections"] = 0
            node["neighbors"] = {}

        for name, node in self.external_nodes.items():
            node["link_count"] = 0
            node["neighbors"] = {}

        heat_transfer_surface_names = ["BuildingSurface:Detailed", "FenestrationSurface:Detailed"]

        htsurfs = {}
        for name in heat_transfer_surface_names:
            if name in self.model:
                htsurfs.update(self.model[name])

        outdoor_count = 0

        for name, surf in self.surfs.items():
            window = None
            try:
                htsurf = htsurfs[surf["surface_name"]]
            except KeyError:
                raise auditor.BadModel('Failed to find heat transfer surface for AirflowNetwork surface "' + name + '"')
            if "building_surface_name" in htsurf:
                window = htsurf
                try:
                    htsurf = htsurfs[window["building_surface_name"]]
                except KeyError:
                    raise auditor.BadModel(
                        'Failed to find window heat transfer surface for AirflowNetwork surface "' + name + '"'
                    )

            bc = htsurf["outside_boundary_condition"]

            linked_nodes = []
            if bc == "Outdoors":
                outdoor_count += 1
                try:
                    external_node_name = surf["external_node_name"]
                except KeyError:
                    # This is probably a model using precomputed WPCs, should check that
                    external_node_name = uuid.uuid4().hex[:6].upper()
                    self.external_nodes[external_node_name] = {"link_count": 0, "zone_name": None, "neighbors": {}}
                    surf["external_node_name"] = external_node_name
                try:
                    external_node = self.external_nodes[external_node_name]
                except KeyError:
                    raise auditor.BadModel(
                        'Failed to find external node "'
                        + external_node_name
                        + '" for AirflowNetwork surface "'
                        + name
                        + '"'
                    )
                external_node["link_count"] += 1
                zone_name = htsurf["zone_name"]
                # Find the multizone zone that points at this zone
                afnzone = None
                for name, node in self.nodes.items():
                    if node["zone_name"] == zone_name:
                        afnzone = node
                        node["link_count"] += 1
                        node["external_connections"] += 1
                        if surf["external_node_name"] in node["neighbors"]:
                            node["neighbors"][surf["external_node_name"]] += 1
                        else:
                            node["neighbors"][surf["external_node_name"]] = 1
                        if zone_name in external_node["neighbors"]:
                            external_node["neighbors"][zone_name] += 1
                        else:
                            external_node["neighbors"][zone_name] = 1
                        break
                if afnzone == None:
                    raise auditor.BadModel('Failed to find AirflowNetwork zone for thermal zone "' + zone_name + '"')
                linked_nodes = [afnzone, external_node]
            elif bc == "Surface":
                zone_name = htsurf["zone_name"]
                # Find the multizone zone that points at this zone
                afnzone = None
                for name, node in self.nodes.items():
                    if node["zone_name"] == zone_name:
                        afnzone = node
                        node["link_count"] += 1
                        break
                if afnzone == None:
                    raise auditor.BadModel('Failed to find AirflowNetwork zone for thermal zone "' + zone_name + '"')
                linked_nodes = [afnzone]
                adjhtsurf = htsurfs[htsurf["outside_boundary_condition_object"]]
                adj_zone_name = adjhtsurf["zone_name"]
                adj_afnzone = None
                for name, node in self.nodes.items():
                    if node["zone_name"] == adj_zone_name:
                        adj_afnzone = node
                        node["link_count"] += 1
                        break
                if adj_afnzone == None:
                    raise auditor.BadModel(
                        'Failed to find AirflowNetwork zone for adjacent thermal zone "' + adj_zone_name + '"'
                    )
                linked_nodes.append(adj_afnzone)
                if adj_zone_name in afnzone["neighbors"]:
                    afnzone["neighbors"][adj_zone_name] += 1
                else:
                    afnzone["neighbors"][adj_zone_name] = 1
                if zone_name in adj_afnzone["neighbors"]:
                    adj_afnzone["neighbors"][zone_name] += 1
                else:
                    adj_afnzone["neighbors"][zone_name] = 1

            surf["nodes"] = linked_nodes
        return True

    def audit(self, **kwargs):
        if self.nodes == {} or self.external_nodes == {} or self.surfs == {}:
            # This is not a super great way to get this done, should reconsider
            self.__extract()
            self.__connect_multizone()

        # for name, surf in netcomps.data['AirflowNetwork:MultiZone:Surface'].items():
        #    if len(surf['nodes']) != 2:
        #        raise Exception('Failed to define all surface linkages')

        #
        # Now we've got the links worked out, so proceed to looking at what was there
        #
        link_histogram = {}
        external_link_histogram = {}
        max_link_node_name = None
        max_links = 0
        max_external_link_node_name = None
        max_external_links = 0
        for name, node in self.nodes.items():
            if node["link_count"] > max_links:
                max_link_node_name = name
                max_links = node["link_count"]
            if node["link_count"] in link_histogram:
                link_histogram[node["link_count"]] += 1
            else:
                link_histogram[node["link_count"]] = 1
            if node["external_connections"] > max_external_links:
                max_external_link_node_name = name
                max_external_links = node["external_connections"]
            if node["external_connections"] in external_link_histogram:
                external_link_histogram[node["external_connections"]] += 1
            else:
                external_link_histogram[node["external_connections"]] = 1

        # print(max_link_node_name, max_external_link_node_name)
        # print(len(self.nodes))

        #
        # For a simple brick zone, 6 multizone links would connect it to all neighbors.
        # In real models, it's unlikely that 6 is a good number to test against, so let's
        # hardcode this as roughly quadruple that, or 25
        #
        large_links = 0
        too_many_links = 0
        way_too_many_links = 0
        for k, v in link_histogram.items():
            if k >= 25:
                large_links += v
            if k >= 50:
                too_many_links += v
            if k >= 100:
                way_too_many_links += v
        # print(large_links, too_many_links, way_too_many_links)

        # Do the same thing for external connections, but using 2 as the ideal, quadruple that
        # would be ~8
        large_external_links = 0
        too_many_external_links = 0
        way_too_many_external_links = 0
        for k, v in external_link_histogram.items():
            if k >= 8:
                large_external_links += v
            if k >= 16:
                too_many_external_links += v
            if k >= 32:
                way_too_many_external_links += v
        # print(large_external_links, too_many_external_links, way_too_many_external_links)

        #
        # Machine-readable output
        #
        self.json["multizone_link_histogram"] = link_histogram
        self.json["max_multizone_links"] = {
            "zone": self.nodes[max_link_node_name]["zone_name"],
            "afn_zone": max_link_node_name,
            "count": max_links,
        }
        self.json["external_link_histogram"] = external_link_histogram
        self.json["max_external_links"] = {
            "zone": self.nodes[max_external_link_node_name]["zone_name"],
            "afn_zone": max_external_link_node_name,
            "count": max_external_links,
        }
        self.json["messages"] = []
        if large_links > 0:
            mesg = "%d zone(s) with greater than 25 links" % large_links
            if too_many_links > 0:
                mesg += ", %d with greater than 50 links" % too_many_links
                if way_too_many_links > 0:
                    mesg += ", %d with greater than 100 links" % way_too_many_links
                    mesg += ", model performance may suffer"
            self.json["messages"].append(mesg)
        if large_external_links > 0:
            mesg = "%d zone(s) with greater than 8 external links" % large_external_links
            if too_many_external_links > 0:
                mesg += ", %d with greater than 16 external links" % too_many_external_links
                if way_too_many_external_links > 0:
                    mesg += ", %d with greater than 32 external links" % way_too_many_external_links
                    mesg += ", model performance may suffer"
            self.json["messages"].append(mesg)


if __name__ == "__main__":
    #
    # The main body of the script, do argument processing first
    #
    parser = argparse.ArgumentParser(description="AirflowNetwork model audit script")
    # args.add_argument('-g', '--graph', help='Generate a graphviz graph',
    #                  default=False, action='store_true')
    parser.add_argument(
        "-g",
        "--graph",
        help="generate a graphviz .dot output file",
        dest="graph",
        metavar="dotfile",
        default=None,
        type=argparse.FileType("w"),
    )
    parser.add_argument("-p", "--pretty", help="write pretty JSON output", default=False, action="store_true")
    parser.add_argument("json_file")

    args = parser.parse_args()

    fp = open(args.json_file, "r")
    model = json.load(fp)
    fp.close()

    auditor = AFN_Auditor(model)

    auditor.audit()

    #
    # Now write it all out
    #
    indent = None
    if args.pretty:
        indent = 2

    json.dump(auditor.json, sys.stdout, indent=indent)

    if args.graph:
        #
        # Generate a graph
        #
        # Give nodes names for displaying
        count = 0
        for name, node in external_nodes.items():
            node["display_name"] = "E%d" % count
            count += 1

        count = 0
        for name, node in nodes.items():
            node["display_name"] = "I%d" % count
            count += 1

        args.graph.write("graph linkages {\n")
        for name, surf in surfs.items():
            args.graph.write("%s -- %s\n" % (surf.nodes[0]["display_name"], surf.nodes[1]["display_name"]))
        args.graph.write("}\n")
        args.graph.close()
