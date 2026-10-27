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


def run_transition(prepare_and_run_transition, idf_text: str, expected_idf_text: str):
    prepare_and_run_transition(
        ori_idf_text=idf_text,
        expected_idf_text=expected_idf_text,
        version_ori=(26, 2, 0),
        version_new=(27, 1, 0),
    )


def test_zone_control_thermostat_without_cutout_inserts_blank_before_all_control_pairs(prepare_and_run_transition):
    idf_text = """
  ZoneControl:Thermostat,
    Four Controls No Cutout,
    Zone One,
    Control Schedule,
    ThermostatSetpoint:SingleHeating,
    Heating Setpoint,
    ThermostatSetpoint:SingleCooling,
    Cooling Setpoint,
    ThermostatSetpoint:SingleHeatingOrCooling,
    Combined Setpoint,
    ThermostatSetpoint:DualSetpoint,
    Dual Setpoint;
    """

    expected_idf_text = """
  ZoneControl:Thermostat,
    Four Controls No Cutout, !- Name
    Zone One,                !- Zone or ZoneList Name
    Control Schedule,        !- Control Type Schedule Name
    ,                        !- Temperature Difference Between Cutout And Setpoint {deltaC}
    ThermostatSetpoint:SingleHeating,  !- Control 1 Object Type
    Heating Setpoint,        !- Control 1 Name
    ThermostatSetpoint:SingleCooling,  !- Control 2 Object Type
    Cooling Setpoint,        !- Control 2 Name
    ThermostatSetpoint:SingleHeatingOrCooling,  !- Control 3 Object Type
    Combined Setpoint,       !- Control 3 Name
    ThermostatSetpoint:DualSetpoint,  !- Control 4 Object Type
    Dual Setpoint;           !- Control 4 Name
"""

    run_transition(prepare_and_run_transition, idf_text, expected_idf_text)


def test_zone_control_thermostat_moves_legacy_cutout_before_all_control_pairs(prepare_and_run_transition):
    idf_text = """
  ZoneControl:Thermostat,
    Four Controls With Cutout,
    Zone Two,
    Control Schedule,
    ThermostatSetpoint:SingleHeating,
    Heating Setpoint,
    ThermostatSetpoint:SingleCooling,
    Cooling Setpoint,
    ThermostatSetpoint:SingleHeatingOrCooling,
    Combined Setpoint,
    ThermostatSetpoint:DualSetpoint,
    Dual Setpoint,
    1.25;
    """

    expected_idf_text = """
  ZoneControl:Thermostat,
    Four Controls With Cutout,  !- Name
    Zone Two,                !- Zone or ZoneList Name
    Control Schedule,        !- Control Type Schedule Name
    1.25,                    !- Temperature Difference Between Cutout And Setpoint {deltaC}
    ThermostatSetpoint:SingleHeating,  !- Control 1 Object Type
    Heating Setpoint,        !- Control 1 Name
    ThermostatSetpoint:SingleCooling,  !- Control 2 Object Type
    Cooling Setpoint,        !- Control 2 Name
    ThermostatSetpoint:SingleHeatingOrCooling,  !- Control 3 Object Type
    Combined Setpoint,       !- Control 3 Name
    ThermostatSetpoint:DualSetpoint,  !- Control 4 Object Type
    Dual Setpoint;           !- Control 4 Name
"""

    run_transition(prepare_and_run_transition, idf_text, expected_idf_text)
