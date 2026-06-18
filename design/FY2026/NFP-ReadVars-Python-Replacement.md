Replace Fortran ReadVars with Python ReadVarsESO
================

**EnergyPlus Development Team**

 - Original Date: 06/18/2026
 - Revision Date: 06/18/2026


## Justification for New Feature ##

ReadVarsESO is a post-processing utility used to convert EnergyPlus ESO and MTR output files into delimited tabular output. The current implementation is written in modern Fortran, which keeps a Fortran build dependency alive for a small utility that is otherwise independent of the simulation engine.

This proposal replaces the Fortran ReadVarsESO program with a vanilla Python implementation. The goal is to preserve the existing command-line behavior for current workflows while making the tool easier to maintain, test, package, and extend.

## E-mail and  Conference Call Conclusions ##

N/A

## Overview ##

The new ReadVarsESO implementation will be a Python script distributed with EnergyPlus and wrapped by platform-appropriate launcher scripts. Existing RVI/MVI based conversion workflows should continue to work, including frequency filters, unlimited column handling, and fixed header behavior.

The Python version will also provide a modern subcommand-based interface for direct inspection and conversion of ESO/MTR files. Initial modern commands include listing available output variables and reading data directly to CSV without requiring a separate RVI/MVI file.

### Legacy behavior to preserve ###

The historical ReadVarsESO interface is positional and file-oriented. When called with no arguments, it reads `eplusout.eso`, selects all available variables from the ESO data dictionary, and writes `eplusout.csv`. When called with an RVI/MVI file, the control file identifies the input ESO/MTR file, the output file, and the requested variables.

Legacy variable requests may be made by report number or by matching variable names from the data dictionary. The RVI/MVI file can also exclude variables by prefixing report numbers or variable requests with `~`. Command-line options select broad output frequencies such as detailed/timestep, hourly, daily, monthly, and annual/run-period output. Existing options also include limited versus unlimited column handling and `fixheader` behavior.

The legacy converter writes delimited output using the output file extension to determine the delimiter: comma-separated output for CSV, tab-separated output for TAB files, and space-separated output for TXT files. It also writes a `readvars.audit` file and preserves the long-standing header, timestamp, and row formatting used by existing workflows.

### Enhancements in the Python version ###

The Python replacement adds a modern subcommand interface while retaining the legacy interface. The `list` command, also available as `variables`, inspects an ESO/MTR data dictionary and reports the available report number, frequency, key, variable name, units, and legacy label. It supports table, CSV, and JSON output, plus filtering by reporting frequency and search text. Internal timestamp dictionary records, such as annual calendar-year records used only to label report periods, are omitted from the modern list output.

The new `read` command converts an ESO/MTR file directly to CSV without requiring an RVI/MVI file. Calling `ReadVarsESO read <input-file>` selects all user-reportable variables and writes a CSV file next to the input using the same base file name. The output file can be configured with `--output`, and conversion can be narrowed with the same frequency and search filters used by `list`.

The modern commands are intended to be easier to script and discover. They avoid legacy audit-file side effects where practical, provide structured output for tooling, and make it possible for users to inspect available variables before selecting data to convert.

## Approach ##

The implementation will port the existing ReadVarsESO behavior to a Python script with no third-party package dependencies. CMake will copy the Python script and wrapper into the EnergyPlus runtime and install locations in place of building a Fortran executable.

The legacy interface will remain available for compatibility. A modern interface will be added alongside it, including:

- `list` or `variables` to inspect available report variables, keys, units, and reporting frequencies.
- `read` to convert an ESO/MTR file directly to CSV.
- Command options for output file selection and basic filtering by frequency or search text.

The modern commands will avoid legacy audit-file side effects where practical and will omit internal timestamp dictionary records that are not user-reportable output variables. Shared parsing and conversion routines will be used where possible so that legacy and modern behavior stay aligned.

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
