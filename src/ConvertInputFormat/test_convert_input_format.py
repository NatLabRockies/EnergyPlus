#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

"""
Test ConvertInputFormat.

This script aims to check that the ConvertInputFormat exe correctly
handles periods in the name of the IDF.

cf: https://github.com/NatLabRockies/EnergyPlus/issues/9419
"""

import argparse
import shutil
import subprocess
from pathlib import Path


def parse_args():
    """Set up argument parser."""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--convertinputformat-exe",
        required=True,
        type=Path,
        help="Path to the ConvertInputFormat exe",
    )
    parser.add_argument(
        "--out-dir",
        required=True,
        type=Path,
        help="Output directory where to output the epJSON",
    )
    parser.add_argument(
        "--input-file",
        required=True,
        type=Path,
        help="Path to the file to convert",
    )
    parser.add_argument(
        "--expected-filename",
        required=True,
        type=Path,
        help="Path to the file we expect to be created",
    )

    args = parser.parse_args()
    return args


def clean_up_directory(out_dir: Path):
    """Delete and remake directory."""
    shutil.rmtree(out_dir, ignore_errors=True)
    out_dir.mkdir(parents=True, exist_ok=True)


def run_convertinputformat(convertinputformat_exe: Path, out_dir: Path, input_file: Path):
    """Run the program."""
    if not convertinputformat_exe.exists():
        raise ValueError(f"ConvertInputFormat exe '{convertinputformat_exe}' does not exist")

    if not out_dir.exists() or not out_dir.is_dir():
        raise ValueError(f"out_dir '{out_dir}' is not a valid directory")

    if not input_file.exists() or not input_file.is_file():
        raise ValueError(f"input_file '{input_file}' is not a valid file")

    full_command_args = [convertinputformat_exe, "--output", out_dir, input_file]
    command_as_strings = [str(x) for x in full_command_args]
    print(f"Running: {' '.join(command_as_strings)}")
    subprocess.check_call(command_as_strings)


if __name__ == "__main__":

    args = parse_args()
    print(args)

    clean_up_directory(out_dir=args.out_dir)
    run_convertinputformat(
        convertinputformat_exe=args.convertinputformat_exe,
        out_dir=args.out_dir,
        input_file=args.input_file,
    )
    out_file = args.out_dir / args.expected_filename
    if not out_file.exists() or not out_file.is_file():
        raise ValueError(f"Test Failed: output file '{out_file}' does not exist!")
