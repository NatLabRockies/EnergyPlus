# EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University
# of Illinois, The Regents of the University of California, through Lawrence
# Berkeley National Laboratory (subject to receipt of any required approvals
# from the U.S. Dept. of Energy), Oak Ridge National Laboratory, managed by UT-
# Battelle, Alliance for Sustainable Energy, LLC, and other contributors. All
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

import os
import re


def add_fuel_type(filename):
    assert os.path.isfile(filename)
    with open(filename, "rU") as f:
        filecontents = f.read()

    def repl(m):
        if m.group(2) == "Coil:Heating:Gas":
            try:
                itemlist = m.group(3).split(",")
                pre_whitespace = re.match(r"\s*", itemlist[1].split("\n")[1]).group()
                width = len(itemlist[1].split("\n")[1]) + 1 + len(re.match(r"\s*", itemlist[2]).group())
                width -= len(pre_whitespace) + len("NaturalGas,")
                post_whitespace = " " * width
                pre, post = itemlist[2].split("\n")
                itemlist[2] = "{}\n{}NaturalGas,{}!- FuelType\n{}".format(pre, pre_whitespace, post_whitespace, post)

                group3 = ",".join(itemlist)
                return m.group(1) + m.group(2) + "," + group3 + ";"
            except:
                return m.group()
        else:
            return m.group()

    newfilecontents = re.sub(r"(\s*)([\w:]+),(.*?);", repl, filecontents, flags=re.DOTALL)

    with open(filename, "w") as f:
        f.write(newfilecontents)


def main():
    this_dir = os.path.dirname(os.path.abspath(__file__))
    example_file_dir = os.path.abspath(os.path.join(this_dir, "..", "..", "testfiles"))
    for dirpath, dirnames, filenames in os.walk(example_file_dir):
        print(dirpath)
        for filename in filenames:
            if filename.endswith(".idf"):
                print("  {filename}".format())
                add_fuel_type(os.path.join(dirpath, filename))


if __name__ == "__main__":
    main()
