***********************
Code Structure Overview
***********************

------------------------------
Overview
------------------------------

The python code provides a framework to support the creation of EnergyPlus objects.  The instructions for these actions are stored in a YAML file which is structured to eliminate as much redundancy as possible, and provide a clear outline that explains how template options affect the final output.

------------------------------
Process Flow
------------------------------

Outlined below are the general steps that the program takes to perform its operation.

1. A schema file is read and validated using the jsonschema package `Versioned Validator`_ class.  In this case, the Draft4Validator is used.  If the `--no-schema` option has been passed to the program, then no validation will occur at any point in the process.
2. The epJSON file is read, validated, and loaded as a dictionary object.
3. All HVACTemplate objects are loaded into HVACTemplate classes for processing.
4. The template objects are processed to create regular objects for the specific system, zone, plant, or loop.

  i. An `OptionTree` is loaded from the yaml file, which specifies build instructions for a given template.
  ii. EnergyPlus objects are created according to the `BuildPath` provided within the `OptionTree`.  A `BuildPath` is an ordered list of objects that follow an air loop path.  These `BuildPaths` are constructed based on HVACTemplate field inputs.
  iii. Additional objects outside of the build path, but specific to the template, are created from instructions provided in a `TemplateObjects` branch.  These are objects such as Controllers, NodeLists, SetpointManagers, etc.
  iv. Objects that connect equipment to each other are created (e.g. Zone:Splitters/Mixers, SupplyPath, Connectors, BranchLists, etc.) through automated functions.

5. The epJSON objects created within each HVACTemplate class are merged together into a single document.
8. The epJSON file is validated against the schema.
9. A dictionary output is provided to the downstream process.  At a minimum, this object will have a key called 'outputPreProcessMessage' which will contain all warnings and errors generated from this process.  On a successful template expansion, a valid epJSON object will also be produced with the file name \<original-file-name\>_expanded.epJSON
10. If the `--backup-files` option has been specified, the optional "\<original-file-name\>_hvac_templates.epJSON" and "\<original-file-name\>_base.epJSON" files will be output.

.. _Versioned Validator: https://python-jsonschema.readthedocs.io/en/stable/validate/#versioned-validators

------------------------------
Yaml Description
------------------------------

This program makes use of a YAML file to hold structured objects as well as instructions for alternative object creation based on template inputs. This `Template Expansion Structure`_ YAML file makes use of `anchors and aliases`_ to reduce the amount of redundant code, ensure object consistency between definitions, and provide a more reliable framework for future development and maintenance.  With these tools, a structure has been created to define EnergyPlus objects and reference node locations without the need to statically type text values for every case.  Values can be directly inserted, or references to template inputs can be made.  All references made are scoped within the HVACTemplate object, and references to other HVACTemplate objects cannot be made.

.. _Template Expansion Structure: https://github.com/john-grando/pyExpandObjects/blob/main/src/resources/template_expansion_structuire.yaml

.. _anchors and aliases: https://support.atlassian.com/bitbucket-cloud/docs/yaml-anchors

**Object Structure**

To hold the necessary information for construction, the Yaml objects which represent EnergyPlus objects in a `BuildPath` have additional higher-level information, and are termed 'super' objects.  These standard format for these objects is as follows:

.. code-block:: yaml

  EnergyPlusObject:
    Fields:
      name: object_name
      field_name: field_value
    Connectors:
      ConnectorPathType:
        Inlet: inlet_node_name
        Outlet: outlet_node_name

* EnergyPlusObject: This field is a valid EnergyPlus object type.
* Fields: This is hierarchical container which holds valid EnergyPlus object field names.  Additionally, a `name` field and value must be provided to give the object a unique identifier for that object type.
* field_name: The exact EnergyPlus field name must be provided.
* field_value: A `complex reference` may be provided.  Please see below for further details
* Connectors: This is a hierarchical container which holds instructions on how an object connects to it's neighbors in a `BuildPath`.
* ConnectorPathType: Must be a selection of AirLoop (only valid option at this time)
* inlet_node_name, outlet_node_name: Must be a field_name provided from Fields

**BuildPath Objects**

This is a list of EnergyPlus 'super' objects which are in order of their fluid flow path.  Their field names mirror those of EnergyPlus objects while also holding extra information necessary to connect their input/output nodes in an overall path.

    * Fields - key/value pairs of EnergyPlus object fields and values.

        * Key: EnergyPlus field name
        * Value: Value to be inserted as default text.  References to template inputs can be made through string formatting, which is the use of brackets '{template_input}'.  A blank set of brackets '{}' will insert the template's unique name identifier.

    * Connectors - object for identifying how the fluid flow loop passes through the object.  This is used to connect nodes between objects in a `BuildPath`.

        * Loop Type [AirLoop]

            * Inlet - EnergyPlus inlet node name for the loop type.
            * Outlet - EnergyPlus outlet node name for the loop type.

Example:

.. code-block:: yaml

  OutdoorAir:Mixer:
    Fields:
      name: '{} OA Mixing Box'
      mixed_air_node_name: '{} Mixed Air Outlet'
      outdoor_air_stream_node_name: '{} Outside Air Inlet'
      relief_air_stream_node_name: '{} Relief Air Outlet'
      return_air_stream_node_name: '{} Return Air Loop Inlet'
    Connectors:
      Air:
        Inlet: outdoor_air_stream_node_name
        Outlet: mixed_air_node_name

Note, some build path objects are created with null connectors (i.e. `{ Connectors: { Air: None }}`).  These objects are created this way such that they can be specifically manipulated in `expand_objects._modify_build_path_for_equipment()`.  The most common reason for this modification is that a build path exists, but it is not directly reflected in the AirLoop system BranchList.

**OptionTree Structure**

.. code-block:: yaml

  OptionTree:
    HVACTemplate:
      Zone: Zone template instructions
      System: System template instructions
      Plant: Plant template instructions

**OptionTree Template Structure**

This section provides a set of instructions for the expansion process.

  * BuildPath - Ordered list of objects to create along the fluid flow path.

    * BaseObjects - `BuildPath` Objects that are inserted in all cases
    * Actions - List of actions that are conditionally applied base on template values

      * ObjectReference - Reference to an existing object in the `BuildPath`
      * Occurrence - The number of times to match the ObjectReference in the `BuildPath` before applying the action.
      * Location - Instruction on where to insert the new object(s) in reference to the ObjectReference.  'Before' or 'After' will insert the object to either side.  An integer input will specify the global `BuildPath` location.
      * ActionType - Instruction whether to insert a new object, remove the existing object, or replace the existing object.

  * BaseObjects - Objects that are created in all cases
  * TemplateObjects - Objects that are conditionally created based on template values

.. code-block:: yaml

  OptionTree:
    HVACTemplate:
      System:
        BuildPath:
          BaseObjects:
          Actions:
            ObjectReference:
            Location:
            Objects:
        BaseObjects:
        TemplateObjects:

**OptionTree Object Structure**

Once a set of instructions has been selected, the specific information to create the object(s) is provided.

  * Objects - 'Super' objects to be inserted in the `BuildPath` or regular objects to be inserted into the epJSON document.
  * Transitions - Transfer of template inputs to object values (e.g. supply_fan_total_efficiency -> Fan Object -> Field [fan efficiency])
  * Mappings - Mappings of template field values that trigger one or more values to be updated in other template fields.  For example, the selection of 'InletVaneDampers' for 'supply_fan_part_load_power_coefficients' creates multiple field name/value pairs to express the fan PLR curve.

* Transitions

    A field value in an object can be created, or overridden.  This set of instructions transfers the value provided in an HVACTemplate field to an EnergyPlus object field.

  .. code-block:: yaml

    - Object Type Reference (can be a regular expression):
        hvac_template_field: object_field

    - Fan:.*:
        supply_fan_delta_pressure: design_pressure_rise

* Transitions With String Reformatting

    A string reformat may be specified to mutate the input value.  For example, if the template value provided for `chilled_water_design_setpoint` is 12.8, then The following code will yield a string value in the schedule_name field of 'HVACTemplate-Always12.8'.  When a string is formatted to 'HVACTemplate-Always[numeric]', a Schedule:Compact object is automateically created.

  .. code-block:: yaml

    Fields: &SetpointManagerScheduledChilledWater
      <<: *SetpointManagerScheduled
      schedule_name: 'HVACTemplate-Always{chilled_water_design_setpoint}'

* Transition Using Different Template Field

    In addition to string reformatting, a separate template field may be updated by specifying the value as another dictionary object.  In this example, if a value is given for dehumidification_setpoint then dehumidifying_relative_humidity_setpoint_schedule_name is updated.

  .. code-block:: yaml

    - ZoneControl:Humidistat:
        dehumidification_setpoint:
          dehumidifying_relative_humidity_setpoint_schedule_name: 'HVACTemplate-Always{dehumidification_setpoint}'

* Transition Including Numerical Operations

    Numerical and other mathematical operations that can performed using the eval() function in Python can be used.  In this example, a maximum value is returned between a static number and a template field.

  .. code-block:: yaml

    - SetpointManager:Warmest:
        maximum_setpoint_temperature: 'max(18, {cooling_coil_design_setpoint}+5.2)'

* Mappings

    A set of field objects can be created or overridden base on one template input.  The mapped values can be statically typed.  In this example 'None' means that no tempalte input was provided.

  .. code-block:: yaml

    - Object Type Reference (can be a regular expression):
        hvac_template_field:
          hvac_template_value:
            object_field: object_value

    - Fan:.*:
        supply_fan_part_load_power_coefficients:
          None:
            fan_power_coefficient_1: 0.0015302446
            fan_power_coefficient_2: 0.0052080574
            fan_power_coefficient_3: 1.1086242
            fan_power_coefficient_4: -0.11635563
            fan_power_coefficient_5: 0
            fan_power_minimum_flow_rate_input_method: Fraction
            fan_power_minimum_flow_fraction: 0

Full Structure Example:

.. code-block:: yaml

  OptionTree:
    HVACTemplate:
      System:
        DedicatedOutdoorAir:
          BuildPath:
            BaseObjects:
            Actions:
              - ...
              - supply_fan_placement:
                  None: &SystemCommonBuildPathActionsSupplyFanPlacementNone
                    Location: -1
                    ActionType: Insert
                    Objects:
                      - Fan:VariableVolume:
                          Fields:
                            << : *FanFields
                            maximum_flow_rate: Autosize
                         Connectors: *FanConnectors
          pressure_rise: 1000
                    Transitions: *FanCommonTransitionsSupply
                    Mappings: *FanCommonMappingsDedicatedOutdoorAir
                  DrawThrough: *SystemCommonBuildPathActionsSupplyFanPlacementNone
                  BlowThrough:
                    ObjectReference: OutdoorAir:Mixer
                    ActionType: Insert
                    Location: After
                    Objects:
                      - Fan:VariableVolume: *FanVariableVolumeSuperObject
                    Transitions: *FanCommonTransitionsSupply
                    Mappings: *FanCommonMappingsDedicatedOutdoorAir
        BaseObjects:
          Objects:
            - ...
            - Sizing:System:
                ...
                100_outdoor_air_in_cooling: 'Yes'
                100_outdoor_air_in_heating: 'Yes'
                central_cooling_design_supply_air_humidity_ratio: 0.00924
                central_heating_design_supply_air_humidity_ratio: 0.003
                type_of_load_to_size_on: VentilationRequirement
                preheat_design_temperature: 2
          Transitions:
            - Sizing:System:
                ...
                cooling_coil_design_setpoint_temperature: central_cooling_design_supply_air_temperature
                cooling_coil_design_setpoint: central_cooling_design_supply_air_temperature

**Complex References**

References to object field values may take multiple forms.  This feature is intended to provide greater flexibility for object definition and to link nodes without relying on static text fields.  References may be specified as follows:

* Static value: numeric or string.
    Use a directly typed value.  Note, the `.format()` method is applied to all string values.  This allows template input values to be directly inserted into a string by simply applying the name within brackets '{}'.  A set of blank brackets will have the unique template name inserte into that space.

  For a given HVACTemplate Object:

  .. code-block:: json

    "HVACTemplate:System:DedicatedOutdoorAir": {
      "DOAS": {
        "gas_heating_coil_efficiency": 0.8
      }
    }

  The inputs can be applied to the object in the YAML file

  .. code-block:: yaml

    Fields: &CoilHeatingFuelFields
      <<: *CoilHeatingCommonFields
      burner_efficiency: '{gas_heating_coil_efficiency}'

* `BuildPath` Location References: string or integer
    Reference a node by it's location in the `BuildPath`.  This is only useful for System and Zone templates, not Plant or Loop templates.

  * Location: When given a string, a regular expression match is made to find an EnergyPlus object type in the `BuildPath`.  When given an integer, the list index of the object in the `BuildPath` is used.
  * Occurrence: The number of matches to make before returning an object.  A value of -1 will return the last matched object.
  * ValueLocation:

    * self - The EnergyPlus object type
    * key - The EnergyPlus unique object name
    * Inlet - The connector inlet
    * Outlet - The connector outlet

  .. code-block:: yaml

    AirLoopHVAC:
      supply_side_outlet_node_names:
        BuildPathReference:
        Location: -1
        ValueLocation: Outlet

* General Reference Value
  Reference an epJSON object in the document.  This is a key/value pair given to retrieve an EnergyPlus object.  Note, only objects that are created within the scope of the given template can be referenced.  The 'self' and 'key' options noted above are available as well.

  .. code-block:: yaml

    Branch:
      name: '{} ChW Branch'
      components:
        - component_object_type:
            Chiller.*: self
          component_name:
            Chiller.*: key
          component_inlet_node_name:
            Chiller.*: chilled_water_inlet_node_name
          component_outlet_node_name:
            Chiller.*: chilled_water_outlet_node_name

----------------------
Command Line Interface
----------------------

`-f, --file FILE\_NAME : Specify file to expand`

This argument may be omitted.  A value passed to the program with no argument will be assumed to be a file name.

`-h, --help : Display help information`

`-nb, --no\_backup : Do no create backup files`

It is not possible to comment sections of code in JSON formatted files.  Therefore, the output expanded files do not have the ability to retain the HVACTemplate objects used to create the current document.  If the original file were to be overwritten, then all template data would be lost.  In an attempt to provide and additional layer of backups, the -nb option is set to False by default which means two files will be created: one with HVACTemplate objects, and one with all other objects.  With these files, the original input file can be created, or specific objects can be copied and pasted.

`-ns, --no\_schema : Skip schema validation checks for pyExpandObjects.  Note, this does not skip other schema validation operations within EnergyPlus itself.`

One benefit of the JSON file format is that files can be validated before simulation.  This means that erroneous inputs can be found before simulation, which saves time debugging output files and reading through logs, unsure of the error source.  This includes syntax errors, values that are out of range, and missing required inputs.  However, situations may occur when the user wishes to skip schema validation, in which case this flag should be used.  By default, schema validation is enabled.

`-o, --output\_directory : Specify output directory.  If not provided, then input file directory is used.`

`-l, --logger\_level LOGGER\_LEVEL: Set logging output level`

Various levels of logging output are available for debugging, and other, purposes.  A valid level, consistent with Python logging naming structure (i.e. DEBUG, INFO, WARNING, ERROR, CRITICAL), must be provided.

`-v, --version : Display version information`

`-wl, --write-logs : Write logs to file`

When this expansion tool is run from its source directory, the output can be written to a file, which is located in the logs directory (logs/base.log).
