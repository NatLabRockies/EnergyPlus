How to run EnergyPlus from the command line
===========================================

EnergyPlus has a command line interface. The options for running EnergyPlus can be shown by typing:

    energyplus --help

This will give the following display of options:

    EnergyPlus, Version X.Y.Z
    Usage: energyplus [options] [input-file]
    Options:
      -a, --annual                 Force annual simulation
      -c, --convert                Output IDF->epJSON or epJSON->IDF, dependent on
                                   input file type
      -d, --output-directory ARG   Output directory path (default: current
                                   directory)
      -D, --design-day             Force design-day-only simulation
      -h, --help                   Display help information
      -i, --idd ARG                Input data dictionary path (default: Energy+.idd
                                   in executable directory)
      -j, --jobs ARG               Multi-thread with N threads; 1 thread with no
                                   arg.
      -m, --epmacro                Run EPMacro prior to simulation
      -p, --output-prefix ARG      Prefix for output file names (default: eplus)
      -r, --readvars               Run ReadVarsESO after simulation
      -s, --output-suffix ARG      Suffix style for output file names (default: L)
                                   L: Legacy (e.g., eplustbl.csv)
                                   C: Capital (e.g., eplusTable.csv)
                                   D: Dash (e.g., eplus-table.csv)
      -v, --version                Display version information
      -w, --weather ARG            Weather file path (default: in.epw in current
                                   directory)
      -x, --expandobjects          Run ExpandObjects prior to simulation
      --convert-only               Only convert IDF->epJSON or epJSON->IDF,
                                   dependent on input file type. No simulation
    Example: energyplus -w weather.epw -r input.idf

EnergyPlus can be run by specifying a number of options followed by the path to the input file (`input-file`). The file itself is usually in IDF (Input Data File) format, but it may also be in IMF (Input Macro File) format to be run with EPMacro using the `--epmacro` option.

Each option has a short form (a single-character preceded by a single dash, e.g., "-h") and a long form (a more descriptive string of characters preceded by double dashes, e.g., "--help").

The options generally fall into four categories:

1. Basic informational switches:
   - `help`
   - `version`
2. Input/output control flags:
   - `idd`
   - `weather`
   - `output-directory`
   - `output-prefix`
   - `output-suffix`
3. Pre- and post-processing switches:
   - `epmacro`
   - `expandobjects`
   - `readvars`
4. Input override switches:
   - `annual`
   - `design-day`
   - `jobs`

Notes
-----

1. As of V9.6, the `--jobs` option is only configured to increase thread usage for `GroundHeatExchanger:System` objects with the g-Function calculation method set to `UBHWTcalc`. As EnergyPlus continues to evolve, this may be adapted to control other features or models.

2. If native csv output is requested using OutputControl:Files, the `--readvars` postprocessing option will overwrite the native csv output.

Examples
--------

1. Using a custom IDD file:

    `energyplus -i custom.idd -w weather.epw input.idf`

2. Pre-processing using EPMacro and ExpandObjects:

    `energyplus -w weather.epw -m -x input.imf`

3. Forcing design-day only simulations:

    `energyplus -D input.idf`

4. Giving all output files the prefix being the same as the input file (`building.idf`) and placing them in a directory called `output`:

    `energyplus -w weather -p building -d output building.idf`

Legacy Mode
-----------

The command line interface is a new feature as of EnergyPlus 8.3. Prior to version 8.3, the EnergyPlus executable took no command line arguments, and instead expected the IDD (Input Data Dictionary) file and the IDF files to be located in the current working directory and named `Energy+.idd` and `in.idf` respectively. If a weather file was required by the simulation, then an `in.epw` file was also required in the same directory. This behavior is still respected if no arguments are passed on the command line.
