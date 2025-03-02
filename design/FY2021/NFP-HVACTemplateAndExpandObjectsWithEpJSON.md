Create HVACTemplate and ExpandObjects Support for epJSON files
================

**John Grando, GARD Analytics**

 - Original Date: February 11, 2021
 - Revision Date:  
   - February 23, 2021

## Justification for New Feature ##

The addition of epJSON as an input file is impacting current workflows which rely on HVACTemplate objects. Similarly, HVACTemplate is useful for learning EnergyPlus, and provides an aid on projects focused on envelope or lighting measures, simplified interfaces, and scripted workflows.  It is important that model files built in the epJSON format have the same functionality as their equivalents written in IDF format in order to support existing workflows as well as foster future adoption.  The HVACTemplate and ExpandObjects features have proven to be valuable in the previous format, and new implementations of these tools should be developed.  Additionally, the explanations provided for this feature are considered the basic operations of the program.  Additional capabilities can be provided in future updates to this package.

## E-mail and  Conference Call Conclusions ##

February 23, 2021:  
  - The ability to turn off schema validation is a contested feature.  Some believe that users should not be passing epJSON incompatible files through the pipeline at all, while others argue that schema validation occurs anyways during pre-simulation and turning it off in pyExpandObjects could provide a performance boost for high performance computing instances.  It was decided to leave this option but with the default set to validate schemas. 
  - There was unanimous support for changing the expanded file naming scheme from <original-file-name>.expepJSON to <original-file-name>_expanded.epJSON.

## Overview ##

In order to achieve greater alignment with other programming initiatives in EnergyPlus, take advantage of the epJSON schema structure, and make use of object-oriented programming methodologies, Python was chosen for this effort.  This updated tool will provide the same deliverable as its predecessor, which is an output file with regular objects that have been mapped from template objects.  However, the epJSON file format will be used for this program.

## Approach ##

The process for delivering this product will be different in order to better align with Python best practices and provide greater flexibility.  First, validation methods will be incorporated to ensure valid files are read and created.  An external package that performs json schema validation will be used to verify files throughout the process.  Second, an HVACTemplate class will be created in Python which will keep data attributes and methods together in a central location as well as provide the option of using inheritance and mixins for sub-classes.  This structure allows the program to be flexible when creating objects. Third, a process will be created which translates epJSON files with HVACTemplate objects to epJSON files with only regular objects. This step will make use of one-to-many mappings stored in Yaml files and built within the class structure. Finally, a rigorous set of tests will be created which verify the functionality and robustness of the program.  With these proposed changes, the ExpandObjects tool will be able to operate in the EnergyPlus file pre-processing pipeline by reading and writing valid epJSON files.

## Testing/Validation/Data Sources ##

Python's built-in unittest package will be used in conjunction with external packages that monitor the areas of code which have tests verifying their functionality (coverage).  Using these modules, multiple small tests (unit) will be created which verify output on the function level.  Coverage reports will be auto-generated with each build and provide confirmation of the package status.  In addition to unit testing, A subset of HVACTemplate-*.idf files will be created in epJSON format, expanded, simulated, and have outputs compared to their idf counterparts.

## Auxiliary Programs Reference Documentation ##

'Chapter 10 ExpandObjects' will be revised to have two main sections, ExpandObjects (10.1) and pyExpandObjects (10.2), The revised sections are as *follows*:

**10.1 ExpandObjects**

- 10.1.1 <- 10.1
- 10.1.2 <- 10.2
- 10.1.3 <- 10.3
- 10.1.4 <- 10.4

**10.2 pyExpandObjects**

***10.2.1 Introduction***  
*Much like the ExpandObjects program in Section 10.1, the pyExpandObjects expands HVACTemplate objects from an input epJSON file into an expanded file that can be directly run in EnergyPlus.  However, pyExpandObjects does not process GroundHeatTransfer objects or provide support for the Slab or Basement executables.  Additionally, pyExpandObjects is a standalone application which means that it must be run manually to create an expanded file.  Details on how to run the program can be found in the Input Output Reference Manual (Chapter 2 - HVACTemplate Objects).*  

*The pyExpandObjects program works as a preprocessor that maps HVACTemplate objects to regular objects in EnergyPlus.  This processor reads an epJSON file and generates and expanded epJSON file.  No further pre-processing should be required after the conversion has been performed.  Unlike ExpandObjects (10.1), a schema validation does occur when the file is read into the program, and error messages will be shown in the usual EnergyPlus error file.  By default, an invalid epJSON file will stop the program, but this requirement can be removed via command line options.  Please see the [documentation](https://epjson-expandobjects.readthedocs.io/en/latest/?badge=latest) or '--help' command line option for further details.  Additionally, the default settings produce two backup files ("\<original-file-name\>_hvac_templates.epJSON", "\<original-file-name\>_base.epJSON") as well as an expanded file with an adjusted name ("\<original-file-name\>_expanded.epJSON"). If the '--no-backup' option is used in the command line, then only the expanded file will be output.*

**10.2.2 HVAC Template Objects Processed**  
All HVACTemplate objects supported by the ExpandObjects program are supported in pyExpandObjects.  Please refer to section 10.1.2 for further details.

**10.2.3 Ground Heat Transfer Processed**  
GroundHeatTransfer objects are not supported by the pyExpandObjects preprocessor.

**10.2.4 Building Surface Objects Processed**  
No building surface objects are modified by the pyExpandObjects preprocessor.

## Input Output Reference Documentation ##

The current HVACTemplate Processing section (2.1) will be revised to directly address how epJSON files will be handled.  The section will be rewritten as *follows*:

***2.1 HVACTemplate Processing***

***2.1.1 IDF File Overview***

**Unlike other EnergyPlus objects, the HVACTemplate objects are not handled by EnergyPlus directly.** *Instead, they are preprocessed by a program called ExpandObjects. If you use EP-Launch or RunEPlus.bat, this preprocessor step is performed automatically using the following sequence:*:  

1. The preprocessor program, ExpandObjects, reads your IDF file and converts all of the HVACTemplate objects into other EnergyPlus objects.
2. The ExpandObjects program copies the original idf file with the HVACTemplate objects commented out (i.e., inserts a “!” in front of the object name and its input fields) plus all of the new objects created in Step 1 and writes a new file which is saved with the extension “expidf”. This “expidf” file may be used as a standard EnergyPlus IDF file if the extension is changed to idf; however, for safety’s sake, both filename and extension should be changed.
3. The EnergyPlus simulation proceeds using the expidf file as the input stream.
4. If there are error messages from EnergyPlus, they will refer to the contents of the expidf file. Specific objects referred to in error messages may exist in the original idf, but they may be objects created by ExpandObjects and only exist in the expidf. Remember that the expidf will be overwritten everytime the original idf is run using EP-Launch or RunEPlus.bat  

If you are trying to go beyond the capabilities of the HVACTemplate objects, one strategy you can use is to start your specification using the HVACTemplate objects, run EnergyPlus using EP-Launch and producing an expidf file, rename that file and start making modifications. This approach may help with getting all of the objects needed and the node names set consistently.

***2.1.2 epJSON File Overview***
**The HVACTemplate objects for this filetype are handled by a standalone application called pyExpandObjects.** *Preprocessing epJSON files must be done manually from the command line.  This may be done by directly calling the program (src/main.py) from the application source directory (src/pyExpandObjects), or from a built executable (see the documentation README for more details: [https://github.com/john-grando/pyExpandObjects#build-instructions](https://github.com/john-grando/pyExpandObjects#build-instructions))*

1. *The preprocessor program, pyExpandObjects, reads your epJSON file, verifies it meets the current schema requirements, and converts all the HVACTemplate objects into other EnergyPlus objects.*
2. *The pyExpandObjects program writes one to three new files.  
   a. *A renamed file with the naming format "\<original-file-name\>_expanded.epJSON". This “expanded” file may be used as a standard EnergyPlus epJSON file.  Note, the HVACTemplate objects will be completely removed from the expanded file, so ensure to save a copy of the original file before deleting anything!*  
   b. *Optional - A file with the name "\<original-file-name\>_hvac_templates.epJSON" which has all the HVACTemplate objects from the input file.*  
  c. *Optional - A file with the name "\<original-file-name\>_base.epJSON" which has all the HVACTemplate objects removed.*
3. *The EnergyPlus simulation may now be run with the "expanded" file as the target.*
4. *If there are error messages from EnergyPlus, they will refer to the contents of the expanded file. Specific objects referred to in error messages may exist in the original epJSON, but they may be objects created by pyExpandObjects and only exist in the expanded file. Remember that the "\<original-file-name\>_expanded.epJSON", "\<original-file-name\>_hvac_templates.epJSON", and "\<original-file-name\>_base.epJSON" files will be overwritten everytime the original epJSON is run using EP-Launch or RunEPlus.bat.  Also note, the "\<original-file-name\>_hvac_templates.epJSON" and "\<original-file-name\>_base.epJSON" files can be merged to recreate the original epJSON file.*

*If you are trying to go beyond the capabilities of the HVACTemplate objects, one strategy you can use is to start your specification using the HVACTemplate objects, run pyExpandObjects manually to produce an epJSON file, rename that file, make modifications, and simulate the new file in EnergyPlus manually. This approach may help with getting all of the objects needed and the node names set consistently.*

***2.1.3 General HVACTemplate Overview***
**Users need to remember that no objects related to HVAC except for HVAC template objects are needed in the IDF */ epJSON* file**. The existence of other objects (unless specifically described in the following sections) may cause unexpected errors to occur. Sizing:Zone, Sizing:System, and Sizing:Plant objects will be generated by the corresponding HVACTemplate objects; the user does not need to create these elsewhere in the input file. There are some exceptions to this rule:  
- HVACTemplate:Plant:Chiller:ObjectReference which requires that the corresponding chiller object be present in the idf */ epJSON* file along with any required curve or performance objects. In this case, the HVACTemplate object does not create the chiller object, but adds all of the connections. HVACTemplate:Plant:Tower:ObjectReference and HVACTemplate:Plant:Boiler;ObjectReferences are similar.  
- For HVACTemplate:Zone:* objects, if Outdoor Air Method = DetailedSpecification, then any referenced DesignSpecification:OutdoorAir and DesignSpecification:ZoneAirDistribution objects must be
present in the idf */ epJSON* file.  
- For HVACTemplate:Zone:VAV and HVACTemplate:Zone:DualDuct, if a Design Specification Outdoor Air Object Name for Control is specified, then the referenced DesignSpecification:OutdoorAir object must be present in the idf */ epJSON* file.

***2.1.3 Command Line Interface***

***2.1.3.1 Simulation Tool***

*The command line interface located in the EnergyPlus root directory (energyplus.exe) provides optional arguments to enable various features of the simulation package.*

```
-a, --annual : Force annual simulation
-c, --convert : Output IDF->epJSON or epJSON->IDF, dependent on input file type
-d, --output-directory ARG : Output directory path (default: current directory)
-D, --design-day : Force design-day-only simulation
-h, --help : Display help information
-i, --idd ARG : Input data dictionary path (default: Energy+.idd in executable directory)
-j, --jobs ARG : Multi-thread with N threads; 1 thread with no arg. (Currently only for G-Function generation)
-m, --epmacro : Run EPMacro prior to simulation
-p, --output-prefix ARG : Prefix for output file names (default: eplus)
-r, --readvars : Run ReadVarsESO after simulation
-s, --output-suffix ARG : Suffix style for output file names (default: L)
  L: Legacy (e.g., eplustbl.csv)
  C: Capital (e.g., eplusTable.csv)
  D: Dash (e.g., eplus-table.csv)
-v, --version : Display version information
-w, --weather ARG : Weather file path (default: in.epw in current directory)
--convert-only : Only convert IDF->epJSON or epJSON->IDF, dependent on input file type. No simulation
```

***IDF Options***

*In order to simulate an IDF file that contains HVACTemplate objects, the input must be run through ExpandObjects.  By specifying this process to run, the simulation is run with a fully expanded file.  Any errors that occur during simulation will refer to the \*.expidf file, not the original \*.idf file*

```
-x, --expandobjects : Run ExpandObjects prior to simulation
```

***epJSON Options***

*Expansion of HVACTemplate objects for epJSON filetypes is not integrated into the EnergyPlus command line interface at this time.  Please see the next section on how to operate the pyExpandObjects expansion tool.*

***2.1.3.2 HVACTemplate Expansion Tools***

***IDF - ExpandObjects***

*No command line options are available for this tool.*

***epJSON - pyExpandObjects***

The following command line arguments may be provided to control the prorgram outputs.

```
-f, --file FILE\_NAME : Specify file to expand
```

This argument may be omitted.  A value passed to the program with no argument will be assumed to be a file name.

```
-h, --help : Display help information
```

```
-nb, --no\_backup : Do no create backup files
```

It is not possible to comment sections of code in JSON formatted files.  Therefore, the output expanded files do not have the ability to retain the HVACTemplate objects used to create the current document.  If the original file were to be overwritten, then all template data would be lost.  In an attempt to provide and additional layer of backups, the -nb option is set to False by default which means two files will be created: one with HVACTemplate objects, and one with all other objects.  With these files, the original input file can be created, or specific objects can be copied and pasted.

```
-ns, --no\_schema : Skip schema validation checks for pyExpandObjects.  Note, this does not skip other schema validation operations within EnergyPlus itself.
```

One benefit of the JSON file format is that files can be validated before simulation.  This means that erroneous inputs can be found before simulation, which saves time debugging output files and reading through logs, unsure of the error source.  This includes syntax errors, values that are out of range, and missing required inputs.  However, situations may occur when the user wishes to skip schema validation, in which case this flag should be used.  By default, schema validation is enabled.

```
-o, --output\_directory : Specify output directory.  If not provided, then input file directory is used.
```

```
-l, --logger\_level LOGGER\_LEVEL: Set logging output level
```

Various levels of logging output are available for debugging, and other, purposes.  A valid level, consistent with Python logging naming structure (i.e. DEBUG, INFO, WARNING, ERROR, CRITICAL), must be provided.

```
-v, --version : Display version information
```

```
-wl, --write-logs : Write logs to file
```

When this expansion tool is run from its source directory, the output can be written to a file, which is located in the logs directory (logs/base.log).

## Engineering Reference ##

None

## Example File and Transition Changes ##

A subset of HVACTemplate-*.idf files in epJSON format will be created as examples.

## References ##

Related information is provided in GitHub:  https://github.com/NREL/EnergyPlus/issues/6865

Current ExpandObjects code: https://github.com/NREL/EnergyPlus/blob/develop/src/ExpandObjects/epfilter.f90

## Design Document ##

To implement the features described above, the following will be done:

- Build epJSON input and output File handlers.
- Document existing HVACTemplate objects and how they are mapped to regular objects in the ExpandObjects process.
- Create HVACTemplate Class hierarchy of objects, using class inheritance and combining common methods wherever meaningful.
- Create yaml file containing default values and template object structures.
- Create a testing suite consisting of all HVACTemplate objects to verify outputs.
- Create ExpandObjects file conversion process.
  - Outputs:
    - "\<original-file-name\>_expanded.epJSON" - Expanded epJSON file
    - "\<original-file-name\>_hvac_templates.epJSON" - File containing HVACTemplate objects (optional output)
    - "\<original-file-name\>_base.epJSON" - Original file without HVACTemplate objects (optional output)
- Test and refactor code based on findings from above item.
- Create copies of selected HVACTemplate-*.idf files in JSON format and test results against the IDF counterparts.
- Consult with EnergyPlus team on implementation process for the new tool.
