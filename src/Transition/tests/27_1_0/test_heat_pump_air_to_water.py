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


def test_heat_pump_air_to_water_inserts_blank_flow_mode_after_minimum_part_load_ratio(prepare_and_run_transition):
    idf_text = """
  HeatPump:AirToWater,
    test_AWHP,               !- Name
    ,                        !- Availability Schedule Name Heating
    ,                        !- Availability Schedule Name Cooling
    Load,                    !- Operating Mode Control Method
    SingleMode,              !- Operating Mode Control Option for Multiple Unit
    ,                        !- Operating Mode Control Schedule Name
    ,                        !- Minimum Part Load Ratio
    20,                      !- Rated Inlet Air Temperature in Heating Mode {C}
    autosize,                !- Rated Air Flow Rate in Heating Mode {m3/s}
    50,                      !- Rated Leaving Water Temperature in Heating Mode {C}
    autosize,                !- Rated Water Flow Rate in Heating Mode {m3/s}
    -20,                     !- Minimum Outdoor Air Temperature in Heating Mode {C}
    25,                      !- Maximum Outdoor Air Temperature in Heating Mode {C}
    MinHeatingSWTvsOAT,      !- Minimum Leaving Water Temperature Curve Name in Heating Mode
    MaxHeatingSWTvsOAT,      !- Maximum Leaving Water Temperature Curve Name in Heating Mode
    1.0,                     !- Sizing Factor for Heating
    25,                      !- Rated Inlet Air Temperature in Cooling Mode {C}
    20,                      !- Rated Air Flow Rate in Cooling Mode {m3/s}
    22,                      !- Rated Leaving Water Temperature in Cooling Mode {C}
    autosize,                !- Rated Water Flow Rate in Cooling Mode {m3/s}
    18,                      !- Minimum Outdoor Air Temperature in Cooling Mode {C}
    40,                      !- Maximum Outdoor Air Temperature in Cooling Mode {C}
    MinCoolingSWTvsOAT,      !- Minimum Leaving Water Temperature Curve Name in Cooling Mode
    MaxCoolingSWTvsOAT,      !- Maximum Leaving Water Temperature Curve Name in Cooling Mode
    0.9,                     !- Sizing Factor for Cooling
    Outdoor Air Inlet Node,  !- Air Inlet Node Name
    Outdoor Air Outlet Node, !- Air Outlet Node Name
    Heating Coil Load Loop Intermediate Node,  !- Hot Water Inlet Node Name
    Heating Coil Load Loop Supply Outlet Node,  !- Hot Water Outlet Node Name
    Cooling Coil Load Loop Intermediate Node,  !- Chilled Water Inlet Node Name
    Cooling Coil Load Loop Supply Outlet Node,  !- Chilled Water Outlet Node Name
    ,                        !- Maximum Outdoor Dry Bulb Temperature For Defrost Operation
    Timed,                   !- Heat Pump Defrost Control
    0.2,                     !- Heat Pump Defrost Time Period Fraction
    150,                     !- Resistive Defrost Heater Capacity {W}
    ,                        !- Defrost Energy Input Ratio Function of Temperature Curve Name
    2,                       !- Heat Pump Multiplier
    FixedSpeed,              !- Control Type
    100,                     !- Crankcase Heater Capacity {W}
    CrankCaseHeaterPowerCurve,  !- Crankcase Heater Capacity Function of Temperature Curve Name
    5,                       !- Maximum Ambient Temperature for Crankcase Heater Operation {C}
    2,                       !- Number of Speeds for Heating
    20000,                   !- Rated Heating Capacity at Speed 1 {W}
    3.49,                    !- Rated COP for Heating at Speed 1 {W/W}
    CapCurveFuncTempHT,      !- Normalized Heating Capacity Function of Temperature Curve Name at Speed 1
    EIRCurveFuncTempHT,      !- Heating Energy Input Ratio Function of Temperature Curve Name at Speed 1
    EIRCurveFuncPLR,         !- Heating Energy Input Ratio Function of PLR Curve Name at Speed 1
    40000,                   !- Rated Heating Capacity at Speed 2 {W}
    3.51,                    !- Rated COP for Heating at Speed 2 {W/W}
    CapCurveFuncTempHT2,     !- Normalized Heating Capacity Function of Temperature Curve Name at Speed 2
    EIRCurveFuncTempHT2,     !- Heating Energy Input Ratio Function of Temperature Curve Name at Speed 2
    EIRCurveFuncPLR2,        !- Heating Energy Input Ratio Function of PLR Curve Name at Speed 2
    ,                        !- Rated Heating Capacity at Speed 3 {W}
    ,                        !- Rated COP for Heating at Speed 3 {W/W}
    ,                        !- Normalized Heating Capacity Function of Temperature Curve Name at Speed 3
    ,                        !- Heating Energy Input Ratio Function of Temperature Curve Name at Speed 3
    ,                        !- Heating Energy Input Ratio Function of PLR Curve Name at Speed 3
    ,                        !- Rated Heating Capacity at Speed 4 {W}
    ,                        !- Rated COP for Heating at Speed 4 {W/W}
    ,                        !- Normalized Heating Capacity Function of Temperature Curve Name at Speed 4
    ,                        !- Heating Energy Input Ratio Function of Temperature Curve Name at Speed 4
    ,                        !- Heating Energy Input Ratio Function of PLR Curve Name at Speed 4
    ,                        !- Rated Heating Capacity at Speed 5 {W}
    ,                        !- Rated COP for Heating at Speed 5 {W/W}
    ,                        !- Normalized Heating Capacity Function of Temperature Curve Name at Speed 5
    ,                        !- Heating Energy Input Ratio Function of Temperature Curve Name at Speed 5
    ,                        !- Heating Energy Input Ratio Function of PLR Curve Name at Speed 5
    ,                        !- Booster Mode On Heating
    ,                        !- Rated Heating Capacity in Booster Mode {W}
    ,                        !- Rated Heating COP in Booster Mode {W/W}
    ,                        !- Normalized Heating Capacity Function of Temperature Curve Name in Booster Mode
    ,                        !- Heating Energy Input Ratio Function of Temperature Curve Name in Booster Mode
    ,                        !- Heating Energy Input Ratio Function of PLR Curve Name in Booster Mode
    2,                       !- Number of Speeds for Cooling
    200000,                  !- Rated Cooling Capacity at Speed 1 {W}
    3.5,                     !- Rated COP for Cooling at Speed 1 {W/W}
    CapCurveFuncTempCL,      !- Normalized Cooling Capacity Function of Temperature Curve Name at Speed 1
    EIRCurveFuncTempCL,      !- Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 1
    EIRCurveFuncPLR,         !- Cooling Energy Input Ratio Function of PLR Curve Name at Speed 1
    400000,                  !- Rated Cooling Capacity at Speed 2 {W}
    3.5,                     !- Rated COP for Cooling at Speed 2 {W/W}
    CapCurveFuncTempCL2,     !- Normalized Cooling Capacity Function of Temperature Curve Name at Speed 2
    EIRCurveFuncTempCL2,     !- Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 2
    EIRCurveFuncPLR2,        !- Cooling Energy Input Ratio Function of PLR Curve Name at Speed 2
    ,                        !- Rated Cooling Capacity at Speed 3 {W}
    ,                        !- Rated COP for Cooling at Speed 3 {W/W}
    ,                        !- Normalized Cooling Capacity Function of Temperature Curve Name at Speed 3
    ,                        !- Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 3
    ,                        !- Cooling Energy Input Ratio Function of PLR Curve Name at Speed 3
    ,                        !- Rated Cooling Capacity at Speed 4 {W}
    ,                        !- Rated COP for Cooling at Speed 4 {W/W}
    ,                        !- Normalized Cooling Capacity Function of Temperature Curve Name at Speed 4
    ,                        !- Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 4
    ,                        !- Cooling Energy Input Ratio Function of PLR Curve Name at Speed 4
    ,                        !- Rated Cooling Capacity at Speed 5 {W}
    ,                        !- Rated COP for Cooling at Speed 5 {W/W}
    ,                        !- Normalized Cooling Capacity Function of Temperature Curve Name at Speed 5
    ,                        !- Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 5
    ,                        !- Cooling Energy Input Ratio Function of PLR Curve Name at Speed 5
    ,                        !- Booster Mode On Cooling
    ,                        !- Rated Cooling Capacity in Booster Mode {W}
    ,                        !- Rated Cooling COP in Booster Mode {W/W}
    ,                        !- Normalized Cooling Capacity Function of Temperature Curve Name in Booster Mode
    ,                        !- Cooling Energy Input Ratio Function of Temperature Curve Name in Booster Mode
    ;                        !- Cooling Energy Input Ratio Function of PLR Curve Name in Booster Mode
    """

    expected_idf_text = """
  HeatPump:AirToWater,
    test_AWHP,               !- Name
    ,                        !- Availability Schedule Name Heating
    ,                        !- Availability Schedule Name Cooling
    Load,                    !- Operating Mode Control Method
    SingleMode,              !- Operating Mode Control Option for Multiple Unit
    ,                        !- Operating Mode Control Schedule Name
    ,                        !- Minimum Part Load Ratio
    ,                        !- Flow Mode
    20,                      !- Rated Inlet Air Temperature in Heating Mode {C}
    autosize,                !- Rated Air Flow Rate in Heating Mode {m3/s}
    50,                      !- Rated Leaving Water Temperature in Heating Mode {C}
    autosize,                !- Rated Water Flow Rate in Heating Mode {m3/s}
    -20,                     !- Minimum Outdoor Air Temperature in Heating Mode {C}
    25,                      !- Maximum Outdoor Air Temperature in Heating Mode {C}
    MinHeatingSWTvsOAT,      !- Minimum Leaving Water Temperature Curve Name in Heating Mode
    MaxHeatingSWTvsOAT,      !- Maximum Leaving Water Temperature Curve Name in Heating Mode
    1.0,                     !- Sizing Factor for Heating
    25,                      !- Rated Inlet Air Temperature in Cooling Mode {C}
    20,                      !- Rated Air Flow Rate in Cooling Mode {m3/s}
    22,                      !- Rated Leaving Water Temperature in Cooling Mode {C}
    autosize,                !- Rated Water Flow Rate in Cooling Mode {m3/s}
    18,                      !- Minimum Outdoor Air Temperature in Cooling Mode {C}
    40,                      !- Maximum Outdoor Air Temperature in Cooling Mode {C}
    MinCoolingSWTvsOAT,      !- Minimum Leaving Water Temperature Curve Name in Cooling Mode
    MaxCoolingSWTvsOAT,      !- Maximum Leaving Water Temperature Curve Name in Cooling Mode
    0.9,                     !- Sizing Factor for Cooling
    Outdoor Air Inlet Node,  !- Air Inlet Node Name
    Outdoor Air Outlet Node, !- Air Outlet Node Name
    Heating Coil Load Loop Intermediate Node,  !- Hot Water Inlet Node Name
    Heating Coil Load Loop Supply Outlet Node,  !- Hot Water Outlet Node Name
    Cooling Coil Load Loop Intermediate Node,  !- Chilled Water Inlet Node Name
    Cooling Coil Load Loop Supply Outlet Node,  !- Chilled Water Outlet Node Name
    ,                        !- Maximum Outdoor Dry Bulb Temperature For Defrost Operation
    Timed,                   !- Heat Pump Defrost Control
    0.2,                     !- Heat Pump Defrost Time Period Fraction
    150,                     !- Resistive Defrost Heater Capacity {W}
    ,                        !- Defrost Energy Input Ratio Function of Temperature Curve Name
    2,                       !- Heat Pump Multiplier
    FixedSpeed,              !- Control Type
    100,                     !- Crankcase Heater Capacity {W}
    CrankCaseHeaterPowerCurve,  !- Crankcase Heater Capacity Function of Temperature Curve Name
    5,                       !- Maximum Ambient Temperature for Crankcase Heater Operation {C}
    2,                       !- Number of Speeds for Heating
    20000,                   !- Rated Heating Capacity at Speed 1 {W}
    3.49,                    !- Rated COP for Heating at Speed 1 {W/W}
    CapCurveFuncTempHT,      !- Normalized Heating Capacity Function of Temperature Curve Name at Speed 1
    EIRCurveFuncTempHT,      !- Heating Energy Input Ratio Function of Temperature Curve Name at Speed 1
    EIRCurveFuncPLR,         !- Heating Energy Input Ratio Function of PLR Curve Name at Speed 1
    40000,                   !- Rated Heating Capacity at Speed 2 {W}
    3.51,                    !- Rated COP for Heating at Speed 2 {W/W}
    CapCurveFuncTempHT2,     !- Normalized Heating Capacity Function of Temperature Curve Name at Speed 2
    EIRCurveFuncTempHT2,     !- Heating Energy Input Ratio Function of Temperature Curve Name at Speed 2
    EIRCurveFuncPLR2,        !- Heating Energy Input Ratio Function of PLR Curve Name at Speed 2
    ,                        !- Rated Heating Capacity at Speed 3 {W}
    ,                        !- Rated COP for Heating at Speed 3 {W/W}
    ,                        !- Normalized Heating Capacity Function of Temperature Curve Name at Speed 3
    ,                        !- Heating Energy Input Ratio Function of Temperature Curve Name at Speed 3
    ,                        !- Heating Energy Input Ratio Function of PLR Curve Name at Speed 3
    ,                        !- Rated Heating Capacity at Speed 4 {W}
    ,                        !- Rated COP for Heating at Speed 4 {W/W}
    ,                        !- Normalized Heating Capacity Function of Temperature Curve Name at Speed 4
    ,                        !- Heating Energy Input Ratio Function of Temperature Curve Name at Speed 4
    ,                        !- Heating Energy Input Ratio Function of PLR Curve Name at Speed 4
    ,                        !- Rated Heating Capacity at Speed 5 {W}
    ,                        !- Rated COP for Heating at Speed 5 {W/W}
    ,                        !- Normalized Heating Capacity Function of Temperature Curve Name at Speed 5
    ,                        !- Heating Energy Input Ratio Function of Temperature Curve Name at Speed 5
    ,                        !- Heating Energy Input Ratio Function of PLR Curve Name at Speed 5
    ,                        !- Booster Mode On Heating
    ,                        !- Rated Heating Capacity in Booster Mode {W}
    ,                        !- Rated Heating COP in Booster Mode {W/W}
    ,                        !- Normalized Heating Capacity Function of Temperature Curve Name in Booster Mode
    ,                        !- Heating Energy Input Ratio Function of Temperature Curve Name in Booster Mode
    ,                        !- Heating Energy Input Ratio Function of PLR Curve Name in Booster Mode
    2,                       !- Number of Speeds for Cooling
    200000,                  !- Rated Cooling Capacity at Speed 1 {W}
    3.5,                     !- Rated COP for Cooling at Speed 1 {W/W}
    CapCurveFuncTempCL,      !- Normalized Cooling Capacity Function of Temperature Curve Name at Speed 1
    EIRCurveFuncTempCL,      !- Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 1
    EIRCurveFuncPLR,         !- Cooling Energy Input Ratio Function of PLR Curve Name at Speed 1
    400000,                  !- Rated Cooling Capacity at Speed 2 {W}
    3.5,                     !- Rated COP for Cooling at Speed 2 {W/W}
    CapCurveFuncTempCL2,     !- Normalized Cooling Capacity Function of Temperature Curve Name at Speed 2
    EIRCurveFuncTempCL2,     !- Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 2
    EIRCurveFuncPLR2,        !- Cooling Energy Input Ratio Function of PLR Curve Name at Speed 2
    ,                        !- Rated Cooling Capacity at Speed 3 {W}
    ,                        !- Rated COP for Cooling at Speed 3 {W/W}
    ,                        !- Normalized Cooling Capacity Function of Temperature Curve Name at Speed 3
    ,                        !- Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 3
    ,                        !- Cooling Energy Input Ratio Function of PLR Curve Name at Speed 3
    ,                        !- Rated Cooling Capacity at Speed 4 {W}
    ,                        !- Rated COP for Cooling at Speed 4 {W/W}
    ,                        !- Normalized Cooling Capacity Function of Temperature Curve Name at Speed 4
    ,                        !- Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 4
    ,                        !- Cooling Energy Input Ratio Function of PLR Curve Name at Speed 4
    ,                        !- Rated Cooling Capacity at Speed 5 {W}
    ,                        !- Rated COP for Cooling at Speed 5 {W/W}
    ,                        !- Normalized Cooling Capacity Function of Temperature Curve Name at Speed 5
    ,                        !- Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 5
    ,                        !- Cooling Energy Input Ratio Function of PLR Curve Name at Speed 5
    ,                        !- Booster Mode On Cooling
    ,                        !- Rated Cooling Capacity in Booster Mode {W}
    ,                        !- Rated Cooling COP in Booster Mode {W/W}
    ,                        !- Normalized Cooling Capacity Function of Temperature Curve Name in Booster Mode
    ,                        !- Cooling Energy Input Ratio Function of Temperature Curve Name in Booster Mode
    ;                        !- Cooling Energy Input Ratio Function of PLR Curve Name in Booster Mode
"""

    run_transition(prepare_and_run_transition, idf_text, expected_idf_text)
