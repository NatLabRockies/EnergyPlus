Notices and Acknowledgments
===========================

.. contents::
   :depth: 2

Copyright Notice
----------------

Copyright (c) 1996-2025 The Board of Trustees of the University of
Illinois and The Regents of the University of California through Ernest
Orlando Lawrence Berkeley National Laboratory. All rights reserved.

Portions of the EnergyPlus\ :sup:`TM` software package have been
developed and copyrighted by other individuals, companies and
institutions. These portions have been incorporated into the EnergyPlus
software package under license.

**In addition to the primary authorship of the LBNL Simulation Research
Group (**\ http://simulationresearch.lbl.gov/\ **) and the UIUC Building
Systems Laboratory, the following have contributed to EnergyPlus
Versions (includes all minor revisions):**

Portions of the EnergyPlus weather processor were developed by US
Department of Energy, Office of Building Technologies.

Portions of the input processing, output processing, weather processor,
BLAST Translator were developed by US Army Corps of Engineers,
Construction Engineering Research Laboratories, 2902 Newmark Drive,
Champaign IL  61821.
http://www.erdc.usace.army.mil/Locations/ConstructionEngineeringResearchLaboratory.aspx

Portions of this software package were developed for Ernest Orlando
Lawrence Berkeley National Laboratory and Florida Solar Energy Center by
Linda Lawrie of DHL Consulting.

Portions of this software package were developed for Ernest Orlando
Lawrence Berkeley National Laboratory and Florida Solar Energy Center by
C.O. Pedersen Associates.

Portions of the EnergyPlus utility software (EP-Launch, IDFEditor,
DOE2Translator, HVAC-Diagram, ExpandObjects, CSVProc,
ParametricPreprocessor, AppGPostProcess, EP-Compare, and convertESOMTR)
were developed by GARD Analytics, Inc. 115 S. Wilke Road, Suite 105,
Arlington Heights, IL, USA, <info@gard.com>,
`www.gard.com <http://www.gard.com/>`__. GARD Analytics performed
independent verification and validation testing of the software after
developing the testing strategy and plan. GARD Analytics was also
responsible for gas absorption chiller, desiccant dehumidifier, ice
storage (simple), table reports, economics and life cycle costing.

Portions of flow resolver, chiller models (absorption, electric, const
cop, engine-driven, gas-turbine), generator models (diesel electric, gas
turbine), furnace models, heat recovery loop, plant loop, plant
condenser loop, air-change dependent inside film coefficients were
developed by Oklahoma State University, 110 Engineering North,
Stillwater, OK 74078.

Portions of EnergyPlus related to the models for EMPD moisture
calculations, DX coils, furnace/unitary systems, air-to-air heat pumps,
changeover-bypass VAV systems, packaged terminal heat pumps, cooling
towers, AirflowNetwork, refrigerated cases, reformulated and electric
EIR chillers, desuperheater air and water heating coils, heat pump water
heaters, desiccant and generic air-to-air heat exchangers, window
screens, and thermal comfort controls were developed by University of
Central Florida, Florida Solar Energy Center (FSEC), 1679 Clearlake
Road, Cocoa, FL  32922,
`www.fsec.ucf.edu/ <http://www.fsec.ucf.edu/>`__.

Portions of the refrigeration model and the exhaust-fired absorption
chiller model were developed by Oak Ridge National Laboratory, Bethel
Valley Road, Oak Ridge, Tennessee 37831.

Portions of EnergyPlus were developed by the National Renewable Energy
Laboratory (NREL), 1617 Cole Blvd, Golden, CO 80401.

Portions of EnergyPlus related to transformer losses model, autosizing
calculations, life cycle costing and chemical battery storage model were
developed by Pacific Northwest National Laboratory (PNNL), P.O. Box 999,
Richland, WA 99352.

EnergyPlus v1.0.1, v1.0.2, v1.0.3, v1.1, v1.1.1 (Wintel platform)
included a link to TRNSYS (The Transient Energy System Simulation Tool)
for photovoltaic calculations developed by Thermal Energy System
Specialists, 2916 Marketplace Drive, Suite 104, Madison, WI 53719; Tel:
(608) 274-2577. EnergyPlus v1.2 and later includes Photovoltaic
calculations implemented in EnergyPlus by Thermal Energy System
Specialists. This model was originally developed by Oystein Ulleberg,
Institute for Energy Technology, Norway – based on the Duffie and
Beckman equivalent one-diode model.

Portions of this software package that convert certain stand-alone heat
transfer models for slab-on-grade and basement foundations were
developed by William Bahnfleth, Cynthia Cogil, and Edward Clements,
Department of Architectural Engineering, Pennsylvania State University,
224 Engineering Unit A, University Park, Pennsylvania 16802-1416, (814)
863-2076.

The concept and initial implementation for the EnergyPlus COM/DLL
version (Wintel platform) was made possible through cooperation with
DesignBuilder Software, Ltd, Andy Tindale – an EnergyPlus collaborative
developer.

The thickness, conductivity, density and specific heat values of the
material layers for the constructions in the Composite Wall Construction
reference data set have been taken from the ASHRAE report “Modeling Two-
and Three-Dimensional Heat Transfer through Composite Wall and Roof
Assemblies in Hourly Energy Simulation Programs (1145-TRP),” by
Enermodal Engineering Limited, Oak Ridge National Laboratory, and the
Polish Academy of Sciences, January 2001.

EnergyPlus v1.2 and later versions contains DELight2, a simulation
engine for daylighting and electric lighting system analysis developed
at Ernest Orlando Lawrence Berkeley National Laboratory.

EnergyPlus v1.2.2 through v3.1 contained links to SPARK, a simulation
engine for detailed system modeling developed at Ernest Orlando Lawrence
Berkeley National Laboratory in conjunction with Ayres Sowell
Associates, Inc.

The airflow calculation portion of the EnergyPlus AirflowNetwork model
was based on AIRNET written by George Walton of the National Institute
for Standards and Technology (NIST), 100 Bureau Drive, Gaithersburg, MD
20899. The EnergyPlus AirflowNetwork model also includes portions of
stack effect and detailed large opening from an early version of COMIS
(Conjunction Of Multizone Infiltration Specialists) developed by a
multinational, multi-institutional effort under the auspices of the
International Energy Agency’s Buildings and Community Systems Agreement
working group focusing on multizone air flow modeling (Annex 23) and now
administered by the Swiss Federal Laboratories for Materials Testing and
Research (EMPA), Division 175, Uberlandstrasse 129, CH-8600 Dubendorf,
Switzerland.

The EnergyPlus model for displacement ventilation and cross-ventilation
(version v1.2 and later) was developed by Guilherme Carrilho da Graca
(Department of Mechanical and Aerospace Engineering, University of
California, San Diego and NaturalWorks) and Paul Linden (Department of
Mechanical and Aerospace Engineering, University of California, San
Diego).

The EnergyPlus models for UFAD served zones were developed by Anna Liu
and Paul Linden at the Department of Mechanical and Aerospace
Engineering, University of California, San Diego.

ASHRAE research project 1254-RP supported the development of the
following features first added in EnergyPlus v1.2.2:

-  DXSystem:AirLoop enhancements (valid as OA system equipment, new
   humidity control options)

-  New set point managers: SET POINT MANAGER:SINGLE ZONE HEATING, SET
   POINT MANAGER:SINGLE ZONE COOLING, and SET POINT MANAGER:OUTSIDE AIR
   PRETREAT

-  New 2-stage DX coil with enhanced dehumidification option:

   -  COIL:DX:MultiMode:CoolingEmpirical

-  Additional DESICCANT DEHUMIDIFIER:SOLID setpoint control option

American Society of Heating Refrigerating and Air-Conditioning
Engineers, Inc., 1791 Tullie Circle, N.E., Atlanta, GA 30329 [1]_. Work
performed by GARD Analytics, Inc., 115 S. Wilke Road, Suite 105,
Arlington Heights, IL, USA [2]_, November 2004. These items were renamed
in V3.0 to:

-  SetpointManager:SingleZone:Heating

-  SetpointManager:SingleZone:Cooling

-  SetpointManager:OutdoorAirPretreat

-  Coil:Cooling:DX:TwoStageWithHumidityControlMode

-  Dehumidifier:Desiccant:NoFans

The Ecoroof (Green Roof) model, first introduced in EnergyPlus v2.0, was
developed at Portland State University, by David Sailor and his
students. It is based on the FASST vegetation models developed by
Frankenstein and Koenig for the US Army Corps of Engineers.

The HAMT (Heat And Moisture Transfer) model, first introduced in
EnergyPlus v3.0.0 was developed by Phillip Biddulph, Complex Built
Environment Systems, The Bartlett School of Graduate Studies, University
College London, Gower Street, London WC1E 6BT, United Kingdom.
http://www.cbes.ucl.ac.uk/.

The SQLite output module, first introduced in EnergyPlus v3.0.0, was
developed by Gregory B. Stark, P.E., Building Synergies, LLC, 1860
Washington Street, Suite 208, Denver, Colorado 80203, United States.

Refrigeration compressor performance data and refrigeration practices
were provided by CDH Energy, Cazenovia, NY 12035.

The external interface was developed by Michael Wetter and Philip Haves
(Lawrence Berkeley National Laboratory) and by Rui Zhang (Carnegie
Mellon University). An earlier upgrade to a development version of
EnergyPlus 3.0 was implemented by Charles Corbin, Anthony Florita,
Gregor Henze and Peter May-Ostendorp (University of Colorado at
Boulder).

Various suggestions for time reduction, improved documentation and other
items have been incorporated from Autodesk, Inc., Bentley Systems, and
others.

Particular recognition goes to Noel Keen (LBNL Computational Research
Division) and Geof Sawaya (Oak Ridge National Laboratory fellow) who
have done extensive profiling and creation of time reduction features
that have gone into the code.

Second Law modified the WaterToAirHeatPump:EquationFit module to include
the variable “WaterCyclingMode”. This variable determines whether the
heat pump water flow is constant, whether it cycles with the compressor,
or whether it is constant when the heat pump is active. WaterFlowMode is
set by the HVAC wrapper object; either ZoneHVAC:WaterToAirHeatPump or
AirLoopHVAC:UnitaryHeatPump:WaterToAir. Second Law, Burlington, VT,
Karen Walkerman.

**NOTICE:** The U.S. Government is granted for itself and others acting
on its behalf a paid-up, nonexclusive, irrevocable, worldwide license in
this data to reproduce, prepare derivative works, and perform publicly
and display publicly. Beginning five (5) years after permission to
assert copyright is granted, subject to two possible five year renewals,
the U.S. Government is granted for itself and others acting on its
behalf a paid-up, non-exclusive, irrevocable worldwide license in this
data to reproduce, prepare derivative works, distribute copies to the
public, perform publicly and display publicly, and to permit others to
do so.

**TRADEMARKS:** EnergyPlus is a trademark of the US Department of
Energy.

Other Software included or referenced directly
----------------------------------------------

The following notices apply to those EnergyPlus distributions which
include the interface to BCVTB or Functional Mock-up Units.

ZLIB DATA COMPRESSION LIBRARY
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

zlib 1.2.5 is a general purpose data compression library.  All the code
is thread safe.  The data format used by the zlib library is described
by RFCs (Request for Comments) 1950 to 1952 in the files
http://www.ietf.org/rfc/rfc1950.txt (zlib format), rfc1951.txt (deflate
format) and rfc1952.txt (gzip format).

All functions of the compression library are documented in the file
zlib.h (volunteer to write man pages welcome, contact zlib@gzip.org).  A
usage example of the library is given in the file example.c which also
tests that the library is working correctly.  Another example is given
in the file minigzip.c.  The compression library itself is composed of
all source files except example.c and minigzip.c.

To compile all files and run the test program, follow the instructions
given at the top of Makefile.in.  In short “./configure; make test”, and
if that goes well, “make install” should work for most flavors of Unix. 
For Windows, use one of the special makefiles in win32/ or
contrib/vstudio/ .  For VMS, use make_vms.com.  Questions about zlib
should be sent to <zlib@gzip.org>, or to Gilles Vollant
<info@winimage.com> for the Windows DLL version.  The zlib home page is
http://zlib.net/ .  Before reporting a problem, please check this site
to verify that you have the latest version of zlib; otherwise get the
latest version and check whether the problem still exists or not.

PLEASE read the zlib FAQ http://zlib.net/zlib_faq.html before asking for
help.

Mark Nelson <markn@ieee.org> wrote an article about zlib for the Jan. 
1997 issue of Dr.  Dobb’s Journal; a copy of the article is available
online [3]_.

The changes made in version 1.2.5 are documented in the file ChangeLog.

Unsupported third party contributions are provided in directory contrib/
.

zlib is available in Java using the java.util.zip package, documented
online [4]_.

A Perl interface to zlib written by Paul Marquess <pmqs@cpan.org> is
available at CPAN (Comprehensive Perl Archive Network) sites, including
http://search.cpan.org/pmqs/IO-Compress-Zlib/.

A Python interface to zlib written by A.M. Kuchling <amk@amk.ca> is
available in Python 1.5 and later versions, see
http://www.python.org/doc/lib/module-zlib.html.

zlib is built into tcl: http://wiki.tcl.tk/4610 .

An experimental package to read and write files in .zip format, written
on top of zlib by Gilles Vollant <info@winimage.com>, is available in
the contrib/minizip directory of zlib.

Notes for some targets:

-  For Windows DLL versions, please see win32/DLL_FAQ.txt

-  For 64-bit Irix, deflate.c must be compiled without any optimization.
   With -O, one libpng test fails. The test works in 32 bit mode (with
   the -n32   compiler flag). The compiler bug has been reported to SGI.

-  zlib doesn’t work with gcc 2.6.3 on a DEC 3000/300LX under OSF/1 2.1
   it works when compiled with cc.

-  On Digital Unix 4.0D (formely OSF/1) on AlphaServer, the cc option
   -std1 is necessary to get gzprintf working correctly. This is done by
   configure.

-  zlib doesn’t work on HP-UX 9.05 with some versions of /bin/cc. It
   works with other compilers. Use “make test” to check your compiler.

-  gzdopen is not supported on RISCOS or BEOS.

-  For PalmOs, see http://palmzlib.sourceforge.net/

Acknowledgments:

  The deflate format used by zlib was defined by Phil Katz.  The deflate
and zlib specifications were written by L.  Peter Deutsch.  Thanks to
all the people who reported problems and suggested various improvements
in zlib; they are too numerous to cite here.

Copyright notice:

 (C) 1995-2010 Jean-loup Gailly and Mark Adler

This software is provided ‘as-is’, without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

#. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

#. Altered source versions must be plainly marked as such, and must not
   be misrepresented as being the original software.

#. This notice may not be removed or altered from any source
   distribution.

  Jean-loup Gailly        Mark Adler

  jloup@gzip.org          madler@alumni.caltech.edu

 

If you use the zlib library in a product, we would appreciate \*not\*
receiving lengthy legal documents to sign.  The sources are provided for
free but without warranty of any kind.  The library has been entirely
written by Jean-loup Gailly and Mark Adler; it does not include
third-party code.

If you redistribute modified sources, we would appreciate that you
include in the file ChangeLog history information documenting your
changes.  Please read the FAQ for more information on the distribution
of modified source versions.

ExPat Copying Notice
~~~~~~~~~~~~~~~~~~~~

Copyright (c) 1998, 1999, 2000 Thai Open Source Software Center Ltd

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
“Software”), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

FMU SDK license
~~~~~~~~~~~~~~~

Copyright (c) 2008-2011, QTronic GmbH. All rights reserved.

The FmuSdk is licensed by the copyright holder under the BSD
License [5]_: Redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following
conditions are met:

-  Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

-  Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY QTRONIC GMBH “AS IS” AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL QTRONIC GMBH BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Minizip Notice
~~~~~~~~~~~~~~

Condition of use and distribution are the same than zlib :

This software is provided ‘as-is’, without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

#. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

#. Altered source versions must be plainly marked as such, and must not
   be misrepresented as being the original software.

#. This notice may not be removed or altered from any source
   distribution.

MODELISAR Notice
~~~~~~~~~~~~~~~~

Copyright (c) 2008-2010, MODELISAR consortium. All rights reserved.

This file is licensed by the copyright holders under the BSD
License [6]_:

Redistribution and use in source and binary forms, with or
withoutmodification, are permitted provided that the following
conditions are met:

-  Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

-  Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

-  Neither the name of the copyright holders nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS
IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Functional Mock-up Unit for Co-Simulation Import Notice
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Functional Mock-up Unit for Co-Simulation Import in EnergyPlus Copyright
(c) 2012, The Regents of the University of California, through Lawrence
Berkeley National Laboratory (subject to receipt of any required
approvals from the U.S. Dept. of Energy). All rights reserved.

If you have questions about your rights to use or distribute this
software, please contact Berkeley Lab’s Technology Transfer Department
at <TTD@lbl.gov>.

NOTICE.  This software was developed under partial funding from the U.S.
Department of Energy.  As such, the U.S. Government has been granted for
itself and others acting on its behalf a paid-up, nonexclusive,
irrevocable, worldwide license in the Software to reproduce, prepare
derivative works, and perform publicly and display publicly.  Beginning
five (5) years after the date permission to assert copyright is obtained
from the U.S. Department of Energy, and subject to any subsequent five
(5) year renewals, the U.S. Government is granted for itself and others
acting on its behalf a paid-up, nonexclusive, irrevocable, worldwide
license in the Software to reproduce, prepare derivative works,
distribute copies to the public, perform publicly and display publicly,
and to permit others to do so.

Modified BSD License agreement

Functional Mock-up Unit for Co-Simulation Import in EnergyPlus Copyright
(c) 2012, The Regents of the University of California, through Lawrence
Berkeley National Laboratory (subject to receipt of any required
approvals from the U.S. Dept. of Energy).  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

Neither the name of the University of California, Lawrence Berkeley
National Laboratory, U.S. Dept. of Energy nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS
IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

You are under no obligation whatsoever to provide any bug fixes,
patches, or upgrades to the features, functionality or performance of
the source code (“Enhancements”) to anyone; however, if you choose to
make your Enhancements available either publicly, or directly to
Lawrence Berkeley National Laboratory, without imposing a separate
written license agreement for such Enhancements, then you hereby grant
the following license: a non-exclusive, royalty-free perpetual license
to install, use, modify, prepare derivative works, incorporate into
other computer software, distribute, and sublicense such enhancements or
derivative works thereof, in binary and source code form.

Functional Mock-up Unit Export of EnergyPlus Notice
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Functional Mock-up Unit Export of EnergyPlus (c)2013, The Regents of the
University of California, through Lawrence Berkeley National Laboratory
(subject to receipt of any required approvals from the U.S. Department
of Energy). All rights reserved.

If you have questions about your rights to use or distribute this
software, please contact Berkeley Lab’s Technology Transfer Department
at <TTD@lbl.gov>.referring to “Functional Mock-up Unit Export of
EnergyPlus (LBNL Ref 2013-088)”.

NOTICE: This software was produced by The Regents of the University of
California under Contract No. DE-AC02-05CH11231 with the Department of
Energy.

For 5 years from November 1, 2012, the Government is granted for itself
and others acting on its behalf a nonexclusive, paid-up, irrevocable
worldwide license in this data to reproduce, prepare derivative works,
and perform publicly and display publicly, by or on behalf of the
Government. There is provision for the possible extension of the term of
this license.

Subsequent to that period or any extension granted, the Government is
granted for itself and others acting on its behalf a nonexclusive,
paid-up, irrevocable worldwide license in this data to reproduce,
prepare derivative works, distribute copies to the public, perform
publicly and display publicly, and to permit others to do so. The
specific term of the license can be identified by inquiry made to
Lawrence Berkeley National Laboratory or DOE. Neither the United States
nor the United States Department of Energy, nor any of their employees,
makes any warranty, express or implied, or assumes any legal liability
or responsibility for the accuracy, completeness, or usefulness of any
data, apparatus, product, or process disclosed, or represents that its
use would not infringe privately owned rights.

Copyright (c) 2013, The Regents of the University of California,
Department of Energy contract-operators of the Lawrence Berkeley
National Laboratory.

All rights reserved.

#. Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

#. Redistributions of source code must retain the copyright notice, this
   list of conditions and the following disclaimer.

#. Redistributions in binary form must reproduce the copyright notice,
   this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

#. Neither the name of the University of California, Lawrence Berkeley
   National Laboratory, U.S. Dept. of Energy nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

#. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   “AS IS”

AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#. You are under no obligation whatsoever to provide any bug fixes,
   patches, or upgrades to the features, functionality or performance of
   the source code (“Enhancements”) to anyone; however, if you choose to
   make your Enhancements available either publicly, or directly to
   Lawrence Berkeley National Laboratory, without imposing a separate
   written license agreement for such Enhancements, then you hereby
   grant the following license: a non-exclusive, royalty-free perpetual
   license to install, use, modify, prepare derivative works,
   incorporate into other computer software, distribute, and sublicense
   such enhancements or derivative works thereof, in binary and source
   code form.

NOTE: This license corresponds to the “revised BSD” or “3-clause BSD”

License and includes the following modification: Paragraph 3. has been
added.

Kiva License
~~~~~~~~~~~~

Copyright (c) 2012-2018 Big Ladder Software LLC. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

#. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

#. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

#. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission from the
   copyright holders.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Other Acknowledgments
---------------------

This work was supported by the Assistant Secretary for Energy Efficiency
and Renewable Energy, Office of Building Technologies of the US
Department of Energy – under the guidance of Dru Crawley from inception
through March 2010.

Additional support was provided by the Gas Technology Institute and the
California Energy Commission.

The ice thermal storage module development was supported by the U.S.
Department of Energy Office of Electricity Delivery and Energy
Reliability.

The HAMT (Heat And Moisture Transfer) model was supported by the
Engineering and Physical Sciences Research Council (EPSRC), the UK
government agency for funding research and training in engineering and
the physical sciences.

The SQLite output module was funded by Building Synergies, LLC and was
made possible by inclusion of software code from the SQLite project
(http://www.sqlite.org/).

The building-integrated photovoltaic thermal (BIPVT) model was
contributed by CanmetENERGY-Ottawa, Natural Resources Canada
(https://www.nrcan.gc.ca/energy/offices-labs/canmet/ottawa-research-centre/5753).
See SolarCollectorPerformance:PhotovoltaicThermal:BIPVT and Pull Request
9594 (https://github.com/NREL/EnergyPlus/pull/9594).

The fuel-fired absorption heat pump
(HeatPump:AirToWater:FuelFired:Heating and Cooling) model development
was funded by Utilization Technology Development
(https://www.utd-co.org) on a subcontract with GTI Energy
(https://www.gti.energy/). See Pull Request 9405
(https://github.com/NREL/EnergyPlus/pull/9405).

.. [1]
   http://www.ashrae.org/

.. [2]
   urlinfo@gard.com; http://www.gard.com/

.. [3]
   http://marknelson.us/1997/01/01/zlib-engine/

.. [4]
   http://java.sun.com/developer/technicalArticles/Programming/compression/

.. [5]
   http://www.opensource.org/licenses/bsd-license.html

.. [6]
   `http://www.opensource.org/licenses/bsd-license.html\\textgreater <http://www.opensource.org/licenses/bsd-license.html\textgreater>`__
