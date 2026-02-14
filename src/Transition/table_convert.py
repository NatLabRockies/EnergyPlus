# EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the
# University of Illinois, The Regents of the University of California, through
# Lawrence Berkeley National Laboratory (subject to receipt of any required
# approvals from the U.S. Dept. of Energy), Oak Ridge National Laboratory,
# managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
# contributors. All rights reserved.
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

import sys
from os import path


def convert2D(file):

    output = ""
    with open(file, "r") as f:
        ns = str(f.read()).split(",")
        num_vals = int(len(ns) / 3)
        ind_vars = [[], []]
        vals = []
        output += "Independent Variable 1, Indpendent Variable 2, Value\n"
        for i in range(num_vals):
            pos = i * 3
            ind_vars[0].append(float(ns[pos]))
            ind_vars[1].append(float(ns[pos + 1]))
            vals.append(float(ns[pos + 2]))
            output += f"{ind_vars[0][i]}, {ind_vars[1][i]}, {vals[i]}\n"
    return output


def convertND(file, ascending=[True, True]):
    output = ""
    with open(file, "r") as f:
        pos = 0
        ns = str(f.read()).split(",")
        N = int(ns[pos])
        pos += 1
        dims = []
        for i in range(N):
            dims.append(int(ns[pos]))
            pos += 1
        ind_vars = [[] for i in range(N)]
        step_size = [0] * N
        for i, dim in enumerate(dims):
            for j in range(dim):
                ind_vars[i].append(float(ns[pos]))
                pos += 1
        num_vals = dims[0]
        mini_table_size = dims[0]
        ref_index = pos
        curr_indices = [0] * N
        incr = [1] * N

        if not ascending[0]:
            incr[0] = -1

        ind_var_list = [[] for i in range(N)]
        for i in range(1, N):
            if i == 1:
                mini_table_size *= dims[i]
                if not ascending[i]:
                    incr[i] = -1
            num_vals *= dims[i]

        for i in range(N):
            ind_var_list[i] = [0.0] * num_vals

        step_size[N - 1] = 1
        for i in range(N - 2, -1, -1):
            step_size[i] = step_size[i + 1] * dims[i + 1]

        num_mini_tables = int(num_vals / mini_table_size)

        vals = [0.0] * num_vals
        for i in range(num_mini_tables):
            ref_index += max(0, N - 2)
            for j in range(mini_table_size):
                flat_index = 0
                for k in range(N):
                    flat_index += curr_indices[k] * step_size[k]
                vals[flat_index] = float(ns[ref_index + j])
                for k in range(N):
                    ind_var_list[k][flat_index] = ind_vars[k][curr_indices[k]]
                # increment indices
                curr_indices[0] += incr[0]
                for k in range(N - 1):
                    if curr_indices[k] == -1 or curr_indices[k] > dims[k] - 1:
                        curr_indices[k + 1] = curr_indices[k + 1] + incr[k + 1]
                        if curr_indices[k] == -1:
                            curr_indices[k] = dims[k] - 1
                        elif curr_indices[k] > dims[k] - 1:
                            curr_indices[k] = 0
            ref_index += mini_table_size

        output += ", ".join([f"Independent Variable {i+1}" for i in range(N)]) + ", Value\n"
        for i in range(num_vals):
            output += ", ".join([f"{ind_var_list[j][i]}" for j in range(N)]) + f", {vals[i]}\n"
    return output


print("table_convert.py")
if len(sys.argv) < 3:
    print("  Error: Specify the file name and table type ('2' or 'N') to run the program.\n")
    print("    e.g.,")
    print("      'python table_convert.py 2D.csv 2'")
    print("      'python table_convert.py 3D.csv N'")
    exit()

input_path = sys.argv[1]
table_type = sys.argv[2]

if not path.exists(input_path):
    print(f"  Error: Could not find input file: {input_path}.")
    exit()

if table_type not in ["2", "N", "n"]:
    print(f"  Error: Invalid table type: {table_type}.\n")
    print("    Acceptable types are:")
    print("      '2' (Table:TwoIndpendentVariables), and")
    print("      'N' (Table:MultidimensionalLookup)")
    exit()

order = [True, True]
if len(sys.argv) > 3:
    if len(sys.argv) > 5 or table_type == "2":
        max_args = 2 if (table_type == "2") else 4
        print(f"  Error: Too many arguments provided for table type = {table_type}. Maximum = {max_args}:\n")
        print("    1: Input table path")
        print("    2: Table type ['2' (Table:TwoIndpendentVariables) or 'N' (Table:MultidimensionalLookup)]")
        print(
            "    3: Order of 1st independent variable ['A' (Ascending) or 'D' (Descending)]. Only for table type = 'N'"
        )
        print(
            "    4: Order of 2nd independent variable ['A' (Ascending) or 'D' (Descending)]. Only for table type = 'N'"
        )
        exit()

    if len(sys.argv) == 4:
        print(f"  Error: Must provide order for both 1st and 2nd independent variables.")
        exit()

    order_in = [sys.argv[3], sys.argv[4]]

    if order_in[0] not in ["A", "a", "D", "d"]:
        print(f"  Error: Invalid order for 1st independent variable: {order_in[0]}.\n")
        print("    Acceptable entries are:")
        print("      'A' (Ascending), and")
        print("      'D' (Descending)")
        exit()

    if order_in[1] not in ["A", "a", "D", "d"]:
        print(f"  Error: Invalid order for 2nd independent variable: {order_in[1]}.\n")
        print("    Acceptable entries are:")
        print("      'A' (Ascending), and")
        print("      'D' (Descending)")
        exit()

    order = [x in ["A", "a"] for x in order_in]

output_path = path.splitext(path.basename(input_path))[0] + "-New.csv"

with open(output_path, "w") as f:
    if table_type == "2":
        f.write(convert2D(input_path))
    else:
        f.write(convertND(input_path, order))

print(f"  Conversion complete! Output written to: {output_path}")
