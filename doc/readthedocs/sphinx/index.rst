EnergyPlus Documentation
========================

EnergyPlus is an open-source whole-building energy simulation program used by engineers,
architects, and researchers to model energy consumption and water use in buildings. This site
brings together the user guides, application guidance, developer documentation, and API references.

New to EnergyPlus? Begin with the :doc:`Quick Start Guide <quick_start/quick_start>` or the
:doc:`Getting Started guide <guides/getting-started/index>`.

Downloads and releases
----------------------

EnergyPlus is released twice each year, usually in March and September. Formal public releases are
recommended for production workflows because development and prerelease versions may introduce
input syntax changes that are not supported by the release transition tools.

`Download the latest EnergyPlus release <https://github.com/NatLabRockies/EnergyPlus/releases/latest>`_
or browse `all available releases <https://github.com/NatLabRockies/EnergyPlus/releases>`_.

.. toctree::
   :maxdepth: 1
   :caption: Start here

   quick_start/quick_start
   essentials/essentials
   tips_and_tricks/tips_and_tricks

.. toctree::
   :maxdepth: 1
   :caption: User guides

   guides/getting-started/index
   guides/input-output-reference/index
   guides/engineering-reference/index
   guides/external-interfaces-application-guide/index
   guides/output-details-and-examples/index
   guides/plant-application-guide/index
   guides/module-developer/index
   guides/interface-developer/index
   guides/using-energyplus-for-compliance/index

.. toctree::
   :maxdepth: 1
   :caption: APIs and supporting documentation

   schema
   c
   api
   auxiliary-programs/auxiliary-programs
   ems-application-guide/ems-application-guide
   acknowledgments/acknowledgments

Support and project resources
-----------------------------

* Visit the `EnergyPlus website <https://energyplus.net>`_ for program information and resources.
* Ask usage and building-simulation questions on `Unmet Hours <https://unmethours.com/>`_.
* Request in-depth, developer-supported assistance from the
  `EnergyPlus Helpdesk <https://energyplushelp.freshdesk.com/>`_.
* Consult the `developer wiki <https://github.com/NatLabRockies/EnergyPlus/wiki>`_ for development
  and build information.

Quality, licensing, and contributions
-------------------------------------

Every EnergyPlus commit and release is exercised by unit, integration, API, and regression tests.
EnergyPlus is distributed under a `BSD-3-like license
<https://github.com/NatLabRockies/EnergyPlus/blob/develop/LICENSE.txt>`_ and accepts contributions
to source code, utilities, tests, documentation, and other project materials. Start with the
`EnergyPlus contribution policy <https://www.energyplus.net/contributing>`_.

For the repository overview and current development information, see the `EnergyPlus project
README`_. On Read the Docs, this link follows the exact source revision used to build the page.

About this documentation
------------------------

This site combines documentation produced by several tools:

* Sphinx builds the documentation site and Python API reference.
* Doxygen generates the C API reference from the EnergyPlus API headers.
* Pandoc converts the LaTeX user guides into navigable HTML.

Read the Docs publishes two continuously maintained versions:

* `latest <https://energyplus.readthedocs.io/en/latest/>`_ follows the ``develop`` branch.
* `stable <https://energyplus.readthedocs.io/en/stable/>`_ follows the most recent EnergyPlus release.

For local build instructions, source locations, contribution guidance, and publication details,
see the `documentation contributor guide`_. On Read the Docs, this link follows the exact source
revision used to build the page.
