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

import re
from pathlib import Path

from base_hook import (
    ROOT_DIR,
    ErrorMessage,
    LogLevel,
    LogMessage,
    collect_files,
    exit_hook,
    flatten_list_of_lists,
    get_base_parser,
    parallel_apply,
    report_log_messages,
)

STATE_HEADER_PATH = ROOT_DIR / "src" / "EnergyPlus" / "Data" / "EnergyPlusData.hh"
STATE_SOURCE_PATH = ROOT_DIR / "src" / "EnergyPlus" / "Data" / "EnergyPlusData.cc"


class StateClass:
    def __init__(self, struct_name: str, instance_name: str):
        self.struct_name = struct_name
        self.instance_name = instance_name
        self.constructed = False
        self.construction_call_matches_struct = False
        self.clear_state_executed = False

    def validate(self) -> list[LogMessage]:
        """
        Checks all the identified conditions are met and if not issues warnings.

        :return: Returns True if the state class is validated or False if not
        """
        log_messages: list[LogMessage] = []
        if not self.constructed:
            log_messages.append(
                ErrorMessage(
                    tool="analyze_state",
                    filepath=STATE_SOURCE_PATH,
                    message=f"State member variable not constructed '{self.instance_name}'",
                )
            )

        if not self.construction_call_matches_struct:
            log_messages.append(
                ErrorMessage(
                    tool="analyze_state",
                    filepath=STATE_SOURCE_PATH,
                    message=f"State member variable constructed with different type '{self.instance_name}'",
                )
            )

        if not self.clear_state_executed:
            log_messages.append(
                ErrorMessage(
                    tool="analyze_state",
                    filepath=STATE_SOURCE_PATH,
                    message=f"State clear_state() call missing '{self.instance_name}'",
                )
            )

        return log_messages


class StateChecker:

    def __init__(self) -> None:
        self.member_variables: set[StateClass] = set()

        # read the state file contents -- if we ever split the data into files this will require modification
        self.header_file_lines: list[str] = STATE_HEADER_PATH.read_text().splitlines()
        self.source_file_lines: list[str] = STATE_SOURCE_PATH.read_text().splitlines()

    def determine_member_variables(self) -> None:
        """
        Finds all member variables using a pretty simplistic clue, the unique_ptr declaration.
        This would change if we ever go to raw pointers or another storage method.
        Currently it looks for lines of the form:
             std::unique_ptr<AirflowNetworkBalanceManagerData> dataAirflowNetworkBalanceManager;
        """
        pattern = re.compile(r"\s*std::unique_ptr<(\w+)> (\w+);")
        for li in self.header_file_lines:
            m = pattern.match(li)
            if m:
                underlying_struct_name = m.group(1)
                member_var_name = m.group(2)
                self.member_variables.add(StateClass(underlying_struct_name, member_var_name))

    def validate_member_variable_construction(self):
        """
        Validates that the state member variables are constructed using a clue of the `make_unique` function.
        """
        pattern = re.compile(r"\s*this->(\w+) = std::make_unique<(\w+)>\(\);")
        for li in self.source_file_lines:
            if li.strip().startswith("#"):
                continue
            m = pattern.match(li)
            if m:
                member_var_name = m.group(1)
                underlying_struct_name = m.group(2)
                for v in self.member_variables:
                    if v.instance_name == member_var_name:
                        v.constructed = True
                        if v.struct_name == underlying_struct_name:
                            v.construction_call_matches_struct = True

    def validate_member_variable_clear_state(self):
        """
        Validates the member's clear_state call is made in an uncommented line
        """
        pattern = re.compile(r"\s*this->(\w+)->clear_state\(\);")
        for li in self.source_file_lines:
            if li.strip().startswith("#"):
                continue
            m = pattern.match(li)
            if m:
                member_var_name = m.group(1)
                for v in self.member_variables:
                    if v.instance_name == member_var_name:
                        v.clear_state_executed = True


if __name__ == "__main__":
    parser = get_base_parser(description="Analyze State Variables", include_files_arg=False)
    args = parser.parse_args()

    sc = StateChecker()
    sc.determine_member_variables()
    sc.validate_member_variable_construction()
    sc.validate_member_variable_clear_state()
    all_good = True
    log_messages: list[LogMessage] = []
    for mv in sc.member_variables:
        log_messages += mv.validate()

    success = report_log_messages(log_messages=log_messages, fail_threshold=LogLevel.ERROR, verbose=args.verbose)
    exit_hook(success=success)
