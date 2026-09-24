Replace Fortran ReadVarsESO with a Python Equivalent
================

Jason W. DeGraw, ORNL

 - Original Date: 06/18/2026
 - Revision Date: N/A

## Justification for New Feature ##

ReadVarsESO is a post-processing utility used to convert EnergyPlus ESO and MTR output files into delimited tabular output (primarily into comma-separated value or CSV form). The current implementation is written in modern Fortran, requiring a Fortran build dependency for a utility that is somewhat independent of the simulation engine. The utility is invoked by the engine if the `-r` flag is passed on the command line, so there is a limited relationship between the engine and the utility. Python has been a requirement for EnergyPlus development for some time now and is a somewhat easier dependency to manage.

This proposal replaces the Fortran ReadVarsESO program with a vanilla Python implementation, where "vanilla" means that there are no dependencies outside of what comes with standard Python. The goal is to preserve the existing command-line behavior for current workflows while making the tool easier to maintain, test, package, and extend. The script has outperformed the Fortran executable in all testing thus far.

## E-mail and  Conference Call Conclusions ##

N/A

## Overview ##

The new ReadVarsESO implementation is a Python script distributed with EnergyPlus and wrapped by platform-appropriate launcher scripts. Existing RVI/MVI based conversion workflows should continue to work, including frequency filters, unlimited column handling, and fixed header behavior.

The Python version provides a modern subcommand-based interface for direct inspection and conversion of ESO/MTR files. Initial modern commands include listing available output variables and reading data directly to CSV without requiring a separate RVI/MVI file.

### Legacy Behavior ###

The historical ReadVarsESO interface is primarily file-oriented. When called with no arguments, it reads `eplusout.eso`, selects all available variables from the ESO data dictionary, and writes `eplusout.csv`. When called with an RVI/MVI file, the control file identifies the input ESO/MTR file, the output file, and the requested variables.

Legacy variable requests may be made by report number or by matching variable names from the data dictionary. The RVI/MVI file can also exclude variables by prefixing report numbers or variable requests with `~`. Command-line options select broad output frequencies such as detailed/timestep, hourly, daily, monthly, and annual/run-period output. Existing options also include limited versus unlimited column handling and `fixheader` behavior.

The legacy converter writes delimited output using the output file extension to determine the delimiter: comma-separated output for CSV, tab-separated output for TAB files, and space-separated output for TXT files. It also writes a `readvars.audit` file and preserves the long-standing header, timestamp, and row formatting used by existing workflows.

### Enhancements in the Python version ###

The Python replacement adds a modern subcommand interface while retaining the legacy interface. The new `list` command inspects an ESO/MTR data dictionary and reports the available report number, frequency, key, variable name, units, and legacy label. It supports table, CSV, and JSON output, plus filtering by reporting frequency and search text. Internal timestamp dictionary records, such as annual calendar-year records used only to label report periods, are omitted from the modern list output.

The new `read` command converts an ESO/MTR file directly to CSV without requiring an RVI/MVI file. Calling `ReadVarsESO read <input-file>` selects all user-reportable variables and writes a CSV file next to the input using the same base file name. The output file can be configured with `--output`, and conversion can be narrowed with the same frequency and search filters used by `list`.

The modern commands are intended to be easier to script and discover. They avoid legacy audit-file side effects where practical, provide structured output for tooling, and make it possible for users to inspect available variables before selecting data to convert.

### Performance of the Python version ###

To investigate the performance of the Python version, a test file with an existing RVI file was selected: 5ZoneDetailedIceStorage.idf. This model includes 35 output variables and the RVI has 34 entries, and an annual simulation of the model results in an ESO file that is approximately 7.3 MB in size. The Python and Fortran versions were each run ten times on an small Windows desktop system running on a Intel Core i7-8700 @ 3.20GHz processor, resulting in the following timings:

|	        | Fortran (ms) |	Python (ms) |
|:-------:|:------------:|:-----------:|
|    1    |  	4720.9429  |  	924.1084  |
|    2	   |   4649.1618  |	  832.2611  |
|    3	   |   4635.4731  |  	831.3671  |
|    4	   |   4647.0617  |	  885.8676  |
|    5	   |   5082.1201  |  	840.9368  |
|    6	   |   4631.1113  |  	843.4879  |
|    7	   |   4633.1652  |  	835.8188  |
|    8	   |   4719.2226  |	  844.0984  |
|    9    |	  4794.5117  |  	843.9699  |
|   10   	|   4707.4851  |	  839.8434  |
| Average |	  4722.02555 |	  852.1759  |
| Std Dev |	 137.3253939 |	  29.61428  |

The Python script is roughly five times faster than the Fortran executable for this test case.

## Approach ##

The implementation ports the existing ReadVarsESO behavior to a Python script with no third-party package dependencies. CMake copies the Python script and wrapper into the EnergyPlus runtime and install locations in place of building a Fortran executable.

The legacy interface will remain available for compatibility. A modern interface will be added alongside it, including:

- `list` to inspect available report variables, keys, units, and reporting frequencies.
- `read` to convert an ESO/MTR file directly to CSV.
- Command options for output file selection and basic filtering by frequency or search text.

The modern commands avoid legacy audit-file side effects where practical and omits internal timestamp dictionary records that are not user-reportable output variables. Shared parsing and conversion routines will be used where possible so that legacy and modern behavior stay aligned.

## Testing/Validation/Data Sources ##

Testing will include focused unit or script-level tests for:

- Legacy RVI/MVI conversion compatibility.
- Listing variables and filtering metadata records.
- Listing variables in table, CSV, and JSON formats.
- Direct modern CSV conversion using the `read` command.
- Configurable output paths and basic frequency/search filtering.
- Default `read` behavior that converts all user-reportable variables when only an input file is supplied.

Existing EnergyPlus regression workflows that invoke ReadVarsESO should continue to pass using the Python wrapper.

## Input Output Reference Documentation ##

The Input Output Reference is not expected to require changes because this feature does not add, remove, or modify EnergyPlus input objects.

The auxiliary programs documentation should be updated to describe both the legacy ReadVarsESO usage and the modern `list` and `read` subcommands.

## Input Description ##

No IDD, epJSON schema, or input object changes are required.

## Outputs Description ##

No simulation output variables are added or changed.

ReadVarsESO output files should remain compatible with existing CSV, TAB, and TXT post-processing workflows for the legacy interface. The modern `read` command will produce CSV output directly, with the output file name configurable by command-line option.

## Engineering Reference ##

No engineering reference changes are expected. This is a post-processing utility replacement and does not alter simulation algorithms.

## Example File and Transition Changes ##

No example file changes are expected.

No IDF transition changes are required. Packaging and scripts that directly invoke the old executable name may need updates to call the Python wrapper, while existing command-line arguments should remain compatible through that wrapper.

## References ##

N/A
