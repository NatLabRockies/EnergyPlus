EnergyPlus [![](https://img.shields.io/github/release/NatLabRockies/energyplus.svg)](https://github.com/NatLabRockies/EnergyPlus/releases/latest)
==========

[![](https://img.shields.io/github/downloads/NatLabRockies/EnergyPlus/latest/total?color=5AC451)](https://github.com/NatLabRockies/EnergyPlus/releases/latest)
[![](https://img.shields.io/github/downloads/natlabrockies/energyplus/total.svg?color=5AC451&label=downloads_since_v8.1)](https://github.com/NatLabRockies/EnergyPlus/releases)

This is the EnergyPlus Development Repository.  EnergyPlus™ is a whole building energy simulation program that engineers, architects, and researchers use to model both energy consumption and water use in buildings.

## Contact/Support

 - The Department of Energy maintains a [public website for EnergyPlus](https://energyplus.net) where you can find much more information about the program.
 - For detailed developer information, consult the [wiki](https://github.com/NatLabRockies/EnergyPlus/wiki).
 - Many users (and developers) of EnergyPlus are active on [Unmet Hours](https://unmethours.com/), so that's a great place to start if you have a question about EnergyPlus or building simulation.
 - For more in-depth, developer-driven support, please utilize the [EnergyPlus Helpdesk](https://energyplushelp.freshdesk.com/).

## Testing

[![](https://github.com/NatLabRockies/EnergyPlus/actions/workflows/test_code_integrity.yml/badge.svg?branch=develop)](https://github.com/NatLabRockies/EnergyPlus/actions/workflows/test_code_integrity.yml)
[![](https://github.com/NatLabRockies/EnergyPlus/actions/workflows/build_documentation.yml/badge.svg?branch=develop)](https://github.com/NatLabRockies/EnergyPlus/actions/workflows/build_documentation.yml)

Every commit and every release of EnergyPlus undergoes rigorous testing.
The testing consists of building EnergyPlus, of course, then there are unit tests, integration tests, API tests, and regression tests.
Since 2014, most of the testing has been performed a fork of the [Decent CI](https://github.com/lefticus/decent_ci) continuous integration system.
We are now adapting our efforts to use the GitHub Actions system to handle more of our testing processes.
In the meantime, while Decent CI is still handling the regression and bulkier testing, results from Decent CI are still available on the testing [dashboard](https://myoldmopar.github.io/EnergyPlusBuildResults/).

## Releases

[![](https://github.com/NatLabRockies/EnergyPlus/workflows/Windows%20Releases/badge.svg)](https://github.com/NatLabRockies/EnergyPlus/actions/workflows/release_windows.yml)
[![](https://github.com/NatLabRockies/EnergyPlus/workflows/Mac%20Releases/badge.svg)](https://github.com/NatLabRockies/EnergyPlus/actions/workflows/release_mac.yml)
[![](https://github.com/NatLabRockies/EnergyPlus/workflows/Linux%20Releases/badge.svg)](https://github.com/NatLabRockies/EnergyPlus/actions/workflows/release_linux.yml)

EnergyPlus is released twice annually, usually in March and September.
It is recommended all use of EnergyPlus in production workflows use these formal, public releases.
Iteration **(pre-)releases** may be created during a development cycle, however users should generally avoid these, as input syntax may change which won't be supported by the major release version transition tools, and could require manual intervention to remedy.
If an interim release is intended for active use by users, such as a bug-fix-only or performance-only re-release, it will be clearly specified on the release notes and a public announcement will accompany this type of release.
Our releases are now built by GitHub Actions.

## Documentation

[![Read the Docs](https://img.shields.io/readthedocs/energyplus?label=docs%20%28latest%29&color=5AC451)](https://energyplus.readthedocs.io/en/latest/)
[![Read the Docs](https://img.shields.io/readthedocs/energyplus?label=docs%20%28stable%29&color=5AC451)](https://energyplus.readthedocs.io/en/stable/)

The [EnergyPlus documentation site](https://energyplus.readthedocs.io/en/latest/) brings together
introductory material, user and developer guides, input-schema documentation, and the C and Python
API references. Start with the [Quick Start Guide](https://energyplus.readthedocs.io/en/latest/quick_start/quick_start.html)
to install EnergyPlus and run a first simulation, or browse the complete documentation for detailed
modeling and development guidance.

Key references include:

- [Getting Started](https://energyplus.readthedocs.io/en/latest/guides/getting-started/index.html) —  a comprehensive introduction to EnergyPlus concepts, workflows, input files, and output files.
- [Input Output Reference](https://energyplus.readthedocs.io/en/latest/guides/input-output-reference/index.html) —  an encyclopedic reference to EnergyPlus input objects and output reports.
- [Engineering Reference](https://energyplus.readthedocs.io/en/latest/guides/engineering-reference/index.html) —  the theoretical background, algorithms, and calculation methods used by EnergyPlus.
- [EnergyPlus epJSON input schema](https://energyplus.readthedocs.io/en/latest/schema.html) —  searchable definitions for EnergyPlus input objects and fields.
- [C API reference](https://energyplus.readthedocs.io/en/latest/c.html) — the native API for embedding EnergyPlus and accessing runtime, data-transfer, functional, and callback interfaces.
- [Python API reference](https://energyplus.readthedocs.io/en/latest/api.html) — Python bindings for the EnergyPlus C API and supporting functionality.

The [`latest`](https://energyplus.readthedocs.io/en/latest/) documentation follows the `develop` branch, while [`stable`](https://energyplus.readthedocs.io/en/stable/) follows the most recent release.
PDF documentation is also included with the packages on the [EnergyPlus releases page](https://github.com/NatLabRockies/EnergyPlus/releases). Big Ladder maintains an independent [archive of web-based EnergyPlus documentation](https://bigladdersoftware.com/epx/docs/) for released versions.

For local build instructions and information about contributing documentation, see the [documentation contributor guide](doc/readthedocs/README.md).

## License & Contributing Development

[![](https://img.shields.io/badge/license-BSD--3--like-5AC451.svg)](https://github.com/NatLabRockies/EnergyPlus/blob/develop/LICENSE.txt)

EnergyPlus is available under a BSD-3-like license.
For more information, check out the [license file](https://github.com/NatLabRockies/EnergyPlus/blob/develop/LICENSE.txt).
The EnergyPlus team accepts contributions to EnergyPlus source, utilities, test files, documentation, and other materials distributed with the program.
The current EnergyPlus contribution policy is now available on the EnergyPlus [contribution policy page](https://www.energyplus.net/contributing).
If you are interested in contributing, please start there, but feel free to reach out to the team.

## Building EnergyPlus

A detailed description of compiling EnergyPlus on multiple platforms is available on the [wiki](https://github.com/NatLabRockies/EnergyPlus/wiki/Building-EnergyPlus).
Also, as we are adapting to using GitHub Actions, the recipes for building EnergyPlus can be found in our [workflow files](https://github.com/NatLabRockies/EnergyPlus/tree/develop/.github/workflows).
