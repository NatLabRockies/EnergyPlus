# EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University
# of Illinois, The Regents of the University of California, through Lawrence
# Berkeley National Laboratory (subject to receipt of any required approvals
# from the U.S. Dept. of Energy), Oak Ridge National Laboratory, managed by UT-
# Battelle, Alliance for Energy Innovation, LLC, and other contributors. All
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

import json
import multiprocessing
import sys
from os import path

import idd_parser
import modify_schema


def generate_epJSON_schema(source_dir_path: str):

    print("Generating the epJSON schema!", file=sys.stderr)
    data = idd_parser.Data()
    idd_path = path.join(source_dir_path, "Energy+.idd")
    if path.exists(idd_path):
        with open(idd_path, "r") as f:
            data.file = f.read()
    else:
        # this script is also used in the sphinx documentation, which doesn't do a CMake configuration run
        # so the runtime/Products/Energy+.idd file is not generated.  The script is just executed on the raw
        # Energy+.idd.in file in the idd folder.  So try to find the .in file if we couldn't find the
        # generated idd file.
        idd_in_path = path.join(source_dir_path, "Energy+.idd.in")
        if path.exists(idd_in_path):
            with open(idd_in_path, "r") as f:
                data.file = f.read()
        else:
            print(f"Could not find E+ IDD, looked for both: {idd_path} and {idd_in_path}.  Aborting")
            sys.exit(1)

    idd_parser.parse_idd(data)
    modify_schema.change_schedule_compact(data.schema)
    modify_schema.change_utility_cost(data.schema)
    modify_schema.change_special_cased_enums(data.schema)
    modify_schema.change_special_cased_name_fields(data.schema)
    modify_schema.change_extensions_name(data.schema)
    modify_schema.change_89_release_issues(data.schema)
    modify_schema.add_explicit_extensible_bounds(data.schema)

    with open(path.join(source_dir_path, "Energy+.schema.epJSON"), "w") as f2:
        f2.write(json.dumps(data.schema, indent=4))


if __name__ == "__main__":
    p = multiprocessing.Process(target=generate_epJSON_schema, args=(sys.argv[1],))
    p.start()

    timeout_seconds = 60
    p.join(timeout=timeout_seconds)

    # If thread is still active
    if p.is_alive():
        print("Timeout, something is wrong with the IDD")
        p.terminate()
        p.join()
        sys.exit(1)

    sys.exit(p.exitcode)
