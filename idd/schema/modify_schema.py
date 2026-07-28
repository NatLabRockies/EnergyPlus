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


def anyOf():
    return [
        {
            "type": "number",
        },
        {"type": "string"},
    ]


def isInt(s):
    try:
        int(s)
        return True
    except ValueError:
        return False


extension_renaming = {
    "LifeCycleCost:UseAdjustment": "multipliers",
    "LifeCycleCost:UsePriceEscalation": "escalations",
    "ElectricLoadCenter:Transformer": "meters",
    "ElectricLoadCenter:Generators": "generator_outputs",
    "Generator:FuelCell:AirSupply": "constituent_fractions",
    "DemandManager:ExteriorLights": "lights",
    "DemandManager:Ventilation": "controllers",
    "DemandManagerAssignmentList": "manager_data",
    "DemandManager:Lights": "lights",
    "DemandManager:Thermostats": "thermostats",
    "DemandManager:ElectricEquipment": "equipment",
    "DaylightingDevice:Tubular": "transition_lengths",
    "Daylighting:Controls": "control_data",
    "ZoneHVAC:Baseboard:RadiantConvective:Steam": "surface_fractions",
    "ZoneHVAC:Baseboard:RadiantConvective:Electric": "surface_fractions",
    "ZoneHVAC:HighTemperatureRadiant": "surface_fractions",
    "ZoneHVAC:LowTemperatureRadiant:SurfaceGroup": "surface_fractions",
    "ZoneHVAC:Baseboard:RadiantConvective:Water": "surface_fractions",
    "ZoneHVAC:VentilatedSlab:SlabGroup": "data",
    "ZoneHVAC:CoolingPanel:RadiantConvective:Water": "surface_fractions",
    "AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl:HR": "loading_indices",
    "AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl": "loading_indices",
    "ZoneTerminalUnitList": "terminal_units",
    "RoomAir:TemperaturePattern:NondimensionalHeight": "pairs",
    "RoomAir:Node:AirflowNetwork:InternalGains": "gains",
    "RoomAirSettings:AirflowNetwork": "nodes",
    "RoomAir:Node:AirflowNetwork:HVACEquipment": "equipment_fractions",
    "RoomAir:TemperaturePattern:SurfaceMapping": "surface_deltas",
    "RoomAir:Node:AirflowNetwork:AdjacentSurfaceList": "surfaces",
    "Controller:MechanicalVentilation": "zone_specifications",
    "WaterUse:Connections": "connections",
    "WaterUse:RainCollector": "surfaces",
    "Output:Table:Annual": "variable_details",
    "Meter:CustomDecrement": "variable_details",
    "Output:Table:Monthly": "variable_details",
    "Output:Table:SummaryReports": "reports",
    "Meter:Custom": "variable_details",
    "UnitarySystemPerformance:Multispeed": "flow_ratios",
    "SurfaceProperty:ExteriorNaturalVentedCavity": "surface",
    "ZoneProperty:UserViewFactors:BySurfaceName": "view_factors",
    "SurfaceProperty:HeatTransferAlgorithm:SurfaceList": "surface",
    "AirLoopHVAC:ZoneSplitter": "nodes",
    "AirLoopHVAC:SupplyPath": "components",
    "AirLoopHVAC:ReturnPath": "components",
    "AirLoopHVAC:ReturnPlenum": "nodes",
    "AirLoopHVAC:ZoneMixer": "nodes",
    "AirLoopHVAC:SupplyPlenum": "nodes",
    "BuildingSurface:Detailed": "vertices",
    "Shading:Zone:Detailed": "vertices",
    "RoofCeiling:Detailed": "vertices",
    "Shading:Site:Detailed": "vertices",
    "Wall:Detailed": "vertices",
    "Space": "tags",
    "SpaceList": "spaces",
    "DesignSpecification:OutdoorAir:SpaceList": "space_specs",
    "ZoneList": "zones",
    "Floor:Detailed": "vertices",
    "Shading:Building:Detailed": "vertices",
    "SolarCollector:UnglazedTranspired:Multisystem": "systems",
    "SolarCollector:UnglazedTranspired": "surfaces",
    "Parametric:SetValueForRun": "values",
    "Parametric:Logic": "lines",
    "Parametric:FileNameSuffix": "suffixes",
    "Parametric:RunControl": "runs",
    "ZoneHVAC:EquipmentList": "equipment",
    "SpaceHVAC:ZoneEquipmentSplitter": "spaces",
    "SpaceHVAC:ZoneEquipmentMixer": "spaces",
    "SpaceHVAC:ZoneReturnMixer": "spaces",
    "AvailabilityManagerAssignmentList": "managers",
    "Table:IndependentVariable": "values",
    "Table:IndependentVariableList": "independent_variables",
    "Table:Lookup": "values",
    "Matrix:TwoDimension": "values",
    "WindowMaterial:GlazingGroup:Thermochromic": "temperature_data",
    "Schedule:Compact": "data",
    "Schedule:Day:Interval": "data",
    "Schedule:Week:Compact": "data",
    "EnergyManagementSystem:GlobalVariable": "variables",
    "EnergyManagementSystem:ProgramCallingManager": "programs",
    "EnergyManagementSystem:Program": "lines",
    "EnergyManagementSystem:Subroutine": "lines",
    "Refrigeration:CaseAndWalkInList": "cases_and_walkins",
    "Refrigeration:CompressorList": "compressors",
    "ZoneHVAC:RefrigerationChillerSet": "chillers",
    "Refrigeration:WalkIn": "zone_data",
    "Refrigeration:TransferLoadList": "transfer_loads",
    "Branch": "components",
    "PipingSystem:Underground:Domain": "pipe_circuits",
    "Connector:Splitter": "branches",
    "Connector:Mixer": "branches",
    "BranchList": "branches",
    "PipingSystem:Underground:PipeCircuit": "pipe_segments",
    "NodeList": "nodes",
    "OutdoorAir:NodeList": "nodes",
    "Fan:SystemModel": "speed_fractions",
    "AirflowNetwork:Distribution:DuctViewFactors": "surfaces",
    "GroundHeatExchanger:System": "vertical_well_locations",
    "GroundHeatExchanger:ResponseFactors": "g_functions",
    "Foundation:Kiva": "blocks",
    "SurfaceProperty:ExposedFoundationPerimeter": "surfaces",
    "SurfaceProperty:SurroundingSurfaces": "surfaces",
    "SurfaceProperty:GroundSurfaces": "ground_surfaces",
    "ZoneHVAC:HybridUnitaryHVAC": "modes",
    "ShadowCalculation": "shading_zone_groups",
    "Schedule:Year": "schedule_weeks",
    "WindowShadingControl": "fenestration_surfaces",
    "PlantEquipmentList": "equipment",
    "CondenserEquipmentList": "equipment",
    "AirLoopHVAC:Mixer": "nodes",
    "AirLoopHVAC:Splitter": "nodes",
    "AirLoopHVAC:DedicatedOutdoorAirSystem": "airloophvacs",
    "PythonPlugin:Variables": "global_py_vars",
    "PythonPlugin:SearchPaths": "py_search_paths",
    "Output:Diagnostics": "diagnostics",
    "MaterialProperty:PhaseChange": "values",
    "MaterialProperty:VariableThermalConductivity": "values",
    "ComfortViewFactorAngles": "angles",
    "ZoneMRTCalculation": "people_names",
}
remaining_objects = [
    "Site:SpectrumData",
    "Schedule:Day:List",
    "MaterialProperty:GlazingSpectralData",
]


def get_schema_object(schema, object_key):
    if ".*" in schema["properties"][object_key]["patternProperties"]:
        return schema["properties"][object_key]["patternProperties"][".*"]
    if R"^.*\S.*$" in schema["properties"][object_key]["patternProperties"]:
        return schema["properties"][object_key]["patternProperties"][R"^.*\S.*$"]
    raise KeyError(R'The patternProperties value is not a valid choice (".*", "^.*\S.*$")')


def change_schedule_compact(schema):
    loc = get_schema_object(schema, "Schedule:Compact")["properties"]["extensions"]["items"]["properties"]["field"]
    loc.pop("type")
    loc["anyOf"] = anyOf()


def change_special_cased_enums(schema):
    loc = get_schema_object(schema, "GroundHeatTransfer:Slab:Insulation")["properties"][
        "ivins_flag_is_there_vertical_insulation"
    ]
    loc.pop("type")
    newAnyOf = anyOf()
    newAnyOf[0]["enum"] = [int(i) for i in loc["enum"][:] if isInt(i)]
    newAnyOf[1]["enum"] = [""]
    loc["anyOf"] = newAnyOf
    loc.pop("enum")

    loc = get_schema_object(schema, "WindowMaterial:Screen")["properties"][
        "angle_of_resolution_for_screen_transmittance_output_map"
    ]
    loc.pop("type")
    newAnyOf = anyOf()
    newAnyOf[0]["enum"] = [int(i) for i in loc["enum"][:] if isInt(i)]
    newAnyOf[1]["enum"] = [""]
    loc["anyOf"] = newAnyOf
    loc.pop("enum")

    loc = get_schema_object(schema, "Refrigeration:System")["properties"]["number_of_compressor_stages"]
    loc.pop("type")
    newAnyOf = anyOf()
    newAnyOf[0]["enum"] = [int(i) for i in loc["enum"][:] if isInt(i)]
    newAnyOf[1]["enum"] = [""]
    loc["anyOf"] = newAnyOf
    loc.pop("enum")

    loc = get_schema_object(schema, "ElectricLoadCenter:Transformer")["properties"]["phase"]
    loc.pop("type")
    newAnyOf = anyOf()
    newAnyOf[0]["enum"] = [int(i) for i in loc["enum"][:] if isInt(i)]
    newAnyOf[1]["enum"] = [""]
    loc["anyOf"] = newAnyOf
    loc.pop("enum")

    loc = get_schema_object(schema, "Zone")["properties"]["zone_outside_convection_algorithm"]["enum"]
    loc.insert(0, "")

    loc = get_schema_object(schema, "Zone")["properties"]["zone_inside_convection_algorithm"]["enum"]
    loc.insert(0, "")


def change_utility_cost(schema):
    legacy_idd = schema["properties"]["UtilityCost:Charge:Block"]["legacy_idd"]["fields"]
    loc = get_schema_object(schema, "UtilityCost:Charge:Block")["properties"]
    for i in range(6, len(legacy_idd)):
        field = legacy_idd[i]
        loc[field].pop("type")
        loc[field]["anyOf"] = anyOf()

    loc = get_schema_object(schema, "UtilityCost:Ratchet")["properties"]
    loc["offset_value_or_variable_name"].pop("type")
    loc["offset_value_or_variable_name"]["anyOf"] = anyOf()
    loc["multiplier_value_or_variable_name"].pop("type")
    loc["multiplier_value_or_variable_name"]["anyOf"] = anyOf()

    loc = get_schema_object(schema, "UtilityCost:Charge:Simple")["properties"]
    loc["cost_per_unit_value_or_variable_name"].pop("type")
    loc["cost_per_unit_value_or_variable_name"]["anyOf"] = anyOf()

    loc = get_schema_object(schema, "UtilityCost:Qualify")["properties"]
    loc["threshold_value_or_variable_name"].pop("type")
    loc["threshold_value_or_variable_name"]["anyOf"] = anyOf()

    loc = get_schema_object(schema, "UtilityCost:Tariff")["properties"]
    loc["minimum_monthly_charge_or_variable_name"].pop("type")
    loc["minimum_monthly_charge_or_variable_name"]["anyOf"] = anyOf()
    loc["monthly_charge_or_variable_name"].pop("type")
    loc["monthly_charge_or_variable_name"]["anyOf"] = anyOf()


def add_explicit_extensible_bounds(schema):
    # Schedule:Year
    loc = get_schema_object(schema, "Schedule:Year")["properties"]["schedule_weeks"]
    loc["minItems"] = 1
    loc["maxItems"] = 53

    # EnergyManagementSystem:Program
    loc = get_schema_object(schema, "EnergyManagementSystem:Program")
    if "required" in loc and "lines" not in loc["required"]:
        loc["required"].append("lines")
    if "required" not in loc:
        loc["required"] = ["lines"]
    loc["properties"]["lines"]["minItems"] = 1


def change_special_cased_name_fields(schema):
    original_name = schema["properties"]["ZoneHVAC:TerminalUnit:VariableRefrigerantFlow"]["legacy_idd"][
        "field_info"
    ].pop("zone_terminal_unit_name")
    schema["properties"]["ZoneHVAC:TerminalUnit:VariableRefrigerantFlow"]["legacy_idd"]["field_info"][
        "name"
    ] = original_name
    schema["properties"]["ZoneHVAC:TerminalUnit:VariableRefrigerantFlow"]["legacy_idd"]["fields"][0] = "name"
    schema["properties"]["ZoneHVAC:TerminalUnit:VariableRefrigerantFlow"]["legacy_idd"]["alphas"]["fields"][0] = "name"
    del get_schema_object(schema, "ZoneHVAC:TerminalUnit:VariableRefrigerantFlow")["required"][0]
    schema["properties"]["ZoneHVAC:TerminalUnit:VariableRefrigerantFlow"]["name"] = get_schema_object(
        schema, "ZoneHVAC:TerminalUnit:VariableRefrigerantFlow"
    )["properties"].pop("zone_terminal_unit_name")

    original_name = schema["properties"]["AirConditioner:VariableRefrigerantFlow"]["legacy_idd"]["field_info"].pop(
        "heat_pump_name"
    )
    schema["properties"]["AirConditioner:VariableRefrigerantFlow"]["legacy_idd"]["field_info"]["name"] = original_name
    schema["properties"]["AirConditioner:VariableRefrigerantFlow"]["legacy_idd"]["fields"][0] = "name"
    schema["properties"]["AirConditioner:VariableRefrigerantFlow"]["legacy_idd"]["alphas"]["fields"][0] = "name"
    del get_schema_object(schema, "AirConditioner:VariableRefrigerantFlow")["required"][0]
    schema["properties"]["AirConditioner:VariableRefrigerantFlow"]["name"] = get_schema_object(
        schema, "AirConditioner:VariableRefrigerantFlow"
    )["properties"].pop("heat_pump_name")


def change_extensions_name(schema):
    for key, value in extension_renaming.items():
        get_schema_object(schema, key)["properties"][value] = get_schema_object(schema, key)["properties"]["extensions"]
        loc = get_schema_object(schema, key)["properties"]
        del loc["extensions"]
        schema["properties"][key]["legacy_idd"]["extension"] = value

    for key in remaining_objects:
        schema["properties"][key]["legacy_idd"]["extension"] = "extensions"


def change_89_release_issues(schema):
    curves = [
        "Curve:Linear",
        "Curve:Quadratic",
        "Curve:Cubic",
        "Curve:Quartic",
        "Curve:Exponent",
        "Curve:Bicubic",
        "Curve:Biquadratic",
        "Curve:QuadraticLinear",
        "Curve:CubicLinear",
        "Curve:Triquadratic",
        "Curve:ExponentialSkewNormal",
        "Curve:Sigmoid",
        "Curve:RectangularHyperbola1",
        "Curve:RectangularHyperbola2",
        "Curve:ExponentialDecay",
        "Curve:DoubleExponentialDecay",
        "Curve:ChillerPartLoadWithLift",
        "Table:Lookup",
    ]
    for curve in curves:
        get_schema_object(schema, curve)["properties"]["output_unit_type"]["enum"] = [
            "",
            "Capacity",
            "Dimensionless",
            "Power",
            "Pressure",
            "Temperature",
        ]

    get_schema_object(schema, "Schedule:Week:Compact")["properties"]["data"]["items"]["properties"]["daytype_list"].pop(
        "enum"
    )
