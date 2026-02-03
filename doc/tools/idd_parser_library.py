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

# python 2/3 compatibility imports
from __future__ import absolute_import, print_function, unicode_literals

import io


class iddFile(object):
    def __init__(self, file_name):

        # phase 0: read in lines of file
        with io.open(file_name, "r", encoding="latin-1") as fo:
            lines = fo.readlines()

        # phases 1 and 2: remove comments and blank lines
        lines_a = []
        self.comments = []
        line_num = 0
        for line in lines:
            line_num += 1
            line_text = line.strip()
            this_line = ""
            if len(line_text) > 0:
                exclamation = line_text.find("!")
                slash = line_text.find("\\")
                comment_point = -1
                if exclamation == -1 and slash == -1:
                    comment_point = -1
                elif exclamation > -1 and slash == -1:
                    comment_point = exclamation
                elif exclamation == -1 and slash > -1:
                    comment_point = slash
                elif exclamation > -1 and slash > -1:
                    comment_point = min(slash, exclamation)
                if comment_point == -1:
                    this_line = line_text
                elif comment_point == 0:
                    this_line = ""
                    self.comments.append(line_text)
                elif comment_point > 0:
                    this_line = line_text[:comment_point]
                    self.comments.append(line_text[comment_point + 1 :])
                if not this_line == "":
                    lines_a.append(this_line)

        # intermediates: join entire array and re-split by semicolon
        idd_data_joined = "".join(lines_a)
        idd_object_strings = idd_data_joined.split(";")

        # phase 3: inspect each object and get it's name
        self.iddObjects = []
        for idf_object in idd_object_strings:
            tokens = idf_object.split(",")
            nice_object = [t.strip() for t in tokens]
            if len(nice_object) == 1:
                if nice_object[0] == "":
                    continue
            self.iddObjects.append(nice_object[0].upper())
